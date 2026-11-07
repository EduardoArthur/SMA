# Mechanics Exploration Document

## Game: Neon Stack (título provisório)
## Date: 2026-11-07
## Explorer: Mira (Mechanics Explorer) ⚙️
## Base: [concept-brief.md](concept-brief.md)

---

## Decisões tomadas nesta sessão

| Tema | Decisão | Motivo |
|------|---------|--------|
| Rotação | Simples + chute básico (testa 0, -1, +1, depois -2/+2 só para a peça I) | Justa perto das paredes sem tabelas de kick complexas |
| Sorteio | Saco de 7 | Evita secas longas; favorece o flow |
| MVP extras | Ghost piece, preview (3 próximas), hold, lock delay | Ferramentas que dão controle e reduzem frustração |
| Implementação | C++ para lógica (grade, regras, pontuação) + Blueprints para visual/juice | Lógica testável e determinística; juice iterável no editor |

---

## Core Verb

### Primary Action
**O jogador primariamente ENCAIXA.**

1. Girar uma peça L para fechar um buraco no canto
2. Deixar uma coluna livre esperando a peça I para limpar 4 linhas
3. Fazer hard drop da peça no lugar apontado pela ghost piece

### Supporting Verbs
| Verbo | Frequência | Contexto |
|------|-----------|---------|
| Mover (esq/dir) | Constante | Posicionar a peça |
| Girar | Constante | Ajustar orientação |
| Soltar (soft/hard drop) | Frequente | Acelerar a jogada |
| Guardar (hold) | Ocasional | Salvar uma peça para depois / trocar uma ruim |
| Planejar | Constante | Olhar o preview e preparar o poço |

---

## Core Loop

### Loop Diagram
```
┌──────────────────────────┐
│ AÇÃO                     │
│ mover / girar / hold     │
│ → soltar a peça          │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│ FEEDBACK                 │
│ pulso de luz + "thump"   │
│ ao travar a peça         │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│ RECOMPENSA               │
│ linhas somem em          │
│ partículas, pontos,      │
│ combo, camada musical    │
└────────────┬─────────────┘
             ▼
┌──────────────────────────┐
│ DECISÃO                  │
│ olhar preview: onde vai  │
│ a próxima? usar hold?    │
└────────────┬─────────────┘
             └──────────► (volta para AÇÃO)
```

### Loop Details

**AÇÃO:** posicionar e soltar a peça
- Input: ←/→ mover, ↑ ou X girar horário, Z girar anti-horário, ↓ soft drop, Espaço hard drop, C/Shift hold, Esc pausa
- Habilidade: leitura espacial rápida e planejamento 1-3 peças à frente

**FEEDBACK:**
- Visual: peça trava com pulso emissivo; ghost piece sempre visível
- Áudio: "tick" ao mover, "whoosh" ao girar, "thump" grave ao travar
- Háptico: n/a (teclado)

**RECOMPENSA:**
- Imediata: linhas explodindo, pontos flutuando, flash em 4 linhas
- Tardia: subir de nível (paleta nova), bater o recorde

**MOTIVAÇÃO:**
- Intrínseca: flow, satisfação de encaixe perfeito, maestria
- Extrínseca: high score

### Loop Timing
- Uma peça: 1-4 s (cai para <1 s em níveis altos)
- Peças por sessão: 100-500
- Sessão alvo: 3-15 min

---

## Secondary Loops

### Micro Loop (momento a momento)
`[ver peça] → [mover/girar] → [ver ghost] → [soltar]`, de 1 a 4 s.

### Session Loop
`[início calmo] → [poço cresce] → [velocidade sobe, música cheia] → [clímax/transe] → [game over + placar]`, de 3 a 15 min.

### Meta Loop
`[recorde] → [tentar de novo] → [melhorar estratégia] → [novo recorde]`

---

## Core Mechanics

1. **Queda e controle de peças:** mover, girar (com chute básico), soft drop, hard drop
2. **Limpeza de linhas:** linhas completas somem; 1-4 linhas por vez, com pontuação crescente
3. **Progressão de velocidade:** nível a cada 10 linhas, gravidade exponencial
4. **Ferramentas de controle:** saco de 7, preview ×3, hold, ghost piece, lock delay

