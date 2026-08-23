#pragma once
// AOB scan com máscara sobre o módulo do jogo. Algoritmo idêntico ao do
// Stracker's Loader (MIT, ver reference/strackers-loader/MHWLoader/scanmem.h);
// reimplementado aqui para não termos dependência de código de terceiros.
#include <windows.h>
#include <psapi.h>
#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

namespace scr {
using byte = unsigned char;

// Converte "80 B9 D3040000 00 74 ??" em (bytes, mask). '?' vira wildcard.
inline std::tuple<std::vector<byte>, std::vector<byte>> parse_aob(std::string_view aob) {
  std::vector<byte> data, mask;
  int hi = -1;
  for (char c : aob) {
    int v;
    if (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
    else if (c == '?') v = -2;              // wildcard nibble
    else continue;                          // espaços etc.
    if (hi == -1) { hi = v; continue; }
    if (hi == -2 || v == -2) { data.push_back(0); mask.push_back(0x00); }
    else { data.push_back((byte)((hi << 4) | v)); mask.push_back(0xFF); }
    hi = -1;
  }
  return {data, mask};
}

inline std::vector<byte*> scan(const std::vector<byte>& bytes, const std::vector<byte>& mask) {
  std::vector<byte*> results;
  HMODULE mod = GetModuleHandleA("MonsterHunterWorld.exe");
  if (!mod) return results;
  MODULEINFO mi{};
  if (!GetModuleInformation(GetCurrentProcess(), mod, &mi, sizeof(mi))) return results;

  byte* start = (byte*)mod;
  byte* end = start + mi.SizeOfImage;
  std::vector<std::tuple<byte, byte>> pat(bytes.size());
  for (size_t i = 0; i < bytes.size(); ++i) pat[i] = {bytes[i], mask[i]};
  auto pred = [](byte a, std::tuple<byte, byte> b) {
    auto [chk, m] = b; return (a & m) == (chk & m);
  };

  byte* addr = start;
  while (addr < end) {
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery(addr, &info, sizeof(info))) break;
    byte* rb = (byte*)info.BaseAddress;
    byte* re = rb + info.RegionSize;
    if (info.State == MEM_COMMIT && !(info.Protect & PAGE_GUARD) &&
        !(info.Protect & PAGE_NOACCESS)) {
      byte* found = std::search(rb, re, pat.begin(), pat.end(), pred);
      while (found != re) {
        results.push_back(found);
        found = std::search(found + 1, re, pat.begin(), pat.end(), pred);
      }
    }
    addr = re;
  }
  return results;
}

inline std::vector<byte*> scan(std::string_view aob) {
  auto [b, m] = parse_aob(aob);
  return scan(b, m);
}
}  // namespace scr
