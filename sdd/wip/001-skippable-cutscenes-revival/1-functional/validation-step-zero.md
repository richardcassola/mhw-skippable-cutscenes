# Passo zero — validar a DLL original (v1.1) no build atual

Objetivo: descobrir se `CutsceneSkip.dll` v1.1 (2025-02-12) funciona no MHW
15.23.00 (build 15539686) com o Stracker's Loader. Sem isso, o problem
statement é uma suposição.

## O que você (Windows) precisa fazer
1. **VC++ Redistributable x64 (2015–2022)** — provavelmente já tem; se o jogo
   reclamar de `MSVCP140.dll`/`VCRUNTIME140_1.dll`, instale.
2. **Stracker's Loader** — baixe a versão mais recente em
   https://www.nexusmods.com/monsterhunterworld/mods/1982 (exige login).
   Extraia **na raiz do jogo** (`D:\SteamLibrary\steamapps\common\Monster Hunter World\`):
   `dinput8.dll`, `loader.dll`, `loader-config.json`.
3. Me avise ("loader instalado"). Eu rodo `scripts/validate-original.sh stage`,
   que copia a DLL original para `nativePC\plugins\` e liga o log
   (`logfile: true`, `logcmd: true`, `logLevel: INFO`). Se preferir fazer à
   mão: copie `reference/original-mod/extracted/nativePC/plugins/CutsceneSkip.dll`
   para `<jogo>\nativePC\plugins\` e edite o `loader-config.json` com esses
   valores.
4. **Inicie o jogo pela Steam.** Deve abrir uma janela de console preta
   "Stracker's Loader". Vá até o menu principal e carregue o save.
5. Entre em uma quest com cutscene que o jogo normalmente **não** deixa pular
   (uma assigned quest com intro de monstro serve; evite *"The Best Kind of
   Quest"*). Observe:
   - apareceu o **prompt de skip**?
   - ao apertar, a cena terminou em ~1 s e a quest seguiu normal?
   - algum crash/tela preta?
6. Feche o jogo e me diga o resultado. Eu leio o log direto do WSL
   (`scripts/validate-original.sh check`).

## O que esperar no `loader.log`
Sucesso:
```
Loading plugin "nativePC\plugins\CutsceneSkip.dll"
Cutscene Skip Loading...
found 1 candidate injection points.      (2 vezes — uma por ponto)
Succesful Memory Allocation              (2 vezes)
Cutscene Skip Loaded!
DONE !
```
Falha típica:
```
found no candidate injection points - Aborting.
```
ou `Failed to load "nativePC\plugins\CutsceneSkip.dll"` (runtime VC++ ausente).

## Como interpretar
| Resultado | Consequência para o projeto |
|-----------|-----------------------------|
| Prompt aparece e skip funciona | Premissa "quebrou" é falsa para este build. Projeto segue como **reprodução em fonte própria + paridade**; os pontos internos da v1.1 são o ponto de partida validado. |
| Log diz `found no candidate` em 1 ou 2 pontos | Confirma quebra e **qual** ponto. Fase técnica começa redescobrindo esse ponto. |
| Log ok, mas sem prompt/skip in-game | Pontos existem mas o comportamento mudou (offsets de struct). Fase técnica: revalidar offsets. |
| Crash | Pior caso; a fase técnica prioriza a falha segura (US-2) antes de qualquer paridade. |
