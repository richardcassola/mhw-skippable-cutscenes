# Skippable Cutscenes: Revival (Monster Hunter: World)

Reimplementação em fonte própria do mod **Skippable Cutscenes** para MHW,
focada em pular cutscenes **sem desconectar da sessão online** (o mod original
avança o relógio da cena localmente, o que dessincroniza a sessão e derruba o
jogador — inclusive solo).

Estado: **em desenvolvimento** (etapa técnica / engenharia reversa em runtime).

## Créditos
Baseado no mod **Skippable Cutscenes** (Nexus 5540) — pesquisa e código de
injeção originais por **Moonbunnie**, publicado por **Tsulin / AsteriskAmpersand**
(equipe Iceborne Community Edition). Este projeto é uma implementação
independente, para uso pessoal, a partir da análise do comportamento do binário
original. O mod original tem permissões restritivas (sem redistribuição/
modificação); nada aqui redistribui os arquivos dele.

## Requisitos
- Monster Hunter: World (Steam), testado no build **15539686 (v15.23.00)**.
- **Stracker's Loader** (Nexus 1982) instalado na raiz do jogo.
- Visual C++ Redistributable x64 (2015–2022).

## Instalação
Copie `SkippableCutscenesRevival.dll` para
`<jogo>\nativePC\plugins\`. Instale apenas **um** mod de cutscene skip por vez
(não use junto com a DLL original nem com o pacote ICE).

## Build (WSL/Linux, cross-compile)
```
sudo apt install -y gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 cmake   # uma vez
./build.sh
```

## Configuração de experimento (dev)
Crie `SkippableCutscenesRevival.cfg` na raiz do jogo com um destes tokens na
1ª linha: `E0` (paridade), `E1`, `E2`, `E3`, `E4`. Sem arquivo = E0.
Ver `sdd/wip/001-skippable-cutscenes-revival/2-technical/spec.md`.

## Aviso
Mod que injeta código no processo do jogo. Use por sua conta e risco, offline
ou em sessões privadas enquanto o skip seguro para multiplayer não estiver
validado.
