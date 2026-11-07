# Mechanics Exploration Document

## Jogo: Trap Builder
## Data: 2026-11-07
## Explorer: Mira (Mechanics Explorer ⚙️)
## Entradas: `concept-brief.md` v1.0, `00-input/GDD-Toon-Defense.pdf`

---

## Decisões de Mecânica (tomadas nesta sessão)

| ID | Tema | Decisão |
|----|------|---------|
| M1 | Bifurcações | **RNG com seed fixa por fase.** Parece aleatório, mas repete igual a cada tentativa |
| M2 | Derrota | **Limite de fugas por fase.** Ex.: até N porcos podem escapar |
| M3 | Posicionamento | **Tiles de rota + tiles marcados** (laterais especiais) |
| M4 | Armadilhas do protótipo | **Bigorna, TNT, Isca Falsa, Ventilador, Piche** (5) |

---

## Verbo Central

### Ação Primária
**O jogador primariamente PLANEJA (posiciona armadilhas no grid).**

Exemplos:
1. Colocar uma bigorna logo após a bifurcação para fechar o atalho depois do primeiro porco.
2. Pôr a isca no fim do caminho mais longo para atrair a fila até a zona do TNT.
3. Ligar um ventilador num tile lateral para empurrar porcos para dentro de uma poça de piche.

### Verbos de Suporte
| Verbo | Frequência | Contexto |
|-------|-----------|----------|
| Analisar (ler rotas e bifurcações) | Constante | Fase Blueprint |
| Comprar (escolher no catálogo) | Frequente | Fase Blueprint, limitado pelo orçamento |
| Assistir | Constante | Fase de Ação (sem input) |
| Ajustar (refazer o plano) | Frequente | Após falha ou para buscar mais estrelas |

---

## Core Loop

### Diagrama
```
┌──────────────────────────────┐
│ AÇÃO: BLUEPRINT              │
│ Ler rotas, comprar e         │
│ posicionar armadilhas        │
└──────────────┬───────────────┘
               │  [JOGAR]
               ▼
┌──────────────────────────────┐
│ FEEDBACK: FASE DE AÇÃO       │
│ Planta se desfaz; porcos     │
│ entram; reações em cadeia    │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ RECOMPENSA                   │
│ Porcos capturados, fugas     │
│ contadas, 1 a 3 estrelas     │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ MOTIVAÇÃO                    │
│ "Dá para fazer mais barato?" │
│ "Se eu mover a bigorna..."   │
└──────────────┬───────────────┘
               └──────────► (volta ao Blueprint, com o plano anterior preservado)
```

### Detalhes do Loop

**AÇÃO (Blueprint):**
- Input: selecionar a armadilha no catálogo → tocar ou clicar num tile válido → (girar a direção, se aplicável) → remover ou mover livremente.
- Habilidade: prever rotas, gerir orçamento, encadear efeitos.
- Informação visível: rotas iluminadas, pontos de spawn, saídas, bifurcações, tiles marcados, orçamento restante, limite de fugas.

**FEEDBACK (Ação):**
- Visual: transição planta azul → cenário colorido; impacto da bigorna; explosão; porcos escorregando no piche; vento do ventilador.
- Áudio: apito de queda, "BOOM" abafado, "boing", grunhidos de porco.
- Háptico (mobile, depois): vibração curta em explosões e impactos.

**RECOMPENSA:**
- Imediata: cada porco capturado gera um contador "+1 jantar" com efeito cômico.
- Ao fim da fase: estrelas e "Jantar servido!" ou "Fugiram!".

**MOTIVAÇÃO:**
- Intrínseca: maestria (otimizar) e curiosidade (combos).
- Extrínseca: estrelas e desbloqueio de fases e armadilhas.

### Tempo do Loop
- Blueprint: 1 a 3 minutos
- Ação: 30 a 90 segundos (com botão de acelerar 2x)
- Fase completa (várias tentativas): 3 a 8 minutos
- Sessão-alvo: 10 a 20 minutos (bom para mobile)

---

## Loops Secundários

### Micro Loop (momento a momento, na Ação)
```
Porco entra no tile → armadilha dispara → reação (morte / desvio / atraso) → próximos porcos recalculam
```
Duração: 1 a 3 s. Exemplo: a bigorna cai, os porcos seguintes param, "pensam" (ícone "?") e viram para o outro galho.

### Loop de Sessão
```
Nova fase → 1ª tentativa (descobrir) → falha ou passa → otimizar → 3 estrelas → próxima
```

### Meta Loop (entre sessões)
```
Estrelas → desbloquear fase ou armadilha → nova combinação → dominar → estrelas
```

---

## Sistemas

### Sistemas Centrais (obrigatórios)

