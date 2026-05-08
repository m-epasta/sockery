#!/usr/bin/env -S v

assets_filename := 'assets_url.csv'
scripts_dir := executable()
project_root := join_path(scripts_dir, '..')
assets_dir := join_path(project_root, 'assets')
assets_filepath := join_path(assets_dir, assets_filename)

text := 'loop,https://staging.svgrepo.com/svg/393132/loop'

write_file(assets_filepath, text) or {
	eprintln('failed to write ${assets_filepath}: ${err}')
	exit(1)
}
exit(0)
