# Neon Stack - Core Mechanics Spec

> **Autor:** Theo (Technical Game Designer) 🔧 · apoio: Diana
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Fonte de regras:** [GDD main.md](../../gdd/main.md) §4-5 · Este documento traz **as regras em forma implementável**: dados, fórmulas, estados e eventos.
> **Público:** Engine Team (C++). Todo 🔧 vai para o DataAsset de configuração (§9).

---

## 1. Core Verb

```yaml
Core Mechanic:
  Verbo: Encaixar
  Descrição: posicionar (mover/girar) e soltar uma peça de 4 blocos no poço
  Frequência:
    Por minuto: 15-60 peças (iniciante → avançado)
    Por sessão: 100-500 peças
  Por que satisfaz:
    Visual: pulso emissivo ao travar; linhas se desintegram em partículas
    Áudio: thump ao travar, chime ao limpar
    Resultado: o poço esvazia, os pontos sobem
  Expressão de habilidade:
    Iniciante: soft drop, gira até "caber", limpa 1-2 linhas
    Expert: hard drop + ghost, planeja com preview/hold, guarda coluna para Quad, usa lock delay para deslizar peças
```

---

## 2. Sistema de coordenadas

- Grade **10 × 22** (`Width = 10`, `TotalHeight = 22`, `VisibleHeight = 20`).
- `X` = coluna 0..9 (esquerda → direita). `Y` = linha 0..21 (**baixo → cima**; Y=0 é o fundo).
- Linhas 20-21 = **zona oculta** (spawn).
- Célula vazia = `0`; célula ocupada = id da peça (1..7), usado para a cor.

---

## 3. Peças (formas e rotação)

Cada peça é definida por uma **caixa N×N** e pelos blocos ocupados na orientação inicial (estado 0). As formas abaixo são lidas **de cima para baixo**, como aparecem na tela.

| Id | Peça | Caixa | Estado 0 (spawn) |
|----|------|-------|------------------|
| 1 | I | 4×4 | `....` / `####` / `....` / `....` |
| 2 | O | 2×2 | `##` / `##` |
| 3 | T | 3×3 | `.#.` / `###` / `...` |
| 4 | S | 3×3 | `.##` / `##.` / `...` |
| 5 | Z | 3×3 | `##.` / `.##` / `...` |
| 6 | J | 3×3 | `#..` / `###` / `...` |
| 7 | L | 3×3 | `..#` / `###` / `...` |

### 3.1 Rotação dentro da caixa
- 4 estados: 0 → R (horário) → 2 → L → 0.
- **Horário:** bloco em (cx, cy) na caixa → (cy, N−1−cx), com cx, cy relativos ao canto inferior esquerdo da caixa (Y para cima).
- **Anti-horário:** (cx, cy) → (N−1−cy, cx).
- A peça **O não gira** (a rotação é ignorada sem som de falha).
- Recomendação: **pré-calcular** os 4 estados de cada peça numa tabela estática (7 × 4 × 4 offsets) em vez de girar em runtime.

### 3.2 Spawn
| Peça | Coluna esquerda da caixa | Ocupa colunas |
|------|--------------------------|---------------|
| I | 3 | 3-6 |
| O | 4 | 4-5 |
| T, S, Z, J, L | 3 | 3-5 |

- A caixa é posicionada de modo que a **linha mais alta ocupada fique em Y = 21**.
- Imediatamente após o spawn, a peça tenta descer 1 linha (fica parcialmente visível).
- Se a posição de spawn colide → **Block Out** (game over).

---

## 4. Rotação com chute básico

Ao pedir rotação, testa os deslocamentos `(dx, dy)` **em ordem** e aplica o primeiro sem colisão:

| Ordem | dx | dy | Peças |
|-------|----|----|-------|
| 1 | 0 | 0 | todas |
| 2 | −1 | 0 | todas |
| 3 | +1 | 0 | todas |
| 4 | −2 | 0 | só I |
| 5 | +2 | 0 | só I |
| 6 | 0 | +1 | todas, se 🔧 `bAllowUpKick = true` |

