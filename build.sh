#!/usr/bin/env bash
# Build cruzado (WSL/MinGW) da DLL do plugin.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"
if ! command -v x86_64-w64-mingw32-g++ >/dev/null; then
  echo "!! MinGW-w64 ausente. Instale (uma vez, precisa de sudo):"
  echo "   sudo apt install -y gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 cmake"
  exit 1
fi
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build --config Release
echo "OK -> $(ls -1 build/SkippableCutscenesRevival.dll 2>/dev/null || echo 'DLL nao encontrada')"
