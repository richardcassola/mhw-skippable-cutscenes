# Opções de solução — disconnect ao pular cutscene

Contexto: o mod, em cenas **não** nativamente puláveis, faz um *seek* local do
timeline (P3: `tempoAtual = duração-60`). O servidor de sessão espera a duração
cheia → desconecta. O mesmo desvio explica o **áudio órfão** (as vozes já
disparadas não são paradas) e o **MP travado** (o outro lado não pulou).

## Descoberta-chave da engenharia reversa (muda o jogo)
O payload P3 do próprio mod já contém esta decisão:
```
cmp byte [rcx+4D3h],0   ; +0x4D3 = flag "nativamente pulável"
jne  -> cai no código ORIGINAL do jogo (skip nativo)   ; se != 0
...seek manual...        ; só se == 0 (cena não-pulável)
```
Ou seja: **cenas nativamente puláveis já passam pelo skip nativo do jogo — e
essas NÃO derrubam nem deixam áudio órfão.** O bug mora **exclusivamente** no
caminho do *seek*, usado nas cenas que o jogo marcou como não-puláveis. Logo,
existe no próprio jogo uma rotina de "terminar cutscene" que é segura para a
sessão. O conserto correto é **fazer as cenas não-puláveis usarem esse mesmo
caminho**, em vez do seek.

## Opções (ranqueadas)

### A. Chamar a rotina nativa de término (conserto correto) — RECOMENDADA
Descobrir, com o jogo rodando, o que a cena nativamente pulável executa ao ser
pulada (a função logo após o `jne` em P1/P3) e fazer o mod chamá-la para
qualquer cena — ou testar o atalho: **setar `+0x4D3 = 1`** na cena atual antes
do skip, deixando o próprio P3 cair no caminho nativo.
- Prós: resolve disconnect **e** áudio de uma vez (mesma raiz); é o jeito
  "certo"; mantém MP.
- Contras: engenharia reversa **em runtime** (o executável tem Denuvo, então
  nada disso é offline); pode ser que o jogo bloqueie o skip nativo em cenas de
  história por um motivo (aí caímos na opção C). Risco médio, recompensa alta.
- Esforço: iterativo, algumas sessões de teste com o jogo aberto.

### B. Fast-forward em vez de salto
Em vez de saltar o relógio, **acelerar** o avanço (ex.: 8–16×) até o fim.
- Prós: a cena percorre todos os estados, só que rápido — pode manter
  áudio/sessão em sincronia; mudança pequena no payload.
- Contras: incerto se o servidor tolera; "pular" vira "acelerar" (~1–2 s em vez
  de instantâneo). Bom **experimento barato** para validar a hipótese da causa.
- Esforço: baixo (ajuste no payload), mas precisa de teste in-game.

### C. Falha fechada em sessão online (mitigação pragmática)
Só oferecer o skip quando NÃO houver risco de derrubar (ex.: detectar
"conectado a sessão com outros" e, nesse caso, não mostrar o prompt).
- Prós: nunca derruba; simples de garantir.
- Contras: mata o uso justamente em grupo; exige achar a flag de "estou em
  sessão online com gente". É um paliativo, não conserto.

### D. Aceitar offline / jogar em sessão privada (custo zero)
Se o uso é majoritariamente **solo**, o disconnect é quase cosmético: a quest
continua, você só fica sem poder receber gente. Jogar deliberadamente em
sessão privada torna o "cair" um não-evento.
- Prós: zero código; o mod atual já serve.
- Contras: não serve se você quer manter a sessão aberta para amigos.

## Recomendação
Começar por **B como experimento** (barato, confirma a causa e talvez já
resolva) e, em paralelo/na sequência, ir para **A** (o conserto de verdade,
que também mata o bug de áudio). **C** fica como rede de segurança se A/B não
derem um skip 100% seguro em online. **D** é o "e enquanto isso" para você
poder jogar hoje.

## Restrição honesta
Todo o A/B/C exige **runtime RE no Windows** (debugger/Cheat Engine no jogo
aberto; Denuvo pode dificultar attach). Eu analiso dumps e escrevo o código
daqui do WSL, mas a parte de rodar/depurar o jogo e testar MP é conduzida por
você na máquina Windows — é um ciclo iterativo, não uma tarde.
