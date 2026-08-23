#pragma once
// Assinaturas e offsets da RE do mod original (v1.1) para o MHW Steam
// 15.23.00 (build 15539686). Fonte: reference/original-mod/ANALYSIS.md.
// TODO(runtime): confirmar unicidade/endereço pelo log na primeira execução.
#include <cstdint>

namespace scr::sig {

inline constexpr const char* GAME_VERSION_TESTED = "15.23.00 (build 15539686)";

// --- Offsets na struct da cutscene (a partir do ponteiro 'this' em rcx/rbx) ---
inline constexpr std::uintptr_t OFF_FLAG_4D1     = 0x4D1; // byte, flag auxiliar
inline constexpr std::uintptr_t OFF_SKIPPABLE    = 0x4D3; // byte: !=0 => nativamente pulável
inline constexpr std::uintptr_t OFF_BLOCK_4F0    = 0x4F0; // byte: bloqueia o skip
inline constexpr std::uintptr_t OFF_TIMELINE_PTR = 0x38;  // ptr p/ objeto de timeline
inline constexpr std::uintptr_t OFF_CUR_TIME     = 0x178; // float, no objeto de timeline
inline constexpr std::uintptr_t OFF_DURATION     = 0x6C;  // int (frames), na struct da cutscene

// --- Sites de injeção (AOB) ---
// P1: rotina "executar skip". No original, para cenas NÃO puláveis faz o seek;
//     cenas puláveis caem no código nativo logo após o ponto de decisão.
inline constexpr const char* SIG_EXECUTE_SKIP =
    "80 B9 D3 04 00 00 00 74 33 80 B9 F0 04 00 00 00 75 2A";

// P2: teste por frame "mostrar prompt de skip?".
inline constexpr const char* SIG_SHOW_PROMPT =
    "40 38 B3 D3 04 00 00 75 0D 40 38 B3 D1 04 00 00 0F 84 C7 02 00 00 48 8B CB E8";

// Offsets de retorno usados pelo original ao voltar da code-cave para o site.
inline constexpr int RET_OFF_EXECUTE_SKIP = 0x29;
inline constexpr int RET_OFF_SHOW_PROMPT  = 0x07;

}  // namespace scr::sig
