# Skippable Cutscenes: Revival — Monster Hunter: World

Plugin que permite **pular as cutscenes que o jogo normalmente não deixa pular**
(as de história/introdução de monstro), com o objetivo de fazer isso **sem
derrubar você da sessão online** — que é o problema do mod original.

> ⚠️ **Status: experimental.** O comportamento sem desconexão ainda está sendo
> validado in-game. Use offline ou em sessão privada até isso ser confirmado.
> Mod que injeta código no processo do jogo — use por sua conta e risco.

---

## Instalação (5 minutos, sem compilar nada)

Você **não precisa compilar** nada: a DLL pronta é baixada na página de
releases. Compilar só interessa a quem for mexer no código.

### Antes de começar
- **Monster Hunter: World** no Steam (PC). Testado no build **15539686
  (v15.23.00)**.
- **Visual C++ Redistributable x64 (2015–2022)** — quase sempre já instalado.
- Saber onde o jogo está instalado. No Steam: *botão direito no jogo →
  Gerenciar → Ver arquivos locais*. Exemplo:
  `D:\SteamLibrary\steamapps\common\Monster Hunter World\`
  (a pasta que tem o `MonsterHunterWorld.exe`). Daqui em diante isso é
  **`<jogo>`**.

### Passo 1 — Instalar o Stracker's Loader
É o carregador de plugins do MHW; sem ele o mod não roda.

1. Baixe em https://www.nexusmods.com/monsterhunterworld/mods/1982 (exige login
   no Nexus).
2. Extraia o conteúdo **direto na pasta `<jogo>`**, ao lado do
   `MonsterHunterWorld.exe`. Devem aparecer três arquivos:

```
<jogo>\
├── MonsterHunterWorld.exe
├── dinput8.dll          <- Stracker's Loader
├── loader.dll           <- Stracker's Loader
└── loader-config.json   <- Stracker's Loader
```

3. Abra o `loader-config.json` no Bloco de Notas e confira que o carregador de
   plugins está ligado:

```json
{
  "logfile": true,
  "logcmd": false,
  "logLevel": "INFO",
  "outputEveryPath": false,
  "enablePluginLoader": true
}
```

### Passo 2 — Baixar a DLL do mod
Vá em **[Releases](../../releases/latest)** e baixe o arquivo:

```
SkippableCutscenesRevival.dll
```

É esse — e **só** esse — arquivo. Ele já vem compilado e não depende de mais
nada. (`dinput8.dll` e `loader.dll` são do Stracker's Loader, do passo 1.)

### Passo 3 — Copiar a DLL para a pasta de plugins
Coloque em `<jogo>\nativePC\plugins\`. Se essas pastas não existirem, crie:

```
<jogo>\nativePC\plugins\SkippableCutscenesRevival.dll
```

### Passo 4 — Criar o arquivo de configuração
Na pasta **`<jogo>`** (a raiz, *não* dentro de `plugins`), crie um arquivo de
texto chamado:

```
SkippableCutscenesRevival.cfg
```

com uma única linha, o modo de funcionamento:

```
E1
```

`E1` é o modo recomendado (pula usando o próprio mecanismo do jogo). Sem esse
arquivo o plugin roda em `E0`, que é só uma cópia do comportamento do mod
original — **esse desconecta**. Os outros modos estão descritos
[mais abaixo](#modos-e0e4).

> No Bloco de Notas, salve como `SkippableCutscenesRevival.cfg` com *Tipo:
> Todos os arquivos*, senão o Windows salva como `.cfg.txt` e o plugin ignora.

### Passo 5 — Um mod de cutscene por vez
Se você já usa o mod original do Nexus (ou o pacote Iceborne Community Edition),
**desative-o**: renomeie o arquivo dele em vez de apagar, assim dá para voltar.

```
<jogo>\nativePC\plugins\CutsceneSkip.dll   ->   CutsceneSkip.dll.disabled
```

### Pronto — é assim que deve ficar

```
<jogo>\
├── MonsterHunterWorld.exe
├── dinput8.dll
├── loader.dll
├── loader-config.json
├── SkippableCutscenesRevival.cfg          <- com "E1" dentro
└── nativePC\
    └── plugins\
        ├── SkippableCutscenesRevival.dll  <- o mod
        └── CutsceneSkip.dll.disabled      <- mod antigo, desativado (se existir)
