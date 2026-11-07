# Neon Stack (título provisório) - Game Design Document

> **Autora:** Diana (Game Design Director) 🎨
> **Data:** 2026-11-07 · **Versão:** 1.0 · **Status:** Draft para revisão
> **Engine:** Unreal Engine 5 (C++ para a lógica + Blueprints para o visual)
> **Entrada:** [ideation-handoff.md](../../01-ideation/ideation-handoff.md) · [mechanics-exploration.md](../../01-ideation/mechanics-exploration.md)

Este GDD é a **fonte de verdade** das regras. Todos os números marcados com 🔧 são ajustáveis e devem ficar expostos ao editor (DataAsset de configuração), nunca fixos no código.

---

## 1. Visão Geral

**Logline:** empilhe, limpe, brilhe. Cada linha vira uma explosão de luz no ritmo da música.

Neon Stack é um puzzle de blocos caindo para PC (teclado). As regras são clássicas e reconhecíveis. O diferencial é o feedback audiovisual neon/synthwave, que transforma cada jogada num pequeno espetáculo e conduz o jogador ao flow.

| Item | Valor |
|------|-------|
| Gênero | Puzzle de blocos caindo |
| Plataforma | PC Windows, teclado |
| Modo | 1 jogador, partida infinita (Maratona) |
| Sessão | 3-15 min |
| Escopo | Protótipo/MVP, dev solo, ~2-3 semanas |

---

## 2. Pillars

Toda decisão de design deve servir a pelo menos um pilar e não pode ferir nenhum.

1. **Controle impecável:** o input responde no mesmo frame. Erros são do jogador, nunca do jogo. *Teste:* "Isso pode fazer o jogador sentir que o jogo o traiu?" Se sim, corta.
2. **Cada ação brilha:** toda ação relevante tem resposta visual e sonora proporcional à sua importância. *Teste:* "O jogador sentiu essa jogada?"
3. **Flow acima de tudo:** nada interrompe o ritmo por mais de ~250 ms durante o jogo; a legibilidade vence o espetáculo. *Teste:* "Isso quebra o transe ou atrapalha a leitura do poço?"

**Anti-pilares (o que o jogo NÃO é):** sem narrativa, sem unlocks/economia, sem mudança nas regras clássicas por causa do ritmo, sem multiplayer.

---

## 3. Target Audience

- **Primário:** jogadores casuais e veteranos de puzzles de blocos que gostam de sessões curtas e de bater o próprio recorde (perfil Achiever).
- **Secundário:** quem aprecia estética synthwave e jogos com muito "juice".
- **Expectativas do público:** controles padrão do gênero, ghost piece, hold, preview e saco de 7. Ausências seriam percebidas como falhas.
- **Acessibilidade mínima:** peças distinguíveis sem depender só de cor (ver Art Bible); opções para reduzir FX e desligar shake.

---

## 4. Core Mechanics

### 4.1 Poço (Board)
- 🔧 **10 colunas × 20 linhas visíveis**, mais **2 linhas ocultas** acima (zona de spawn) = 10×22.
- Coordenadas: coluna 0-9 (esquerda→direita), linha 0-21 (baixo→cima).

### 4.2 Peças
7 peças de 4 blocos: **I, O, T, S, Z, J, L**. Cada uma tem uma cor e um padrão próprios (Art Bible).
- Nascem com o centro nas colunas 3-6, nas linhas ocultas, e descem 1 linha imediatamente (ficando visíveis).
- Orientação inicial padrão por peça (definida nas tabelas do Theo).

### 4.3 Movimento
| Ação | Tecla | Comportamento |
|------|-------|---------------|
| Mover esquerda/direita | ← / → | 1 coluna; ao segurar: espera 🔧 **DAS 170 ms**, depois repete a cada 🔧 **ARR 50 ms** |
| Girar horário | ↑ ou X | ver 4.4 |
| Girar anti-horário | Z | ver 4.4 |
| Soft drop | ↓ (segurar) | gravidade × 🔧 **20**; +1 ponto/célula |
| Hard drop | Espaço | cai até o fundo e trava na hora; +2 pontos/célula |
| Hold | C ou Shift | ver 4.7 |
| Pausa | Esc | ver 7.3 |

