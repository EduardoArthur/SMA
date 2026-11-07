# Trap Builder - Design Handoff

> **De:** Time de Design (Diana, Theo, Marco, Aurora, Anton; coordenação: Dylan 📋)
> **Para:** Time de Engine Godot 4 (Érico, Guilherme, Gabi, Gina)
> **Data:** 2026-11-07
> **Trilha:** Standard (sem fase de Planning) · Protótipo · Solo, iniciante em Godot · 1–4 semanas
> **Status:** ✅ Pronto para a Fase de Engine

---

## 1. Resumo em 30 Segundos

Tower defense **sem torres**: você é o **Lobo Mau** emboscando porquinhos na floresta com engenhocas **Catapimba S.A.** Você planeja numa **planta azul** (Blueprint) com orçamento fechado e depois **só assiste** (Ação). Os porcos seguem um **grafo de rotas** com bifurcações decididas por **hash determinístico**. São **5 armadilhas**, **5 fases** validadas por simulação, e de 1 a 3 estrelas por eficiência.

**O que precisa estar certo acima de tudo:** o **determinismo**. Mesmo plano = mesmo resultado, sempre.

---

## 2. Documentos (ordem de leitura recomendada)

| # | Documento | Para quê |
|---|-----------|----------|
| 1 | `02-design/gdd/gdd.md` | Regras do jogo, features F01–F15 com critérios de aceite |
| 2 | `02-design/technical/technical-design.md` | **Spec de implementação:** ticks, inteiros, hash, Resources, fichas, sinais, testes |
| 3 | `02-design/levels/level-design.md` | 5 fases com mapas, números e soluções de referência |
| 4 | `02-design/levels/tools/sim.js` | **Implementação de referência** da simulação (~250 linhas JS) para portar |
| 5 | `02-design/levels/tools/levels.js` | Dados das 5 fases (nós, arestas, barrancos, marcados, seeds, orçamentos) |
| 6 | `02-design/art/art-bible.md` | Paletas, specs de sprite, lista de ~40 assets |
| 7 | `02-design/audio/audio-bible.md` | Lista de 22 SFX, música, buses, integração por sinais |
| — | `01-ideation/ideation-handoff.md` | Contexto e visão (opcional) |

---

## 3. Decisões Consolidadas (D001–D019)

| ID | Decisão |
|----|---------|
| D001 | Trilha Standard, escopo de protótipo |
| D002 | Premissa **Lobo Emboscador**: os porcos atravessam a floresta; fuga = chegar à saída |
| D003 | Bifurcação por `fork_hash(seed, id_porco, id_nó)`; isca tem prioridade; aresta bloqueada é excluída |
| D004 | Derrota quando `fugas > limite_fugas` |
| D005 | Armadilhas em tiles **ROTA**; ventilador só em tiles **MARCADO** |
| D006 | Armadilhas: Bigorna (150), TNT (200), Isca (75), Ventilador (**125**), Piche (50) |
| D007 | O lobo assiste e reage num canto fixo (6 expressões) |
| D008 | Ventilador: BARRANCO captura, ROTA desloca, o resto bloqueia |
| D009 | Bigorna sem alternativa: os porcos escalam (120 ticks); os que já estão na aresta **voltam** (obrigatório já no P2) |
| D010 | Onda única por fase |
| D011 | ★ passou · ★★ 0 fugas · ★★★ 0 fugas + gasto ≤ par |
| D012 | Fila de 1 porco por tile, sem ultrapassagem |
| D013 | Remover armadilha no Blueprint devolve 100% |
| D014 | **Barranco lota com 2 porcos por tile** |
| D015 | Ventilador custa 125 |
| D016 | Par definido pelo solver; orçamento = par + 125–150 |
| D017 | Marca fictícia **Catapimba S.A.** |
| D018 | Arte vetorial própria, top-down com personagens em 3/4 |
| D019 | Áudio: SFX gratuitos + música CC0/CC-BY, com `CREDITS.md` |

---

## 4. Restrições Técnicas Inegociáveis

1. **Simulação em ticks fixos de 60 Hz, só com inteiros** (`TILE_UNITS = 1200`, velocidade do porco = 30/tick). Nada de `delta` na lógica.
2. **Sem física** no movimento. Os sprites só **interpolam** o estado da simulação.
3. **Ordem fixa do tick:** spawn → timers (armadilhas, depois porcos) → movimento (por id) → gatilhos → ventiladores → fim.
4. **Hash próprio** (`technical-design.md` §2.5). Nunca `randi()`, `hash()` ou `RandomNumberGenerator` na simulação.
5. **A simulação não conhece UI, áudio nem lobo:** ela só emite sinais no `GameEvents` (§6 da spec).
6. **Dados em Resources** (`TrapData`, `PigData`, `LevelData`); balanceamento no Inspector.
7. **Paridade com a referência:** o port GDScript deve reproduzir os resultados de `node tools/validate.js` (mesmas capturas, fugas e estrelas para as referências).

---

## 5. Backlog Enxuto (substitui `sprint-plan.md` / `stories.md`)

> O workflow `godot-architecture` pede saídas da fase de Planning, que a trilha Standard pula. Este backlog cumpre esse papel: cada story aponta para os critérios de aceite do GDD (F01–F15).

