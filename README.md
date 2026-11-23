# Mario Tennis: Recompiled, on the web

> [!WARNING]
> **This is n64hub/MarioTennisRecomp-web, a fork, not MarioTennisRecomp.** Its `main` branch is Ieatzcrayonz's
> [MarioTennisRecomp](https://github.com/Ieatzcrayonz/MarioTennisRecomp) moved onto
> [RecompFrontend](https://github.com/N64Recomp/RecompFrontend) and built into a web page (WebGPU, through
> [N64Recomp-browser](https://github.com/n64hub/N64Recomp-browser)), and everything on it past Ieatzcrayonz's commits
> was written by AI (Claude). It is not made, endorsed or supported by MarioTennisRecomp's author: nothing here is
> offered upstream, and please don't take issues with this fork to them. Building it: [BUILDING.md](BUILDING.md).
>
> This port is here to build out and test N64Recomp-browser's web support on another game than the one it was written
> for: what it turns up is fixed there where it can be, so every recomp's page gets the fix.

**Play it in the browser: https://n64hub.com/mario-tennis/**, with your own dump of Mario Tennis (USA, NM8E), picked
in the launcher and kept in the browser. It needs WebGPU (Chrome or Edge 113 and up, or Firefox). The page is built
from this branch and published with N64Recomp-browser's `tools/publish_page.sh`, so it can be a commit or two behind.

## What this fork changes

- The launcher and menus are RecompFrontend's, which every N64Recomp-browser page uses, in place of the copy of
  Zelda 64: Recompiled's that upstream carries (and its sunny court launcher screen with it). `lib/rt64`,
  `lib/N64ModernRuntime` and `lib/RecompFrontend` are upstream's at the commits N64Recomp-browser's web forks are built
  on, submodules here, patched by `lib-patches/apply.cmake`: the game's own change to RT64 first, then
  N64Recomp-browser's `patches/`, for the native build too.
- The game's code is recompiled when it is built (`tools/recompile.sh`), as upstream's development build does it, and
  not from the player's ROM as the program starts, as its release build does: a page can't make code at run time.
  So upstream's changes to N64Recomp's live recompiler and to the runtime for it are left out.
- `lib-patches/rt64.patch` is upstream's change to RT64 without its diagnostics: the images the game draws and then
  samples (shadows, the speed board) read from RDRAM, its full-screen copies kept on the GPU, two crash fixes, and the
  game's own widescreen (below). Its shadows' depth, each G_ZS_PRIM triangle at exactly its prim depth, is rt64-web's
  `RT64_EXACT_PRIM_DEPTH`, which `CMakeLists.txt` turns on. In place of its merged copy to RAM, which kept the game's
  heap from being drawn over, the program turns on rt64-web's late copy, which keeps what the CPU wrote too
  (`src/main/main.cpp`).
- Widescreen is upstream's: under Expand, the Graphics tab's default, the game draws its own wider 3D view, stretched
  to the page, with the opening flyover at 4:3 (`src/game/mt_widescreen.cpp`). Upstream's crop of the 304-wide menus
  to the window's width is left out: it draws the picture wider than the canvas, which WebGPU refuses, so the menus
  keep their black sides, centred (`src/game/mt_dlfix.cpp`).
- Upstream's Local Multiplayer setting is RecompFrontend's Assign players: up to four players, and until players are
  assigned each pad plays a port of its own in the order the pads connected, the keyboard port 1. Its four cheats are
  on the General tab, all Off by default.
- Upstream's own desktop options are left out: the choice of DirectX 12 or Vulkan (RT64 picks), the CRT shaders
  through librashader, and the Stretch aspect ratio, and so is its change to the runtime's audio status, as the page
  carries no runtime patch of the game's (the browser paces its sound). So are upstream's screenshots and its
  launcher's splash picture.
- The page takes no mods (a browser can't load recompiled code at run time) and builds the game's shaders before it
  draws with them, from `src/rt64_shaders.txt`.

What was played on the page and how it measured is in [BUILDING.md](BUILDING.md#what-was-played).


# Mario Tennis: Recompiled

A native PC port of **Mario Tennis (N64, USA)**, made with [N64Recomp](https://github.com/N64Recomp/N64Recomp)
and the [RT64](https://github.com/rt64/rt64) renderer. The original game code is translated into C and
compiled to a Windows program, so it runs natively instead of through an emulator.

> **No game files are included.** This repository holds only the port's own code, tools and artwork.
> You supply the game yourself, and it and everything generated from it stay on your machine.

## Which ROM to use
Use a dump of your own **Mario Tennis** cartridge, **North American (USA) release**. Other regions
(Europe, Japan) won't work.

| | |
|---|---|
| Name | `Mario Tennis (USA).z64` |
| Game code | `NM8E` |
| Format | `.z64` (big-endian). Convert `.n64` or `.v64` files to `.z64` first, e.g. with Tool64 |
| SHA-1 | `999047f07cec931ffbdcc7b33b8502ef602807ee` |

To check your file on Windows, run this in PowerShell and compare the result with the SHA-1 above:

```powershell
Get-FileHash "Mario Tennis (USA).z64" -Algorithm SHA1
```

If it doesn't match, the port won't recognise the file. The usual cause is the wrong region or a modified (hacked or translated) ROM.

## Download
Get `MarioTennisRecompiled-Windows.zip` from the [Releases](https://github.com/Ieatzcrayonz/MarioTennisRecomp/releases) page,
unzip it, run `MarioTennisRecompiled.exe`, pick your ROM and press Start game. The download contains no game
code or data: the game is translated from your own ROM when it starts.

## Features
- Runs at any resolution: 720p, 1080p, 1440p, 4K or matching the window.
- Widescreen: Aspect Ratio **Expand** makes the game draw a wider 3D view matching your window (16:9,
  ultrawide and so on); menus and the score display are stretched to fit. The opening airship flyover stays 4:3.
- Controller support (including DualSense), with rumble and remappable controls.
- Local multiplayer for up to 4 players: each controller is its own player, in the order they were connected.
  "On + Keyboard" also makes the keyboard a player, so one controller and a keyboard can play 2-player
  (Settings → General → Local Multiplayer).
- Saves work as on the cartridge.
- Bonus characters: Donkey Kong Jr. and Shy Guy are unlocked from the start.
- Transfer Pak characters: Alex, Harry, Nina and Kate from the Game Boy Color version are on Character Select without a Transfer Pak (level 25, starting stats).
- Optional cheats, all off by default (Settings → General): **All Courts**, **Star Characters**,
  **All Tournaments Cleared** and **Infinite Bounces**. The first three are saved with your game
  like normal unlocks, so they stay unlocked after you switch the cheat off.
- A new sunny court launcher screen.
- DirectX 12 or Vulkan (Settings → Graphics → Graphics API).
- Optional CRT shaders from RetroArch (CRT-Royale, CRT-Royale Fast, CRT-Lottes) via librashader
  (Settings → Graphics → Shader).

## Building (Windows)
Requirements: Visual Studio 2022+ Build Tools, LLVM/clang-cl, CMake, Ninja, Python 3 and Git Bash.

1. Clone [Zelda64Recomp](https://github.com/Zelda64Recomp/Zelda64Recomp) with `--recursive` and make a
   `lib` link in this folder pointing at its `lib` folder (it provides RT64, RmlUi, N64ModernRuntime and friends).
2. Apply the patches in `patches/` to `lib/N64ModernRuntime`, `lib/N64ModernRuntime/N64Recomp` and `lib/rt64`.
3. Generate the recompiled code from your ROM: `python tools/gen_symbols.py "<path to Mario Tennis (USA).z64>"`,
   then `sh tools/recompile.sh`.
4. Build: `tools\build_game.bat` (edit the Visual Studio path in it if yours differs). The result is
   `build\MarioTennisRecompiled.exe`. Run it with this folder as the working directory.
5. Release build: `tools\build_live.bat` (output in `build-live\`). It needs no generated code, so step 3 can
   be skipped: the exe translates the game's code, and runs its audio microcode, from the player's ROM at startup.
6. Optional, for the Shader option: build `librashader.dll` from
   [librashader](https://github.com/SnowflakePowered/librashader) (tag `librashader-v0.12.1`) with
   `cargo xtask build --profile release`, and configure with `-DLIBRASHADER_DLL=<path>\librashader.dll` so it is
   copied next to the exe (`tools\build_live.bat` does this for `F:\Recomps\src\librashader`). The shader presets
   in `slang-shaders/` are copied automatically. Without the DLL the game runs normally and the option does nothing.

Technical notes on how the port works are in [NOTES.md](NOTES.md).

## Credits
- N64Recomp, N64ModernRuntime and the Zelda64Recomp frontend, by Wiseguy and contributors (GPL-3.0).
- RT64 by Darío and contributors.
- [librashader](https://github.com/SnowflakePowered/librashader) by Ronny Chan (MPL-2.0 / GPL-3.0).
- CRT-Royale by TroggleMonkey (GPL-2.0+), CRT-Lottes by Timothy Lottes (public domain), from the libretro
  [slang-shaders](https://github.com/libretro/slang-shaders) collection. See [slang-shaders/README.md](slang-shaders/README.md).
- Luckiest Guy font by Astigmatic (Apache 2.0).

Mario Tennis is a trademark of Nintendo. This project is not affiliated with or endorsed by Nintendo.

## License
GPL-3.0, the same as the Zelda64Recomp code it builds on. See [LICENSE](LICENSE).
