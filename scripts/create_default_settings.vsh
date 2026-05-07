#! /usr/bin/env -S v

home := getenv('HOME')
chdir(home)!
mkdir_all('.config/sockery')!
chdir('${home}/.config/sockery')!
create('settings.conf')!
write_file('settings.conf', 'w_width = 800\nw_height = 600\n')!