Os controles podem ser remapeados via Enhanced Input em versão futura (fora do MVP).

### 4.4 Rotação (simples + chute básico)
Ao girar, testa as posições nesta ordem e usa a primeira sem colisão:
1. Mesma posição
2. 1 coluna à esquerda
3. 1 coluna à direita
4. *Só a peça I:* 2 colunas à esquerda, depois 2 à direita
5. 🔧 1 linha acima (opcional, ligado por padrão)

Se nenhuma servir, a rotação é cancelada sem penalidade. A peça O não gira.

### 4.5 Gravidade e Lock Delay
- A peça desce 1 linha a cada `T(nível)` segundos (ver 5.2).
- **Lock delay:** ao tocar uma superfície, espera 🔧 **0,5 s** e trava.
  - Mover ou girar com sucesso reinicia o timer, até 🔧 **15 resets** por peça. Ao esgotar, trava no próximo contato.
  - Se a peça sair do chão (ex.: moveu para fora de uma borda), volta a cair normalmente.
- Hard drop ignora o lock delay.

### 4.6 Sorteio: saco de 7
- Embaralha as 7 peças (Fisher-Yates) e entrega em sequência; quando o saco esvazia, gera outro.
- A fila mantém sempre ≥ 3 peças visíveis no **preview**.

### 4.7 Hold
- Guarda a peça atual; se já houver uma guardada, troca as duas.
- **Uma vez por peça:** o slot bloqueia (visual apagado) até a próxima peça travar.
- A peça que sai do hold volta à orientação inicial e nasce no spawn.

### 4.8 Ghost piece
Silhueta da peça na posição de pouso do hard drop, atualizada a cada movimento.

### 4.9 Limpeza de linhas
- Ao travar, verifica as linhas que a peça ocupa; as linhas completas são removidas e as de cima descem (gravidade "ingênua": linhas descem inteiras, sem cascata de blocos soltos).
- 🔧 A animação de limpeza dura **≤ 250 ms**. A próxima peça nasce ao fim dela.

### 4.10 Game Over (Block Out / Lock Out)
- **Block out:** a nova peça nasce sobreposta a blocos existentes.
- **Lock out:** uma peça trava inteiramente nas linhas ocultas (20-21).
- Ao ocorrer: sequência de game over (blocos apagam de cima para baixo) → tela de resultados.

---

## 5. Pontuação e Progressão

### 5.1 Pontuação
| Evento | Pontos |
|--------|--------|
| 1 linha (Single) | 🔧 100 × nível |
| 2 linhas (Double) | 🔧 300 × nível |
| 3 linhas (Triple) | 🔧 500 × nível |
| 4 linhas (Quad) | 🔧 800 × nível |
| Combo | 🔧 50 × combo × nível (combo = nº de limpezas consecutivas − 1) |
| Soft drop | 1 por célula |
| Hard drop | 2 por célula |

- **Combo:** começa em −1; cada peça que limpa linhas soma +1; uma peça que trava sem limpar zera para −1. O combo só pontua a partir de 1.
- O nível usado no cálculo é o nível **antes** do level up causado pela própria limpeza.

### 5.2 Níveis e velocidade
- Sempre começa no nível **1**.
- Sobe 1 nível a cada 🔧 **10 linhas** (total acumulado).
- Tempo por linha: `T(n) = max(0.05, (0.8 - (n-1) × 0.007) ^ (n-1))` segundos 🔧
- A tabela completa por nível fica a cargo do Theo (balance/progression).
- **Sem nível máximo** no MVP; se o playtest mostrar teto injogável, limitar em 🔧 20.

### 5.3 Progressão estética
A cada 🔧 **5 níveis** muda a paleta do fundo/grade (e, se implementado, entra uma camada musical). Detalhes na Art Bible e na Audio Bible.

### 5.4 Recorde
- O high score fica salvo localmente (SaveGame), junto com linhas e nível atingidos.
- Novo recorde → destaque "NOVO RECORDE" na tela de resultados.

---

## 6. Game Feel & Feedback (resumo)

Detalhamento visual: **Aurora** (Art Bible / VFX). Áudio: **Anton** (Audio Bible).

