client_s4_sources = run_command(
	'py',
	meson.project_source_root() / 'tools' / 'meson_glob.py',
	meson.current_source_dir(), '.',
).stdout().strip().split('\n')

client_s4_rc = import('windows').compile_resources(
	'resource.rc',
	depends: [crash_handler],
	depend_files: files(
		'resource.h',
		'resources/dvar_list.json',
		'resources/steam/steam_api64_GSE.dll',
		'resources/steam/steam_modules.zip',
		'resources/steam/steam_settings.zip',
		'resources/lua/ui/frontEnd/mp/QuickPlayButton.lua',
		'resources/gsc/give_weapon.gsc',
		'resources/dw/playlists/playlists-readable/unified_playlist.aggr29.txt',
		'resources/dw/playlists/playlists-readable/unified_playlist.aggr47.txt',
		'resources/dw/playlists/playlists-readable/unified_playlist.aggr70.txt',
		'resources/dw/playlists/playlists-readable/unified_playlist.aggr86.txt',
		'resources/dw/playlists/playlists-readable/unified_playlist.aggr182.txt'
	),
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated'),
		include_directories('../crash_handler')
	]
)

client_s4 = shared_library('version',
	sources: [client_s4_sources, client_s4_rc],
	include_directories: [
		include_directories('.'),
		include_directories('../common'),
		include_directories('../generated')
	],
	dependencies: [
		common_dep,
		asmjit_dep,
		magic_enum_dep,
		xsk_gsc_utils_dep,
		xsk_gsc_s4_dep
	],
	override_options: ['cpp_std=c++latest'],
	name_prefix: '',
)
