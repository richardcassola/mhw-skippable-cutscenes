# Análise do mod original — Skippable Cutscenes v1.1 (Nexus 5540)

Data da análise: 2026-08-18. Análise estática (strings, imports, desmontagem
parcial com `objdump`). **Não executado in-game ainda.**

## Arquivo
- Zip: `Cutscene Skip 1.1-5540-1-1-1739388623.zip` (Nexus timestamp
  `1739388623` = 2025-02-12 20:50 UTC).
- Conteúdo: **um único arquivo** `nativePC/plugins/CutsceneSkip.dll`
  (PE32+ x64, 79.872 bytes, 6 seções; DLL compilada em 2025-02-12 20:28).
- PDB path embutido:
  `D:\Games SSD\ICE Workspace\Dev\iceborne-community-edition\iceborne-community-edition\Code\ICE Plugins Source\CutsceneSkip\x64\Release\CutsceneSkip.pdb`
  → compilada por Tsulin dentro do workspace do **ICE (Iceborne Community
  Edition)**. Os repositórios públicos do ICE (github.com/AsteriskAmpersand/
  Ice-Stable e Ice-Experimental) distribuem apenas a DLL compilada
  (`ICE/icecode/CutsceneSkip.dll`, build de 2023-10 por MoonBunnie, PDB
  `C:\Users\MoonBunnie\source\repos\CutsceneSkip\...`). **Sem fonte pública.**
- Cópia da build do ICE (2023) em `ice-nov2024/CutsceneSkip.dll` para
  comparação: mesmas imports; assinaturas ofuscadas (blob), mensagens extras
  (`found no base address`, `insufficient candidate injection points`,
  `Invalid injection parameters`, `Profiling initiated!`).

