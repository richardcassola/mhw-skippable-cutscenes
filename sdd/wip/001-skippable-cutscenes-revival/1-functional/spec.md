# Functional Specification: skippable-cutscenes-revival

- Feature: 001-skippable-cutscenes-revival
- Status: draft **v2** (revisada após passo zero; aguardando diagnóstico A/B + aprovação)
- Data: 2026-08-18
- Referências: `meta.md`, `reference/original-mod/ANALYSIS.md`,
  `1-functional/validation-step-zero.md`

## Problem Statement

O mod **Skippable Cutscenes** (Nexus 5540, v1.1) permite pular qualquer
cutscene do Monster Hunter: World com o botão de skip que o próprio jogo já
usa. No build atual (15.23.00) ele **carrega e o skip acontece** (validado no
passo zero), mas o jeito como pula — avançar o relógio da cutscene localmente
até perto do fim — tem efeitos colaterais reais, confirmados pela comunidade
no Nexus e vividos pelo usuário:

- **Multiplayer**: quem pula fica dessincronizado de quem não pulou; relatos
  de "os outros ficam presos na cutscene inteira" e, no caso do usuário,
  **queda de conexão na missão**.
- **Áudio órfão**: vozes/efeitos da cutscene continuam tocando por cima do
  gameplay depois do skip.
- **Crashes** ao pular cutscenes que o jogo já permitia pular (Safi'jiva,
  Fatalis) e softlock em *"The Best Kind of Quest"*.

O mod é fonte fechada, com permissões que não permitem modificá-lo, e sem
atualização desde fev/2025. O usuário quer **entender o que está acontecendo
e ter um skip que funcione direito** — em especial sem derrubar sessões
multiplayer — em código próprio, versionado, que falhe de forma segura quando
o jogo mudar e diga por que falhou.

Ordem de prioridade da v1: (1) diagnosticar e corrigir o problema de
multiplayer; (2) manter a paridade de skip do original onde ele funciona;
(3) o resto (áudio, crashes em cutscenes nativas) entra se o diagnóstico
mostrar causa comum ou custo baixo — senão vai para o backlog.

## Comportamento de referência (mod original v1.1)

Reconstruído por análise do binário (detalhes em `ANALYSIS.md`). É o contrato
que a v1 deve reproduzir:

1. Durante **qualquer cutscene in-game**, o jogo passa a exibir o **prompt de
   skip nativo** (o mesmo que aparece nas poucas cutscenes que já são
   puláveis), **exceto nos últimos ~6 segundos** da cutscene.
2. Ao acionar o skip, a cutscene **avança para ~1 segundo antes do fim** e
   termina naturalmente. Não é um "abortar": os eventos de fim da cutscene
   (progresso de história, aparição do monstro, transição para gameplay)
   ocorrem como se ela tivesse sido assistida inteira.
3. Cutscenes que **já eram nativamente puláveis** continuam usando o skip
   nativo do jogo, sem alteração.
4. Não há tecla nova, configuração, interface ou arquivo extra: o único
   artefato é o mod dentro da pasta de mods do jogo.
