#pragma once
// Aplicação dos experimentos (patches em memória). Tudo fail-safe: só aplica se
// a assinatura for única E o byte alvo for exatamente o esperado; caso
// contrário, não toca em nada e loga. Patches são em memória (revertem ao
// fechar o jogo; trocar a DLL desfaz tudo no próximo boot).
#include <windows.h>
#include "log.h"
#include "scanmem.h"
#include "signatures.h"
#include "experiment.h"

namespace scr {

// Escreve `n` bytes em `dst` contornando a proteção de página.
inline bool patch_bytes(scr::byte* dst, const scr::byte* src, size_t n) {
  DWORD old = 0;
  if (!VirtualProtect(dst, n, PAGE_EXECUTE_READWRITE, &old)) return false;
  memcpy(dst, src, n);
  DWORD tmp;
  VirtualProtect(dst, n, old, &tmp);
  FlushInstructionCache(GetCurrentProcess(), dst, n);
  return true;
}

// E1: força o "caminho nativo de tratamento de skip" para TODA cutscene.
// No site P2, o jogo faz:
//   +0x07: 75 0D   jne native_handle     ; salta se a cena é pulável
// Trocamos 0x75 (jne) por 0xEB (jmp) => salto incondicional => toda cutscene
// passa a ser tratada pelo skip nativo (seguro para a sessão online).
inline bool apply_E1_native_flag() {
  auto hits = scr::scan(scr::sig::SIG_SHOW_PROMPT);
  if (hits.size() != 1) {
    SCR_ERR("E1: site show_prompt nao unico (%zu) - nada aplicado.", hits.size());
    return false;
  }
  scr::byte* jcc = hits[0] + 7;  // o 'jne' (0x75) logo apos o cmp [rbx+4D3]
  if (*jcc != 0x75) {
    SCR_ERR("E1: byte inesperado em +7: 0x%02X (esperava 0x75) - nada aplicado.", *jcc);
    return false;
  }
  scr::byte jmp = 0xEB;  // jmp rel8 (mesmo deslocamento 0x0D)
  if (!patch_bytes(jcc, &jmp, 1)) {
    SCR_ERR("E1: VirtualProtect/escrita falhou @ %p.", (void*)jcc);
    return false;
  }
  SCR_INFO("E1 aplicado: jne->jmp @ %p (toda cutscene via skip nativo).", (void*)jcc);
  return true;
}

inline void apply_experiment(scr::Experiment exp) {
  switch (exp) {
    case scr::Experiment::E0_Parity:
      SCR_INFO("E0: baseline (probe), nenhuma injecao.");
      break;
    case scr::Experiment::E1_NativeFlag:
      apply_E1_native_flag();
      break;
    default:
      SCR_WARN("Experimento %s ainda nao implementado - nada aplicado.",
               scr::experiment_name(exp));
      break;
  }
}

}  // namespace scr
