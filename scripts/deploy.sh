#!/usr/bin/env bash
# Deploy do nosso plugin no jogo e leitura do log.
#   probe   -> tira a DLL original, instala a nossa, zera nosso log
#   restore -> volta a DLL original (para jogar) e tira a nossa
#   log     -> mostra o SkippableCutscenesRevival.log
#   cfg Ex  -> escreve o experimento (E0..E4) no .cfg do jogo
set -euo pipefail
GAME="${MHW_DIR:-/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World}"
HERE="$(cd "$(dirname "$0")/.." && pwd)"
OURS="$HERE/build/SkippableCutscenesRevival.dll"
PLUG="$GAME/nativePC/plugins"
ORIG_SRC="$HERE/reference/original-mod/extracted/nativePC/plugins/CutsceneSkip.dll"
LOG="$GAME/SkippableCutscenesRevival.log"

case "${1:-log}" in
  probe)
    mkdir -p "$PLUG"
    [ -f "$PLUG/CutsceneSkip.dll" ] && mv -v "$PLUG/CutsceneSkip.dll" "$PLUG/CutsceneSkip.dll.disabled"
    cp -v "$OURS" "$PLUG/SkippableCutscenesRevival.dll"
    [ -f "$LOG" ] && mv "$LOG" "$LOG.$(date +%H%M%S).bak"
    echo "OK. Abra o jogo pela Steam, chegue ao menu/carregue o save, feche e rode: $0 log"
    ;;
  restore)
    rm -fv "$PLUG/SkippableCutscenesRevival.dll"
    [ -f "$PLUG/CutsceneSkip.dll.disabled" ] && mv -v "$PLUG/CutsceneSkip.dll.disabled" "$PLUG/CutsceneSkip.dll" || cp -v "$ORIG_SRC" "$PLUG/CutsceneSkip.dll"
    echo "DLL original restaurada."
    ;;
  cfg)
    echo "${2:-E0}" > "$GAME/SkippableCutscenesRevival.cfg"
    echo "cfg -> $(cat "$GAME/SkippableCutscenesRevival.cfg")"
    ;;
  clean)
    rm -fv "$PLUG/SkippableCutscenesRevival.dll"
    # deixa o jogo SEM nenhum mod de cutscene (original continua .disabled)
    echo "Jogo SEM mod de cutscene (baseline). Original segue desativado."
    ;;
  log)
    [ -f "$LOG" ] && cat "$LOG" || echo "(sem log ainda — rode o jogo com a sonda instalada)"
    ;;
  *) echo "uso: $0 {probe|restore|log|cfg E0..E4}"; exit 2 ;;
esac