- Nenhum serviu → rotação **cancelada**, sem penalidade, sem som de giro (opcional: som de "falha" bem sutil).
- Rotação bem-sucedida em contato com o chão conta como **lock reset** (§6).
- **Colisão** = algum bloco fora de X∈[0,9], Y<0, ou sobre uma célula ocupada. Y>21 não colide (permite girar no topo).

---

## 5. Movimento horizontal (DAS / ARR)

| Parâmetro | Padrão 🔧 | Faixa nas opções |
|-----------|-----------|------------------|
| DAS (atraso de repetição) | 170 ms | 50-300 ms |
| ARR (intervalo de repetição) | 50 ms | 0-100 ms |

Algoritmo por direção:
1. **Pressionou:** move 1 coluna imediatamente; zera `DasTimer`.
2. **Segurando:** `DasTimer += dt`. Quando `DasTimer ≥ DAS`, passa a repetir: a cada `ARR` acumulado, move 1 coluna (pode mover várias num frame se dt for grande).
3. **ARR = 0:** ao carregar o DAS, move até a parede no mesmo frame.
4. **← e → ao mesmo tempo:** vale a **última tecla pressionada**; ao soltá-la, volta para a outra (com DAS reiniciado).
5. Movimento bloqueado por colisão não consome nada e não toca som.

---

## 6. Gravidade, soft drop, hard drop e lock delay

### 6.1 Gravidade
- `GravityInterval(nível)` em segundos (tabela em [progression-system.md](../progression/progression-system.md) §3):
  `T(n) = max(MinGravity, (0.8 − (n−1) × 0.007)^(n−1))`, com 🔧 `MinGravity = 0.05`
- Acumulador: `GravityTimer += dt`; enquanto `GravityTimer ≥ intervalo`, tenta descer 1 e subtrai o intervalo.

### 6.2 Soft drop
- Enquanto segurado: `intervalo = min(T(n) / 20, 0.033)` 🔧 (≥ ~30 linhas/s).
- **+1 ponto** por linha descida via soft drop.
- Não trava a peça instantaneamente: o lock delay continua valendo.

### 6.3 Hard drop
- Calcula a menor posição válida (a mesma do ghost), move a peça, soma **+2 pontos por linha** descida e **trava imediatamente**.

### 6.4 Lock delay
| Parâmetro | Padrão 🔧 |
|-----------|-----------|
| `LockDelay` | 0,5 s |
| `MaxLockResets` | 15 |

- **Grounded** = a peça não consegue descer 1 linha.
- Ao ficar grounded: `LockTimer` começa a contar.
- Movimento ou rotação **bem-sucedidos** enquanto grounded: `LockTimer = 0`, `LockResets++` (só se `LockResets < MaxLockResets`).
- Se a peça deixa de estar grounded, o `LockTimer` pausa/zera e a gravidade volta a agir (os resets **não** voltam).
- `LockTimer ≥ LockDelay` → **trava**.
- Com `LockResets == MaxLockResets`: trava assim que estiver grounded e o timer estourar, sem novos resets.

---

## 7. Sorteio (saco de 7), fila e hold

### 7.1 Randomizer
```
Bag = [I,O,T,S,Z,J,L] embaralhado (Fisher-Yates, RNG com seed opcional p/ testes)
Queue: sempre mantém ≥ PreviewCount (3) + 1 peças; ao faltar, concatena um novo Bag
Next() = Queue.PopFront()
```
- 🔧 `PreviewCount = 3`
- Seed fixa em modo de teste para reprodutibilidade (útil em testes automatizados).

### 7.2 Hold
```
OnHold():
  if !bCanHold: return (feedback: slot pisca apagado)
  if HoldPiece == None: HoldPiece = Current.Type; SpawnNext()
  else: swap(HoldPiece, Current.Type); Spawn(tipo trocado)
  bCanHold = false
OnPieceLocked(): bCanHold = true
```
- A peça que entra volta ao **estado 0** e posição de spawn; o spawn segue as regras de §3.2 (pode gerar Block Out).