## Tipo: plugin do Stracker's Loader
- Export único: `?onLoad@@YAXXZ` → `void onLoad()` (entry point de plugin do
  Stracker's Loader).
- Imports de `loader.dll`: `loader::LOG::LOG(LogLevel)`, `loader::LOG::~LOG`,
  `loader::MinLogLevel` → logging via `loader.h` do Stracker.
- Imports relevantes de `KERNEL32`: `GetModuleHandleA`,
  `K32GetModuleInformation`, `GetSystemInfo`, `VirtualQuery`, `VirtualAlloc`,
  `VirtualProtect`, `GetCurrentProcess`. Resto é CRT/MSVC (`MSVCP140`,
  `VCRUNTIME140[_1]`, `api-ms-win-crt-*`) → precisa do **VC++ 2015-2022 x64
  Redistributable**.
- **Não importa nenhuma API de input** (sem `GetAsyncKeyState`, sem XInput,
  sem DirectInput). O "botão" é o prompt de skip nativo do jogo.

## Mecanismo (reconstruído — alta confiança, validar in-game)
Estratégia: **AOB scan + code cave**. Em `onLoad()`:
1. `GetModuleHandleA("MonsterHunterWorld.exe")` + `K32GetModuleInformation`
   → base e tamanho do módulo (mensagens de erro: `Failed to Acquire Module`,
   `Module retrieval error.`).
2. Para cada assinatura de busca (`Mask and Search String have different
   sizes.` indica scanner com máscara), varre o módulo → log
   `found N candidate injection points.` ou
   `found no candidate injection points - Aborting.`
3. `VirtualQuery`/`GetSystemInfo` para achar região livre próxima (jmp rel32
   precisa de ±2 GB), `VirtualAlloc` (log `Succesful Memory Allocation`),
   escreve o payload + trampolim, `VirtualProtect` no site original e grava o
   `jmp` para a cave.
4. Logs de ciclo: `Cutscene Skip Loading...` / `Loaded!` / `DONE !`.

Globais construídos em static-init (desmontagem em `0x180001020`–`0x1800011cf`):
- `buscas      = { P1, P2 }`   (`std::vector<std::string>` @ `0x180014a68`)
- `payloads    = { P3, P4 }`   (`std::vector<std::string>` @ `0x180014a98`)
- `retOffsets  = { 0x29, 0x07 }` (`std::vector<int>` @ `0x180014a80`) — offset,
  relativo ao site, para onde a cave salta de volta
- `siteOffsets = { 0, 0 }` (array const @ `0x18000fac8`) — site = hit + offset
- nome `"Cutscene Skip"` (`std::string` @ `0x180014a48`)

Fluxo confirmado (`DllMain` → `onLoad()` @ `0x180003370` → injetor @
`0x180002ad0`): para cada `i`, converte `buscas[i]` em bytes, varre o módulo,
exige ≥ 1 hit (senão `found no candidate injection points - Aborting.`), usa o
**primeiro** hit, converte `payloads[i]`, aloca cave, grava payload + salto de
volta para `site + retOffsets[i]`, sobrescreve o site com `jmp cave`.
- Injeção 0: site = P1; volta para `site+0x29` — o corpo do skip nativo,
  já depois das checagens que P3 refaz por conta própria.
- Injeção 1: site = P2; volta para `site+0x07` = **o `jne` original**: P4 só
  recalcula o ZF (`test eax,eax`), o `jne` do jogo decide.

### Assinaturas (texto puro na DLL v1.1)
```
P1 (busca)   80 B9 D3040000 00 74 33 80 B9 F0040000 00 75 2A
P2 (busca)   40 38 B3 D3040000 75 0D 40 38 B3 D1040000 0F84 C7020000 48 8B CB E8
P3 (payload) 80 B9 F0040000 00 75 3C 48 8B 41 38 F3 48 0F2C 90 78010000 8B 41 6C 83 E8 3C
             39 C2 7F 25 80 B9 D3040000 00 75 1D 66 48 0F7E C2 52 F3 0F2A C0 48 8B 41 38
             66 0F7E 80 78010000 5A 66 48 0F6E C2 C3
P4 (payload) 48 8B 43 38 F3 0F2C 88 78010000 8B 43 6C 2D 80010000 39 C1 7C 02 31 C0 85 C0
```

### Estrutura do objeto de cutscene (offsets inferidos das assinaturas)
| Offset | Uso inferido |
|--------|--------------|
| `+0x4D1` | flag (byte) — junto com `+0x4D3` decide se o prompt de skip aparece |
| `+0x4D3` | flag **skippable** (byte) — cutscene nativamente pulável |
| `+0x4F0` | flag (byte) — estado que bloqueia o skip (ex.: já pulando / não tocando) |
| `+0x38`  | ponteiro para o timeline/player |
| `+0x38 → +0x178` | **tempo atual** (float, em frames) |
| `+0x6C`  | **duração** total (int, frames) |

### P1 (site) → P3 (payload): execução do skip
P1 é o início de uma rotina do jogo do tipo "executar skip":
`if (!this->skippable) goto A; if (this->f4F0) goto B; …`
P3, decodificado:
```
cmp byte [rcx+4F0h],0 ; jne → ret            ; bloqueado → nada
mov rax,[rcx+38h]
cvttss2si rdx,[rax+178h]                      ; cur = (int)tempoAtual
mov eax,[rcx+6Ch] ; sub eax,60                ; fim = duração - 60 frames (~1 s)
cmp edx,eax ; jg → ret                        ; já perto do fim → nada
cmp byte [rcx+4D3h],0 ; jne → (cai após o ret = trampolim → código original)
                                              ; nativamente skippable → skip nativo
movq rdx,xmm0 ; push rdx
cvtsi2ss xmm0,eax
mov rax,[rcx+38h] ; movd [rax+178h],xmm0      ; tempoAtual = duração - 60
pop rdx ; movq xmm0,rdx
ret
```
→ **O "skip" é um *seek* para 1 s antes do fim**, não um abort. Assim os
eventos de fim de cutscene (flags de progresso, spawns, transição) continuam
disparando. Efeito colateral: eventos scriptados no meio da cutscene são
pulados — provável causa do bug conhecido em *"The Best Kind of Quest"*
(carrinho do pesquisador trava).

### P2 (site) → P4 (payload): exibição/aceitação do prompt de skip
P2 é o teste por frame do jogo: `if (!skippable && !f4D1) goto longe (sem
prompt); else { call f(this); … }`. P4:
```
mov rax,[rbx+38h] ; cvttss2si ecx,[rax+178h]  ; cur
mov eax,[rbx+6Ch] ; sub eax,384               ; duração - 384 frames (~6,4 s)
cmp ecx,eax ; jl +2 ; xor eax,eax ; test eax,eax
```
→ substitui o teste "é skippable?" por "**ainda não está nos últimos ~6,4 s**".
Toda cutscene passa a mostrar o prompt de skip, exceto no fim.

## Por que quebra em updates
Depende de (a) 2 sequências exatas de bytes do `.text` do jogo e (b) offsets
de struct (`0x4D1/0x4D3/0x4F0/0x38/0x178/0x6C`). Qualquer recompilação do exe
que altere codegen nesses pontos zera os hits (`found no candidate injection
points - Aborting.`) ou, pior, acerta em lugar errado.

## Denuvo — restrição de verificação
`MonsterHunterWorld.exe` (build Steam `15539686`, linkado 2024-08-30):
seção `.text` com entropia **8.000** e entry point na seção `.bind` →
**Denuvo**. O código só existe descriptografado em memória: scan das
assinaturas **em disco é inútil** (0 hits até para fragmentos genéricos).
Verificação de assinaturas exige o jogo em execução (log do plugin/loader ou
dump de memória).

## Estado da premissa "parou de funcionar" (a validar)
- Última versão do MHW Steam: **15.23.00 (2024-10-07)** = build `15539686`
  = a instalada. Nenhum update desde então (steamcmd.net, public branch).
- A v1.1 do mod é de **2025-02-12**, posterior ao último update do jogo, e usa
  AOB scan (não endereços fixos). É plausível que **funcione hoje** com um
  Stracker's Loader compatível com 15.23 — o que a instalação atual não tem
  (sem `dinput8.dll`/`loader.dll`, sem `nativePC/`).
- **Passo zero do projeto**: instalar Stracker's Loader (compatível 15.23) +
  esta DLL, rodar o jogo, ler o log. O resultado define se partimos de
  "reproduzir e refatorar" ou de "re-descobrir assinaturas".

## Estado real no Nexus (lido em 2026-08-18 via proxy de leitura)
Página: criado por **Moonbunnie**, upload por **AsteriskAmpersand** (equipe
ICE); upload original 2021-08-06; última atualização **2025-02-12 (v1.1)**;
requisito: Stracker's Loader. **Permissões restritivas**: sem re-upload, sem
modificação e sem uso de assets sem autorização do autor → qualquer resultado
deste projeto é de uso pessoal, implementação própria.

Sticky do autor: se não abre a "janelinha preta" do Stracker no boot, o loader
está mal instalado (VCRedist incluso) — a maioria dos "not working" é isso.

Problemas reportados na v1.1 (2025-02 → 2026-08), por frequência/impacto:
1. **Áudio continua tocando depois de pular** (vozes/SFX da cutscene sobre o
   gameplay) — bug com 5 respostas (set/2025) + posts jan/mar/ago 2026.
   Workaround sugerido pela comunidade: usar a v1.0. Coerente com o mecanismo
   de *seek*: o timeline avança, mas as vozes já disparadas não são paradas.
2. **Multiplayer**: "se você pula antes dos outros, eles ficam presos
   assistindo a cutscene inteira" (bug 2026-08-08, v1.1) → dessincronia do
   estado de cutscene entre host e convidados. Relato do usuário deste
   projeto: **queda de conexão na missão** — compatível com o mesmo problema.
3. **Crashes**: pular cedo no Fatalis (out/2025); crash ao apertar X/Options
   em cutscenes **nativamente puláveis** (Safi'jiva) — workaround: apertar
   botão direito do mouse antes do X (jan/2026); crash fora de fullscreen
   (jan/2026); "1.0 tem crashes aleatórios".
4. *"The Best Kind of Quest"*: v1.1 passou a permitir pular a intro → física
   do carrinho quebra e softlock na parte da Rathian (fev/2025).
5. **Controle**: sem prompt/botão no DualSense; só ESC no teclado funciona
   (jul/2026); não funciona no Steam Deck (abr/2026).
6. Compatibilidade com SharpPluginLoader questionada (dez/2024).

Conclusão: a v1.1 **carrega e o skip funciona** no build 15539686, mas o
mecanismo (seek local do timeline) tem efeitos colaterais reais: áudio órfão,
dessincronia/queda em MP, crashes em cutscenes nativamente puláveis. O
problema a resolver não é "fazer carregar", é **fazer o skip corretamente**.

## Diagnóstico 2026-08-18 (usuário: "estava solo e caí")
- `loader.log` da sessão: plugin carregou e injetou sem erro (Loading… →
  Loaded! → DONE!). A queda **não** é falha de carga do mod.
- Processo `MonsterHunterWorld.exe` estava **rodando** ao checar (PID 30964,
  início 20:16) → não foi crash duro do executável (ou o usuário reabriu).
- Event Log (Windows) **sem** `Application Error`/`Hang` do MHW nas últimas
  24h. Há vários `Windows Error Reporting` **LiveKernelEvent 117** (TDR de GPU
  — driver de vídeo reiniciou) recorrentes ao longo do dia (09:08, 13:03,
  15:40, 15:51, 17:37), **anteriores** à sessão do jogo (20:16). Provável
  problema de driver/GPU independente do mod; não coincide com a queda.
  GPU: RTX 3050, driver 32.0.16.1047 (18/05/2026).
- Ponto-chave: **no MHW não existe "solo offline" de verdade** para o jogo
  principal — quests solo rodam dentro de uma *sessão online* conectada aos
  servidores da Capcom. "Cair" solo é quase sempre um **disconnect de sessão**
  (código estilo `50382-MW1`), não perda de partida P2P. Hipótese: o *seek*
  local do timeline (P1/P3) diverge do estado que o servidor espera para a
  cutscene → o servidor derruba a sessão. Isso explica cair **mesmo solo** e
  bate com o bug de MP ("outros ficam presos") — é a mesma dessincronia, vista
  do outro lado.
- A validar no A/B (agora trivial, sem amigos): reproduzir a queda solo COM e
  SEM o mod, anotando o **código de erro** e o momento (ao pular vs. durante).

## Sintoma confirmado (usuário, 2026-08-18)
"A missão continuou, mas eu estava offline. Então ninguém se juntaria." A queda
veio **depois** da cutscene, já em gameplay. Não é crash nem quest failure: é
**disconnect da sessão online** (communication error). A quest, por ser local,
segue; mas o cliente foi rebaixado a sessão privada/offline → SOS não traz
ninguém.

Causa (hipótese de alta confiança, casando mecanismo + sintoma + bug de MP do
Nexus): o skip é um **seek local** do timeline da cutscene (P1/P3 avançam
`tempoAtual` para `duração-60`). O servidor/estado de sessão espera que a
cutscene dure o tempo cheio; o cliente sai dela vários segundos antes. Essa
divergência de estado dispara a proteção de sincronismo da sessão e o servidor
derruba o cliente para offline — o mesmo evento que, visto do outro lado, deixa
"os outros presos assistindo a cutscene inteira" (bug 2026-08-08 no Nexus).
Não foi validado por A/B ainda (usuário pediu só explicação).

Implicação para o design: um skip "correto" não pode ser um seek local
silencioso. Opções a investigar na fase técnica (jogo rodando):
1. Chamar a **rotina nativa de término/skip** da cutscene em vez de mexer no
   relógio — se ela existir e sincronizar com a sessão (é o que as cutscenes
   nativamente puláveis já fazem sem derrubar).
2. Sinalizar o fim ao subsistema de sessão junto com o seek.
3. **Falha fechada**: não oferecer skip enquanto conectado a uma sessão que
   possa derrubar (menos desejável — mata o uso em grupo).

## Insight decisivo (2026-08-18): o bug está só no caminho do seek
Reler P3: `cmp byte [rcx+4D3h],0 ; jne -> código original do jogo`. Cenas
nativamente puláveis (+0x4D3 != 0) já caem no **skip nativo**, que NÃO derruba
a sessão nem deixa áudio órfão. O seek manual só roda em cenas não-puláveis.
Portanto disconnect + áudio órfão têm a **mesma raiz** (seek que não notifica
sessão/áudio) e o jogo **já possui** uma rotina de término segura. Conserto
correto = rotear cenas não-puláveis pelo caminho nativo (opção A em
`2-technical/solution-options.md`).
