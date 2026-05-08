#include <dirent.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

FILE* tty;

#define MAX_PATH 4096
#define MAX_LINE 8192

typedef struct {
    char* path;
    char* name;
    int score;
} Match;

Match* matches = NULL;
int match_count = 0;
int match_capacity = 0;
pthread_mutex_t matches_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int search_complete = 0;

// Queue for BFS
typedef struct {
    char** items;
    int head, tail, capacity;
} StrQueue;

void init_q(StrQueue* q) {
    q->capacity = 1024;
    q->items = malloc(q->capacity * sizeof(char*));
    q->head = q->tail = 0;
}

void push_q(StrQueue* q, const char* str) {
    if (q->tail >= q->capacity) {
        q->capacity *= 2;
        q->items = realloc(q->items, q->capacity * sizeof(char*));
    }
    q->items[q->tail++] = strdup(str);
}

char* pop_q(StrQueue* q) {
    if (q->head < q->tail) {
        return q->items[q->head++];
    }
    return NULL;
}

int empty_q(StrQueue* q) { return q->head >= q->tail; }

void free_q(StrQueue* q) {
    for (int i = q->head; i < q->tail; i++)
        free(q->items[i]);
    free(q->items);
}

int direxists(const char* dir) {
    struct stat st;
    if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1;
    }
    return 0;
}

int fuzzy_score(const char* str, const char* pattern) {
    if (strcmp(str, pattern) == 0)
        return 1000;
    return 0;
}

void ensure_gcd_dir() {
    char gcd_dir[MAX_PATH];
    snprintf(gcd_dir, sizeof(gcd_dir), "%s/.gcd", getenv("HOME"));
    struct stat st;
    if (stat(gcd_dir, &st) != 0) {
        mkdir(gcd_dir, 0755);
    }
}

void save_mapping(const char* shortcut, const char* full_path) {
    ensure_gcd_dir();
    char csv_path[MAX_PATH];
    char tmp_path[MAX_PATH];
    const char* home = getenv("HOME");
    snprintf(csv_path, sizeof(csv_path), "%s/.gcd/mappings.csv", home);
    snprintf(tmp_path, sizeof(tmp_path), "%s/.gcd/mappings.csv.tmp", home);

    FILE* file = fopen(csv_path, "r");
    FILE* tmp = fopen(tmp_path, "w");
    if (!tmp) {
        if (file)
            fclose(file);
        return;
    }

    int found = 0;
    if (file) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            if (strlen(line) == 0)
                continue;

            char existing_shortcut[MAX_PATH] = {0};
            sscanf(line, "%[^,]", existing_shortcut);

            if (strcmp(existing_shortcut, shortcut) == 0) {
                fprintf(tmp, "%s,%s\n", shortcut, full_path);
                found = 1;
            } else {
                fprintf(tmp, "%s\n", line);
            }
        }
        fclose(file);
    }

    if (!found) {
        fprintf(tmp, "%s,%s\n", shortcut, full_path);
    }

    fclose(tmp);
    rename(tmp_path, csv_path);
}

int dirmapped(const char* dir, char* mapped_path) {
    char csv_path[MAX_PATH];
    snprintf(csv_path, sizeof(csv_path), "%s/.gcd/mappings.csv",
             getenv("HOME"));
    FILE* file = fopen(csv_path, "r");
    if (!file)
        return 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char shortcut[MAX_PATH], path[MAX_PATH];
        if (sscanf(line, "%[^,],%[^\n]", shortcut, path) == 2) {
            if (strcmp(shortcut, dir) == 0) {
                strncpy(mapped_path, path, MAX_PATH);
                fclose(file);
                return 1;
            }
        }
    }
    fclose(file);
    return 0;
}

