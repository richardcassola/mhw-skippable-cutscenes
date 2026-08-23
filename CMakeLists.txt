cmake_minimum_required(VERSION 3.16)
project(SkippableCutscenesRevival CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(SkippableCutscenesRevival SHARED
  src/dllmain.cpp
)
set_target_properties(SkippableCutscenesRevival PROPERTIES
  PREFIX ""                       # sem "lib" no nome
  OUTPUT_NAME "SkippableCutscenesRevival"
)
# DLL mínima e autossuficiente: sem dependências de runtime MinGW.
target_link_options(SkippableCutscenesRevival PRIVATE
  -static -static-libgcc -static-libstdc++
  -Wl,--enable-stdcall-fixup
)
target_link_libraries(SkippableCutscenesRevival PRIVATE psapi)
target_compile_options(SkippableCutscenesRevival PRIVATE -O2 -Wall -Wextra)