```

---

## Como usar e conferir se funcionou

1. Inicie o jogo **pela Steam**. Uma janela de console preta do *Stracker's
   Loader* abre junto — é normal.
2. Chegue ao menu principal. Só isso já faz o plugin carregar.
3. Entre numa quest cuja cutscene o jogo normalmente não deixa pular. O prompt
   de skip deve aparecer — o mesmo do jogo, com o botão que ele indicar na tela.

Se quiser confirmar que o plugin carregou, feche o jogo e abra estes dois
arquivos de texto na pasta `<jogo>`:

**`loader.log`** — o carregador achou o plugin:
```
[ 13:44:51 ] Found config file
[ 13:44:52 ] Loading plugin "nativePC\\plugins\\SkippableCutscenesRevival.dll"
[ 13:44:52 ] DONE !
```

**`SkippableCutscenesRevival.log`** — o plugin se instalou:
```
[ 21:42:52 ] [INFO] SkippableCutscenesRevival. Alvo testado: 15.23.00 (build 15539686). Experimento: E1 (rota nativa via flag)
[ 21:42:52 ] [INFO] site 'execute_skip (P1)': 1 candidato @ 0000000141a83380 (ret_off=+0x29)
[ 21:42:52 ] [INFO] site 'show_prompt (P2)': 1 candidato @ 0000000141a85e16 (ret_off=+0x7)
[ 21:42:52 ] [INFO] E1 aplicado: +0x4D3=1 em P2 e P1 (toda cutscene via skip nativo).
[ 21:42:52 ] [INFO] DONE.
```

## Deu problema?

| Sintoma | O que é / o que fazer |
|---|---|
| Não abre a janela preta do loader | Stracker's Loader não instalado, ou você abriu o jogo por fora da Steam. Refaça o passo 1. |
| `loader.log` não menciona o plugin | A DLL não está em `<jogo>\nativePC\plugins\`, ou `enablePluginLoader` está `false`. |
| `Failed to load ...` no `loader.log` | Falta o Visual C++ Redistributable x64 (2015–2022). |
| `0 candidatos` no log do plugin | O jogo foi atualizado e as assinaturas mudaram — este build do mod não serve para a sua versão. Nada é injetado (o jogo continua normal). |
| `N candidatos (ambiguo)` no log | Mesma coisa: por segurança nada é injetado. |
| Pula a cena mas cai da sessão online | Você está no modo `E0`, ou o `.cfg` foi salvo como `.cfg.txt`. Confira o passo 4. |
| Quero jogar sem o mod | Ver *Desinstalar* abaixo. |

## Desinstalar

1. Apague `<jogo>\nativePC\plugins\SkippableCutscenesRevival.dll`.
2. Se tinha o mod antigo, renomeie `CutsceneSkip.dll.disabled` de volta para
   `CutsceneSkip.dll`.
3. Opcional: apague `SkippableCutscenesRevival.cfg` e os arquivos `.log` da
   pasta `<jogo>`.

O Stracker's Loader pode ficar — ele não altera nada sozinho.

## Modos (E0…E4)

O token da 1ª linha do `.cfg`. Para jogar, use **`E1`**; os outros existem para
o desenvolvimento.

| Modo | O que faz |
|---|---|
| `E0` | Paridade com o mod original (força o prompt e adianta o relógio da cena). É o baseline de comparação e **reproduz a desconexão** de propósito. |
| `E1` | **Recomendado.** Marca a cena como pulável (`+0x4D3 = 1`) e deixa o **skip nativo do jogo** terminá-la. Hipótese: sem desconexão e sem áudio tocando depois. |
| `E2` | Chama diretamente a função de término nativa da cutscene (versão completa do E1). |
| `E3` | Fast-forward: avança o tempo em passos por frame até ~1 s do fim, em vez de saltar. |
| `E4` | Falha fechada: não oferece o prompt quando há outros jogadores na sessão (salvaguarda, combinável com E1/E2). |

---

## Para desenvolvedores

### Compilar (só se for mexer no código)
Cross-compile de Linux/WSL para Windows:

```bash
sudo apt install -y gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 cmake   # uma vez
./build.sh                                                            # -> build/SkippableCutscenesRevival.dll
```

### Instalar/testar a partir do WSL
`scripts/deploy.sh` automatiza a instalação acima. O caminho do jogo vem da
variável `MHW_DIR` (padrão:
`/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World`).

```bash
./scripts/deploy.sh probe      # desativa a DLL antiga, instala a nossa, arquiva o log anterior
./scripts/deploy.sh cfg E1     # escreve o modo no .cfg da raiz do jogo
# (rodar o jogo)
./scripts/deploy.sh log        # mostra SkippableCutscenesRevival.log
./scripts/deploy.sh restore    # volta a DLL original e remove a nossa
./scripts/deploy.sh clean      # deixa o jogo sem nenhum mod de cutscene (baseline)
```

### Onde está o quê
- `src/` — plugin (scanner de assinaturas, code caves, modos E0…E4).
- `sdd/wip/001-skippable-cutscenes-revival/` — especificações; a técnica
  (`2-technical/spec.md`) traz offsets, pontos de injeção e critérios de cada
  experimento.
- `reference/` — material de análise (o binário do mod original **não** é
  versionado).

## Créditos
Baseado no mod **Skippable Cutscenes** (Nexus 5540) — pesquisa e código de
injeção originais por **Moonbunnie**, publicado por **Tsulin /
AsteriskAmpersand** (equipe Iceborne Community Edition). Este projeto é uma
implementação independente, em fonte própria, feita a partir da análise do
comportamento do binário original. O mod original tem permissões restritivas
(sem redistribuição/modificação); **nada aqui redistribui os arquivos dele**.
