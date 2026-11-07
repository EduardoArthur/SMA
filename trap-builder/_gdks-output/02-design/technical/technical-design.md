# Trap Builder - Technical Design (Fichas Técnicas)

> **Autor:** Theo 🔧 (Technical Game Designer)
> **Versão:** 0.1 · **Data:** 2026-11-07
> **Base:** `02-design/gdd/gdd.md` v0.1
> **Público:** você, implementando em Godot 4 (GDScript)

---

## 0. Resumo para o Programador

1. A simulação roda em **ticks fixos de 60 Hz**, com **inteiros**. Nada de física e nada de `delta` na lógica.
2. A posição do porco na aresta é um **inteiro em sub-unidades** (`TILE_UNITS = 1200` por tile).
3. Bifurcações usam um **hash próprio** (abaixo). Não use `randi()`, `hash()` nem `RandomNumberGenerator` para isso.
4. Os dados ficam em **Resources** (`TrapData`, `PigData`, `LevelData`): o balanceamento muda no Inspector, sem tocar em código.
5. A ordem de processamento dentro de um tick é **fixa** (seção 2.3). É ela que garante o determinismo.

---

## 1. Achados de Balanceamento (correções ao GDD v0.1)

| # | Problema | Evidência | Correção | Status |
|---|----------|-----------|----------|--------|
| B1 | **A Fase 1 é impossível de completar com ★★** | Bigorna + Piche, 4 porcos. A bigorna captura 1 e o piche não captura. Seriam 4 bigornas = 600 > orçamento de 400 | Fase 1 com **2 porcos**, só Bigorna, orçamento 450 / par 300 / fugas 1 (ver 7.1) | ✅ aplicado no GDD |
| B2 | **O piche não acumula porcos** | Com spawn a cada 1,5 s e 1,5 tiles/s, o piche (×0,5) leva 1,33 s por tile, menos que o intervalo, então não forma fila | **Intervalo de spawn = 1,0 s** (60 ticks). O piche passa a levar 1,33 s, maior que 1,0 s, e a fila cresce 0,33 s por porco | ✅ aplicado |
| B3 | **O TNT pega menos porcos do que parece** | Com fila de 1 porco por tile, a captura máxima é o número de tiles de rota dentro do 3×3. Numa reta são **3 porcos**; numa curva em U, até 6 | Documentado como regra de level design: **TNT brilha em curvas** | 📝 regra |
| B4 | **Ventilador + Barranco captura sem limite** por 125 | Contínuo, empurra todos os porcos que passam | ~~Custo 175~~ → **substituído por D014**: barranco lota com 2 porcos por tile; ventilador volta a 125 (ver level-design B6) | ✅ revisado |
| B5 | **A sombra da bigorna (0,4 s) deixa o porco escapar** | A 1,5 tiles/s, o porco sai do tile em ~0,67 s; com variação, ele poderia sair antes da queda | O porco que dispara a bigorna fica **paralisado olhando para cima** (`OLHANDO`) até a queda. Clássico cartoon | ✅ regra |

---

## 2. Modelo de Simulação

### 2.1 Constantes globais (`res://data/sim_constants.gd`, autoload ou `const`)

| Constante | Valor | Observação |
|-----------|-------|------------|
| `TICK_RATE` | 60 | ticks por segundo de jogo |
| `TILE_UNITS` | 1200 | sub-unidades por tile (divisível por 2, 3, 4, 5, 6, 8, 10, 12...) |
| `TILE_PX` | 64 | só visual |
| `SPAWN_INTERVAL_TICKS` | 60 | 1,0 s (B2) |
| `SPEED_MULT_2X` | 2 | modo 2x = 2 ticks de simulação por frame lógico |
| `TRANSITION_MS` | 700 | Blueprint → Ação (visual, fora da simulação) |

### 2.2 Loop

