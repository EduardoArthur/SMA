# Trap Builder - Game Design Document (Protótipo)

> **Autora:** Diana 🎨 (Game Design Director)
> **Versão:** 0.1 (protótipo) · **Data:** 2026-11-07
> **Engine:** Godot 4 · **Plataforma do protótipo:** PC (mouse) → Mobile (toque) depois
> **Entradas:** `01-ideation/ideation-handoff.md`, `mechanics-exploration.md`, `00-input/GDD-Toon-Defense.pdf`
> **Status:** Documento vivo. Números marcados com `*` são valores iniciais de balanceamento.

---

## Índice
1. Visão & Pilares
2. Escopo do Protótipo
3. Fluxo de Jogo
4. Grid & Tiles
5. Rotas & Bifurcações
6. Porcos
7. Armadilhas
8. Economia, Derrota & Estrelas
9. O Lobo
10. Interface
11. Fases do Protótipo
12. Áudio & Feedback (mínimo)
13. Features & Critérios de Aceite
14. Dependências entre Sistemas
15. Registro de Decisões
16. Questões em Aberto

---

## 1. Visão & Pilares

> O Lobo Mau cansou de soprar casas. Agora ele planeja armadilhas cartoon e assiste os porquinhos caírem nelas.

| Pilar | Significa | Na prática |
|-------|-----------|------------|
| **P1. Planejar é jogar** | Toda decisão acontece no Blueprint | Zero input na Ação (exceto velocidade e pausa) |
| **P2. Justo e legível** | A derrota é culpa do plano, nunca do azar | Seed fixa; prévia de rota; as armadilhas nunca falham sozinhas |
| **P3. O caos é a recompensa** | Assistir precisa ser divertido | Reações em cadeia, lobo reagindo, slapstick |
| **P4. Menos é mais** | Eficiência é a maestria | Orçamento fechado; ★★★ para o plano mais barato |

**Teste de qualquer feature nova:** ela fortalece pelo menos um pilar sem enfraquecer outro?

---

## 2. Escopo do Protótipo

| Dentro | Fora (pós-protótipo) |
|--------|----------------------|
| 1 bioma (floresta) | Patos voadores, veículos |
| Porco comum (+ capacete, se sobrar tempo) | Banana, mola, ímã, foguete, urso, buraco, pavio, corda |
| 5 armadilhas: Bigorna, TNT, Isca, Ventilador, Piche | Progressão e desbloqueios persistentes, save |
| 5 fases | Mobile, narrativa, cutscenes |
| Blueprint ↔ Ação, estrelas, lobo reativo | Música original (usar placeholder) |

---

## 3. Fluxo de Jogo

```
[Menu] → [Seleção de Fase] → [BLUEPRINT] ⇄ [AÇÃO] → [RESULTADO] → (Repetir | Próxima | Menu)
                                  ▲                        │
                                  └─── "Ajustar plano" ────┘  (o plano é preservado)
```

### Máquina de estados da fase
| Estado | Entra quando | Input permitido | Sai quando |
|--------|--------------|-----------------|------------|
| `BLUEPRINT` | Fase carregada / "Ajustar" | Posicionar, girar, remover armadilhas; "Jogar" | Clique em **Jogar** |
| `TRANSICAO` | Saída do Blueprint | Nenhum | Animação de ~0,7 s termina |
| `ACAO` | Fim da transição | Pausa, velocidade 1x/2x, "Abortar" | Todos os porcos capturados ou fugidos / abortar |
| `RESULTADO` | Fim da Ação | Repetir, Ajustar, Próxima, Menu | Escolha do jogador |

**Regra:** "Ajustar" e "Repetir" restauram o mapa ao estado inicial **com o plano anterior já posicionado**. "Abortar" na Ação também volta ao Blueprint com o plano.

---

## 4. Grid & Tiles

- Grid ortogonal, tile de 64×64 px* (o mapa cabe numa tela 1280×720 sem scroll; ex.: 18×10 tiles).
- Cada tile tem um tipo:

