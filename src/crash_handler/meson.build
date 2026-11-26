crash_handler_sources = run_command(
	'py',
	meson.project_source_root() / 'tools' / 'meson_glob.py',
	meson.current_source_dir(), '.',
).stdout().strip().split('\n')

crash_handler_rc = import('windows').compile_resources(
	'resource.rc',
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated')
	]
)

crash_handler = executable('zpCrashHandler',
	sources: [crash_handler_sources, crash_handler_rc],
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated'),
	],
	dependencies: [
		common_dep,
		nlohmann_json_dep,
	],
	link_args: ['/SUBSYSTEM:WINDOWS', '/MANIFEST:EMBED'],
	override_options: ['cpp_std=c++latest'],
	win_subsystem: 'windows',
)