#### S1: Grid & Tiles
- **Propósito:** espaço do jogo. Tipos de tile: `ROTA`, `MARCADO` (lateral especial), `BLOQUEADO` (decoração), `SPAWN`, `SAÍDA`.
- **Interação:** o jogador posiciona armadilhas em `ROTA` ou `MARCADO`, conforme a armadilha.
- **Complexidade:** Baixa · **Prioridade:** Crítica · **Depende de:** nada.

#### S2: Rotas & Bifurcações (Pathfinding)
- **Propósito:** grafo de rotas pré-definidas com nós de bifurcação; os inimigos seguem arestas.
- **Regra (M1):** em cada bifurcação, o porco escolhe entre os galhos **disponíveis** usando RNG determinístico.
  - **Recomendação técnica:** a escolha deve ser `hash(seed_da_fase, id_do_porco, id_da_bifurcação)`, e **não** uma sequência única de RNG. Assim, mudar a posição de uma armadilha não "reembaralha" as escolhas dos outros porcos, e o jogador consegue raciocinar sobre o próprio plano.
  - **Galho bloqueado** (bigorna caída) é removido das opções → o hash escolhe entre os restantes.
  - **Isca** sobrescreve o RNG: na bifurcação anterior à isca, todos escolhem o galho que leva a ela.
- **Complexidade:** Média · **Prioridade:** Crítica · **Depende de:** S1.

#### S3: Armadilhas
- **Propósito:** as ferramentas do jogador. Cada armadilha tem: custo, tiles válidos, gatilho, efeito, persistência e direção (opcional).
- **Complexidade:** Média · **Prioridade:** Crítica · **Depende de:** S1, S2, S4.

#### S4: Inimigos (Porcos)
- **Propósito:** agentes que andam pela rota até a saída.
- **Estados:** `ANDANDO`, `LENTO` (piche), `EMPURRADO` (ventilador), `ATRAÍDO` (isca, comendo), `CAPTURADO`, `FUGIU`.
- **Protótipo:** porco comum. Opcional: porco de capacete (imune à bigorna).
- **Complexidade:** Média · **Prioridade:** Crítica.

#### S5: Fases (Blueprint ↔ Ação)
- **Propósito:** máquina de estados `BLUEPRINT → AÇÃO → RESULTADO → BLUEPRINT`.
- **Regra:** ao voltar ao Blueprint, **o plano anterior é preservado**. Isso é essencial para o loop de iteração.
- **Complexidade:** Baixa · **Prioridade:** Crítica.

### Sistemas de Suporte

#### S6: Economia & Pontuação
- **Orçamento fixo por fase**, sem renda durante a onda.
- **Derrota (M2):** se as `fugas` passarem do `limite_de_fugas` da fase → falha.
- **Estrelas (proposta para validar no Design):**
  - ★ passou (fugas ≤ limite)
  - ★★ nenhuma fuga
  - ★★★ nenhuma fuga **e** gasto ≤ `orçamento_par` (meta de eficiência da fase)
- **Complexidade:** Baixa · **Prioridade:** Alta · **Pode simplificar:** sim (só passou/falhou no 1º build).

#### S7: Catálogo (UI de compra)
- Lista das armadilhas com custo; desabilita o que não cabe no orçamento.
- **Complexidade:** Baixa · **Prioridade:** Alta.

### Fora do Protótipo
Patos voadores, veículos, armaduras extras, banana, mola, ímã, foguete, urso, buraco, pavio, corda, progressão e desbloqueios, save/load.

---

## Armadilhas do Protótipo

| Armadilha | Tipo | Tiles | Gatilho | Efeito | Persistência | Custo* |
|-----------|------|-------|---------|--------|--------------|--------|
| **Bigorna** | Uso único / rota | ROTA | 1º porco no tile | Captura 1 porco; vira **obstáculo** que bloqueia a aresta | Permanece como bloqueio | 150 |
| **TNT** | Uso único / área | ROTA | Porco no tile | Explosão em raio (ex.: 3×3) que captura todos na área | Some | 200 |
| **Isca Falsa** | Uso único / atração | ROTA | Passiva (bifurcação anterior) + 1º porco que come | Força a escolha do galho; o porco para X s comendo | Some após ser comida | 75 |
| **Ventilador** | Contínuo / comportamento | MARCADO | Sempre ligado na Ação | Empurra porcos em uma direção (alcance de N tiles); não causa dano | Toda a onda | 125 |
| **Piche** | Contínuo / atraso | ROTA | Porco no tile | Reduz a velocidade (ex.: 50%) enquanto ele estiver no tile | Toda a onda | 50 |

*Custos ilustrativos, para balancear no Design.

