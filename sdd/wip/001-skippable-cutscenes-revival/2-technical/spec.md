# Technical Specification: skippable-cutscenes-revival

- Status: draft (etapa técnica — sonda E0-probe compilada e instalada 2026-08-18)
- Data: 2026-08-18
- Base: `1-functional/spec.md` (v2), `reference/original-mod/ANALYSIS.md`,
  `2-technical/solution-options.md`
- Objetivo da etapa: reproduzir o skip em **fonte própria** e implementar a
  **Opção A** (rotear cenas não-puláveis pelo término nativo, seguro para a
  sessão), eliminando o disconnect e o áudio órfão.

## Estratégia central: "o plugin é o depurador"
O `MonsterHunterWorld.exe` tem Denuvo, que resiste a debuggers externos
(x64dbg/attach costuma ser bloqueado). Contorno: **nosso próprio plugin roda
dentro do processo** — ele faz os experimentos e loga o resultado, sem attach
externo. Cheat Engine entra só como apoio de exploração (achar o objeto da
cutscene, observar flags/tempo); o CE costuma funcionar no MHW.

Cada experimento é uma pequena variação do nosso plugin, testada in-game, com o
resultado lido do `loader.log`. Ciclo: editar → compilar (WSL) → copiar p/
`nativePC\plugins` → abrir o jogo → reproduzir → ler log.

## Arquitetura do plugin
- Alvo: DLL x64 Windows, `nativePC/plugins/SkippableCutscenesRevival.dll`.
- Carga: o loader instalado (Stracker/QuestLoader, jun/2024) só faz
  `LoadLibraryA` — **não** chama `onLoad` por nome. Portanto o trabalho é
  disparado no `DllMain(DLL_PROCESS_ATTACH)`, criando uma **thread** (evita
  loader-lock) que espera o módulo do jogo e injeta. Exportamos também
  `onLoad()` por compatibilidade, com **guarda de execução única** para não
  injetar duas vezes caso um loader mais novo o chame. (OQ-T1: confirmar qual
  dispara — instrumentar ambos com log.)
- Sem dependência de `loader.dll`: logging próprio em arquivo
  (`SkippableCutscenesRevival.log` na raiz do jogo), para não depender do ABI
  MSVC do `loader::LOG` (compilamos com MinGW).
- Componentes:
  - `scanmem` — AOB scan com máscara (reimplementação própria; algoritmo
    idêntico ao do Stracker MIT em `reference/strackers-loader/`).
  - `sig` — tabela de assinaturas e offsets (nossa RE), isolada em um header,
    **anotada com a versão do jogo** (15.23.00 / build 15539686).
  - `hook` — instalação de trampolim/code-cave (mesma técnica do original) OU
    MinHook (já vendorizado no repo do Stracker) — decisão em OQ-T2.
  - `experiment` — comportamento selecionável (E0..E4) por um arquivo de config
    simples, para iterar sem recompilar tudo.

## Assinaturas e offsets (da RE — validar em runtime)
Da `ANALYSIS.md` (build 15539686). São ponto de partida; confirmar endereço/
unicidade pelo log na primeira execução.
- Estrutura da cutscene:
  - `+0x4D1`, `+0x4D3` (byte): flags de "pulável". `+0x4D3 != 0` → o jogo já
    trata como nativamente pulável.
  - `+0x4F0` (byte): estado que bloqueia o skip.
  - `+0x38` → objeto de timeline; `+0x178` (float) tempo atual; `+0x6C` (int)
    duração (frames).
- Sites (AOB, texto em `ANALYSIS.md`):
  - P1 = rotina "executar skip" (onde o original faz o seek para cenas
    não-puláveis; cenas puláveis caem no código nativo logo após).
  - P2 = teste por frame "mostrar prompt?".

## Escada de experimentos (E0 → E4)
Cada um é um build; o objetivo é achar o menor que resolva.

- **E0 — Paridade (baseline).** Reproduz o original (força prompt + seek). Deve
  reproduzir o skip **e** o disconnect. Serve para validar scanner/cave e ter
  base de comparação. Critério: comportamento == v1.1.