```gdscript
# SimulationRunner.gd (Node)
var _accum := 0.0
func _process(delta: float) -> void:
    if state != ACAO or paused: return
    _accum += delta * speed_mult           # speed_mult = 1 ou 2
    while _accum >= 1.0 / SimConst.TICK_RATE:
        _accum -= 1.0 / SimConst.TICK_RATE
        step()                              # lógica 100% inteira
```
A parte visual (sprites) **interpola** a partir do estado da simulação. A simulação nunca lê a posição dos sprites.

### 2.3 Ordem de processamento de um tick (`step()`)
1. **Spawn:** se `tick == spawn_tick[i]`, cria o porco `i` (quando o tile de spawn estiver livre; senão espera).
2. **Timers:** decrementa os timers das armadilhas (sombra da bigorna, pavio do TNT) e dos porcos (comendo, escalando, empurrão). Resolve os que zerarem, **na ordem do id da armadilha** e depois do id do porco.
3. **Movimento:** move os porcos **em ordem crescente de `id_porco`** (o mais antigo primeiro).
4. **Gatilhos de entrada:** para cada porco que **entrou** num tile neste tick (em ordem de id), dispara a armadilha do tile.
5. **Ventilador:** aplica os empurrões pendentes (porco que entrou num tile de corrente).
6. **Fim:** se `capturados + fugidos == total`, emite `wave_finished`.

### 2.4 Ocupação (fila por tile, D012)
- O porco ocupa o tile `floor(progress / TILE_UNITS)` da aresta atual.
- Um porco **não avança** para um tile ocupado por outro porco. Ele para no limite (`progress` máximo = início do próximo tile − 1).
- **Exceção:** porcos em `VOLTANDO` ignoram a ocupação (evita deadlock na volta da aresta bloqueada).
- **Tiles de nó** (bifurcação/junção) também seguem a regra de 1 porco.

### 2.5 Hash de bifurcação (D003)
```gdscript
# Determinístico, estável entre versões do Godot e entre plataformas.
static func fork_hash(seed: int, pig_id: int, node_id: int) -> int:
    var h := (seed * 73856093) ^ (pig_id * 19349663) ^ (node_id * 83492791)
    h &= 0xFFFFFFFF
    h = ((h ^ (h >> 16)) * 0x45d9f3b) & 0xFFFFFFFF
    h = ((h ^ (h >> 16)) * 0x45d9f3b) & 0xFFFFFFFF
    h = h ^ (h >> 16)
    return h

static func choose_edge(seed, pig_id, node_id, options: Array) -> int:
    # options: ids das arestas de saída ORDENADOS por id (ordem estável!)
    return options[fork_hash(seed, pig_id, node_id) % options.size()]
```
**Teste:** escreva um teste que imprime `choose_edge` para seed=1, porcos 0..9 e nó 0, e compare com os valores esperados em todas as execuções.

---

## 3. Estruturas de Dados (Resources)

### 3.1 `TrapData.gd` (`extends Resource`)
| Campo | Tipo | Descrição |
|-------|------|-----------|
| `id` | `StringName` | `&"anvil"`, `&"tnt"`, `&"bait"`, `&"fan"`, `&"tar"` |
| `display_name` | `String` | Nome no catálogo |
| `cost` | `int` | Moedas |
| `valid_tiles` | `int` (flags) | `TILE_ROUTE = 1`, `TILE_MARKED = 2` |
| `has_direction` | `bool` | Só o ventilador |
| `persistent` | `bool` | Contínua durante a onda? |
| `icon` | `Texture2D` | Catálogo |
| `scene` | `PackedScene` | Visual da armadilha |
| `params` | `Dictionary` | Parâmetros específicos (seção 4) |

### 3.2 `PigData.gd`
| Campo | Tipo | Comum | Capacete |
|-------|------|-------|----------|
| `id` | `StringName` | `&"pig"` | `&"pig_helmet"` |
| `speed` | `int` (units/tick) | **30** (= 1,5 tiles/s) | **24** (= 1,2 tiles/s) |
| `immune_anvil` | `bool` | false | true |
| `scene` | `PackedScene` | — | — |