## Regras detalhadas (proposta para o GDD)

### Poço
- 10 colunas × 20 linhas visíveis + 2 linhas ocultas acima (spawn)
- Peças nascem centralizadas no topo (colunas 3-6)

### Peças
7 peças de 4 blocos: I, O, T, S, Z, J, L. Cada uma tem cor neon própria (definir na Art Bible).

### Rotação (simples + chute básico)
1. Tenta girar no lugar.
2. Se colidir, testa deslocar −1, +1 coluna.
3. Só para a peça I: testa também −2, +2.
4. (Opcional) testa subir 1 linha para sair do chão.
5. Se tudo falhar, a rotação é cancelada.
- A peça O não gira.

### Sorteio (saco de 7)
Embaralha as 7 peças (Fisher-Yates), entrega em ordem e reabastece quando o saco esvazia. O preview lê da fila, que mantém sempre pelo menos 3 peças.

### Gravidade e níveis
- Nível sobe a cada **10 linhas**.
- Tempo por linha de queda (s): `max(0.05, (0.8 - (nível-1) × 0.007) ^ (nível-1))`
  - Nível 1 = 1,00 s · nível 5 ≈ 0,36 s · nível 10 ≈ 0,06 s… (ajustar no playtest; talvez limitar a progressão até ~nível 15)
- Soft drop: 20× a gravidade atual.

### Lock delay
- Ao tocar o chão, a peça espera **0,5 s** antes de travar.
- Mover/girar com sucesso reinicia o timer, no máximo **15 vezes** por peça (evita "girar para sempre").
- Hard drop trava imediatamente.

### Hold
- Guarda a peça atual (ou troca com a guardada).
- Só 1 vez por peça; libera ao travar a próxima.
- A peça volta à orientação inicial.

### Controles (sensação)
- **DAS** (atraso antes de repetir o movimento ao segurar): ~170 ms
- **ARR** (intervalo da repetição): ~50 ms
- Valores expostos para ajuste no editor.

### Pontuação
| Evento | Pontos |
|--------|--------|
| 1 linha | 100 × nível |
| 2 linhas | 300 × nível |
| 3 linhas | 500 × nível |
| 4 linhas | 800 × nível |
| Combo (limpezas consecutivas) | +50 × combo × nível |
| Soft drop | 1 por célula |
| Hard drop | 2 por célula |

### Game over
Uma nova peça nasce sobreposta a blocos existentes ("block out") ou trava totalmente acima da área visível.

---

## Systems Inventory

### Core Systems (C++)

#### 1. Board (grade)
- **Propósito:** guardar o estado 10×22, testar colisão, detectar e remover linhas
- **Complexidade:** Baixa · **Prioridade:** Crítica · **Dependências:** nenhuma

#### 2. Piece Controller (peça ativa)
- **Propósito:** mover, girar (com chutes), gravidade, lock delay, hard/soft drop, ghost
- **Complexidade:** Média · **Prioridade:** Crítica · **Dependências:** Board, Input

#### 3. Randomizer (saco de 7 + fila)
- **Propósito:** fornecer a próxima peça e alimentar o preview
- **Complexidade:** Baixa · **Prioridade:** Crítica · **Dependências:** nenhuma

#### 4. Game Rules / Scoring
- **Propósito:** pontos, linhas, nível, gravidade, combo, game over, high score
- **Complexidade:** Baixa · **Prioridade:** Crítica · **Dependências:** Board, Piece Controller

### Supporting Systems

#### 5. Hold
- **Complexidade:** Baixa · **Prioridade:** Alta · **Pode simplificar:** Sim

#### 6. Input (Enhanced Input + DAS/ARR)
- **Complexidade:** Média · **Prioridade:** Alta · **Pode simplificar:** Sim (sem DAS no início)

#### 7. Save (high score)
- **Complexidade:** Baixa · **Prioridade:** Alta · USaveGame com o recorde local

### Visual / Juice Systems (Blueprints)

#### 8. Board View
- Renderiza blocos (Instanced Static Mesh ou sprites), ghost, preview, hold
- Escuta eventos do C++, sem lógica própria

#### 9. Juice / FX
- Niagara (limpeza de linha), camera shake, flash, pontos flutuantes, pulso da grade
- **Prioridade:** Alta (é o gancho), mas vem *depois* do loop jogável