### Marco P1: Grid, Rotas e Porco Andando (≈ 3–5 dias)
| Story | Descrição | Aceite |
|-------|-----------|--------|
| S01 | Projeto base: pastas (§9 da spec), autoloads `GameEvents` e `SimConst`, janela 1280×720 | Projeto roda; autoloads carregam |
| S02 | Resources: `TrapData`, `PigData`, `LevelData`, `RouteNode`, `RouteEdge`, `SpawnEntry` | `.tres` editáveis no Inspector |
| S03 | Fase 1 montada: `TileMapLayer` com Custom Data `tile_type` + grafo de rotas | F01 |
| S04 | `RouteGraph` + `fork_hash` + teste T4 | F02, F03 |
| S05 | `SimulationRunner` (tick fixo) + porco andando, fila por tile, spawn a cada 60 ticks, fuga na saída | F02; T1, T2 |

### Marco P2: Blueprint ↔ Ação + Bigorna (≈ 3–4 dias)
| Story | Descrição | Aceite |
|-------|-----------|--------|
| S06 | Máquina de estados `BLUEPRINT → TRANSICAO → ACAO → RESULTADO`, com plano preservado | F04 |
| S07 | Posicionar e remover armadilhas no Blueprint (mouse; tiles válidos destacados) | F04 |
| S08 | Bigorna completa: sombra → captura → bloqueio → `VOLTANDO` → escalar | F05; T5 |
| S09 | **Paridade da Fase 1:** referências ★★★ 300 e ★ 150 iguais ao `validate.js` | T1 |

### Marco P3: Economia + 4 Armadilhas (≈ 4–6 dias)
| Story | Descrição | Aceite |
|-------|-----------|--------|
| S10 | Orçamento e catálogo (débito, devolução, itens desabilitados) | F06 |
| S11 | TNT | F07 |
| S12 | Isca (controle de bifurcação + âncora + aviso "sem bifurcação") | F08; T7 |
| S13 | Ventilador (giro, corrente, empurrão, barranco com lotação 2) | F09; T6 |
| S14 | Piche (velocidade × 1/2 inteira) | F10 |
| S15 | Derrota, estrelas e tela de resultado | F11 |
| S16 | Fases 2–5 em `.tres` + **paridade com `validate.js`** | F14 |

### Marco P4: Juice e Polimento (restante)
| Story | Descrição | Aceite |
|-------|-----------|--------|
| S17 | Lobo reativo (6 expressões via sinais) | F12 |
| S18 | Transição visual Blueprint → Ação | F13 |
| S19 | Prévia de rota (arestas tracejadas) + destaque da fuga na derrota | F15 |
| S20 | Áudio (`AudioDirector` + SFX + música) + `CREDITS.md` | audio-bible |
| S21 | Arte final vetorial (substituir placeholders) | art-bible |
| S22 | Export web (itch.io) | Build roda no navegador |

> **Corte de escopo, se o prazo apertar:** P1 + P2 + S10, S11, S15 já **validam o conceito** (Fase 1 + Fase 2). Ventilador, isca, lobo e juice podem ficar para depois.

---

## 6. Notas para o Projeto Godot Existente

- `project.godot`: **Godot 4.7, renderer GL Compatibility**. Bom para web e mobile; manter.
- **Viewport:** definir `display/window/size` como **1280×720**. O mapa de 18×10 tiles (1152×640) cabe com margem para o HUD. `stretch/mode = canvas_items` e `aspect = expand` já estão configurados.
- **Filtro de textura** padrão: Linear (arte vetorial).
- O projeto tem `Jolt Physics` configurado para 3D, irrelevante aqui. **Não usar física** (restrição 2).

---

## 7. Questões em Aberto

| # | Questão | Quando decidir |
|---|---------|----------------|
| Q2 | Título final (Trap Builder × Toon Defense) | Antes do export (S22) |
| Q3 | Mostrar a escolha de cada porco depois da 1ª tentativa? | Depois do playtest do P3 |
| Q4 | Porco de capacete no protótipo? | Se o P3 terminar adiantado |
| B7 | Piche fraco? (custo 25 ou × 1/3) | Playtest |
| — | Fase 2 com ★★★ difícil demais? (par 450 ou trocar ordem) | Playtest |
| — | Fase 5: par validado só por busca local | Registrar o melhor plano dos playtests |

---

## 8. Time de Engine

| Agente | Papel | Comece com |
|--------|-------|------------|
| **Guilherme** 🟢 | Godot Architect: árvore de cenas, autoloads, mapa de sinais | `*godot-architecture` ← **ponto de partida** |
| **Gina** 🔗 | Node Specialist: cenas, composição, Resources | `*scene-specs`, `*resource-design` |
| **Gabi** 📜 | GDScript Lead: specs de classes, prompts de implementação | `*class-specs-godot`, `*code-prompt-godot` |
| **Érico** 🎯 | Engine Coordinator: revisão e checklist | `*review`, `*checklist` |

> Nota: o menu do GameMaster lista outros nomes para o Time de Engine (Ulysses, Priscilla…), mas os agentes instalados são os quatro acima.

---

## 9. Checklist do Handoff

- [x] GDD com critérios de aceite (F01–F15)
- [x] Spec técnica implementável (ticks, hash, Resources, sinais, testes)
- [x] 5 fases com dados, mapas e soluções validadas por simulação
- [x] Implementação de referência + validador
- [x] Art bible e audio bible mínimas
- [x] Decisões D001–D019 consolidadas
- [x] Backlog enxuto (substitui a fase de Planning)
- [ ] Narrative design (opcional, dispensado: premissa e marca já definidas)

---

**Fase de Design concluída! 🎉**

*Handoff por Dylan, GD-KS Design Coordinator*
