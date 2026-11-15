# Building

The game's code is recompiled at build time from your own dump of Mario Tennis (USA, NM8E, big-endian `.z64`, SHA-1
999047f07cec931ffbdcc7b33b8502ef602807ee). The program, native or the page, then takes the same dump from its
launcher's Load ROM.

## 1. Clone with the submodules

```bash
git clone --recurse-submodules https://github.com/n64hub/MarioTennisRecomp-web
```

`lib/` holds RT64, N64ModernRuntime (with N64Recomp) and RecompFrontend from their upstream repositories, at the
commits [N64Recomp-browser](https://github.com/n64hub/N64Recomp-browser)'s web forks are built on, and
N64Recomp-browser, whose `patches/` puts the web support into their checkouts and whose `frontend/` makes the page.

## 2. Tools

- CMake 3.20+, Ninja, clang for the program (clang-cl on Windows), SDL2 and GTK 3 on Linux
  (`sudo apt-get install cmake ninja-build libsdl2-dev libgtk-3-dev clang`).
- Python 3 with [rabbitizer](https://github.com/Decompollaborate/rabbitizer), which `tools/gen_symbols.py` finds the
  game's functions with: `python3 -m venv .venv && .venv/bin/pip install -r tools/requirements.txt`, then the
  venv's `bin` on `PATH` (`Scripts` on Windows).
- For the page: emsdk (`emsdk install latest`) and [naga](https://github.com/gfx-rs/wgpu/tree/trunk/naga)
  (`cargo install naga-cli`).

## 3. The recompiled code

```bash
# The game's own change to lib/rt64, then N64Recomp-browser's patches in lib/rt64, lib/N64ModernRuntime and
# lib/RecompFrontend
cmake -P lib-patches/apply.cmake lib

# N64Recomp and RSPRecomp, from the patched N64Recomp in lib/
cmake -S lib/N64ModernRuntime/N64Recomp -B build-n64recomp -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-n64recomp --target N64RecompCLI RSPRecomp
cp build-n64recomp/N64Recomp build-n64recomp/RSPRecomp .                  # .exe on Windows

# build_assets/ and syms/ from the ROM, then RecompiledFuncs/ and rsp/aspMain.cpp
sh tools/recompile.sh <your dump>
```

The native program needs the patches as much as the page does: the code the patched N64Recomp writes uses the patched
`recomp.h` (`RECOMP_TAIL_CALL`). The web support in them is behind `__EMSCRIPTEN__` and changes nothing natively.
Configuring the program (step 4, or the page) applies them too if they aren't in yet, and does nothing if they are,
and writes `include/recomp.h`, the patched `recomp.h` with the game's software TLB in every memory access
(`tools/make_recomp_h.py`). `git apply --reverse lib-patches/rt64.patch` in `lib/rt64` takes the game's change out,
after N64Recomp-browser's (`git apply --reverse lib/N64Recomp-browser/patches/<name>.patch`).

## 4. Native

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build --target MarioTennisRecompiled
```

Run it from the repository's root (it reads `assets/`). A `portable.txt` there keeps settings, saves and the stored
ROM beside it.

## The web page

```bash
embuilder build sdl2-mt freetype-legacysjlj      # once
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release -DRT64_NAGA_EXECUTABLE=<naga>
cmake --build build-web --target MarioTennisRecompiled
```

Keep `build-web` inside the checkout and put [ccache](https://ccache.dev) on `PATH`: N64Recomp-browser's
`frontend/build_speed.cmake` puts it in front of every compile, and a second fresh build dir then compiles almost
nothing.

`build-web/MarioTennisRecompiled.html` must be served with `Cross-Origin-Opener-Policy: same-origin` and
`Cross-Origin-Embedder-Policy: require-corp` (threads need SharedArrayBuffer), or with `coi_serviceworker.js` beside
it, which the build puts there. N64Recomp-browser's `tools/serve.py build-web` serves it so.

### The shader list

The page specializes the shaders in `src/rt64_shaders.txt` at build time (`RT64_PREBUILT_SHADER_LIST`), so that it
never draws with RT64's slow general shader. The list is recorded by `tools/record_shaders.sh <build-web> <rom>`, which
plays `tools/shader_plan.json` with N64Recomp-browser's `tools/record_shaders.mjs`: the page opened with
`?record-shaders` prints every shader the game asks for, and the plan goes through the menus with A as a player does,
to a match on each court and into each mode. A configure that ran before the list was there doesn't see it: configure
`build-web` again (`cmake build-web`) after the first recording. Its header says what was played and what wasn't.

`tools/play_plan.json` is the page's regular play, 2-3 minutes with every timeout counted, which N64Recomp-browser's
`tools/play_page.mjs` plays to measure the page: the menus to Singles against Luigi on the Hard Court, and 100 s of
the match. `tools/play_plan_extended.json`, under 5 minutes, plays 3.3 minutes of the same match, for a long play.
Each plan's header says what it plays.

## What was played

Every run of the page, in Chrome and Firefox, is a line of N64Recomp-browser's `plays.jsonl` (game `mt`), with what it
measured. The page was played through the menus to matches on all four courts, in the Mushroom Cup, Ring Shot, Bowser
Stage, Piranha Challenge and doubles (`src/rt64_shaders.txt`'s header), with the players' shadows, the speed board and
the replay box drawn.