#### 10. Música reativa (nice-to-have)
- Faixa em camadas (stems) ligadas por combo/nível
- **MVP:** Não (só se sobrar tempo)

---

## Systems Interaction Map

```
          ┌───────────┐
 Input ──►│  Piece    │──── colisão ───►┌──────────┐
          │ Controller│◄─── estado ─────│  Board   │
          └─────┬─────┘                 └────┬─────┘
                │ peça travada                │ linhas limpas
                ▼                             ▼
          ┌───────────┐  próxima peça   ┌───────────┐
          │Randomizer │────────────────►│Game Rules │──► Save (high score)
          │  + Hold   │                 │ /Scoring  │
          └───────────┘                 └─────┬─────┘
                                              │ eventos (delegates C++)
        ══════════════ fronteira C++ │ Blueprints ══════════════
                                              ▼
                      ┌────────────┐   ┌────────────┐   ┌──────────────┐
                      │ Board View │   │ Juice / FX │   │ Música/Áudio │
                      └────────────┘   └────────────┘   └──────────────┘
```

### Key Interactions
1. O **Piece Controller** consulta o **Board** para colisão; ao travar, escreve a peça no Board.
2. O **Board** informa as linhas limpas → **Game Rules** calcula pontos, combo, nível e nova gravidade → volta para o Piece Controller.
3. O **C++ emite eventos** (`OnPieceLocked`, `OnLinesCleared(rows, count)`, `OnLevelUp`, `OnGameOver`, `OnHoldChanged`, `OnQueueChanged`). Os Blueprints só escutam. **A regra de ouro: o visual nunca altera o estado do jogo.**

---

## Depth Analysis

**Alta profundidade, baixa complexidade.** São só 7 peças e 6 comandos.

Fontes de profundidade:
- ✅ **Trade-off risco/recompensa:** guardar uma coluna para a peça I (800×) vs. limpar já com segurança
- ✅ **Hold** cria decisões situacionais (salvar a I ou se livrar de uma S/Z ruim)
- ✅ **Preview** permite planejar 2-3 jogadas à frente
- ✅ **Lock delay** premia a destreza em velocidades altas
- ✅ **Combo** recompensa limpezas seguidas

Sinais de alerta: nenhum. Não há recursos extras nem regras com exceções além do chute da peça I.

---

## Feel & Juice

### Core Action Feel
**Sensação alvo:** "precisa e responsiva, com impacto luminoso". O input nunca pode atrasar; o juice reage *depois* da ação, não antes.

**Referências:**
1. Puzzles de blocos competitivos: resposta instantânea de controle (DAS/ARR)
2. Tetris Effect: limpeza de linhas como espetáculo audiovisual

### Feedback Requirements

| Ação | Visual | Áudio | Timing |
|------|--------|-------|--------|
| Mover | — (ou leve rastro) | tick curto | 0 ms |
| Girar | leve "squash" da peça | whoosh | 0 ms |
| Travar | pulso emissivo nos 4 blocos | thump grave | 0-100 ms |
| Hard drop | rastro vertical + micro-shake | impacto | 0-80 ms |
| 1-3 linhas | linha se desintegra em partículas | chime crescente (tom sobe com nº de linhas) | ~200 ms |
| 4 linhas | flash de tela + shake forte + partículas grandes | acorde | ~350 ms |
| Combo | contador pulsa, grade brilha mais | camada musical entra | junto |
| Level up | troca de paleta + texto "LEVEL X" | sweep | ~500 ms |
| Game over | blocos apagam de cima para baixo | descida grave | ~1,5 s |

### Polish Notes
- **Animação de limpeza:** não pausar o jogo por mais de ~250 ms (cuidado com o flow).
- **Screen shake:** só em hard drop (leve) e em 4 linhas (forte); opção de desligar.
- **Bloom:** forte nos blocos e moderado na grade; legibilidade acima de tudo.

---

## Progression Systems

- **Jogador:** baseada em habilidade; não há unlocks.
- **Dificuldade:** gravidade exponencial por nível; muda a cada 10 linhas. Sem rubber-banding.
- **Conteúdo:** paleta e camada musical mudam a cada ~5 níveis (recompensa estética).

