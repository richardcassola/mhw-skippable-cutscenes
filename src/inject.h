#pragma once
// Aplicação dos experimentos (patches em memória). Tudo fail-safe: só aplica se
// a assinatura for única E os bytes do site forem exatamente os esperados; caso
// contrário, não toca em nada e loga. Patches são em memória (revertem ao
// fechar o jogo; trocar a DLL desfaz tudo no próximo boot).
#include <windows.h>
#include <vector>
#include "log.h"
#include "scanmem.h"
#include "signatures.h"
#include "experiment.h"
#include "cave.h"

namespace scr {

// Caves instaladas nesta sessão (para o monitor de contadores).
inline std::vector<Cave>& caves() { static std::vector<Cave> v; return v; }

inline byte* unique_site(const char* name, const char* aob) {
  auto hits = scan(aob);
  if (hits.size() != 1) {
    SCR_ERR("%s: site nao unico (%zu candidatos) - nada aplicado.", name, hits.size());
    return nullptr;
  }
  return hits[0];
}

// E1 (Opção A, atalho): marcar TODA cutscene como nativamente pulável.
// Gravamos `+0x4D3 = 1` no objeto da cutscene imediatamente antes das duas
// checagens nativas dessa flag, e deixamos o código ORIGINAL do jogo decidir e
// executar o skip — o mesmo caminho das cenas que o jogo já deixa pular (que
// não derrubam a sessão nem deixam áudio órfão). Nenhum seek nosso.
//
//   P2 (teste por frame "mostrar prompt?"), this em rbx:
//     site+0: 40 38 B3 D3040000   cmp byte [rbx+4D3h], sil   <- sobrescrito
//     site+7: 75 0D               jne native_prompt
//   cave:     mov byte [rbx+4D3h], 1 ; cmp byte [rbx+4D3h], sil ; jmp site+7
//
//   P1 (rotina "executar skip"), this em rcx:
//     site+0: 80 B9 D3040000 00   cmp byte [rcx+4D3h], 0     <- sobrescrito
//     site+7: 74 33               je  not_skippable
//   cave:     mov byte [rcx+4D3h], 1 ; cmp byte [rcx+4D3h], 0 ; jmp site+7
inline bool apply_E1_native_flag() {
  byte* p2 = unique_site("E1/P2", sig::SIG_SHOW_PROMPT);
  byte* p1 = unique_site("E1/P1", sig::SIG_EXECUTE_SKIP);
  if (!p1 || !p2) return false;

  Cave c2{}, c1{};
  const std::vector<byte> body_p2 = {
      0xC6, 0x83, 0xD3, 0x04, 0x00, 0x00, 0x01,   // mov byte [rbx+4D3h], 1
      0x40, 0x38, 0xB3, 0xD3, 0x04, 0x00, 0x00,   // cmp byte [rbx+4D3h], sil (original)
  };
  const std::vector<byte> body_p1 = {
      0xC6, 0x81, 0xD3, 0x04, 0x00, 0x00, 0x01,   // mov byte [rcx+4D3h], 1
      0x80, 0xB9, 0xD3, 0x04, 0x00, 0x00, 0x00,   // cmp byte [rcx+4D3h], 0 (original)
  };
  if (!install_cave("E1/P2 (prompt)", p2, 7, body_p2, c2)) return false;
  if (!install_cave("E1/P1 (skip)", p1, 7, body_p1, c1)) {
    SCR_WARN("E1: P1 falhou; P2 ficou instalado (prompt forcado, sem skip).");
    caves().push_back(c2);
    return false;
  }
  caves().push_back(c2);
  caves().push_back(c1);
  SCR_INFO("E1 aplicado: +0x4D3=1 em P2 e P1 (toda cutscene via skip nativo).");
  return true;
}

inline void apply_experiment(Experiment exp) {
  switch (exp) {
    case Experiment::E0_Parity:
      SCR_INFO("E0: baseline (probe), nenhuma injecao.");
      break;
    case Experiment::E1_NativeFlag:
      apply_E1_native_flag();
      break;
    default:
      SCR_WARN("Experimento %s ainda nao implementado - nada aplicado.",
               experiment_name(exp));
      break;
  }
}

// Loga os contadores das caves quando mudam (a cada ~2 s). É o nosso
// "breakpoint": diz se o jogo passou pelos sites e quantas vezes.
inline DWORD WINAPI cave_monitor(LPVOID) {
  std::vector<uint32_t> last(caves().size(), 0);
  for (;;) {
    Sleep(2000);
    for (size_t i = 0; i < caves().size(); ++i) {
      uint32_t h = *caves()[i].hits;
      if (h != last[i]) {
        SCR_INFO("cave[%zu] site %p: %u execucoes", i, (void*)caves()[i].site, h);
        last[i] = h;
      }
    }
  }
  return 0;
}

}  // namespace scr