| Tipo | Armadilhas permitidas | Porcos andam? | Empurrão do ventilador |
|------|----------------------|---------------|------------------------|
| `ROTA` | Bigorna, TNT, Isca, Piche | Sim | Porco é deslocado para ele e segue na rota desse tile |
| `MARCADO` | Ventilador | Não | Bloqueia (como `BLOQUEADO`) |
| `BARRANCO` | Nenhuma | Não | **Porco cai = capturado** (até 2 por tile; depois lota) |
| `BLOQUEADO` | Nenhuma | Não | Empurrão para; porco fica onde está |
| `SPAWN` | Nenhuma | Início | — |
| `SAIDA` | Nenhuma | Fim (= fuga) | — |

- **1 armadilha por tile.**
- No Blueprint, os tiles válidos para a armadilha selecionada ficam destacados.

**Critério visual:** no Blueprint, `BARRANCO` deve ser claramente distinto (ex.: hachurado com aviso de perigo), porque é a informação mais importante para o ventilador.

---

## 5. Rotas & Bifurcações

### Modelo
- As rotas são um **grafo dirigido feito à mão** por fase: **nós** (spawn, bifurcações, junções, saída) ligados por **arestas** (sequência ordenada de tiles `ROTA`).
- Todas as arestas apontam para a saída; não há ciclos.

### Escolha nas bifurcações (D003)
Ao chegar num nó com várias arestas de saída, o porco escolhe:
1. **Isca:** se alguma aresta de saída leva a uma isca ativa (ainda não comida) **antes da próxima bifurcação**, escolhe essa aresta. Com várias iscas, vale a de menor id da aresta.
2. **Senão:** entre as arestas **não bloqueadas**, escolhe `opcoes[ hash(seed_fase, id_porco, id_no) % n ]`.
3. **Se todas estiverem bloqueadas:** escolhe entre todas pelo mesmo hash e **escala a bigorna** (ver 7.1).

> **Por que hash e não uma sequência de RNG:** mover uma armadilha não muda as escolhas dos outros porcos. O jogador aprende "o porco 3 vai pela esquerda".

### Recálculo quando a bigorna cai
- Porcos que **ainda não passaram** pela bifurcação daquela aresta: decidem normalmente (a aresta está bloqueada).
- Porcos **já na aresta**, antes da bigorna: **voltam** até o nó anterior e decidem de novo, sem a aresta bloqueada.
  - ~~Simplificação no P2: escalar~~ → **não usar**: quebra o ★★★ da fase 1 (ver technical-design 7.2). Implementar o retorno já no P2.

### Prévia no Blueprint
- Todas as arestas aparecem iluminadas; os spawns mostram quantos porcos virão.
- Ao posicionar uma bigorna, a aresta afetada fica **tracejada** ("bloqueada após o 1º porco").
- Ao posicionar uma isca, uma seta destaca a bifurcação que ela controla.

---

## 6. Porcos

| Atributo | Porco Comum | Porco de Capacete (opcional) |
|----------|-------------|------------------------------|
| Velocidade | 1,5 tiles/s* | 1,2 tiles/s* |
| Bigorna | Capturado | **Imune** (fica tonto 1 s e passa por baixo) |
| Demais armadilhas | Normal | Normal |

### Estados
`ANDANDO` → (`LENTO` | `EMPURRADO` | `COMENDO` | `ESCALANDO` | `VOLTANDO`) → `CAPTURADO` | `FUGIU`

- **Movimento:** interpolação tile a tile ao longo da aresta. **Sem física** (determinismo).
- **Spawn:** a fase define a lista de porcos (tipo e spawn), saindo em intervalos de 1,0 s* (era 1,5 s; ver technical-design B2). O `id_porco` é a ordem na lista.
- **Captura:** o porco vira um ícone que voa até o contador do lobo ("+1 🍖").
- **Fuga:** o porco sai rindo pela saída e o contador de fugas pisca.

---

## 7. Armadilhas

Todas as armadilhas têm: `custo`, `tiles_validos`, `gatilho`, `efeito`, `persistencia`, `direcao` (opcional).

### 7.1 Bigorna (uso único / rota) · 150*
- **Tile:** `ROTA` · **Gatilho:** 1º porco que **entra** no tile. Ele fica paralisado olhando para cima até a queda (technical-design B5).
- **Efeito:** sombra cresce (0,4 s) → bigorna cai → porco capturado.
- **Persistência:** a bigorna fica no tile como **obstáculo** e a aresta passa a ser `bloqueada`.
- **Escalar:** um porco sem alternativa escala a bigorna (2 s* parado) e segue.
- **Aceite:** depois da queda, os porcos seguintes evitam a aresta sempre que houver alternativa.

