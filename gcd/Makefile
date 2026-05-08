# Makefile for gcd
# Usage: make install  - installs gcd and sets up bash integration
#        make uninstall - removes gcd and cleans up .bashrc
#        make clean     - removes compiled binary

CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = gcd
SRC = gcd.c
PREFIX = $(HOME)/.local
BINDIR = $(PREFIX)/bin
GCD_DIR = $(HOME)/.gcd
BASHRC = $(HOME)/.bashrc
ZSHRC = $(HOME)/.zshrc

define GCD_SCRIPT

export PATH=$$PATH:$(BINDIR)

handle_gcd() {
    local result=$$($(BINDIR)/gcd "$$1" 2>&1)
    local cd_cmd=$$(echo "$$result" | grep "^cd ")

    if [ -n "$$cd_cmd" ]; then
        eval "$$cd_cmd"
        return 0
    fi
    echo -e "$$result"
    return 1
}

gcd() {
    if [ -z "$$1" ]; then
        cd ~
        return
    fi

    local old_dir=$$(pwd)
    if handle_gcd "$$1"; then
        if [ "$$old_dir" != "$$(pwd)" ]; then
            echo -e "\033[32m➜ Changed to: $$(pwd)\033[0m"
        fi
    fi
}

gcd_list() {
    local csv_file="$$HOME/.gcd/mappings.csv"
    if [ -f "$$csv_file" ]; then
        echo -e "\033[1;33mSaved GCD Mappings:\033[0m"
        echo -e "\033[36mShortcut → Path\033[0m"
        echo -e "-----------------------------"
        cat "$$csv_file" | while IFS=',' read -r shortcut path; do
            printf "\033[32m%-20s\033[0m → %s\n" "$$shortcut" "$$path"
        done
    else
        echo "No mappings saved yet."
    fi
}

clean_gcd() {
    local csv_file="$$HOME/.gcd/mappings.csv"
    if [ -f "$$csv_file" ]; then
        rm "$$csv_file"
        echo "All mappings removed."
    else
        echo "No mappings file found."
    fi
}

gcd_rm() {
    if [ -z "$$1" ]; then
        echo "Usage: gcd_rm SHORTCUT"
        return 1
    fi

    local csv_file="$$HOME/.gcd/mappings.csv"
    if [ -f "$$csv_file" ]; then
        grep -v "^$$1," "$$csv_file" > "$$csv_file.tmp"
        mv "$$csv_file.tmp" "$$csv_file"
        echo "Removed mapping for '$$1'"
    else
        echo "No mappings file found."
    fi
}
endef
export GCD_SCRIPT

RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

.PHONY: all install uninstall clean help check_shell backup_bashrc add-mapping

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "$(BLUE) Compiling $(TARGET)...$(NC)"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
	@echo "$(GREEN)✓ Compilation successful$(NC)"

install: $(TARGET) check_shell
	@echo "$(BLUE) Installing gcd...$(NC)"
	
	@mkdir -p $(BINDIR)
	@mkdir -p $(GCD_DIR)
	
	@cp $(TARGET) $(BINDIR)/$(TARGET)
	@chmod 755 $(BINDIR)/$(TARGET)
	@echo "$(GREEN)✓ Binary installed to $(BINDIR)/$(TARGET)$(NC)"
	
	@$(MAKE) backup_bashrc
	
	@if [ -f $(BASHRC) ]; then \
		echo "$(BLUE) Detected bash, checking $(BASHRC)...$(NC)"; \
		if ! grep -q "# gcd function" $(BASHRC) 2>/dev/null; then \
			echo "$$GCD_SCRIPT" >> $(BASHRC); \
			echo "$(GREEN)✓ Added gcd function to $(BASHRC)$(NC)"; \
		else \
			echo "$(YELLOW)⚠ gcd function already exists in $(BASHRC), skipping...$(NC)"; \
		fi \
	fi
	
	@if [ -f $(ZSHRC) ]; then \
		echo "$(BLUE) Detected zsh, checking $(ZSHRC)...$(NC)"; \
		if ! grep -q "# gcd function" $(ZSHRC) 2>/dev/null; then \
			echo "$$GCD_SCRIPT" >> $(ZSHRC); \
			echo "$(GREEN)✓ Added gcd function to $(ZSHRC)$(NC)"; \
		else \
			echo "$(YELLOW)⚠ gcd function already exists in $(ZSHRC), skipping...$(NC)"; \
		fi \
	fi
	
	@echo "$(GREEN) Installation complete!$(NC)"
	@echo "$(YELLOW)Please run: source $(BASHRC) (or restart your terminal)$(NC)"
	@echo "$(BLUE)Usage examples:$(NC)"
	@echo "  gcd Documents    # Search and cd to Documents"
	@echo "  gcd_list         # List all saved mappings"
	@echo "  gcd_rm Documents # Remove mapping for Documents"