> Fórmula: `speed_units_per_tick = tiles_por_segundo × TILE_UNITS / TICK_RATE`. Ex.: 1,5 × 1200 / 60 = 30.

### 3.3 `LevelData.gd`
| Campo | Tipo | Descrição |
|-------|------|-----------|
| `level_id` | `int` | 1..5 |
| `title` | `String` | "O Atalho" |
| `map_scene` | `PackedScene` | Cena com `TileMapLayer` (blueprint e colorido) |
| `tile_types` | `Dictionary[Vector2i, int]` | Ou ler de *custom data* do TileSet (recomendado) |
| `nodes` | `Array[RouteNode]` | `{id:int, cell:Vector2i, kind: SPAWN/FORK/JOIN/EXIT}` |
| `edges` | `Array[RouteEdge]` | `{id:int, from:int, to:int, cells:Array[Vector2i]}` (sem incluir as células dos nós) |
| `spawns` | `Array[SpawnEntry]` | `{pig: PigData, node_id:int}`, em ordem (índice = `id_porco`) |
| `seed` | `int` | Seed da fase |
| `budget` | `int` | Orçamento |
| `par` | `int` | Orçamento-par (★★★) |
| `escape_limit` | `int` | Fugas permitidas |
| `available_traps` | `Array[TrapData]` | Catálogo da fase |

> **Dica Godot 4:** marque o tipo de tile com **Custom Data Layers** no TileSet (`tile_type: int`). Os nós e arestas podem ser montados com `Marker2D`/`Line2D` na cena e convertidos para dados no `_ready()`. Assim você desenha a fase visualmente no editor.

### 3.4 Estado de runtime (não é Resource)
```gdscript
class PigState:    id, data, edge_id, progress:int, state:int, timer:int, pushed_by:Array[int]
class TrapState:   id, data, cell:Vector2i, dir:Vector2i, triggered:bool, timer:int, consumed:bool
```

---

## 4. Fichas das Armadilhas

> Tempos em **ticks** (60 = 1 s). Custos já balanceados para a v0.1.

### 4.1 🔨 Bigorna: `anvil`
| Campo | Valor |
|-------|-------|
| Custo | **150** |
| Tiles | `ROUTE` |
| Gatilho | 1º porco que **entra** no tile |
| `params.shadow_ticks` | **24** (0,4 s) |
| `params.climb_ticks` | **120** (2 s) |
| Persistência | Fica como obstáculo; a aresta vira `blocked` |

**Sequência:**
1. O porco entra → estado `OLHANDO` (parado) → a sombra cresce por 24 ticks.
2. A bigorna cai → o porco é `CAPTURADO` (se `immune_anvil`: fica tonto 30 ticks e segue).
3. `edge.blocked = true`. Porcos já nessa aresta, **antes** do tile, passam a `VOLTANDO` até o nó `from` e decidem de novo.
4. Um porco que chega ao tile da bigorna sem alternativa → `ESCALANDO` por 120 ticks → segue.

**Casos-limite:**
- Bigorna posicionada no tile **de um nó**: não permitido (só em tiles de aresta). Simplifica o grafo.
- Duas bigornas na mesma aresta: a 2ª só é alcançada por porcos que escalaram a 1ª.

### 4.2 🧨 TNT: `tnt`
| Campo | Valor |
|-------|-------|
| Custo | **200** |
| Tiles | `ROUTE` |
| Gatilho | Porco entra no tile |
| `params.fuse_ticks` | **18** (0,3 s) |
| `params.radius` | **1** (área 3×3, distância de Chebyshev ≤ 1) |
| Persistência | Consumida |

**Efeito:** quando o pavio zera, captura **todos** os porcos cujo tile ocupado esteja a uma distância de Chebyshev ≤ 1 do TNT. Isso **inclui** porcos em `ESCALANDO`, `COMENDO` e `VOLTANDO`.

