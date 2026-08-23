# Feature: skippable-cutscenes-revival

## Metadata
- **Number**: 001
- **Created**: 2026-08-18
- **Created By**: Richard Lemos <rlemos@f3capital.com.br>
- **Mode**: standard
- **Current Stage**: technical

## Project Type
- **Type**: mvp

Quality gates: build + lint/format obrigatórios. Testes unitários apenas no que
é testável fora do processo do jogo (ex.: pattern scanner, leitura de config,
mapeamento de teclas). Hooks nativos e comportamento in-game são validados
manualmente com o jogo rodando (checklist de smoke test na etapa de tasks).
Docker: não se aplica ao alvo (DLL Windows); opcional apenas como container de
cross-compile — decisão na etapa técnica.

## Repositories & Branches
- **/home/rlemos/mhw-skippable-cutscenes** — work: `main` — base: `main` — target: `MHW Steam build 15539686` (`/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World`)

Repositório criado localmente com `git init -b main` (aprovado pelo usuário em
2026-08-18). Sem remote, sem push.

## Contexto de origem
Continuidade do mod **Skippable Cutscenes** (Nexus Mods, Monster Hunter: World,
ID 5540 — https://www.nexusmods.com/monsterhunterworld/mods/5540), que parou de
funcionar após updates do jogo.

O que se sabe do mod original (fontes públicas; a página do Nexus está atrás de
Cloudflare e não pôde ser lida diretamente):
- Autor: **Tsulin**; publicado ~2023-04; última versão **1.1** (arquivo do
  Nexus datado de 2025-02-12; mirrors citam 2024-03, data não confiável).
- Créditos a **Moonbunnie** pela pesquisa do cutscene skip e pelo "improved
  injection code".
- Premissa: **pular cutscenes com o pressionar de um botão**.
- Distribuição: pasta `nativePC/` → plugin do **Stracker's Loader**
  (`nativePC/plugins/*.dll`), código nativo injetado no processo do jogo.
  É por isso que quebra em updates do `MonsterHunterWorld.exe` (assinaturas /
  offsets mudam).
- Issues conhecidas do original: a quest *"The Best Kind of Quest"* não pode ser
  pulada (trava o carrinho do pesquisador); pular a cutscene **não** libera o
  SOS flare automaticamente em MP.
- **Nenhum código-fonte público encontrado.** A DLL foi compilada no workspace
  do ICE (Iceborne Community Edition); os repos do ICE só distribuem o binário.
  Reaproveitamento = engenharia reversa da DLL (feita em parte, ver
  `reference/original-mod/ANALYSIS.md`).

## Achados da análise da DLL (2026-08-18)
- Plugin do Stracker's Loader: export `onLoad()`, log via `loader.dll`.
- **AOB scan + code cave**: 2 assinaturas de busca (P1, P2) e 2 payloads
  (P3, P4), em texto puro na DLL. Sem hook de input: o mod força o **prompt de
  skip nativo** do jogo em toda cutscene (exceto nos últimos ~6,4 s) e, ao
  apertar, faz **seek do timeline para ~1 s antes do fim** (não aborta), para
  que os eventos de fim de cutscene ainda disparem.
- Depende de offsets de struct (`0x4D1/0x4D3/0x4F0`, `0x38→0x178`, `0x6C`).
- `MonsterHunterWorld.exe` tem **Denuvo** (`.text` entropia 8.0): assinaturas
  só verificáveis com o jogo rodando; scan em disco é inútil.
- **Premissa a validar**: última versão do jogo é 15.23.00 (2024-10-07, build
  `15539686` = a instalada); a v1.1 do mod é de 2025-02-12, *posterior*. Pode
  ser que a v1.1 funcione hoje com Stracker's Loader compatível — a instalação
  atual não tem loader nenhum. **Passo zero**: instalar loader + DLL e ler o
  log antes de qualquer decisão técnica.

## Decisões do usuário (registradas em 2026-08-18)
| Tema | Decisão |
|------|---------|
| Diretório do projeto | `/home/rlemos/mhw-skippable-cutscenes` (projeto dedicado) |
| Tipo de projeto | MVP |
| Arquivos do mod original | Zip recebido em 2026-08-18 (`Cutscene Skip 1.1-5540-1-1-1739388623.zip`), copiado para `reference/original-mod/` (ignorado pelo git). Análise estática em `reference/original-mod/ANALYSIS.md` |
| Reaproveitar vs. reescrever | **Em aberto** — decidir na etapa técnica, após o passo zero (validar a v1.1 in-game com Stracker's Loader) |
| Git | `git init -b main` local, sem remote |
| Escopo v1 (spec funcional) | **1:1 com o original** (prompt nativo em toda cutscene + skip para o fim). Sem melhorias; falha segura e log diagnóstico entram por padrão. Melhorias vão para o backlog |
| Distribuição | **Uso pessoal** por enquanto; créditos a Tsulin/Moonbunnie no README; sem Nexus |
| Passo zero | Executado em 2026-08-18: a v1.1 **carrega e o skip funciona** no build 15539686 com Stracker's Loader (jun/2024); log em `1-functional/step-zero-loader.log`. **Porém o usuário relata queda de conexão na missão (multiplayer)** — compatível com bugs reportados no Nexus para a v1.1 (dessincronia MP, áudio órfão, crashes). Ver `ANALYSIS.md` § "Estado real no Nexus" |
| Problema real (confirmado 2026-08-18) | O skip é um **seek local** do timeline; o servidor de sessão espera a duração cheia e **desconecta** o cliente (sintoma do usuário: solo, a queda veio após a cutscene, "missão continuou mas eu fiquei offline, ninguém se juntaria"). Mesma dessincronia do bug de MP do Nexus. Não é crash nem quest-fail |
| Rumo escolhido (2026-08-18) | **Opção A** (conserto correto: rotear cenas de história pelo caminho de término nativo, seguro para a sessão), começando pelo experimento mais barato. DLL original permanece instalada no jogo enquanto desenvolvemos. Ver `2-technical/spec.md` |

## Ambiente detectado
- MHW instalado em `/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World`
  (Steam `appmanifest_582010.acf`: `buildid 15539686`).
- Instalação **limpa**: sem `nativePC/`, sem Stracker's Loader (`dinput8.dll`),
  sem plugins.
- Desenvolvimento em **WSL2** (Linux). O alvo é uma DLL x64 Windows: exige
  cross-compile (mingw-w64 / clang) ou toolchain MSVC no Windows — decisão na
  etapa técnica.

## Riscos conhecidos (a detalhar na spec funcional)
- R-01: Stracker's Loader pode não estar atualizado para o build atual do jogo;
  sem loader, nenhum plugin nativo carrega.
- R-02: Assinaturas/offsets do skip mudam a cada update — o design precisa
  prever pattern scanning e/ou tabela por versão para não quebrar de novo.
- R-03: (mitigado) DLL original obtida; sem fonte, mas mecanismo reconstruído.
- R-04: Denuvo impede validar assinaturas offline — todo ciclo de teste exige
  abrir o jogo.
- R-05: (revisado) A v1.1 carrega, mas o skip por *seek* tem efeitos
  colaterais (MP, áudio, crashes). O escopo passa a incluir **diagnóstico
  in-game** do problema de MP e um mecanismo de skip mais correto — isso exige
  engenharia reversa em tempo de execução (jogo rodando), iterativa.
- R-06: Testes de multiplayer exigem segundo jogador/conta e coordenação; cada
  iteração custa abrir o jogo e reproduzir o cenário.
- R-07: Permissões do mod original no Nexus são restritivas (sem modificação/
  redistribuição): resultado é implementação própria, uso pessoal.

## Stages
- functional: { status: approved-implicit, note: problema confirmado pelo sintoma do usuário; spec v2 reflete US-6 }
- technical: { status: in-progress, draft: 2-technical/spec.md }
- technical: { status: pending }
- tasks: { status: pending }
- implementation: { status: pending }

## Rollback History
<!-- Populated by /sdd.rollback -->