### 7.2 TNT (uso único / área) · 200*
- **Tile:** `ROTA` · **Gatilho:** porco entra no tile → pavio de 0,3 s* → explosão.
- **Efeito:** captura **todos** os porcos em 3×3 (raio 1, incluindo diagonais) no momento da explosão.
- **Persistência:** some.
- **Aceite:** porcos que entram na área durante o pavio também são pegos.

### 7.3 Isca Falsa (uso único / atração) · 75*
- **Tile:** `ROTA` · **Passiva:** controla a bifurcação imediatamente anterior (regra 5.1).
- **Gatilho:** 1º porco que entra no tile → `COMENDO` por 3 s*, parado, ocupando o tile.
- **Fila:** os porcos seguintes param **atrás** dele (fila de 1 por tile) até ele terminar.
- **Persistência:** some depois de comida. A bifurcação volta à regra do hash.
- **Aceite:** com uma isca ativa, 100% dos porcos escolhem o galho dela.

### 7.4 Ventilador (contínuo / comportamento) · 125 (D015; o barranco limitado substitui o aumento de custo)
- **Tile:** `MARCADO` · **Direção:** 4 direções, girada no Blueprint.
- **Área:** os 3* tiles em linha à frente dele (a "corrente de ar").
- **Efeito:** um porco que entra num tile `ROTA` da corrente é **empurrado 1 tile** na direção do vento (0,25 s*):
  - Destino `BARRANCO` → **capturado** (cai). **Cada tile de barranco lota com 2 porcos** (D014); lotado, funciona como `BLOQUEADO`.
  - Destino `ROTA` → o porco passa a seguir a aresta desse tile. Se cair "para trás" na mesma aresta, refaz o caminho.
  - Destino `BLOQUEADO`, `MARCADO`, fora do mapa ou tile com bigorna → não se move.
- Cada porco é empurrado **no máximo uma vez por ventilador** (evita loops).
- **Persistência:** a onda inteira.
- **Aceite:** o ventilador nunca captura sozinho fora de `BARRANCO` e nunca trava porcos em loop.

### 7.5 Piche (contínuo / atraso) · 50*
- **Tile:** `ROTA` · **Efeito:** velocidade × 1/3 enquanto o porco está no tile (D021; era × 0,5).
- **Persistência:** a onda inteira. Não acumula com outro piche (1 por tile).
- **Aceite:** porcos atrás de um porco lento **não** o ultrapassam (fila por tile), o que gera **acúmulo** para o TNT.

### Matriz de Interações
| | Bigorna | TNT | Isca | Ventilador | Piche |
|---|---|---|---|---|---|
| **Bigorna** | — | Desvia para a zona do TNT | Bloquear o galho anula a isca | O empurrão para na bigorna | Desvia para o piche |
| **TNT** | | — | O porco comendo vira âncora → fila → explosão | Empurra para a área | Acúmulo → explosão |
| **Isca** | | | — | Empurra para o galho da isca | Atrasa a fila atrás da isca |
| **Ventilador** | | | | — | Porco lento fica mais tempo na corrente |

---

## 8. Economia, Derrota & Estrelas

### Por fase
| Campo | Exemplo |
|-------|---------|
| `orcamento` | 600 |
| `orcamento_par` | 425 (gasto ≤ par → ★★★ possível) |
| `limite_fugas` | 2 |
| `porcos` | lista de spawns |
| `armadilhas_disponiveis` | subconjunto das 5 |

### Regras
- O dinheiro é gasto ao posicionar e **devolvido integralmente** ao remover no Blueprint.
- Não há renda durante a Ação.
- **Derrota:** `fugas > limite_fugas`.
- **Estrelas (G5, padrão adotado):**
  - ★ passou (`fugas ≤ limite_fugas`)
  - ★★ `fugas == 0`
  - ★★★ `fugas == 0` **e** `gasto ≤ orcamento_par`
- A melhor marca de estrelas fica salva **em memória** durante a sessão (sem save em disco no protótipo).

