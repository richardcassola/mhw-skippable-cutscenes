# Log de implementação

## 2026-08-18
- **Disco cheio resolvido**: raiz estava 46G/49G (100%). Causa principal:
  `~/.cache/yarn` = 10G. Limpo via `yarn cache clean --all` → 11G livres.
  O `apt install` do MinGW na verdade completou; o erro que apareceu era só o
  trigger do `man-db` (`/var/cache/man`), cosmético.
- **Toolchain OK**: `gcc/g++-mingw-w64-x86-64` 13-win32 + `cmake` instalados.
- **Sonda compilada**: `./build.sh` → `build/SkippableCutscenesRevival.dll`
  (PE32+ x64; exporta `onLoad`; importa só KERNEL32+msvcrt; 2.3 MB estático).
- **Instalada no jogo** via `scripts/deploy.sh probe`: DLL original renomeada
  para `CutsceneSkip.dll.disabled`; nossa DLL em `nativePC/plugins/`.
- Aguardando: usuário abrir o jogo (até o menu basta) → ler
  `SkippableCutscenesRevival.log` (`scripts/deploy.sh log`) para confirmar que
  o scanner acha P1/P2 e qual gatilho de carga (DllMain vs onLoad) dispara.

## Resultado da sonda (2026-08-18 21:29) — VERDE
Jogo aberto até o menu; `SkippableCutscenesRevival.log`:
- `execute_skip (P1)`: **1 candidato** @ `0x141a83380`
- `show_prompt  (P2)`: **1 candidato** @ `0x141a85e16`
- Carga disparou (via DllMain thread); logging próprio OK.
Conclusão: nosso scanner + assinaturas próprias acham os dois pontos de
injeção de forma **única** no build 15539686. Base validada → seguir para E1.
(Base de imagem 0x140000000; .text descriptografada em memória apesar do Denuvo,
como esperado.) OQ-T1: o gatilho foi o DllMain (o loader só faz LoadLibrary).

## Manutenção — Claude Code
Auto-update falhava por causa de um stub de 0 byte da versão 2.1.235 (resquício
do disco cheio). Removido o stub + espaço liberado → `claude update` concluiu
(2.1.234 → 2.1.235). Ativo após reiniciar a sessão.

## Estado do jogo
`deploy.sh restore`: mod ORIGINAL de volta (`CutsceneSkip.dll`), nossa sonda
removida. Jogável com o skip (com o disconnect conhecido). Nossa DLL fica em
`build/` para o próximo experimento.

## E1 implementado e instalado (2026-08-18)
- `src/inject.h`: patch fail-safe. E1 = trocar `jne` (0x75) por `jmp` (0xEB) no
  site P2+7 → toda cutscene passa pelo caminho nativo de skip do jogo (o mesmo
  das cenas já puláveis, seguro para a sessão). 1 byte, reversível, só aplica
  se a assinatura for única e o byte for 0x75.
- Build OK; instalado via `deploy.sh probe` + `deploy.sh cfg E1`.
- Aguardando teste in-game do usuário: prompt aparece? pula? mantém online
  (não cai)? áudio limpo? crash? Ler `SkippableCutscenesRevival.log` depois.

## Resultado do E1 (2026-08-18 21:42) — NEGATIVO + reviravolta
- Log confirma: patch aplicado (`jne->jmp @ 0x141a85e1d`).
- In-game (cutscene do Diablos): **NÃO pulou** (E1 não produz skip; minha
  hipótese sobre aquele branch estava errada) **E** deu **communication error
  227-MW1** — mesmo sem ter pulado nada.
- Pesquisa: **227-MW1 é um erro GERAL de conexão** do MHW online, que atinge
  também jogadores SEM mod (aparece desde o lançamento do Iceborne; relatado
  como instabilidade de sessão/servidor). Não é específico de "skip errado".
- Consequência: a premissa "o skip causa a queda" está **abalada**. A queda
  aconteceu sem skip, e o código de erro é de instabilidade geral. Duas
  possibilidades: (a) nosso patch E1 causou a queda por si; (b) a queda é
  independente do mod (rede/sessão do usuário). **Falta o controle que nunca
  fizemos: rodar SEM mod nenhum.**
- PLANO: baseline sem mod. Se 227-MW1 ainda ocorrer sem mod → problema é de
  conexão/sessão, não do mod (e a solução é rede/jogar offline, não código).
  Se sumir → algum mod causa, e E1 foi nocivo; abordagem mais segura no E2.
- Bloqueio operacional: jogo estava aberto (DLL travada) — trocar arquivos só
  com o jogo fechado.