- **E1 — Rota nativa via flag (Opção A, atalho).** Ao acionar o skip numa cena
  não-pulável, **setar `+0x4D3 = 1`** e deixar o caminho nativo do jogo pular.
  Hipótese: some o disconnect e o áudio órfão (mesma raiz). Critério: skip
  funciona e **não** desconecta (testar solo — se solo não cai, é a solução).
  Se o jogo ignorar o skip nativo em cena de história ou crashar → E2.
- **E2 — Chamar o término nativo (Opção A, completo).** Identificar a função
  que a cena nativamente pulável chama ao ser pulada (logo após o ponto de
  decisão em P1) e **chamá-la diretamente** para qualquer cena, com os
  argumentos certos. Critério igual ao E1. Exige mapear a função em runtime
  (Cheat Engine + log do nosso plugin).
- **E3 — Fast-forward (Opção B, fallback).** Em vez de saltar, avançar o tempo
  em passos por frame (hook no site por-frame) até ~1 s do fim. Hipótese:
  percorrer os estados mantém sessão/áudio em sincronia. Critério: sem
  disconnect; skip vira ~1–2 s.
- **E4 — Falha fechada online (Opção C, rede de segurança).** Detectar
  "conectado a sessão com outros" e não oferecer o prompt nesse caso. Requer
  achar a flag/estado de sessão (OQ-T3). Combinável com E1/E2 como salvaguarda.

## Toolchain (decisão)
- **MinGW-w64 no WSL** (recomendado): `gcc-mingw-w64-x86-64` +
  `g++-mingw-w64-x86-64` (candidato apt 13.2.0). Build cruzado a partir do WSL,
  ciclo rápido, sem depender de Visual Studio. Não linka o `loader::LOG` (MSVC)
  — por isso logging próprio.
  - Bloqueio atual: `sudo` exige senha → **o usuário instala** (uma vez):
    `sudo apt install -y gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64`
    (dica: rodar via prefixo `!` nesta sessão).
- Alternativa: MSVC Build Tools no Windows (não detectado na máquina; mais
  pesado; só se o MinGW esbarrar em algo do ABI). Fica como plano B.
- Build system: CMake + toolchain file MinGW; saída direto para um `build/`.

## Protocolo de teste (por experimento)
1. `cmake --build` (WSL) → DLL.
2. Copiar para `nativePC\plugins\` (remover a DLL original antes, para não ter
   dois mecanismos — ver EC-2.2).
3. Abrir o jogo, ler `SkippableCutscenesRevival.log`: assinaturas achadas
   (endereço, nº de candidatos), injeção aplicada.
4. Entrar em cena de história não-pulável, acionar skip.
5. Registrar: pulou? desconectou (missão continua offline)? áudio órfão? crash?
6. Comparar com E0. Anotar na tabela de `4-implementation/`.

## Riscos técnicos
- RT-1: MinGW gerar DLL que o loader não carrega (CRT/ABI). Mitig.: DLL
  mínima, sem dependências além de kernel32; testar carga (E0) antes de tudo.
- RT-2: Denuvo detectar a escrita no `.text` (code-cave/patch) como tamper.
  Baixa probabilidade (o original faz isso e roda), mas a considerar se houver
  crash na injeção.
- RT-3: E2 exigir engenharia reversa de assinatura de função não trivial
  (parâmetros/registradores) — mais demorado; E1 pode tornar E2 desnecessário.
- RT-4: A flag de "sessão online com outros" (E4) pode ser difícil de achar.

## Open Questions (técnicas)
- OQ-T1: `DllMain` vs `onLoad` — qual o loader instalado dispara? (instrumentar)
- OQ-T2: code-cave manual (como o original) vs MinHook — qual usar? (MinHook é
  mais robusto e já é MIT/vendorizável.)
- OQ-T3: onde está o estado "conectado a sessão com N jogadores"? (para E4)
- OQ-T4: E1 basta, ou o jogo bloqueia o skip nativo em cenas de história?
  (o experimento responde)
