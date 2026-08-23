// Entrada do plugin. O loader instalado (Stracker/QuestLoader jun/2024) só faz
// LoadLibraryA — não chama onLoad. Então disparamos no DllMain(ATTACH) via
// thread (evita loader-lock). Exportamos onLoad() com guarda, caso um loader
// mais novo o chame.
//
// >>> Este arquivo ainda NÃO instala a injeção. O próximo passo (E0) implementa
//     a code-cave/hook. Por ora, ele apenas: acha os sites por AOB, loga
//     endereço/nº de candidatos e o experimento selecionado. Isso já valida,
//     in-game, que a DLL carrega, que o scanner acha os pontos no build atual e
//     confirma OQ-T1 (DllMain vs onLoad). É a primeira medição real.
#include <windows.h>
#include <atomic>
#include "log.h"
#include "scanmem.h"
#include "signatures.h"
#include "experiment.h"
#include "inject.h"

namespace {
std::atomic<bool> g_ran{false};

void report_site(const char* name, const char* aob, int ret_off) {
  auto hits = scr::scan(aob);
  if (hits.empty()) {
    SCR_ERR("site '%s': 0 candidatos - assinatura mudou? (jogo != %s)",
            name, scr::sig::GAME_VERSION_TESTED);
  } else if (hits.size() > 1) {
    SCR_WARN("site '%s': %zu candidatos (ambiguo) - abortaria injecao",
             name, hits.size());
  } else {
    SCR_INFO("site '%s': 1 candidato @ %p (ret_off=+0x%X)",
             name, (void*)hits[0], ret_off);
  }
}

DWORD WINAPI worker(LPVOID) {
  if (g_ran.exchange(true)) return 0;  // guarda de execucao unica
  // Espera o executavel do jogo estar mapeado.
  for (int i = 0; i < 300 && !GetModuleHandleA("MonsterHunterWorld.exe"); ++i)
    Sleep(100);

  scr::Experiment exp = scr::read_experiment();
  SCR_INFO("SkippableCutscenesRevival - probe. Alvo testado: %s. Experimento: %s",
           scr::sig::GAME_VERSION_TESTED, scr::experiment_name(exp));
  report_site("execute_skip (P1)", scr::sig::SIG_EXECUTE_SKIP,
              scr::sig::RET_OFF_EXECUTE_SKIP);
  report_site("show_prompt (P2)", scr::sig::SIG_SHOW_PROMPT,
              scr::sig::RET_OFF_SHOW_PROMPT);
  scr::apply_experiment(exp);
  SCR_INFO("DONE.");
  return 0;
}
}  // namespace

extern "C" __declspec(dllexport) void onLoad() {
  CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(h);
    CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
  }
  return TRUE;
}