**Capacidade máxima** = nº de tiles de rota dentro do 3×3 (B3): reta = 3 · curva em L = 5 · U apertado = 6+.

### 4.3 🥣 Isca Falsa: `bait`
| Campo | Valor |
|-------|-------|
| Custo | **75** |
| Tiles | `ROUTE` (fora de tiles de nó) |
| `params.eat_ticks` | **180** (3 s) |
| Controle | O nó `from` da aresta da isca, **se** esse nó tiver ≥ 2 saídas |
| Persistência | Consumida depois de comida |

**Regras:**
- Enquanto `!consumed`, todo porco que decide no nó controlado escolhe a aresta da isca (prioridade sobre o hash, mas **não** sobre `blocked`).
- O 1º porco a entrar no tile → `COMENDO` por 180 ticks e ocupa o tile, formando fila atrás dele.
- Quando ele termina: `consumed = true`, o porco segue e o nó volta ao hash.
- Se a isca estiver numa aresta cujo `from` não é bifurcação: ela só segura (âncora), não atrai. **A UI avisa** no Blueprint: "sem bifurcação para controlar".

### 4.4 🌀 Ventilador: `fan`
| Campo | Valor |
|-------|-------|
| Custo | **125** (D015) |
| Tiles | `MARKED` |
| Direção | `Vector2i` em {(1,0), (-1,0), (0,1), (0,-1)}; girar = rotação de 90° |
| `params.range` | **3** tiles em linha (a corrente para no 1º `BLOCKED`/`MARKED`) |
| `params.push_ticks` | **15** (0,25 s de animação; o porco fica em `EMPURRADO`) |
| Persistência | A onda inteira |

**Resolução do empurrão** (porco P entra num tile `ROUTE` da corrente do ventilador F, com F fora de `P.pushed_by`):
1. `destino = tile_atual + F.dir`.
2. Conforme o tipo do destino:
   - `BARRANCO` com menos de `RAVINE_CAP = 2` porcos → P é `CAPTURADO` (animação de queda) e o contador do tile sobe. Lotado → nada acontece (D014).
   - `ROUTE` livre (sem porco e sem bigorna) → P passa para a aresta/posição desse tile (`progress` = início do tile). Se o tile for um **nó**, P decide ali.
   - Qualquer outro caso → nada acontece.
3. `P.pushed_by.append(F.id)`.

**Índice auxiliar:** pré-calcule `cell_to_edge: Dictionary[Vector2i, {edge_id, index}]` no `_ready()` da fase.

### 4.5 🛢️ Piche: `tar`
| Campo | Valor |
|-------|-------|
| Custo | **50** |
| Tiles | `ROUTE` |
| `params.speed_num / speed_den` | **1 / 2** (velocidade × 0,5) |
| Persistência | A onda inteira |

**Efeito:** enquanto P ocupa o tile, avança `speed × 1 / 2` unidades por tick (30 → 15; capacete 24 → 12). Use frações inteiras, nunca `0.5` em float.

**Tempo no tile:** `TILE_UNITS / (speed/2)` = 1200 / 15 = **80 ticks** (1,33 s), mais que o intervalo de spawn de 60 ticks, então a fila cresce **20 ticks por porco** (B2).

---

## 5. Fórmulas

### 5.1 Tempo de percurso
```
ticks_aresta = Σ (TILE_UNITS / velocidade_efetiva_no_tile)
```
Ex.: 10 tiles sem piche para o porco comum = 10 × 40 = 400 ticks (6,7 s).

### 5.2 Crescimento de fila num gargalo
```
atraso_acumulado(n) = n × max(0, tempo_no_gargalo − SPAWN_INTERVAL_TICKS)
```
- Piche: 80 − 60 = **+20 ticks por porco**
- Isca: 180 ticks para o 1º porco; os seguintes se encostam (fila de 1 por tile)