void* search_thread(void* arg) {
    char* target = (char*)arg;
    char home_path[MAX_PATH];
    snprintf(home_path, sizeof(home_path), "%s", getenv("HOME"));
    char desktop_path[MAX_PATH + 256];
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", home_path);

    StrQueue q_desktop, q_normal, q_dot;
    init_q(&q_desktop);
    init_q(&q_normal);
    init_q(&q_dot);

    if (direxists(desktop_path)) {
        push_q(&q_desktop, desktop_path);
    }

    DIR* dir = opendir(home_path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            char full_path[MAX_PATH + 256];
            snprintf(full_path, sizeof(full_path), "%s/%s", home_path,
                     entry->d_name);

            if (strcmp(full_path, desktop_path) == 0)
                continue;

            int is_dir = 0;
            if (entry->d_type == DT_DIR) {
                is_dir = 1;
            } else if (entry->d_type == DT_UNKNOWN) {
                struct stat st;
                if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                    is_dir = 1;
                }
            }

            if (is_dir) {
                if (entry->d_name[0] == '.')
                    push_q(&q_dot, full_path);
                else
                    push_q(&q_normal, full_path);
            }
        }
        closedir(dir);
    }

    StrQueue* queues[] = {&q_desktop, &q_normal, &q_dot};
    for (int i = 0; i < 3; i++) {
        StrQueue* q = queues[i];
        while (!empty_q(q)) {
            char* curr = pop_q(q);
            char* base = strrchr(curr, '/');
            base = base ? base + 1 : curr;

            int score = fuzzy_score(base, target);
            if (score > 0) {
                pthread_mutex_lock(&matches_mutex);
                if (match_count >= match_capacity) {
                    match_capacity =
                        match_capacity == 0 ? 64 : match_capacity * 2;
                    matches = realloc(matches, match_capacity * sizeof(Match));
                }
                matches[match_count].path = strdup(curr);
                matches[match_count].name = strdup(base);
                matches[match_count].score = score;
                match_count++;

                for (int j = match_count - 1; j > 0; j--) {
                    if (matches[j].score > matches[j - 1].score) {
                        Match temp = matches[j];
                        matches[j] = matches[j - 1];
                        matches[j - 1] = temp;
                    } else {
                        break;
                    }
                }
                pthread_mutex_unlock(&matches_mutex);
            }

            DIR* d = opendir(curr);
            if (d) {
                struct dirent* ent;
                while ((ent = readdir(d)) != NULL) {
                    if (strcmp(ent->d_name, ".") == 0 ||
                        strcmp(ent->d_name, "..") == 0)
                        continue;
                    char next_path[MAX_PATH + 256];
                    snprintf(next_path, sizeof(next_path), "%s/%s", curr,
                             ent->d_name);

                    int is_dir = 0;
                    if (ent->d_type == DT_DIR) {
                        is_dir = 1;
                    } else if (ent->d_type == DT_UNKNOWN) {
                        struct stat st;
                        if (stat(next_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                            is_dir = 1;
                        }
                    }

                    if (is_dir) {
                        if (i == 2) {
                            push_q(q, next_path);
                        } else {
                            if (ent->d_name[0] == '.')
                                push_q(&q_dot, next_path);
                            else
                                push_q(q, next_path);
                        }
                    }
                }
                closedir(d);
            }
            free(curr);
        }
    }

    free_q(&q_desktop);
    free_q(&q_normal);
    free_q(&q_dot);
    search_complete = 1;
    return NULL;
}

struct termios orig_termios;
void disable_raw_mode() {
    fprintf(tty, "\033[?25h\033[?1049l\033[?7h");
    fflush(tty);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fprintf(tty, "\033[?1049h\033[?25l\033[?7l");
    fflush(tty);
}

int get_term_height() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_row == 0) {
        return 24;
    }
    return w.ws_row;
}