### Pontos a definir no Design
- **Ventilador empurra para onde?** Se empurrar para fora da rota, para um tile `BLOQUEADO` ou `MARCADO`, o porco "cai fora" (é capturado)? Ou só volta para a rota? *Sugestão: empurrar para um tile não-rota = cair no barranco = captura. Isso cria o combo ventilador + bordas do mapa.*
- **Bigorna bloqueando o único caminho:** o que acontece se não houver galho alternativo? *Sugestão: o UI impede posicionar uma bigorna que isole a saída (regra estilo "não pode fechar o labirinto") ou os porcos passam por cima dela. A decidir.*
- **Isca + bigorna:** se a bigorna bloquear o galho da isca, a isca deixa de atrair.

---

## Mapa de Interação entre Sistemas

```
             ┌──────────────┐
             │  S6 Economia │ limita
             └──────┬───────┘
                    ▼
┌──────────┐   ┌──────────────┐  dispara  ┌──────────────┐
│ S1 Grid  │──►│ S3 Armadilhas│◄─────────►│ S4 Porcos    │
└────┬─────┘   └──────┬───────┘  afeta    └──────┬───────┘
     │                │ bloqueia / atrai          │ consulta
     │                ▼                           ▼
     │         ┌──────────────┐  escolha  ┌──────────────┐
     └────────►│ S2 Rotas     │──────────►│ RNG seed     │
               └──────────────┘           └──────────────┘
                    ▲
             ┌──────┴───────┐
             │ S5 Fases     │ controla quando tudo roda
             └──────────────┘
```

### Interações-chave (onde mora a profundidade)
1. **Bigorna → Rotas:** remove um galho → redistribui a fila → **afunilamento**.
2. **Isca → Rotas:** sobrescreve o RNG → **controle determinístico** de um galho.
3. **Piche → TNT:** os porcos lentos se **acumulam** → mais porcos no raio do TNT.
4. **Ventilador → Rotas/Grid:** desloca porcos → pode empurrar para fora da rota ou para dentro de outra armadilha.
5. **Isca → TNT:** o porco parado comendo vira **âncora** para a fila se acumular.

> **Combo-assinatura do protótipo:** bigorna desvia a fila → piche desacelera → isca segura o primeiro → TNT pega o grupo inteiro. Se esse combo for divertido, o jogo funciona.

---

## Análise de Profundidade

**Fontes de profundidade (✅):**
- Armadilhas que **interagem** entre si (5 interações relevantes com só 5 armadilhas).
- Trade-offs reais: TNT caro e certeiro **contra** piche + isca baratos e frágeis.
- Várias soluções válidas por fase, com estrelas premiando a mais eficiente.
- Consequências adiadas: a bigorna só muda a rota *depois* do primeiro porco.

**Alertas de complexidade (⚠️) e mitigação:**
- **RNG invisível:** mitigado pela seed fixa. Ainda vale considerar mostrar a escolha dos porcos no replay.
- **Recálculo de rota difícil de prever:** no Blueprint, ao posicionar a bigorna, mostrar uma prévia tracejada da rota que fica.
- **Muitos números:** manter só custo, orçamento e limite de fugas na UI.

---

## Feel & Juice

**Feel-alvo:** *slapstick com peso*. Antecipação → impacto → exagero.

**Referências:**
1. Cartoons clássicos de perseguição: antecipação (sombra crescendo antes da bigorna), "squash & stretch".
2. *Poly Bridge*: o prazer de assistir o plano funcionar (ou desabar).

| Ação | Visual | Áudio | Timing |
|------|--------|-------|--------|
| Transição Blueprint → Ação | Linhas brancas se desfazem, cores preenchem | "Swoosh" + início da trilha | 600–800 ms |
| Bigorna | Sombra cresce → queda → porco achatado | Apito descendente + "CLANG" | ~400 ms de sombra |
| TNT | Pavio curto → flash → nuvem de fumaça cartoon | "BOOM" abafado | Pavio 300 ms |
| Piche | Porco afunda os pés, passos lentos | "Squelch" | Contínuo |
| Ventilador | Linhas de vento, orelhas do porco balançando | Zumbido | Contínuo |
| Isca | Porco para, coraçõezinhos, mastiga | "Nhac nhac" | X s |
| Fuga | Porco sai rindo, contador de fugas pisca | "Hahaha" + buzina | — |

**Polimento (pós-protótipo):** screen shake leve só no TNT, partículas de poeira, velocidade 2x na Ação.

---

## Progressão