### 7.3 Ghost
- `GhostY` = menor Y onde a peça atual (mesmo X e rotação) não colide. Recalcular após cada movimento, rotação ou mudança no tabuleiro.

---

## 8. Travamento, limpeza, pontuação e game over

### 8.1 Ao travar
1. Escreve os 4 blocos no Board.
2. Se **todos** os blocos têm Y ≥ 20 → **Lock Out** (game over).
3. Verifica as linhas completas **entre as linhas ocupadas pela peça**.
4. Se houver linhas: entra em `ClearingLines` (anim 🔧 `LineClearDelay = 0.25 s`), depois remove as linhas e desce as de cima.
5. Atualiza pontuação/combo/nível (§8.2).
6. `bCanHold = true`; spawn da próxima peça.

### 8.2 Pontuação

```
LinesClearedThisLock = k (0..4)
LevelForScore = Level atual (antes do level up)

if k > 0:
    Combo += 1                              // Combo começa em -1
    Score += LineScore[k] * LevelForScore    // LineScore = [0,100,300,500,800] 🔧
    if Combo >= 1: Score += ComboBonus * Combo * LevelForScore   // ComboBonus = 50 🔧
    TotalLines += k
    Level = StartLevel + floor(TotalLines / LinesPerLevel)       // LinesPerLevel = 10 🔧
else:
    Combo = -1

Soft drop: Score += 1 por linha · Hard drop: Score += 2 por linha
```

**Exemplos (nível 3):**
| Situação | Cálculo | Pontos |
|----------|---------|--------|
| Single sem combo | 100×3 | 300 |
| Quad | 800×3 | 2400 |
| 2ª limpeza seguida (Double), combo=1 | 300×3 + 50×1×3 | 1050 |
| 3ª seguida (Single), combo=2 | 100×3 + 50×2×3 | 600 |
| Hard drop de 15 linhas | 2×15 | 30 |

### 8.3 Game over
- **Block Out:** spawn colide (inclui o spawn vindo do hold).
- **Lock Out:** a peça trava inteiramente em Y ≥ 20.
- Ao ocorrer: estado `GameOver`, emite `OnGameOver(FinalScore, TotalLines, Level, bNewHighScore)`; salva o recorde.

---

## 9. Máquina de estados

```
            ┌──────────┐ Start
            │  Ready   │────────┐
            └──────────┘        ▼
                          ┌──────────┐ colisão no spawn
     ┌───────────────────►│ Spawning │──────────────────► GameOver
     │                    └────┬─────┘
     │                         ▼
     │                    ┌──────────┐  não desce  ┌──────────┐
     │                    │ Falling  │────────────►│ Grounded │ (lock timer)
     │                    │          │◄────────────│          │
     │                    └────┬─────┘   desceu    └────┬─────┘
     │                 hard drop│                       │ timer estourou
     │                         ▼                        ▼
     │                    ┌────────────────────────────────┐ lock out
     │                    │            Locking             │─────────► GameOver
     │                    └────┬────────────────────┬──────┘
     │               sem linhas│                    │ k>0
     │                         │              ┌─────▼──────┐
     └─────────────────────────┴──────────────│ClearingLines│ (0,25 s)
                                              └─────────────┘
  Paused: congela tudo (timers não avançam); entra/sai de qualquer estado exceto GameOver.
```

- **Input aceito:** mover/girar/hold/drops só em `Falling` e `Grounded`. Durante `ClearingLines`, as entradas de movimento **são guardadas** apenas para o DAS (segurar ← durante a animação já carrega o DAS para a próxima peça).

---

## 10. Eventos (C++ → Blueprints)

Delegates dinâmicos `BlueprintAssignable`. O visual **só escuta**.

