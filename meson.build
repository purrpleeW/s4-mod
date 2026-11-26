project('s4-mod', 'cpp', 'c',
	meson_version: '>=1.1.0',
	default_options: [
		'cpp_std=c++latest',
		'c_std=c17',
		'buildtype=release',
		'vsenv=true'
	]
)

run_command('py', '-3', meson.project_source_root() / 'tools/update_git.py', check: true)

add_project_arguments(
	'/wd4273',
	'/wd4459',
	'/wd4100',
	'/wd4389',
	'/wd5054',
	'/wd4996',
	'/wd4018',
	'/wd4456',
	'/wd4458',
	'/wd4208',
	'/wd4172',
	'/wd4267',
	'/wd4312',
	'/wd4244',
	'/wd4189',
	'/wd4245',
	language: ['cpp', 'c']
)

add_project_arguments(
	'/clang:-masm=intel',
	'/clang:-Wno-pragma-system-header-outside-header',
	'/clang:-fno-delayed-template-parsing',
	'/clang:-Wno-microsoft-include',
	'/clang:-Wno-microsoft-cast',
	'/clang:-Wno-microsoft-string-literal-from-predefined',
	'/clang:-Wno-c++17-extensions',
	'/clang:-Wno-invalid-token-paste',
	'/clang:-Wno-inconsistent-dllimport',
	'/clang:-Wno-c99-designator',
	'/clang:-Wno-unused-function',
	'/clang:-Wno-unused-private-field',
	'/clang:-Wno-missing-braces',
	'/clang:-mavx2',
	language: ['cpp', 'c']
)

add_project_arguments(
	'/permissive',
	'/UUNICODE',
	'/U_UNICODE',
	'/D_MBCS',
	'/D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR',
	language: ['cpp', 'c']
)

if get_option('buildtype') == 'release'
	add_project_arguments('-DNDEBUG', language: ['cpp', 'c'])
	add_project_link_arguments('/IGNORE:4702', '/LTCG', language: ['cpp', 'c'])
else
	add_project_arguments('-DDEBUG', '-D_DEBUG', language: ['cpp', 'c'])
endif

if get_option('dev')
	add_project_arguments('-DCLIENT_PRERELEASE=1', language: ['cpp', 'c'])
endif

if get_option('protect')
	add_project_arguments('-DCLIENT_PROTECTED=1', language: ['cpp', 'c'])
endif

if get_option('f2p')
	add_project_arguments('-DCLIENT_F2P=1', language: ['cpp', 'c'])
endif

if get_option('offline')
	add_project_arguments('-DCLIENT_OFFLINE=1', language: ['cpp', 'c'])
endif

add_project_link_arguments('/INCREMENTAL:NO', '/SAFESEH:NO', language: ['cpp', 'c'])

if host_machine.system() != 'windows' or host_machine.cpu_family() != 'x86_64'
	error('s4-mod requires a Windows x64 toolchain.')
endif

subdir('vendor')
subdir('src')