int main(int argc, char* argv[]) {
    tty = fopen("/dev/tty", "w+");
    if (!tty)
        tty = stderr;

    if (argc != 2) {
        fprintf(tty, "\033[33mUsage: gcd FOLDER\033[0m\n");
        return 1;
    }

    char* dir = argv[1];
    char mapped_path[MAX_PATH];

    if (direxists(dir) == 1) {
        char absolute_path[MAX_PATH];
        if (realpath(dir, absolute_path)) {
            printf("cd \"%s\"\n", absolute_path);
            save_mapping(dir, absolute_path);
        } else {
            printf("cd \"%s\"\n", dir);
        }
        return 0;
    }

    if (dirmapped(dir, mapped_path) == 1 && direxists(mapped_path) == 1) {
        printf("cd \"%s\"\n", mapped_path);
        return 0;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, search_thread, dir);

    enable_raw_mode();

    int selected_idx = 0;
    int scroll_offset = 0;
    char* chosen_path = NULL;

    while (1) {
        pthread_mutex_lock(&matches_mutex);
        int count = match_count;
        int is_done = search_complete;

        int term_h = get_term_height();
        int list_h = term_h - 6;
        if (list_h < 5)
            list_h = 5;

        if (count > 0 && selected_idx >= count)
            selected_idx = count - 1;
        if (selected_idx < scroll_offset)
            scroll_offset = selected_idx;
        if (selected_idx >= scroll_offset + list_h)
            scroll_offset = selected_idx - list_h + 1;

        fprintf(tty, "\033[H");
        fprintf(tty,
                "\033[K\033[1;33mSearching for '%s' under ~/...\033[0m %s\n",
                dir, is_done ? "(Done)" : "(Searching...)");
        fprintf(tty, "\033[K\033[36m%-4s %-30s %s\033[0m\n", "#",
                "Directory Name", "Path");
        fprintf(tty, "\033[K---------------------------------------------------"
                     "-------------\n");

        int lines_rendered = 0;
        for (int i = scroll_offset; i < scroll_offset + list_h && i < count;
             i++) {
            char display_path[MAX_PATH];
            char* m_path = matches[i].path;
            if (strlen(m_path) > 60) {
                snprintf(display_path, sizeof(display_path), "...%s",
                         m_path + strlen(m_path) - 57);
            } else {
                strncpy(display_path, m_path, MAX_PATH);
            }

            if (i == selected_idx) {
                fprintf(tty,
                        "\033[K\033[7m\033[32m%-4d\033[39m "
                        "\033[37m%-30.30s\033[39m %s\033[0m\n",
                        i + 1, matches[i].name, display_path);
            } else {
                fprintf(tty,
                        "\033[K\033[32m%-4d\033[39m \033[37m%-30.30s\033[39m "
                        "%s\033[0m\n",
                        i + 1, matches[i].name, display_path);
            }
            lines_rendered++;
        }

        for (int i = lines_rendered; i < list_h; i++) {
            fprintf(tty, "\033[K\n");
        }

        fprintf(tty, "\033[K---------------------------------------------------"
                     "-------------\n");
        fprintf(
            tty,
            "\033[KUse UP/DOWN, j/k: Move | ENTER: Select | ESC or q: Cancel");
        fprintf(tty, "\033[J");
        fflush(tty);

        if (is_done && count == 0) {
            pthread_mutex_unlock(&matches_mutex);
            fprintf(tty,
                    "\n\033[31m❌ No directories found matching '%s'\033[0m\n",
                    dir);
            return 1;
        }

        pthread_mutex_unlock(&matches_mutex);

        struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
        int ret = poll(&pfd, 1, 100);

        if (ret > 0) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                if (c == 27) {
                    char seq[3];
                    struct pollfd esc_pfd = {STDIN_FILENO, POLLIN, 0};
                    if (poll(&esc_pfd, 1, 50) > 0) {
                        if (read(STDIN_FILENO, &seq[0], 1) == 1 &&
                            (seq[0] == '[' || seq[0] == 'O')) {
                            if (poll(&esc_pfd, 1, 50) > 0 &&
                                read(STDIN_FILENO, &seq[1], 1) == 1) {
                                pthread_mutex_lock(&matches_mutex);
                                if (seq[1] == 'A' && selected_idx > 0)
                                    selected_idx--;
                                if (seq[1] == 'B' &&
                                    selected_idx < match_count - 1)
                                    selected_idx++;
                                pthread_mutex_unlock(&matches_mutex);
                            }
                        }
                    } else {
                        break;
                    }
                } else if (c == '\n' || c == '\r') {
                    pthread_mutex_lock(&matches_mutex);
                    if (match_count > 0 && selected_idx < match_count) {
                        chosen_path = strdup(matches[selected_idx].path);
                    }
                    pthread_mutex_unlock(&matches_mutex);
                    break;
                } else if (c == 'q' || c == 'Q') {
                    break;
                } else if (c == 'j') {
                    pthread_mutex_lock(&matches_mutex);
                    if (selected_idx < match_count - 1)
                        selected_idx++;
                    pthread_mutex_unlock(&matches_mutex);
                } else if (c == 'k') {
                    pthread_mutex_lock(&matches_mutex);
                    if (selected_idx > 0)
                        selected_idx--;
                    pthread_mutex_unlock(&matches_mutex);
                }
            }
        }
    }

    disable_raw_mode();

    if (chosen_path) {
        printf("cd \"%s\"\n", chosen_path);
        save_mapping(dir, chosen_path);
        free(chosen_path);
    } else {
        fprintf(tty, "\n\033[31mCancelled.\033[0m\n");
        return 1;
    }

    return 0;
}
