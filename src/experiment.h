#pragma once
// Comportamento selecionável em runtime, sem recompilar: lê o primeiro token de
// SkippableCutscenesRevival.cfg na raiz do jogo. Default: E0 (paridade).
#include <cstdio>
#include <cstring>

namespace scr {
enum class Experiment {
  E0_Parity,       // reproduz o original (força prompt + seek). Baseline: deve derrubar.
  E1_NativeFlag,   // seta +0x4D3=1 e deixa o skip nativo agir (Opção A, atalho).
  E2_NativeCall,   // chama a rotina de término nativa (Opção A, completo).
  E3_FastForward,  // avança o tempo por frame (Opção B).
  E4_FailClosed,   // não oferece skip em sessão online (Opção C).
};

inline Experiment read_experiment() {
  FILE* f = std::fopen("SkippableCutscenesRevival.cfg", "r");
  if (!f) return Experiment::E0_Parity;
  char buf[64] = {0};
  std::fgets(buf, sizeof(buf), f);
  std::fclose(f);
  if (std::strncmp(buf, "E1", 2) == 0) return Experiment::E1_NativeFlag;
  if (std::strncmp(buf, "E2", 2) == 0) return Experiment::E2_NativeCall;
  if (std::strncmp(buf, "E3", 2) == 0) return Experiment::E3_FastForward;
  if (std::strncmp(buf, "E4", 2) == 0) return Experiment::E4_FailClosed;
  return Experiment::E0_Parity;
}

inline const char* experiment_name(Experiment e) {
  switch (e) {
    case Experiment::E0_Parity:      return "E0 (paridade/baseline)";
    case Experiment::E1_NativeFlag:  return "E1 (rota nativa via flag)";
    case Experiment::E2_NativeCall:  return "E2 (chamar término nativo)";
    case Experiment::E3_FastForward: return "E3 (fast-forward)";
    case Experiment::E4_FailClosed:  return "E4 (falha fechada online)";
  }
  return "?";
}
}  // namespace scr
