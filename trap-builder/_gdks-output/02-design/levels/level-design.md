# Trap Builder - Level Design (Protótipo)

> **Autores:** Diana 🎨 + Marco ⚙️ · **Validação numérica:** Theo 🔧
> **Versão:** 0.1 · **Data:** 2026-11-07
> **Base:** `gdd/gdd.md` v0.1 · `technical/technical-design.md` v0.1
> **Fonte da verdade dos dados:** `levels/tools/levels.js` (os mapas abaixo são gerados dele)

---

## 0. Método

O level design não foi feito "no olho". Foi validado por **simulação**:

1. Escrevemos um **simulador de referência** (`tools/sim.js`) que implementa exatamente a spec do Theo: ticks de 60 Hz, inteiros, `fork_hash`, fila de 1 porco por tile, `VOLTANDO`, ventilador etc.
2. Cada fase foi desenhada e passou pelo **solver** (`tools/solve.js`):
   - **Busca exaustiva** de todos os planos com até 3 armadilhas (até 4 na Fase 4) dentro do orçamento.
   - **Busca local** (hill-climbing com reinícios) para planos maiores.
3. O **par** de cada fase foi definido a partir da **solução mais barata encontrada**, e não por chute.
4. `node tools/validate.js` re-simula todas as soluções de referência. **Rode esse comando depois de qualquer mudança de regra ou número.**

```bash
cd _gdks-output/02-design/levels/tools
node validate.js            # rápido: confere as referências
node validate.js --search   # lento (minutos): roda o solver em todas as fases
```

> **Bônus para o Engine:** `sim.js` é uma implementação completa da lógica de simulação, com ~250 linhas. Dá para portar quase linha a linha para GDScript e usar `validate.js` como **teste de regressão**: o port em Godot deve produzir os mesmos resultados.

---

## 1. Achados Sistêmicos (mudanças de regra)

| # | Achado | Evidência do solver | Decisão |
|---|--------|---------------------|---------|
| **B6** | **Isca + Ventilador + Barranco é degenerado** | Na Fase 5, a primeira versão capturava **os 10 porcos por 250** (isca + 1 ventilador). O resto do kit perdia o sentido | **D014: cada tile de BARRANCO lota com 2 porcos.** Lotado, ele funciona como BLOQUEADO (o empurrão não acontece). Espelha a regra do "Buraco" do GDD original |
| B4' | Com o limite do barranco, o custo de 175 para o ventilador deixou a Fase 4 cara demais | Fase 4 ficou sem solução dentro do orçamento | **D015: o ventilador volta a custar 125.** O limite do barranco resolve o B4 melhor do que o preço |
| B7 | **O piche quase nunca aparece em soluções ótimas** | Só entra na solução mínima da Fase 2 | 📝 **Observar no playtest.** Candidatos: custo 25 ou velocidade ×1/3. Não mudar antes de jogar |
| B8 | A seed pode tornar uma fase "fácil por sorte" | Fase 4: o mínimo (250) usa só 2 ventiladores, porque a seed 4 manda os 4 porcos pelo galho de cima | Aceito: **aprender a seed faz parte do puzzle** (pilar P2). O par 300 abre 33 soluções, inclusive com isca |

---

## 2. Legenda dos Mapas

```
S spawn    X saída    + bifurcação    o junção    . rota
~ barranco (lota com 2)    m tile marcado (ventilador)    (espaço) bloqueado
A bigorna  T TNT  I isca  F ventilador  p piche
```
Coordenadas: `[x, y]` com origem no canto superior esquerdo, grid de 18×10.

---

## 3. Fases

### Fase 1: O Atalho
| Orçamento | Par | Fugas | Porcos | Seed | Armadilhas |
|-----------|-----|-------|--------|------|------------|
| 450 | **300** | 1 | 2 | 1 | Bigorna |

```
   ......
   .    .
   .    .
S..+....o...X
```
- **Ensina:** a bigorna captura **e bloqueia**; o porco seguinte é forçado para o outro galho.
- **Solver:** mínimo com 0 fugas = **300**, com 100 combinações. A passagem mais barata (★) custa 150, com 1 fuga.
- **Referência ★★★ (300):** bigorna no atalho `[5,4]` + bigorna na volta `[3,2]`. Funciona **com qualquer seed**.
- **Referência ★ (150):** 1 bigorna no tronco `[2,4]`. O 2º porco escala e foge.
- **Momento de ensino:** a primeira vez que o jogador vê um porco **dar meia-volta** (`VOLTANDO`).

### Fase 2: Fila Indiana
| Orçamento | Par | Fugas | Porcos | Seed | Armadilhas |
|-----------|-----|-------|--------|------|------------|
| 550 | **400** | 1 | 5 | 2 | Bigorna, Piche, TNT |

```
   ... ...
   . . . .
   . ... .
   .     .
S..+.....o. ...X
          . .
          . .
          ...
```
- **Ensina:** a **junção** e as **curvas** concentram porcos; o piche segura a fila; o TNT pega o grupo.
- **Solver:** mínimo = **400**, com **apenas 3 soluções** (fase de "eureka"). Com 450 há mais 4. A passagem mais barata custa 350, com 1 fuga.
- **Referência ★★★ (400):** TNT no último tile do zigue-zague antes da junção `[9,4]` + piche logo depois da junção `[10,5]` + bigorna no fundo do U `[10,8]`.
- **Dica de design:** o TNT na **junção** cobre os dois galhos ao mesmo tempo (regra B3: o TNT brilha onde rotas se encontram).