uninstall:
	@echo "$(RED)  Uninstalling gcd...$(NC)"
	
	@rm -f $(BINDIR)/$(TARGET)
	@echo "$(GREEN)✓ Removed binary$(NC)"
	
	@rm -rf $(GCD_DIR)
	@echo "$(GREEN)✓ Removed $(GCD_DIR)$(NC)"
	
	@if [ -f $(BASHRC) ]; then \
		cp $(BASHRC) $(BASHRC).backup; \
		sed -i.bak '/# --- GCD INSTALLER START ---/,/# --- GCD INSTALLER END ---/d' $(BASHRC); \
		sed -i.bak '/^$$/N;/^\n$$/D' $(BASHRC); \
		echo "$(GREEN)✓ Removed gcd entries from $(BASHRC)$(NC)"; \
	fi
	
	@if [ -f $(ZSHRC) ]; then \
		cp $(ZSHRC) $(ZSHRC).backup; \
		sed -i.bak '/# --- GCD INSTALLER START ---/,/# --- GCD INSTALLER END ---/d' $(ZSHRC); \
		sed -i.bak '/^$$/N;/^\n$$/D' $(ZSHRC); \
		echo "$(GREEN)✓ Removed gcd entries from $(ZSHRC)$(NC)"; \
	fi
	
	@echo "$(GREEN) Uninstall complete!$(NC)"
	@echo "$(YELLOW)Backup files saved as $(BASHRC).backup and $(ZSHRC).backup$(NC)"

clean:
	@echo "$(BLUE) Cleaning up...$(NC)"
	@rm -f $(TARGET)
	@echo "$(GREEN)✓ Cleaned$(NC)"

backup_bashrc:
	@if [ -f $(BASHRC) ]; then \
		if [ ! -f $(BASHRC).gcd.backup ]; then \
			cp $(BASHRC) $(BASHRC).gcd.backup; \
			echo "$(GREEN)✓ Backed up $(BASHRC) to $(BASHRC).gcd.backup$(NC)"; \
		fi \
	fi
	@if [ -f $(ZSHRC) ]; then \
		if [ ! -f $(ZSHRC).gcd.backup ]; then \
			cp $(ZSHRC) $(ZSHRC).gcd.backup; \
			echo "$(GREEN)✓ Backed up $(ZSHRC) to $(ZSHRC).gcd.backup$(NC)"; \
		fi \
	fi

check_shell:
	@echo "$(BLUE) Checking shell environment...$(NC)"
	@if [ -z "$$HOME" ]; then \
		echo "$(RED)❌ HOME environment variable not set!$(NC)"; \
		exit 1; \
	fi
	@echo "$(GREEN)✓ HOME=$$HOME$(NC)"
	@if [ ! -f $(BASHRC) ] && [ ! -f $(ZSHRC) ]; then \
		echo "$(YELLOW)⚠ No .bashrc or .zshrc found. Creating $(BASHRC)$(NC)"; \
		touch $(BASHRC); \
	fi

help:
	@echo "$(BLUE)GCD Makefile Commands:$(NC)"
	@echo ""
	@echo "  $(GREEN)make install$(NC)          - Compile and install gcd with bash/zsh integration"
	@echo "  $(GREEN)make uninstall$(NC)        - Remove gcd and clean up configuration files"
	@echo "  $(GREEN)make clean$(NC)            - Remove compiled binary"
	@echo "  $(GREEN)make add-mapping$(NC)      - Add a directory mapping non‑interactively"
	@echo "                                       Usage: make add-mapping SHORTCUT=NAME DIR=/path"
	@echo "  $(GREEN)make help$(NC)             - Show this help message"
	@echo ""
	@echo "$(BLUE)Example for CI / GitHub Actions:$(NC)"
	@echo "  make install"
	@echo "  make add-mapping SHORTCUT=build DIR=$(pwd)/build_output"
	@echo "  gcd build   # will now cd into ./build_output"

add-mapping:
	@if [ -z "$(SHORTCUT)" ] || [ -z "$(DIR)" ]; then \
		echo "$(RED)Error: SHORTCUT and DIR are required.$(NC)"; \
		echo "Usage: make add-mapping SHORTCUT=name DIR=/absolute/path"; \
		exit 1; \
	fi
	@mkdir -p $(GCD_DIR)
	@if [ -f $(GCD_DIR)/mappings.csv ]; then \
		grep -v "^$(SHORTCUT)," $(GCD_DIR)/mappings.csv > $(GCD_DIR)/mappings.csv.tmp || true; \
		mv $(GCD_DIR)/mappings.csv.tmp $(GCD_DIR)/mappings.csv; \
	fi
	@echo "$(SHORTCUT),$(DIR)" >> $(GCD_DIR)/mappings.csv
	@echo "$(GREEN)✓ Mapping added: $(SHORTCUT) → $(DIR)$(NC)"

