param(
	[ValidateSet('release', 'debug')]
	[string] $Configuration = 'release',
	[string] $BuildDirectory,
	[ValidateRange(1, 256)]
	[int] $Jobs = 4,
	[switch] $SetupOnly
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

foreach ($command in @('git', 'py', 'clang-cl', 'llvm-rc')) {
	if (-not (Get-Command $command -ErrorAction SilentlyContinue)) {
		throw "Missing $command. Install Git, Python 3.10+, LLVM 22.1+, and Visual Studio 2022 C++ build tools with the Windows SDK and ATL."
	}
}

& py -3 -c 'import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)'
if ($LASTEXITCODE -ne 0) {
	throw 'Python 3.10 or newer is required.'
}

& py -3 -c "import importlib.util, sys; sys.exit(0 if all(importlib.util.find_spec(name) for name in ('mesonbuild', 'ninja')) else 1)"
if ($LASTEXITCODE -ne 0) {
	& py -3 -m pip install -r (Join-Path $PSScriptRoot 'requirements.txt')
	if ($LASTEXITCODE -ne 0) {
		throw 'Installing Meson and Ninja failed.'
	}
}

$pythonScripts = & py -3 -c "import sysconfig; print(sysconfig.get_path('scripts')); print(sysconfig.get_path('scripts', scheme='nt_user'))"
if ($LASTEXITCODE -ne 0) {
	throw 'Locating the Python build tools failed.'
}
$env:PATH = ($pythonScripts -join ';') + ';' + $env:PATH

if (-not $BuildDirectory) {
	$BuildDirectory = Join-Path $root 'build'
}
elseif (-not [IO.Path]::IsPathRooted($BuildDirectory)) {
	$BuildDirectory = Join-Path $root $BuildDirectory
}
$BuildDirectory = [IO.Path]::GetFullPath($BuildDirectory)

Push-Location $root
try {
	& git submodule update --init --depth 1 --jobs $Jobs
	if ($LASTEXITCODE -ne 0) {
		throw 'Initializing the pinned dependencies failed.'
	}

	$configurationFile = Join-Path $BuildDirectory 'meson-info\meson-info.json'
	if (Test-Path -LiteralPath $configurationFile) {
		$previous = Get-Content -LiteralPath $configurationFile -Raw | ConvertFrom-Json
		if ($previous.directories.source -ne $root -or $previous.directories.build -ne $BuildDirectory) {
			throw 'This build directory belongs to another location. Use -BuildDirectory with a fresh directory after moving the repository.'
		}
	}

	$setupArguments = @(
		'setup',
		$BuildDirectory,
		'--native-file',
		(Join-Path $root 'meson_amd64.ini'),
		'--buildtype',
		$Configuration,
		'-Ddev=true'
	)
	if (Test-Path -LiteralPath (Join-Path $BuildDirectory 'meson-private\coredata.dat')) {
		$setupArguments += '--reconfigure'
	}
	& py -3 -m mesonbuild.mesonmain @setupArguments
	if ($LASTEXITCODE -ne 0) {
		throw 'Configuring s4-mod failed.'
	}

	if (-not $SetupOnly) {
		& py -3 -m mesonbuild.mesonmain compile -C $BuildDirectory -j $Jobs src/crash_handler/zpCrashHandler
		if ($LASTEXITCODE -ne 0) {
			throw 'Building the crash handler failed.'
		}

		& py -3 -m mesonbuild.mesonmain compile -C $BuildDirectory -j $Jobs src/client-s4/version src/piped_console/zpPipedConsole
		if ($LASTEXITCODE -ne 0) {
			throw 'Building s4-mod failed.'
		}
		Write-Host "Client DLL: $(Join-Path $BuildDirectory 'src\client-s4\version.dll')"
		Write-Host "Crash handler: $(Join-Path $BuildDirectory 'src\crash_handler\zpCrashHandler.exe')"
		Write-Host "Piped console: $(Join-Path $BuildDirectory 'src\piped_console\zpPipedConsole.exe')"
	}
}
finally {
	Pop-Location
}
