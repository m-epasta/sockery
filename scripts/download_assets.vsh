#!/usr/bin/env -S v

// TODO: Finish this file
import net.http

const assets_count = 1

fn check_assets(assets_dir string, assets_count int, filepath string) int {
	curr_user_assets_list := ls(assets_dir) or { return 1 }
	if curr_user_assets_list.len != assets_count {
		for i := 0; i <= assets_count; i++ {
			go download_assets(i, assets_dir)
		}
	}

	return 0
}

fn csv_lookup(which int) !string {
	return ''
}

fn download_assets(which int, assets_dir string) {
	// url from file is trusted because a workflow is runned to verify each url
	url := csv_lookup(which) or {
		// NOTE: Should not be an error I think
		exit(0)
	}

	file := http.get(url) or {
		eprintln('Failed to fetch asset ${which}')
		exit(1)
	}

	cp(file.body, join_path(assets_dir, csv_lookup(which) or {
		eprintln('Could not lookup on the csv file for asset ${which}')
		exit(1)
	})) or {
		eprintln('Failed to copy asset ${which} to assets dir')
		exit(1)
	}
}

fn check_assets_file_completness(path string) {}

println('Checking assets completness...')

assets_filename := 'assets.csv'
script_dir := executable()
project_root := join_path(script_dir, '..')
assets_dir := join_path(project_root, 'assets/')
assets_file := join_path(assets_dir, assets_filename)

if !exists(assets_dir) || exists(assets_file) {
	eprintln('assets dir or ${assets_filename} does not exit. You can copy the ${assets_filename} file from the remote repository, create at root assets/ and paste the ${assets_filename} file into assets/${assets_filename}')
	exit(1)
}

check_assets(assets_dir, assets_count, assets_file)