| Evento | Visual | Áudio | Intensidade |
|--------|--------|-------|-------------|
| Mover | — | tick | mínima |
| Girar | squash leve | whoosh | mínima |
| Travar | pulso emissivo | thump | baixa |
| Hard drop | rastro + micro-shake | impacto | média |
| 1-3 linhas | desintegração em partículas | chime (tom sobe por nº de linhas) | média |
| Quad | flash + shake forte + partículas grandes | acorde | alta |
| Combo ≥ 2 | contador pulsa, grade brilha | camada musical (opcional) | escala com combo |
| Level up | troca de paleta + "LEVEL X" | sweep | média |
| Game over | blocos apagam do topo | descida grave | alta |

Regras:
- O feedback **nunca atrasa o input** (pilar 1).
- Opções de **Intensidade de FX** e **Screen shake** controlam a escala dos efeitos (seção 7.4).

---

## 7. Telas e Fluxo

### 7.1 Fluxo
```
[Tela inicial] ──Jogar──► [Jogo] ──Esc──► [Pausa] ──Continuar──► [Jogo]
      │  ▲                   │                │
   Opções │                game over        Reiniciar ──► [Jogo]
      ▼  │                   ▼                │
  [Opções]            [Resultados] ──Jogar de novo──► [Jogo]
                             │                 Sair ──► [Tela inicial]
                             └──Menu──► [Tela inicial]
```

### 7.2 Tela inicial
- Logo, **recorde atual**, botões **Jogar**, **Opções** e **Sair**.
- Sempre começa no nível 1 (sem seleção de nível no MVP).
- Navegação por teclado (setas + Enter).

### 7.3 Pausa
- Esc pausa e **cobre o poço** (evita planejar com o jogo parado). Preview e hold também ficam ocultos.
- Menu: **Continuar**, **Reiniciar**, **Opções**, **Sair para o menu**.
- Ao continuar: contagem 🔧 3-2-1 curta (≤ 1,5 s) antes de retomar.

### 7.4 Opções (salvas localmente)
| Opção | Tipo | Padrão |
|-------|------|--------|
| Volume da música | slider 0-100 | 🔧 70 |
| Volume dos SFX | slider 0-100 | 🔧 80 |
| Intensidade de FX | Baixa / Média / Alta | Alta |
| Screen shake | Liga / Desliga | Liga |
| DAS | slider 50-300 ms | 170 |
| ARR | slider 0-100 ms (0 = instantâneo) | 50 |

### 7.5 HUD (layout 16:9, poço central)
```
┌──────────────────────────────────────────────────────┐
│                  horizonte synthwave                 │
│   ┌────────┐     ┌──────────────┐     ┌────────┐     │
│   │  HOLD  │     │              │     │  NEXT  │     │
│   │  [  ]  │     │              │     │  [  ]  │     │
│   └────────┘     │              │     │  [ ]   │     │
│                  │    POÇO      │     │  [ ]   │     │
│   PONTOS         │   10 × 20    │     └────────┘     │
│   000000         │              │                    │
│   NÍVEL  1       │              │                    │
│   LINHAS 0       │              │                    │
│   RECORDE        │              │                    │
│   000000         └──────────────┘                    │
└──────────────────────────────────────────────────────┘
```
- O 1º item do preview é maior; os outros 2 menores.
- Combo ativo aparece temporariamente perto do poço ("COMBO ×3").
- Pontos de cada limpeza flutuam saindo das linhas limpas.

### 7.6 Resultados
Pontuação final, linhas, nível, "NOVO RECORDE" (se for o caso), **Jogar de novo** e **Menu**.

---

## 8. Feature List & Critérios de Aceite