- **Tipo:** desbloqueio por fases (cada fase nova apresenta uma armadilha ou um novo tipo de bifurcação).
- **Curva:** em platôs. Cada armadilha nova ganha 1 fase "tutorial" e depois fases de combinação.
- **Dificuldade:** mais bifurcações, mais porcos por onda, orçamento menor, limite de fugas menor.
- **Sequência sugerida de fases do protótipo:**
  1. **Bigorna** (1 bifurcação, aprender o desvio)
  2. **TNT + Piche** (agrupar e explodir)
  3. **Isca** (controlar uma bifurcação)
  4. **Ventilador** (empurrar para fora da rota)
  5. **Desafio:** todas as armadilhas, 2 bifurcações, orçamento apertado

---

## Riscos

### Técnicos
| Risco | Prob. | Impacto | Mitigação |
|-------|-------|---------|-----------|
| Recálculo de rota e bloqueios em Godot (iniciante) | M | A | Grafo de rotas simples (nós e arestas) feito à mão, em vez de A* livre; `AStarGrid2D` só se necessário |
| Determinismo quebrar com física | M | A | **Não usar física** para o movimento: avanço por tile/interpolação; ventilador como deslocamento discreto |
| Transição blueprint/colorido | B | M | Duas camadas (TileMap blueprint e TileMap colorido) com shader/fade simples |

### Design
| Risco | Prob. | Impacto | Mitigação |
|-------|-------|---------|-----------|
| A "Ação passiva" ficar chata | M | A | Ações curtas, velocidade 2x, juice nos impactos |
| Jogador não entender por que falhou | A | A | Prévia de rota no Blueprint; destacar o porco que fugiu e por onde |
| Ventilador ambíguo | M | M | Regras simples e uma fase-tutorial dedicada |

### Balanceamento
| Risco | Prob. | Impacto | Mitigação |
|-------|-------|---------|-----------|
| TNT dominante | M | M | Custo alto, raio pequeno, porcos espaçados por padrão |
| Limite de fugas tornar fases fáceis demais | M | B | 3 estrelas exigem zero fugas |

---

## Prioridades do Protótipo (Godot 4, solo, iniciante)

### P1: Grid + Rotas + Porco andando (3–5 dias)
- **Provar:** os porcos andam por um grafo com 1 bifurcação e escolhem o galho por seed fixa.
- **Sucesso:** mesma fase, mesmo resultado em 10 execuções.

### P2: Blueprint ↔ Ação + Bigorna (3–4 dias)
- **Provar:** posicionar no Blueprint, apertar Jogar, a bigorna captura e bloqueia, e os demais recalculam.
- **Sucesso:** o "Desvio" é legível e satisfatório.

### P3: Orçamento, fugas e estrelas + as outras 4 armadilhas (4–6 dias)
- **Provar:** o combo-assinatura (bigorna → piche → isca → TNT) funciona e é divertido.
- **Sucesso:** um playtester busca a 3ª estrela por conta própria.

### P4: Juice e 5 fases (restante)
- Transição visual, sons e as 5 fases da sequência acima.

---

## Jogos de Referência

| Jogo | Aprender | Evitar |
|------|----------|--------|
| Poly Bridge | Loop planejar → testar → iterar; plano preservado | Física imprevisível |
| Kingdom Rush | Legibilidade de rotas e ondas | Microgerenciamento durante a onda |
| The Incredible Machine | Prazer da reação em cadeia | Soluções únicas e rígidas |
| Into the Breach | Informação clara sobre o que vai acontecer | — |

---

## Questões em Aberto

1. **Ventilador:** empurrar para fora da rota captura o porco? (Design)
2. **Bigorna isolando a saída:** proibir no posicionamento ou deixar os porcos pularem? (Design)
3. **Fórmula de estrelas:** validar a proposta (passou / zero fugas / zero fugas + orçamento par). (Design)
4. **Onda única ou várias ondas por fase?** (Design)
5. **Mostrar a escolha nas bifurcações** depois da 1ª tentativa (ícones "esse porco vai para a esquerda")? (Design/UX)
6. **Movimento sem física** para garantir o determinismo: validar com o Engine Team.
7. **Marca fictícia** das engenhocas (pendente do Sparky).

---

## Recomendações

### Para o Design Team
1. Desenhar as 5 fases do protótipo em papel antes (rotas, bifurcações, seed esperada).
2. Definir as regras de borda do ventilador e da bigorna.
3. Fechar a fórmula de estrelas e os orçamentos-par por fase.

### Para o Engine Team
1. Movimento **determinístico, sem física**; RNG por `hash(seed, porco, bifurcação)`.
2. Rotas como **grafo explícito** (Resource com nós e arestas) em vez de pathfinding livre.
3. Armadilhas como cenas Godot com um script base `Trap` (custo, tiles válidos, `on_trigger`, `is_persistent`).

---

*Exploração por Mira, GD-KS Mechanics Explorer*
*Gerado: 2026-11-07*
