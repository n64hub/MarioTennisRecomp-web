#!/bin/sh
# Recompiles the game from its ROM, everything the build takes from it: the recomp image (build_assets/, the ROM with
# every code segment decompressed and appended) and the symbols (tools/gen_symbols.py), the game's code
# (RecompiledFuncs/, N64Recomp mt.us.toml) and its audio microcode (rsp/aspMain.cpp, RSPRecomp aspMain.us.toml).
# Run from the repository's root, with N64Recomp and RSPRecomp built from lib/N64ModernRuntime/N64Recomp after
# lib-patches/apply.cmake and copied there (BUILDING.md):
#
#   sh tools/recompile.sh "<path to Mario Tennis (USA).z64>"
#
# tools/check_branches.py and tools/find_spins.py check the recompiled code and keep mt.us.toml's spin hooks, when the
# config or the symbols change; the build doesn't need them.
set -e
python3 tools/gen_symbols.py "$1"
./N64Recomp mt.us.toml
./RSPRecomp aspMain.us.toml
