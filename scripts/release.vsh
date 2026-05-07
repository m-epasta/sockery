#!/usr/bin/env -S v

project := 'sockery'
output_name := 'release'

script_dir := dir(executable())
project_root := join_path(script_dir, '..')
build_dir := join_path(project_root, 'build')
artifact_dir := join_path(project_root, 'artifacts')

println('Building optimized release binary...')

if exists(build_dir) {
	rmdir_all(build_dir)!
}

mkdir_all(build_dir)!
chdir(build_dir)!

cflags := '-O3 -march=native -flto -ffunction-sections -fdata-sections -DNDEBUG'
ldflags := '-s -flto -Wl,--gc-sections'
configure := 'cmake ${project_root} -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="${cflags}" -DCMAKE_EXE_LINKER_FLAGS_RELEASE="${ldflags}"'

println('Configuring...')
mut result := execute(configure)
if result.exit_code != 0 {
	eprintln('CMake configure failed')
	eprintln(result.output)
	exit(1)
}

println('Building...')
result = execute('cmake --build . --config Release')
if result.exit_code != 0 {
	eprintln('Build failed')
	eprintln(result.output)
	exit(1)
}

println('Copying to artifacts...')
mkdir_all(artifact_dir)!
mv(join_path(build_dir, project), join_path(artifact_dir, output_name))!

println('Stripping...')
result = execute('strip ${join_path(artifact_dir, output_name)}')
if result.exit_code != 0 {
	eprintln('Strip failed')
	exit(1)
}

println('Release build complete: ${join_path(artifact_dir, output_name)}')