---

## 9. O Lobo (G2: assiste e reage)

- Sprite fixo num canto do mapa (arbusto, binóculo), visível só na Ação e no Resultado. No Blueprint, aparece como "engenheiro" ao lado do catálogo (opcional).
- **Reações** (prioridade de cima para baixo; cada uma dura ~1 s):

| Evento | Reação |
|--------|--------|
| Porco foge | Arranca o chapéu / pisoteia |
| Captura múltipla (≥ 2 em 1 s) | Comemora com guardanapo e talheres |
| Captura simples | Lambe os beiços |
| Ocioso | Observa de binóculo |
| Resultado ★★★ | Banquete |
| Resultado: derrota | Cai sentado, cara de "de novo não" |

- **Aceite:** toda captura e toda fuga geram reação visível em até 0,2 s.

---

## 10. Interface

### Blueprint
- **Topo:** orçamento restante · fugas permitidas · ícones dos porcos da onda.
- **Lateral/baixo:** catálogo com as armadilhas disponíveis (ícone, custo; desabilitada se não couber no orçamento).
- **Mouse:**
  - Esquerdo: posicionar / selecionar.
  - Direito ou `R`: girar (ventilador).
  - Arrastar para fora / `Del`: remover.
- **Botão JOGAR** grande e sempre visível.

### Ação
- Pausa, velocidade 1x/2x e Abortar. Contadores de capturas e fugas.

### Resultado
- Estrelas, gasto/par, fugas/limite e botões: Repetir · Ajustar · Próxima · Menu.
- **Na derrota:** destacar **por qual saída e por qual aresta** cada porco fugiu (pilar P2).

---

## 11. Fases do Protótipo (validadas por simulação; detalhes em `levels/level-design.md`)

| # | Nome | Ensina | Armadilhas | Porcos | Bifurc. | Orç. / Par* | Fugas |
|---|------|--------|-----------|--------|---------|-------------|-------|
| 1 | O Atalho | Bigorna bloqueia e desvia | Bigorna | 2 | 1 | 450 / 300 | 1 |
| 2 | Fila Indiana | Piche acumula, TNT limpa | TNT, Piche, Bigorna | 4 | 1 | 550 / 400 | 1 |
| 3 | Cheiro de Lavagem | Isca controla a bifurcação | Isca, TNT, Piche | 6 | 2 | 600 / 475 | 1 |
| 4 | Ventania | Ventilador + barrancos | Ventilador, Isca, Piche | 4 | 1 | 450 / 300 | 1 |
| 5 | O Banquete | Combo-assinatura | Todas | 10 | 2 | 900 / 750 | 2 |

**Regra de design de fase:** cada fase precisa de pelo menos **uma solução ★★★ verificada à mão** antes de ser implementada.

---

## 12. Áudio & Feedback (mínimo)

| Evento | SFX | Visual |
|--------|-----|--------|
| Blueprint → Ação | Swoosh | Linhas brancas se desfazem → cores |
| Bigorna | Apito descendente + CLANG | Sombra → achatamento |
| TNT | Pavio + BOOM | Flash + nuvem |
| Isca | Nhac nhac | Corações |
| Ventilador | Zumbido | Linhas de vento |
| Piche | Squelch | Pés afundando |
| Captura | Pop | Ícone voa ao contador |
| Fuga | Risada + buzina | O contador pisca em vermelho |

Placeholders gratuitos são aceitáveis no protótipo.

---

## 13. Features & Critérios de Aceite