5. Efeitos conhecidos do original (Nexus, 2025–2026): dessincronia/queda em
   multiplayer; áudio da cutscene continua após o skip; crash ao pular
   cutscenes nativamente puláveis (Safi'jiva/Fatalis) sem antes fechar o
   prompt; softlock em *"The Best Kind of Quest"*; sem prompt no DualSense
   (só ESC no teclado). A v1 **não** aceita o primeiro item; os demais são
   tratados conforme prioridade do problem statement.

## User Stories

### US-1: Pular cutscene com o botão nativo
**Como** jogador, **quero** ver o prompt de skip em qualquer cutscene e
pulá-la com o botão que o jogo já usa, **para** não reassistir cenas em
quests repetidas, novos personagens ou sessões com amigos.

#### Acceptance Criteria
- AC-1.1: Em uma cutscene que o jogo **não** permite pular, o prompt de skip
  aparece e permanece visível até ~6 s antes do fim.
- AC-1.2: Acionar o skip (teclado ou controle, sem remapeamento) faz a
  cutscene terminar em ~1 s e o jogo prosseguir com o **mesmo estado** que
  teria após assisti-la inteira (mesma quest, mesmo monstro em campo, mesmo
  progresso de história, sem tela presa/preta).
- AC-1.3: Em uma cutscene que o jogo **já** permite pular, o comportamento é
  idêntico ao jogo sem o mod.
- AC-1.4: Nos últimos ~6 s da cutscene, o prompt não aparece e acionar o botão
  não tem efeito.
- AC-1.5: O comportamento é o mesmo em quests solo, quests de história
  (assigned) e quests opcionais/eventos com cutscene.

#### Edge Cases
- EC-1.1: Acionar o skip várias vezes seguidas → efeito único; sem travar,
  sem repetir a cena, sem crash.
- EC-1.2: Acionar o skip no primeiro frame da cutscene → funciona (não exige
  tempo mínimo assistido).
- EC-1.3: Cutscene em sessão multiplayer → ver US-6 (não é best-effort).
- EC-1.4: *"The Best Kind of Quest"* → comportamento conhecido do original
  (pode travar o carrinho). Documentado no README como limitação; não é
  critério de aceite.
- EC-1.5: Cutscenes que não passam pelo mesmo mecanismo interno (ex.: vídeos
  pré-renderizados, abertura do jogo) podem não exibir prompt → aceitável;
  a lista real do que é/não é coberto sai do teste in-game (ver OQ-2).

### US-2: Falha segura quando o jogo muda
**Como** jogador, **quero** que, se o mod não for compatível com a versão do
jogo, ele simplesmente não faça nada, **para** que um update nunca me impeça de
jogar nem corrompa o jogo.

#### Acceptance Criteria
- AC-2.1: Se qualquer ponto interno necessário não for encontrado **ou** for
  encontrado de forma ambígua (mais de um candidato), o mod **não aplica nada**
  e o jogo abre e roda normalmente, sem prompt extra.
- AC-2.2: O mod nunca aplica parcialmente: ou aplica todos os pontos, ou
  nenhum.
- AC-2.3: Nenhum cenário de incompatibilidade resulta em crash, tela preta ou
  travamento na inicialização atribuível ao mod.

#### Edge Cases
- EC-2.1: Loader ausente/desatualizado → mod não carrega; sem efeito e sem
  crash (comportamento do próprio loader).
- EC-2.2: Duas cópias do mesmo mecanismo instaladas (ex.: este mod + a DLL
  original ou o pacote ICE, que já inclui um Cutscene Skip) → não suportado;
  documentar "instale apenas um".

### US-3: Diagnóstico claro para manutenção
**Como** mantenedor do mod, **quero** que o log do loader diga exatamente o que
foi encontrado e o que falhou, **para** identificar em minutos o que quebrou
após um update do jogo.

#### Acceptance Criteria
- AC-3.1: Em carga bem-sucedida, o log registra: nome/versão do mod, versão
  do jogo detectada, cada ponto interno buscado com **quantidade de
  candidatos** e endereço, e a confirmação de aplicação.
- AC-3.2: Em falha, o log registra qual ponto falhou e por quê (não
  encontrado / ambíguo / falha de aplicação) e que o mod foi **desativado**.
- AC-3.3: Todas as mensagens são prefixadas com o nome do mod para serem
  filtráveis no meio de outros plugins.
- AC-3.4: A carga adiciona no máximo alguns segundos ao tempo de
  inicialização e **nada** ao custo por frame além do que o jogo já executa
  ao decidir sobre o prompt de skip.

### US-4: Instalação e remoção triviais
**Como** jogador, **quero** instalar copiando uma pasta para a raiz do jogo e
remover apagando um arquivo, **para** ter a mesma experiência do mod original.

#### Acceptance Criteria
- AC-4.1: Instalação = copiar a pasta do pacote para a raiz do jogo (mesmo
  layout do original: dentro da pasta de mods do loader).
- AC-4.2: Pré-requisitos (loader do ecossistema MHW e runtime necessário)
  documentados no README, com versões testadas.
- AC-4.3: Remoção = apagar o arquivo do mod; nenhum outro arquivo do jogo é
  alterado ou criado (além do log que o loader já gera).
- AC-4.4: README com créditos explícitos a **Tsulin** (mod original) e
  **Moonbunnie** (pesquisa e injeção), link do mod original e nota de que o
  comportamento foi reproduzido a partir do binário para uso pessoal.

### US-5: Fonte própria e mantenível
**Como** mantenedor, **quero** que todo o comportamento esteja em código-fonte
legível e versionado, com os pontos dependentes da versão do jogo isolados em
um único lugar, **para** que atualizar para um novo build seja "trocar
assinaturas + testar", sem reengenharia.

#### Acceptance Criteria
- AC-5.1: O repositório contém a fonte completa, instruções de build
  reproduzível e o pacote final é gerado a partir dela.
- AC-5.2: Os dados dependentes do build do jogo (pontos internos, offsets,
  payloads) ficam concentrados e comentados, com a versão do jogo para a qual
  foram validados.
- AC-5.3: A documentação do mecanismo (`ANALYSIS.md`) é mantida junto com o
  código e atualizada quando os pontos mudam.
- AC-5.4: O que puder ser testado sem o jogo (interpretação das assinaturas,
  regras de decisão do scanner: 0/1/N candidatos) tem teste automatizado.

### US-6: Skip que não derruba nem prende a sessão multiplayer
**Como** jogador em grupo, **quero** poder pular (ou ser impedido de pular)
sem que a conexão caia ou os outros fiquem presos, **para** usar o mod nas
sessões com amigos — que é onde mais se repete cutscene.

#### Acceptance Criteria
- AC-6.1: Existe um diagnóstico registrado (cenário, quem pulou, quem estava
  na cutscene, erro exibido) que estabelece **se e como** o skip causa a
  queda — com teste A/B (mesmo cenário sem o mod). Sem isso, nada é "corrigido".
- AC-6.2: Em sessão com outros jogadores, pular uma cutscene **não** derruba a
  conexão de ninguém nem deixa outro jogador preso na cena, no cenário
  reproduzido no diagnóstico.
- AC-6.3: Se não for possível pular com segurança em determinado contexto de
  multiplayer, o mod **não oferece o prompt** nesse contexto (falha fechada),
  em vez de pular e quebrar a sessão.
- AC-6.4: O comportamento solo (US-1) não regride.

#### Edge Cases
- EC-6.1: Só o host tem o mod / só um convidado tem o mod / todos têm →
  o resultado esperado de cada combinação é documentado a partir do
  diagnóstico; a regra mínima é AC-6.2/6.3 em qualquer combinação.
- EC-6.2: Cutscene disparada quando outro jogador está entrando na sessão →
  mesma regra (fechada em caso de dúvida).

### US-7: Skip limpo (áudio e cutscenes nativas) — condicional
**Como** jogador, **quero** que, após pular, não sobre áudio da cena e que
pular cutscenes que o jogo já permitia pular não trave o jogo, **para** o skip
ser transparente.

#### Acceptance Criteria
- AC-7.1: Após o skip, nenhuma voz/efeito da cutscene continua tocando sobre o
  gameplay.
- AC-7.2: Em cutscenes nativamente puláveis, acionar o skip com o mod não
  causa crash (o caminho nativo é preservado — AC-1.3).
- Entra na v1 **somente** se o diagnóstico de US-6 apontar causa comum (mesmo
  mecanismo de término da cutscene) ou se o custo for baixo; caso contrário,
  vai para o backlog com o que foi aprendido.

## Constraints & Assumptions
- Alvo: **Monster Hunter: World Steam, x64, versão 15.23.00 (build
  15539686)** — o build instalado e o último publicado pela Capcom.
- Ecossistema: o mod é um plugin do **Stracker's Loader** (mesmo mecanismo de
  distribuição do original). Sem o loader, nada acontece.