---

## Risk Analysis

### Technical Risks
| Risco | Prob. | Impacto | Mitigação |
|------|-------|---------|-----------|
| Curva de C++ no UE5 para iniciante | A | M | Lógica pequena e isolada (UObject/Actor puro); specs detalhadas do Engine Team; manter a opção de fazer em BP |
| Timing de input ruim (tick/DAS) | M | A | Lógica em tempo acumulado (delta), não em frames; valores expostos |
| Juice atrasando a lógica | M | M | Eventos assíncronos; o visual nunca bloqueia o estado |

### Design Risks
| Risco | Prob. | Impacto | Mitigação |
|------|-------|---------|-----------|
| Excesso de efeitos prejudica a leitura | M | A | Opção de intensidade de FX; playtest em nível alto |
| Curva de velocidade muito íngreme | M | M | Tabela ajustável via DataAsset/config |

### Balance Risks
| Risco | Prob. | Impacto | Mitigação |
|------|-------|---------|-----------|
| Lock delay exploitável | B | B | Limite de 15 resets |

---

## Prototype Priorities

### Priority 1: Board + peça + regras (C++)
- **Objetivo:** provar que o loop básico funciona
- **Escopo:** Board 10×22, 7 peças, mover/girar/cair, travar, limpar linhas, game over; visual cru (cubos)
- **Sucesso:** dá para jogar uma partida completa até o game over
- **Estimativa:** 3-5 dias

### Priority 2: Ferramentas de controle
- **Objetivo:** controle justo e responsivo
- **Escopo:** saco de 7, preview ×3, hold, ghost, lock delay, hard/soft drop, DAS/ARR
- **Sucesso:** jogar no nível 10 parece justo; erros são do jogador, não do jogo
- **Estimativa:** 2-4 dias

### Priority 3: Pontuação + progressão + HUD
- **Escopo:** pontos, combo, níveis, gravidade, high score salvo, HUD, pausa, reiniciar
- **Estimativa:** 1-2 dias

### Priority 4: Neon + juice (Blueprints)
- **Escopo:** materiais emissivos + bloom, Niagara de limpeza, shakes, SFX, troca de paleta
- **Sucesso:** limpar 4 linhas dá vontade de fazer de novo
- **Estimativa:** 3-5 dias

### Priority 5 (opcional): Música reativa
- **Estimativa:** 2-3 dias

**Total estimado:** cerca de 2 a 3 semanas, dentro do prazo do protótipo.

---

## Reference Games

| Jogo | O que aprender | O que evitar |
|------|----------------|--------------|
| Puzzles de blocos clássicos/competitivos | Responsividade, saco de 7, lock delay | Copiar nome, logo, visual ou música |
| Tetris Effect | Juice e música reativa como recompensa | Escopo sinestésico grande |
| Lumines | Integração ritmo + puzzle | Mudar regras base por causa do ritmo |

---

## Open Questions

1. (Design) Pausa visível com o poço aparecendo, ou tela coberta?
2. (Design) Tela inicial: só "Jogar" + recorde, ou escolha de nível inicial?
3. (Engine) Renderização dos blocos: Instanced Static Mesh 3D ou Paper2D/UMG? (recomendação: ISM com câmera ortográfica)
4. (Engine) Onde vive a lógica: `AActor` de tabuleiro ou `UObject` puro + `AActor` de view? (recomendação: classe C++ pura testável + Actor fino)
5. (Design) Título final

---

## Recommendations

### For Design Team
1. O GDD deve manter as regras desta seção como fonte de verdade, com todos os números ajustáveis.
2. A Art Bible precisa definir a paleta neon das 7 peças pensando em daltonismo (forma + brilho, não só cor).

### For Engine Team
1. Separar a lógica (C++, sem dependência de rendering) da view (Blueprints), comunicando por delegates dinâmicos (`BlueprintAssignable`).
2. Expor ao editor gravidade, DAS/ARR, lock delay e pontuação (`UPROPERTY(EditAnywhere)` ou DataAsset).
3. Usar Enhanced Input; tempo acumulado por delta, não por frame.

---

*Exploration by Mira, GD-KS Mechanics Explorer*
*Generated: 2026-11-07*
