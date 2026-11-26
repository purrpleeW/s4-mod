piped_console_sources = run_command(
	'py',
	meson.project_source_root() / 'tools' / 'meson_glob.py',
	meson.current_source_dir(), '.',
).stdout().strip().split('\n')

piped_console_rc = import('windows').compile_resources(
	'resource.rc',
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated')
	]
)

piped_console = executable('zpPipedConsole',
	sources: [piped_console_sources, piped_console_rc],
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated'),
	],
	dependencies: [
		common_dep,
		nlohmann_json_dep,
	],
	override_options: ['cpp_std=c++latest'],
	name_prefix: '',
)