| ID | Feature | Critérios de aceite | Prioridade |
|----|---------|---------------------|------------|
| F01 | Grid & tiles | Mapa carregado de uma cena/recurso; tipos de tile corretos; tiles válidos destacados | P1 |
| F02 | Grafo de rotas | Porcos vão do spawn à saída seguindo arestas; nenhum sai da rota sem ventilador | P1 |
| F03 | Bifurcação determinística | Mesma fase + mesmo plano = mesmo resultado em 10 execuções | P1 |
| F04 | Estados Blueprint ↔ Ação | Plano preservado em Repetir/Ajustar/Abortar | P2 |
| F05 | Bigorna | Ver 7.1; recálculo de rota funcionando | P2 |
| F06 | Economia & catálogo | Custo debitado/devolvido; itens sem saldo desabilitados | P3 |
| F07 | TNT | Ver 7.2 | P3 |
| F08 | Isca | Ver 7.3 | P3 |
| F09 | Ventilador | Ver 7.4; giro no Blueprint | P3 |
| F10 | Piche + fila por tile | Ver 7.5; sem ultrapassagem | P3 |
| F11 | Derrota & estrelas | Ver 8; tela de resultado correta | P3 |
| F12 | Lobo reativo | Ver 9 | P4 |
| F13 | Transição visual | Blueprint azul → cenário colorido em ≤ 0,8 s | P4 |
| F14 | 5 fases | Cada fase com uma solução ★★★ verificada | P4 |
| F15 | Prévia de rota e feedback de derrota | Arestas tracejadas; destaque da fuga | P4 |

---

## 14. Dependências entre Sistemas

```
F01 Grid ──► F02 Rotas ──► F03 Bifurcação ──► F05 Bigorna ──► F15 Prévia
                │                │
                ▼                ▼
          F10 Piche/Fila    F08 Isca
                │
                ▼
             F07 TNT        F09 Ventilador (F01 + F02)
F04 Estados ──► F11 Estrelas ──► F14 Fases
F06 Economia ──► F11
F12 Lobo (eventos de captura/fuga)   F13 Transição (F04)
```

---

## 15. Registro de Decisões

| ID | Decisão | Origem |
|----|---------|--------|
| D001 | Trilha Standard, escopo de protótipo, 1–4 semanas | init |
| D002 | Premissa Lobo Emboscador | Sparky |
| D003 | Bifurcação por `hash(seed, porco, nó)` | Mira |
| D004 | Derrota por limite de fugas | Mira |
| D005 | Posicionamento em Rota + Marcado | Mira |
| D006 | Armadilhas: Bigorna, TNT, Isca, Ventilador, Piche | Mira |
| **D007** | **O lobo assiste e reage** num canto fixo do mapa | Diana |
| **D008** | **Ventilador depende do tile:** `BARRANCO` captura, `ROTA` desloca, o resto bloqueia | Diana |
| **D009** | **Bigorna sem alternativa: os porcos escalam** (2 s) | Diana |
| **D010** | **Onda única** por fase | Diana |
| **D011** | Estrelas: passou / zero fugas / zero fugas + gasto ≤ par | Diana (proposta Mira) |
| **D012** | Fila de 1 porco por tile, sem ultrapassagem | Diana |
| **D013** | Remover armadilha no Blueprint devolve 100% do custo | Diana |
| **D014** | **Barranco lota com 2 porcos por tile** | Level design (solver) |
| **D015** | **Ventilador custa 125** | Level design (solver) |
| **D016** | **Par = mínimo do solver; orçamento = par + 125–150** | Level design |
| **D017** | **Marca fictícia: Catapimba S.A.** | Aurora |
| **D018** | **Arte vetorial própria** (Inkscape/Figma), top-down com personagens em 3/4, contorno `#2B1B17` | Aurora |
| **D019** | **Áudio: SFX gratuitos + música CC0/CC-BY**, com CREDITS.md | Anton |
| **D021** | **Piche: velocidade ÷ 3** (era ÷ 2; playtest: piche fraco) | Playtest |
| **D022** | **Fase 2 com 4 porcos** (era 5; playtest: difícil demais) | Playtest |

---

## 16. Questões em Aberto

| # | Questão | Dono | Quando |
|---|---------|------|--------|
| Q1 | ~~Marca fictícia (G3)~~ → **Catapimba S.A.** (D017) | Aurora | ✅ |
| Q2 | Título final (G1) | Usuário | Qualquer momento |
| Q3 | Mostrar a escolha de cada porco depois da 1ª tentativa? | Diana / UX | Após o playtest do P3 |
| Q4 | Porco de capacete entra no protótipo? | Diana | Se o P3 terminar adiantado |
| Q5 | ~~Valores `*` de balanceamento~~ → v0.1 em `technical/technical-design.md` | Theo | ✅ |

---

*GDD por Diana, GD-KS Game Design Director*
*Próximos: Theo (fichas técnicas) · Diana + Marco (level design das 5 fases) · Aurora (art bible mínima)*