### 5.3 Economia e estrelas
```
gasto = Σ cost(armadilhas posicionadas)          # restrição: gasto ≤ budget
falha  = fugas > escape_limit
stars  = 0 se falha
       = 3 se fugas == 0 e gasto ≤ par
       = 2 se fugas == 0
       = 1 caso contrário
```

### 5.4 Eficiência de captura por moeda (referência de balanceamento)
| Armadilha | Custo | Capturas típicas | Moedas / captura |
|-----------|-------|------------------|------------------|
| Bigorna | 150 | 1 (+ desvio) | 150 |
| TNT | 200 | 2–5 | 40–100 |
| Ventilador + Barranco | 125 | 1–2 por tile de barranco | 62–125 |
| Piche | 50 | 0 (multiplicador) | — |
| Isca | 75 | 0 (controle + âncora) | — |

**Regra de balanceamento:** nenhuma solução ★★★ deve usar só um tipo de armadilha capturadora, exceto na fase 1 (tutorial).

---

## 6. Eventos (sinais) para UI, Lobo e Áudio

Defina num autoload `GameEvents.gd`:

| Sinal | Payload | Quem escuta |
|-------|---------|-------------|
| `phase_changed(state)` | `BLUEPRINT/TRANSICAO/ACAO/RESULTADO` | UI, Transição |
| `budget_changed(spent, budget)` | ints | UI |
| `trap_placed(trap_id, cell)` / `trap_removed(...)` | — | UI, Áudio |
| `trap_triggered(trap_id, cell)` | — | Áudio, VFX |
| `pig_spawned(pig_id)` | — | UI |
| `pig_captured(pig_id, cause)` | `cause`: anvil/tnt/ravine | **Lobo**, Áudio, UI |
| `pig_escaped(pig_id, exit_cell, path_edges)` | — | **Lobo**, UI (destaque da fuga) |
| `edge_blocked(edge_id)` | — | Prévia de rota |
| `wave_finished(captured, escaped, spent)` | — | Resultado |

**Lobo:** "captura múltipla" = ≥ 2 `pig_captured` em ≤ 60 ticks.

---

## 7. Fases: Dados Revisados

### 7.1 Tabela (substitui a seção 11 do GDD v0.1)

> **Atualizado:** os números finais e os mapas validados estão em `levels/level-design.md` (par por solver). A tabela abaixo é a hipótese original, mantida como histórico.
| # | Nome | Armadilhas | Porcos | Bif. | Orç. | Par | Fugas | Solução ★★★ de referência |
|---|------|-----------|--------|------|------|-----|-------|---------------------------|
| 1 | O Atalho | Bigorna | 2 | 1 | 450 | 300 | 1 | Bigorna em A + bigorna em B: o 1º porco é capturado e bloqueia o seu galho, então o 2º vai obrigatoriamente pelo outro |
| 2 | Fila Indiana | Bigorna, Piche, TNT | 5 | 1 | 550 | 450 | 1 | Bigorna fecha o galho curto → no tronco final em curva L: piche + TNT logo depois (≈ 4 capturas) |
| 3 | Cheiro de Lavagem | Isca, Piche, TNT | 6 | 2 | 600 | 475 | 1 | Isca na 1ª bifurcação puxa todos para o galho com curva → piche + TNT na curva → 2º TNT para os restantes |
| 4 | Ventania | Ventilador, Isca, Piche | 6 | 1 | 500 | 300 | 1 | Isca força o galho que passa ao lado do barranco → ventilador empurra cada porco para o barranco |
| 5 | O Banquete | Todas | 10 | 2 | 900 | 700 | 2 | Combo-assinatura em dois galhos (validar no level design) |

> ⚠️ Fases 2 a 5: as soluções de referência são **hipóteses**. Diana e Marco precisam desenhar o mapa e **simular à mão** (ou no próprio protótipo) antes de fechar os números. A fase 1 está verificada.