- O executável do jogo é protegido (Denuvo): **toda validação de compatibilidade
  exige o jogo em execução**. Não há teste offline dos pontos internos.
- Ambiente de desenvolvimento em WSL2; o jogo roda no Windows do mesmo
  computador; o log do loader é legível a partir do WSL
  (`/mnt/d/SteamLibrary/steamapps/common/Monster Hunter World/loader.log`).
- Uso pessoal: sem publicação no Nexus na v1; sem obrigações de suporte.
- Tipo MVP: build/lint como gate; testes automatizados só no que roda fora do
  jogo (AC-5.4); o resto é checklist manual in-game.

## Out of Scope (v1)
- Arquivo de configuração (on/off, lista de exclusão de cutscenes).
- Auto-skip sem apertar botão.
- Tecla dedicada/configurável (hook de input); prompt/botão no DualSense.
- SOS automático após skip.
- Correção do caso *"The Best Kind of Quest"* (softlock por eventos pulados).
- Suporte a outras versões do jogo (anteriores/consoles), a outros loaders
  ou a convivência com o ICE.
- Publicação no Nexus (exigiria permissão do autor original) e overlay/aviso
  in-game de incompatibilidade.

Itens acima são candidatos ao backlog (`sdd/backlog.md`) após a v1.

## Open Questions
- **OQ-1 (passo zero) — RESOLVIDA 2026-08-18**: a v1.1 **funciona** no build
  15539686 com o Stracker's Loader (jun/2024). Prompt e skip confirmados
  in-game; log em `step-zero-loader.log`. Ponto de partida da spec técnica:
  reproduzir os mesmos pontos internos e validar **paridade** com a v1.1.
  Problem statement a revisar conforme decisão de escopo do usuário.
- **OQ-5 (diagnóstico A/B — bloqueia US-6)**: o usuário reproduz o cenário
  de queda **com** e **sem** a DLL original (`scripts/validate-original.sh
  unstage`/`stage`) e registra: solo ou MP; host ou convidado; quem tinha o
  mod; momento da queda (durante a cena, ao pular, ao outro entrar); código
  de erro exibido; se os outros ficaram presos na cena. Protocolo em
  `diagnostico-mp.md`.
- **OQ-6**: qual o mecanismo do jogo para término de cutscene em MP (o skip
  nativo das cutscenes puláveis sincroniza com os outros jogadores?) — a
  responder na fase técnica com o jogo em execução; define se a solução é
  "chamar o término nativo" ou "não oferecer skip em MP".
- **OQ-2**: quais tipos de cutscene o mecanismo cobre de fato (in-game
  realtime vs. vídeo, abertura do jogo, cutscenes de vila)? Sai da checklist
  in-game da etapa de tasks; a spec aceita "as mesmas do original".
- **OQ-3**: quais quests/cutscenes usar como cenário de teste repetível
  (assigned quest com intro de monstro, novo save)? Definir na etapa de tasks.
- **OQ-4**: versão exata do Stracker's Loader a fixar como "testada" no README
  (a que o usuário instalar no passo zero).