| # | Feature | Prioridade | Critério de aceite |
|---|---------|-----------|--------------------|
| F1 | Board + colisão + limpeza | P1 | Peças param em blocos/bordas; linhas completas somem e as de cima descem corretamente |
| F2 | 7 peças + rotação com chute | P1 | Todas giram nos 2 sentidos; giram junto à parede quando o chute cabe; O não gira |
| F3 | Gravidade + lock delay | P1 | A peça trava após 0,5 s no chão; resets limitados a 15; hard drop trava na hora |
| F4 | Game over | P1 | Block out e lock out encerram a partida e abrem os resultados |
| F5 | Saco de 7 + preview ×3 | P2 | Em cada bloco de 7 peças sorteadas, as 7 aparecem uma vez; o preview mostra as próximas 3 |
| F6 | Hold | P2 | Guarda/troca; bloqueado até a próxima peça travar |
| F7 | Ghost piece | P2 | Sempre coincide com a posição final do hard drop |
| F8 | DAS/ARR | P2 | Segurar ← move 1, espera DAS e repete a cada ARR; valores ajustáveis |
| F9 | Pontuação + combo + níveis | P3 | Valores batem com a tabela 5.1; nível sobe a cada 10 linhas; gravidade segue T(n) |
| F10 | High score + opções salvos | P3 | Persistem após fechar e reabrir o jogo |
| F11 | Telas (inicial, pausa, opções, resultados) + HUD | P3 | Fluxo 7.1 completo, navegável só por teclado |
| F12 | Visual neon (materiais + bloom + paleta por nível) | P4 | Blocos legíveis em todos os níveis; paleta muda a cada 5 níveis |
| F13 | Juice (Niagara, shakes, flash, pontos flutuantes) | P4 | Todos os eventos da tabela 6 têm feedback; opções de FX/shake funcionam |
| F14 | SFX | P4 | Todos os eventos da tabela 6 têm som; volumes respeitam as opções |
| F15 | Música reativa (faixa única + filtro) | P5 | Música em loop com volume configurável; filtro abre com o nível e brilha com o combo (audio-bible §3.2) |

### Dependências
```
F1 ─► F2 ─► F3 ─► F4
 │           │
 └► F5 ─► F6 │
       F7 ◄──┘
F3 ─► F8
F1+F3 ─► F9 ─► F10
F9 ─► F11
F11 ─► F12 ─► F13
F13 ─► F14 ─► F15
```

---

## 9. Arquitetura de alto nível (para o Engine Team)

- **Lógica em C++, sem rendering:** Board, Piece, Randomizer, Hold, Rules/Scoring e um GameMode/Actor que faz o tick.
- **View em Blueprints:** escuta delegates (`OnPieceSpawned`, `OnPieceMoved`, `OnPieceLocked`, `OnLinesCleared`, `OnLevelUp`, `OnHoldChanged`, `OnQueueChanged`, `OnScoreChanged`, `OnGameOver`).
- **Configuração:** um `UDataAsset` com todos os 🔧.
- **Persistência:** `USaveGame` com recorde e opções.
- O visual **nunca escreve** no estado do jogo.

---

## 10. Fora do escopo (MVP)

Modos extras (Sprint/Ultra), seleção de nível inicial, remapeamento de teclas, gamepad, T-spins/back-to-back, placares online, conquistas, "poço vivo".

---

## 11. Perguntas em aberto

| Pergunta | Responsável | Bloqueia? |
|----------|-------------|-----------|
| ~~Paleta das 7 peças + padrões para daltonismo~~ | Aurora | ✅ Resolvido |
| ~~Lista final de SFX e direção musical~~ | Anton | ✅ Resolvido |
| ~~Tabela de gravidade por nível e offsets de rotação~~ | Theo | ✅ Resolvido |
| Título final e logo | Dev | Não |

---

## 12. Documentos relacionados (fase de Design)

| Documento | Responsável | Status |
|-----------|-------------|--------|
| `core-design/pillars/design-pillars.md` | Diana | ✅ v1.0 |
| `core-design/mechanics/core-mechanics.md` | Theo | ✅ v1.0 |
| `core-design/loops/gameplay-loops.md` | core-loop-specialist | ✅ v1.0 |
| `core-design/progression/progression-system.md` | Theo | ✅ v1.0 |
| `art/art-bible.md` | Aurora | ✅ v1.0 |
| `audio/audio-bible.md` | Anton | ✅ v1.0 |
| `level-design/levels.md` | Leo | ✅ v1.0 |

---

*GDD por Diana, GD-KS Game Design Director. Documento vivo: atualizar após cada playtest.*
