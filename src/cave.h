#pragma once
// Code-caves: aloca memória executável a até ±2 GB do site (alcance do
// `jmp rel32`), copia o corpo do patch, salta de volta para o site e
// sobrescreve o site com `jmp cave` (+ NOPs). Cada cave tem um contador de
// execuções (dword) que o corpo incrementa — é o nosso "breakpoint de log".
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include "log.h"
#include "scanmem.h"

namespace scr {

struct Cave {
  byte* site = nullptr;   // onde o jmp foi escrito
  byte* mem  = nullptr;   // página alocada (corpo + salto de volta)
  volatile uint32_t* hits = nullptr;  // contador dentro da página
};

inline constexpr size_t CAVE_PAGE     = 0x1000;
inline constexpr size_t CAVE_HITS_OFF = 0x800;  // contador fica no meio da página

inline bool rel32_fits(const byte* from, const byte* to) {
  intptr_t d = (intptr_t)to - (intptr_t)(from + 5);
  return d >= INT32_MIN && d <= INT32_MAX;
}

// Procura região livre abaixo do site (e, se faltar, acima) dentro de ±2 GB.
inline byte* alloc_near(byte* site, size_t size) {
  SYSTEM_INFO si{};
  GetSystemInfo(&si);
  const uintptr_t gran = si.dwAllocationGranularity;
  const uintptr_t s = (uintptr_t)site;
  const uintptr_t lo = s > 0x7FF00000 ? s - 0x7FF00000 : gran;
  const uintptr_t hi = s + 0x7FF00000;

  auto try_at = [&](uintptr_t a) -> byte* {
    void* p = VirtualAlloc((void*)a, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    return (byte*)p;
  };

  // Para baixo: do site até lo, pulando regiões ocupadas.
  for (uintptr_t a = (s & ~(gran - 1)) - gran; a > lo;) {
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery((void*)a, &mbi, sizeof(mbi))) break;
    uintptr_t base = (uintptr_t)mbi.BaseAddress;
    if (mbi.State == MEM_FREE && mbi.RegionSize >= size) {
      uintptr_t cand = (base + mbi.RegionSize - size) & ~(gran - 1);
      if (cand >= base && cand >= lo)
        if (byte* p = try_at(cand)) return p;
    }
    if (base < gran) break;
    a = (base - 1) & ~(gran - 1);
  }
  // Para cima: do site até hi.
  for (uintptr_t a = (s & ~(gran - 1)) + gran; a < hi;) {
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery((void*)a, &mbi, sizeof(mbi))) break;
    uintptr_t base = (uintptr_t)mbi.BaseAddress;
    if (mbi.State == MEM_FREE && mbi.RegionSize >= size) {
      uintptr_t cand = (base + gran - 1) & ~(gran - 1);
      if (cand + size <= base + mbi.RegionSize && cand + size <= hi)
        if (byte* p = try_at(cand)) return p;
    }
    a = base + mbi.RegionSize;
  }
  return nullptr;
}

inline bool write_code(byte* dst, const byte* src, size_t n) {
  DWORD old = 0;
  if (!VirtualProtect(dst, n, PAGE_EXECUTE_READWRITE, &old)) return false;
  std::memcpy(dst, src, n);
  DWORD tmp;
  VirtualProtect(dst, n, old, &tmp);
  FlushInstructionCache(GetCurrentProcess(), dst, n);
  return true;
}

inline void put_rel32(byte* at, const byte* from, const byte* to) {
  int32_t d = (int32_t)((intptr_t)to - (intptr_t)(from + 5));
  std::memcpy(at, &d, 4);
}

// Instala a cave. `overwrite_len` (>= 5) deve cair em fronteira de instrução
// no site; os bytes sobrescritos precisam ser re-executados pelo `body` (ou
// ser descartáveis). Depois do body, a cave salta para `site + overwrite_len`.
// O body NÃO deve usar `rip`-relativo com alvo fora da cave.
inline bool install_cave(const char* name, byte* site, size_t overwrite_len,
                         const std::vector<byte>& body, Cave& out) {
  if (overwrite_len < 5) {
    SCR_ERR("%s: overwrite_len %zu < 5.", name, overwrite_len);
    return false;
  }
  byte* mem = alloc_near(site, CAVE_PAGE);
  if (!mem || !rel32_fits(site, mem)) {
    SCR_ERR("%s: sem memoria a +-2GB do site %p (mem=%p).", name, (void*)site, (void*)mem);
    if (mem) VirtualFree(mem, 0, MEM_RELEASE);
    return false;
  }
  std::memset(mem, 0xCC, CAVE_PAGE);           // int3 em tudo que nao for codigo
  volatile uint32_t* hits = (volatile uint32_t*)(mem + CAVE_HITS_OFF);
  *hits = 0;

  // Corpo: inc dword [rip+rel32] (contador) ; body ; jmp site+overwrite_len
  std::vector<byte> code;
  code.push_back(0xFF); code.push_back(0x05);  // inc dword ptr [rip+rel32]
  int32_t inc_rel = (int32_t)((mem + CAVE_HITS_OFF) - (mem + 6));
  code.insert(code.end(), (byte*)&inc_rel, (byte*)&inc_rel + 4);
  code.insert(code.end(), body.begin(), body.end());
  size_t jmp_at = code.size();
  code.push_back(0xE9); code.resize(code.size() + 4);
  put_rel32(&code[jmp_at + 1], mem + jmp_at, site + overwrite_len);
  if (code.size() > CAVE_HITS_OFF) {
    SCR_ERR("%s: corpo grande demais (%zu).", name, code.size());
    VirtualFree(mem, 0, MEM_RELEASE);
    return false;
  }
  std::memcpy(mem, code.data(), code.size());
  FlushInstructionCache(GetCurrentProcess(), mem, code.size());

  // Site: jmp cave + NOPs. Escreve o opcode E9 por ultimo (reduz a janela em
  // que outra thread veria um salto pela metade).
  std::vector<byte> patch(overwrite_len, 0x90);
  patch[0] = 0xE9;
  put_rel32(&patch[1], site, mem);
  if (!write_code(site + 1, &patch[1], overwrite_len - 1) ||
      !write_code(site, &patch[0], 1)) {
    SCR_ERR("%s: falha ao escrever o jmp no site %p.", name, (void*)site);
    VirtualFree(mem, 0, MEM_RELEASE);
    return false;
  }
  out.site = site;
  out.mem = mem;
  out.hits = hits;
  SCR_INFO("%s: cave @ %p (corpo %zu bytes), site %p -> jmp (sobrescritos %zu).",
           name, (void*)mem, code.size(), (void*)site, overwrite_len);
  return true;
}

}  // namespace scr