### 7.2 Verificação da Fase 1 (exemplo do método)
- Mapa: `SPAWN → nó F (bifurcação) → aresta A (4 tiles) / aresta B (6 tiles) → nó J (junção) → 3 tiles → SAÍDA`.
- Porcos 0 e 1, intervalo de 60 ticks.
- **Plano ★★★ (300):** bigorna em A[1] e em B[1].
  - Porco 0 escolhe pelo hash, digamos A → captura em A[1] → A bloqueada.
  - Linha do tempo (t = 0 quando o porco 0 está em F): o porco 0 anda 2 tiles até A[1] (80 ticks) → sombra (24) → a bigorna cai em **t = 104**.
  - O porco 1 chega em F em **t = 60**, quando A **ainda não** está bloqueada. Ele pode escolher A pelo hash e entrar na aresta atrás do porco 0.
  - Em t = 104, A é bloqueada → o porco 1 (na aresta A, antes da bigorna) entra em `VOLTANDO` até F → decide de novo → só sobra B → captura em B[1]. **0 fugas.** ✔
  - Se o porco 0 escolher B: simétrico. ✔ **O resultado independe da seed.**
  - ⚠️ **Consequência:** a regra `VOLTANDO` (4.1, passo 3) é **obrigatória** para a fase 1. A simplificação "todos escalam" do GDD quebra o ★★★, porque o porco 1 escalaria a bigorna e fugiria. Implementar `VOLTANDO` já no P2.
- **Plano ★ (150):** 1 bigorna no tronco antes de F → captura o porco 0; o porco 1 escala e foge → 1 fuga ≤ limite 1. ✔
- **Plano ★★ caro (300):** 2 bigornas em série no tronco → 0 fugas, gasto 300 = par, então na verdade também é ★★★. Aceitável: são duas soluções diferentes.

---

## 8. Testes de Determinismo (checklist de QA)

| # | Teste | Esperado |
|---|-------|----------|
| T1 | Rodar a fase 1 com o plano ★★★ 10 vezes | Resultado idêntico (capturas, fugas, tick final) |
| T2 | Rodar em 1x e em 2x | Resultado idêntico |
| T3 | Mover uma armadilha de lugar e voltar | As escolhas dos porcos que não passam pela armadilha não mudam |
| T4 | `choose_edge` com valores fixos | Lista de saída igual à de referência gravada no teste |
| T5 | Bigorna bloqueando a única rota | Porcos escalam; ninguém trava |
| T6 | Ventilador em direção a um tile ocupado | Nenhum empurrão; nenhum porco sobreposto |
| T7 | Isca sem bifurcação | Aviso no Blueprint; em jogo, só segura |

---

## 9. Estrutura de Pastas Sugerida (Godot)

```
res://
├── data/
│   ├── sim_constants.gd
│   ├── traps/   anvil.tres, tnt.tres, bait.tres, fan.tres, tar.tres
│   ├── pigs/    pig.tres, pig_helmet.tres
│   └── levels/  level_01.tres … level_05.tres
├── scripts/
│   ├── resources/  trap_data.gd, pig_data.gd, level_data.gd, route_node.gd, route_edge.gd, spawn_entry.gd
│   ├── sim/        simulation_runner.gd, route_graph.gd, fork_hash.gd, pig_state.gd, trap_state.gd
│   ├── game/       game_events.gd (autoload), level_controller.gd (máquina de estados)
│   └── ui/         catalog.gd, hud.gd, result_screen.gd
└── scenes/
    ├── levels/  level_01.tscn …
    ├── traps/   anvil.tscn …
    ├── pigs/    pig.tscn
    ├── wolf/    wolf.tscn
    └── ui/      catalog.tscn, hud.tscn, result.tscn
```

---

*Fichas por Theo, GD-KS Technical Game Designer*
*Próximo: Diana + Marco (level design das fases 2 a 5, validando as soluções de referência)*
