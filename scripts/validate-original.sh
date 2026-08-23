#!/usr/bin/env bash
# Passo zero: valida a DLL original (v1.1) no build atual do MHW.
#   ./scripts/validate-original.sh stage   -> copia DLL + habilita log no loader-config.json
#   ./scripts/validate-original.sh check   -> confere loader instalado e resume o loader.log
#   ./scripts/validate-original.sh unstage -> remove a DLL original do jogo (mantém loader/config)
set -euo pipefail

GAME="${MHW_DIR:-/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World}"
HERE="$(cd "$(dirname "$0")/.." && pwd)"
DLL_SRC="$HERE/reference/original-mod/extracted/nativePC/plugins/CutsceneSkip.dll"
DLL_DST="$GAME/nativePC/plugins/CutsceneSkip.dll"
CFG="$GAME/loader-config.json"
LOG="$GAME/loader.log"

loader_ok() {
  local ok=1
  for f in dinput8.dll loader.dll; do
    if [ -f "$GAME/$f" ]; then echo "  [ok] $f"; else echo "  [FALTA] $f"; ok=0; fi
  done
  [ "$ok" = 1 ]
}

case "${1:-check}" in
  stage)
    echo "== Stracker's Loader em '$GAME':"
    loader_ok || { echo "!! Instale o Stracker's Loader (Nexus 1982) na raiz do jogo antes de continuar."; exit 1; }
    mkdir -p "$(dirname "$DLL_DST")"
    cp -v "$DLL_SRC" "$DLL_DST"
    python3 - "$CFG" <<'PY'
import json, sys, os
p = sys.argv[1]
cfg = json.load(open(p)) if os.path.exists(p) else {}
cfg.update({"logfile": True, "logcmd": True, "logLevel": "INFO", "enablePluginLoader": True})
json.dump(cfg, open(p, "w"), indent=2)
print("  loader-config.json ->", cfg)
PY
    [ -f "$LOG" ] && { mv -v "$LOG" "$LOG.$(date +%Y%m%d-%H%M%S).bak"; }
    echo "== Pronto. Inicie o jogo pela Steam, chegue ao menu/quest com cutscene e depois rode: $0 check"
    ;;
  check)
    echo "== Loader:"; loader_ok || true
    echo "== DLL original no jogo: $([ -f "$DLL_DST" ] && echo presente || echo ausente)"
    echo "== loader-config.json:"; [ -f "$CFG" ] && cat "$CFG" || echo "  (ausente)"
    echo "== loader.log:"
    if [ -f "$LOG" ]; then
      grep -nE 'Loading plugin|Failed to load|Cutscene Skip|candidate injection|Memory Allocation|Aborting|DONE|version|Version' "$LOG" | tail -40 || true
    else
      echo "  (ausente — habilite logfile no loader-config.json e rode o jogo)"
    fi
    ;;
  unstage)
    rm -fv "$DLL_DST"
    ;;
  *) echo "uso: $0 {stage|check|unstage}"; exit 2 ;;
esac