| Evento | Parâmetros | Uso principal |
|--------|-----------|---------------|
| `OnPieceSpawned` | Type, Cells[4] | desenhar peça ativa |
| `OnPieceMoved` | Cells[4], GhostCells[4], bByGravity | atualizar peça/ghost; SFX tick (se não for gravidade) |
| `OnPieceRotated` | Cells[4], GhostCells[4], KickIndex | SFX whoosh, squash |
| `OnPieceHardDropped` | FromY, ToY, Cells[4] | rastro, micro-shake |
| `OnPieceLocked` | Type, Cells[4] | pulso emissivo, thump |
| `OnLinesCleared` | Rows[], Count, Combo, ScoreGained | partículas, chime/acorde, pontos flutuantes |
| `OnBoardChanged` | (nenhum: a view lê o Board) | redesenhar blocos fixos |
| `OnScoreChanged` | Score, Lines, Level | HUD |
| `OnLevelUp` | NewLevel | troca de paleta, "LEVEL X" |
| `OnHoldChanged` | HeldType, bCanHold | slot de hold |
| `OnQueueChanged` | Next[3] | preview |
| `OnPausedChanged` | bPaused | cobrir poço, menu |
| `OnGameOver` | Score, Lines, Level, bNewHighScore | sequência final, resultados |

---

## 11. Configuração (DataAsset `UNeonStackConfig`)

| Campo | Tipo | Padrão |
|-------|------|--------|
| BoardWidth / VisibleHeight / HiddenRows | int | 10 / 20 / 2 |
| PreviewCount | int | 3 |
| DAS / ARR (padrão) | float (s) | 0.170 / 0.050 |
| SoftDropFactor / SoftDropMaxInterval | float | 20 / 0.033 |
| LockDelay / MaxLockResets | float / int | 0.5 / 15 |
| bAllowUpKick | bool | true |
| LineClearDelay | float (s) | 0.25 |
| LineScore | int[5] | 0,100,300,500,800 |
| ComboBonus | int | 50 |
| SoftDropPointsPerCell / HardDropPointsPerCell | int | 1 / 2 |
| LinesPerLevel / StartLevel | int | 10 / 1 |
| GravityBase / GravityLevelStep / MinGravity | float | 0.8 / 0.007 / 0.05 |
| MaxLevel | int (0 = sem teto) | 0 |
| PaletteEveryNLevels | int | 5 |
| RandomSeed | int (0 = aleatória) | 0 |

---

## 12. Interações entre mecânicas

|  | Mover | Girar | Gravidade | Lock delay | Hold | Ghost | Hard drop |
|--|-------|-------|-----------|-----------|------|-------|-----------|
| **Mover** | – | ✓ (chute) | E: deslizar sob saliências | ✓ reset | – | ✓ atualiza | ✓ |
| **Girar** |  | – | E: girar no chão | ✓ reset | – | ✓ atualiza | ✓ |
| **Lock delay** |  |  | ✓ | – | – | – | X ignora |
| **Hold** |  |  | – | – | – | ✓ | X bloqueado após uso |

✓ projetado · E emergente (habilidade avançada) · X bloqueado de propósito

---

## 13. Casos de teste (para a lógica em C++)

| # | Cenário | Esperado |
|---|---------|----------|
| T1 | Peça T encostada na parede esquerda, girar horário | Gira com chute +1 se necessário |
| T2 | Peça I vertical na coluna 0, girar | Usa o chute +2 ou +1 |
| T3 | Peça O, girar | Nada muda, sem evento de rotação |
| T4 | 4 linhas completas com uma I vertical | k=4; Score += 800×nível |
| T5 | Linhas 0 e 2 completas, a 1 não | Remove 0 e 2; linha 1 desce para Y=0 |
| T6 | 16 movimentos no chão | Trava após o 15º reset + 0,5 s |
| T7 | Hold duas vezes na mesma peça | O 2º é ignorado |
| T8 | 14 peças seguidas com seed fixa | Cada grupo de 7 contém as 7 peças |
| T9 | Spawn sobre blocos | Block Out → GameOver |
| T10 | Hard drop de Y=18 para Y=0 | +36 pontos (18 linhas × 2) e trava |
| T11 | Combo: 3 limpezas seguidas, nível 1 | 2ª soma +50, 3ª soma +100 |
| T12 | Nível 1 → 10 linhas limpas | Level = 2; a pontuação da limpeza usou o nível 1 |

---

*Spec por Theo, GD-KS Technical Game Designer.*
