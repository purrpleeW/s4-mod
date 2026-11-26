[![license](https://img.shields.io/badge/license-All%20Rights%20Reserved-red)](LICENSE)
[![discord](https://img.shields.io/badge/discord-join-5865F2?logo=discord&logoColor=white)](https://discord.gg/demonware)

# S4-Mod

S4-Mod is a client for **Call of Duty®: Vanguard** that enables offline play, GSC scripting, modding tools, and basic LAN connectivity.

<p align="center">
	<img src="assets/s4mod_banner.png" alt="S4-Mod" width="800" />
</p>

## Download

Build the latest client from source using the instructions below

## Installation

- Place the built `version.dll` in the game's root folder so the game loads it automatically.
- If your game build includes Ricochet anti-cheat, launch the anti-cheat separately using your batch files (or ours, using the Discord below).

## Compile from source code

Requires Windows x64, Git, Python 3.10+, LLVM 22.1+, and Visual Studio C++ build tools with the Windows SDK and ATL.

- Clone the Git repo. **DO NOT download it as ZIP**, it will not work.
- Run `Build.cmd` to get dependencies, configure the project, and build the client automatically.

```bat
git clone https://git.zeromods.xyz/ZeroMods/s4-mod.git
cd s4-mod
Build.cmd
```

The built client is located at `build/src/client-s4/version.dll`.

After adding or removing source files, run `Refresh.cmd` to regenerate the Meson build files without compiling, `Build.cmd` also refreshes them automatically too.

## Features

- Offline play and modding features that aren't available in a retail install.
- GSC scripting and other basic modding features.
- Basic LAN connectivity -> use `party_getSessionInfo` to copy session information to your clipboard, or `connect <ip>` to connect directly.

## Credits

- [momo5502](https://github.com/momo5502) - project framework and XLabs work
- [auroramod/iw7-mod](https://github.com/auroramod/iw7-mod) - in-game console base, framework development and heavy inspiration
- Xifil, luxploit and purrplee for S4-Mod development

## Disclaimer

This software has been created purely for the purposes of academic research. It is not intended to be used to attack other systems. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.
