# Diagnóstico A/B — queda de conexão na missão (OQ-5)

> **Atualização 2026-08-18**: usuário caiu **solo**. Ótimo — dispensa amigos.
> No MHW, quest solo ainda roda dentro de uma sessão online; "cair" solo é
> quase sempre disconnect de servidor (código tipo `50382-MW1`). Hipótese:
> o seek local diverge do estado esperado e o servidor derruba a sessão.
> O dado que falta é o **código de erro** e o **momento** da queda.

Objetivo: provar se a queda é causada pelo mod (e em que condição) antes de
mexer em qualquer coisa. Duas rodadas, mesmo cenário, só muda a DLL.

## Preparação (eu faço do WSL)
- Rodada **A (com mod)**: `scripts/validate-original.sh stage`
- Rodada **B (sem mod)**: `scripts/validate-original.sh unstage`
  (o loader continua instalado — se a queda persistir em B, testamos depois
  sem o loader também).

## Cenário (você, no Windows) — repetir igual nas duas rodadas
1. Mesma quest, mesma cutscene, mesmos jogadores.
2. Anotar **antes**: você é host ou convidado? Quem mais tem o mod?
3. Durante: quem apertou skip, e em que momento (início/meio da cena)?
4. Se cair: **código de erro** na tela (ex.: `50382-MW1`) e o momento exato
   (durante a cena, logo ao pular, quando alguém entrou, minutos depois).
5. Os outros ficaram presos assistindo a cena inteira? Alguém foi expulso ou
   só você?
6. Feche o jogo; eu leio `loader.log`.

## Rodada B (sem mod)
Mesmo roteiro. Se **também** cair → a causa não é o skip (rede/NAT/servidor
ou, menos provável, o loader). Se **não** cair → é o skip, e o momento anotado
na rodada A diz onde procurar.

## Extra útil (se der)
- Uma rodada A em **solo** na mesma quest: se solo nunca cai e MP cai, é
  dessincronia de estado de cutscene entre jogadores.
- Uma rodada A onde **ninguém aperta skip** (só com o prompt aparecendo): se
  cair mesmo assim, o problema está na injeção do prompt (P2/P4), não no
  seek (P1/P3).

## Registro do resultado
| Rodada | Solo/MP | Você (host/conv.) | Quem tem mod | Quem pulou / quando | Caiu? Código | Outros presos? |
|--------|---------|-------------------|--------------|---------------------|--------------|----------------|
| A      |         |                   |              |                     |              |                |
| B      |         |                   |              |                     |              |                |
| A-solo |         |                   |              |                     |              |                |
| A-sem-skip |     |                   |              |                     |              |                |