### Fase 3: Cheiro de Lavagem
| Orçamento | Par | Fugas | Porcos | Seed | Armadilhas |
|-----------|-----|-------|--------|------|------------|
| 600 | **475** | 1 | 6 | 3 | Isca, Piche, TNT |

```
  ...... ......
  .    . .    .
  .    . .    .
S.+    o.+....o.X
  .    .
  .  ...
  .  .
  ....
```
- **Ensina:** a **isca controla a bifurcação** e vira **âncora** de fila.
- **Solver:** mínimo = **475** (bateu com a estimativa do Theo), com 90 soluções. A passagem mais barata custa 325, com 1 fuga.
- **Referência ★★★ (475):** isca no 1º tile do galho de cima `[2,1]` + TNT ao lado dela `[3,1]` + TNT no 1º tile do galho de baixo `[2,5]`.
- **Comportamento emergente** (o solver descobriu): a isca segura o 1º porco e a fila se acumula até a bifurcação. Quando a isca acaba, a seed manda alguém para baixo, o TNT de baixo explode e pega quem estava na bifurcação. É exatamente o tipo de reação em cadeia do pilar P3.

### Fase 4: Ventania
| Orçamento | Par | Fugas | Porcos | Seed | Armadilhas |
|-----------|-----|-------|--------|------|------------|
| 450 | **300** | 1 | 4 | 4 | Ventilador, Isca, Piche |

```
      ~  ~
   ..........
   .  m  m  .
   .        .
S..+        o..X
   .        .
   .   m    .
   ..........
       ~
```
- **Ensina:** o ventilador empurra para o barranco; **o barranco lota** (2 por tile); a isca escolhe qual galho os ventiladores cobrem.
- **Solver** (exaustivo até 4 armadilhas, 1,2 M simulações): mínimo = **250** (solução única: 2 ventiladores em cima, porque a seed manda todos para cima). Com par 300 são **33 soluções** ★★★.
- **Referência ★★★ (250):** ventiladores em `[6,3]` e `[9,3]` soprando para cima.
- **Alternativa de ensino (≤ 300):** isca + ventiladores. Funciona com qualquer seed, então é a solução "robusta".

### Fase 5: O Banquete
| Orçamento | Par | Fugas | Porcos | Seed | Armadilhas |
|-----------|-----|-------|--------|------|------------|
| 900 | **750** | 2 | 10 | 5 | Todas |

```
     ~~~
   ........ ....
   .  m   . .  .
   .      . .m .
S..+      o.+..o.X
   .  ... .  ~~
   .  . . .
   .... ...
```
- **Ensina:** combinar tudo. Duas bifurcações, barrancos limitados e um U para o TNT.
- **Solver** (busca local, 4 seeds × 150 reinícios; **não exaustivo**): melhor encontrado = **725**. Nenhum plano com até 2 armadilhas passa.
- **Referência ★★★ (725):** ventilador `[6,3]`↑ + isca `[10,3]` + TNT `[12,4]` + ventilador `[13,4]`↓ + TNT `[11,5]`.
- **Referência ★★★ (750):** isca `[12,2]` + TNT `[15,4]` + TNT `[16,5]` + ventilador `[6,3]`↑ + bigorna `[7,6]`.
- ⚠️ Como a busca não foi exaustiva, um jogador pode achar algo **abaixo de 725**. Isso não é problema (★★★ = gasto ≤ par), mas vale registrar o melhor plano dos playtests.

---

## 4. Curva do Protótipo

| Fase | Porcos | Bifurc. | Armadilhas novas | Folga do orçamento sobre o par | Dificuldade do ★★★ |
|------|--------|---------|------------------|--------------------------------|--------------------|
| 1 | 2 | 1 | Bigorna | +150 | Fácil (100 soluções) |
| 2 | 5 | 1 | Piche, TNT | +150 | **Difícil (3 soluções)** |
| 3 | 6 | 2 | Isca | +125 | Média (90) |
| 4 | 4 | 1 | Ventilador | +150 | Média (33) |
| 5 | 10 | 2 | — (todas) | +150 | Difícil |

> ⚠️ **A Fase 2 tem o ★★★ mais difícil** do protótipo, antes de fases mais complexas. Se o playtest mostrar frustração, suba o par para 450 (7 soluções) ou troque a ordem das fases 2 e 3.

---

## 5. Regras de Level Design (checklist para fases futuras)

1. Toda fase deve ter **pelo menos uma solução ★★★ validada** em `validate.js`.
2. **Par = mínimo encontrado pelo solver** (ou um pouco acima, para abrir mais soluções).
3. **Orçamento = par + 125–150**, para dar espaço a soluções "feias mas que passam".
4. **Barranco:** no máximo 2–3 tiles por fase, sempre adjacentes a um tile marcado.
5. **TNT:** desenhe curvas, junções e U onde valha a pena explodir.
6. A **solução mais barata não deve depender só da seed**. Se depender (Fase 4), o par deve aceitar também a solução robusta.
7. Uma armadilha nova por fase, no máximo.

---

## 6. Decisões

| ID | Decisão |
|----|---------|
| D014 | Cada tile de BARRANCO lota com 2 porcos e depois vira BLOQUEADO para empurrões |
| D015 | O ventilador custa 125 (reverte o B4 de 175) |
| D016 | O par das fases é definido pelo solver; orçamento = par + 125–150 |

---

*Level design por Diana + Marco, validado pelo Theo, GD-KS Design Team*
