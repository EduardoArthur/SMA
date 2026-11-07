# Trap Builder - Synthesis Report

> **Por:** Ivy 🎯 (Ideation Coordinator)
> **Data:** 2026-11-07
> **Entradas:** `concept-brief.md` v1.0, `mechanics-exploration.md`, `00-input/GDD-Toon-Defense.pdf`
> **Status:** Síntese concluída. Pronto para `*validate` → `*handoff`

---

## 1. Visão Unificada

**Trap Builder** é um **tower defense sem torres / puzzle de estratégia** para **PC (protótipo) e Mobile (depois)**, voltado a jogadores casuais e mid-core que gostam de puzzles de otimização e humor cartoon.

O jogador vive a fantasia de ser **o Lobo Mau, que finalmente virou um gênio das engenhocas**. Em vez de soprar casas, ele arma **emboscadas cartoon de uso único** na floresta por onde os porquinhos passam. A mecânica central é um ciclo **Blueprint → Ação**: o jogador planeja numa planta azul, com rotas bifurcadas e orçamento fechado, e depois **só assiste** à reação em cadeia. O jogo se diferencia de *Kingdom Rush* (sem torres e sem interação na onda) e de *Poly Bridge* / *The Incredible Machine* (inimigos dinâmicos e rotas que mudam com as armadilhas).

A experiência central é **tensão no planejamento → riso e alívio na execução**. O escopo é um **protótipo jogável** em **1 a 4 semanas**, solo, em **Godot 4**: 5 armadilhas, porco comum e de 3 a 5 fases.

> **Logline:** o Lobo Mau cansou de soprar casas. Agora ele planeja armadilhas cartoon e assiste os porquinhos caírem nelas.
> **Gancho:** "Planeje como engenheiro, assista como plateia."

---

## 2. Checklist de Completude

### Conceito
- [x] Título provisório: **Trap Builder** (o PDF chama "Toon Defense"; ver Lacuna G1)
- [x] Gênero e subgênero
- [x] Plataformas
- [x] Gancho
- [x] Fantasia central
- [x] Elevator pitch

### Mercado
- [x] Público-alvo (concept brief)
- [x] Competidores comparados (tabela de diferenciação)
- [ ] Análise de mercado formal: **não feita (aceito, escopo de protótipo)**
- [ ] Preço e janela de lançamento: **fora do escopo do protótipo**

### Mecânicas
- [x] Verbo central: **planejar**
- [x] Core loop e loops secundários
- [x] Sistemas (S1 a S7) e mapa de interações
- [x] Prioridades do protótipo (P1 a P4)
- [x] Riscos técnicos

---

## 3. Contradições Encontradas e Resolvidas

| # | Contradição | Resolução |
|---|-------------|-----------|
| C1 | O concept brief dizia "capturar **todos** antes que escapem", mas a decisão M2 definiu **limite de fugas** | ✅ Brief corrigido: falha só se passar do limite; zero fugas vale mais estrelas |
| C2 | O brief listava **banana** no protótipo; a decisão M4 trocou por ventilador + piche | ✅ Corrigido na sessão da Mira |
| C3 | O momento "Fracasso Hilário" usa o **porco de capacete**, que é **opcional** no protótipo | ⚠️ Aceito: o momento é da visão final. No protótipo, o "fracasso" é o porco que escapa pelo galho descoberto |
| C4 | A fantasia "gênio cujas engenhocas **funcionam**" contra o tom slapstick do "Coiote", em que tudo dá errado | ✅ Resolvido conceitualmente: **as armadilhas nunca falham sozinhas**. O humor vem dos porcos (espertos, sortudos) e dos planos incompletos do jogador. Isso mantém o jogo justo e determinístico |

---

## 4. Lacunas

| # | Lacuna | Dono sugerido | Prioridade |
|---|--------|---------------|------------|
| G1 | **Título final:** "Trap Builder" ou "Toon Defense"? | Usuário | Baixa |
| G2 | **O lobo aparece em cena?** Assiste de um arbusto, reage a acertos e erros, ou só existe no menu/catálogo? Afeta arte, feedback e humor | Design (Diana / Aurora) | **Alta** |
| G3 | **Marca fictícia** do catálogo de engenhocas (substitui "Acme") | Design (Aurora / Nina) | Média |
| G4 | Regras de borda: ventilador empurrando para fora da rota e bigorna isolando a saída | Design (Diana / Theo) | **Alta** |
| G5 | Fórmula de estrelas e orçamento-par por fase | Design (Theo) | **Alta** |
| G6 | Onda única ou várias ondas por fase | Design (Diana) | Média |
| G7 | Patos voadores: quais armadilhas os afetam (pós-protótipo) | Design | Baixa |
| G8 | Análise de mercado formal | Marcus (pós-protótipo) | Baixa |

---

## 5. Critérios de Sucesso

### Sucesso Criativo (protótipo)
- [ ] Um playtester entende o ciclo Blueprint → Ação **sem explicação** em até 1 fase.
- [ ] O **combo-assinatura** (bigorna → piche → isca → TNT) arranca uma reação positiva (riso, "de novo!").
- [ ] Um playtester **refaz uma fase já vencida** para buscar a 3ª estrela por conta própria.
- [ ] Ninguém atribui uma derrota a "azar" (a seed fixa funcionando).

### Sucesso Pessoal *(proposta da Ivy, confirmar)*
- [ ] Aprender os fundamentos do Godot 4: cenas, sinais, TileMap, Resources, máquina de estados.
- [ ] Terminar um protótipo jogável de ponta a ponta em até 4 semanas.
- [ ] Ter uma peça de portfólio publicável (ex.: build web no itch.io).

### Sucesso Comercial
- Fora do escopo do protótipo. Reavaliar depois da validação do core loop.

---

## 6. Resumo de Riscos

### Alta Prioridade
| Risco | Impacto | Mitigação |
|-------|---------|-----------|
| Curva de aprendizado do Godot + prazo curto | Alto | Escopo P1→P4 com corte claro; P1 e P2 já validam o conceito; reaproveitar o aprendizado do snake-godot |
| Jogador não entender por que falhou (Ação sem input) | Alto | Prévia de rota no Blueprint; destacar o porco que fugiu e o caminho dele |
| Determinismo quebrado (física / RNG sequencial) | Alto | Movimento sem física; RNG por `hash(seed, porco, bifurcação)` |

### Média Prioridade
| Risco | Impacto | Mitigação |
|-------|---------|-----------|
| A fase de Ação passiva ficar chata | Médio | Ações curtas, velocidade 2x, juice nos impactos |
| Regras do ventilador ambíguas | Médio | Regra simples + fase-tutorial dedicada |
| TNT dominante | Médio | Custo alto, raio pequeno |

### Riscos Aceitos
| Risco | Por que aceito |
|-------|----------------|
| Sem análise de mercado | O protótipo valida diversão, não viabilidade comercial |
| UX mobile não validada | O protótipo é PC-first; o input por toque/clique já é compatível |
| Referências de IP de terceiros (Acme, Coiote, Bad Piggies) | Usadas só como tom; G3 garante uma identidade própria |

---

## 7. Recomendações para o Time de Design

### Foco Prioritário
1. **Resolver G2, G4 e G5** antes de desenhar fases. Elas mudam como cada fase é construída.
2. **Desenhar as 5 fases do protótipo em papel** (rotas, bifurcações, seed esperada, orçamento-par).
3. **Ficha de cada armadilha** (Theo): custo, tiles válidos, gatilho, efeito, persistência, interações.
4. **Direção de arte mínima** (Aurora): estilo blueprint + paleta cartoon + silhueta do porco e do lobo.

### Ajuste de Trilha Sugerido (Standard + protótipo)
| Doc de Design | Recomendação |
|---------------|--------------|
| GDD (Diana) | ✅ Essencial, versão enxuta |
| Level Design (Diana + Marco; não há Level Designer dedicado) | ✅ Essencial: as 5 fases |
| Technical Design (Theo) | ✅ Essencial: fichas de armadilha, regras de rota, estrelas |
| Art Bible (Aurora) | 🟡 Mínimo (1 página) |
| Audio Bible (Anton) | 🟡 Mínimo: lista de SFX |
| Narrative (Nina) | ⚪ Opcional: só a premissa e a marca fictícia |

---

## 8. Documentos

| Documento | Status | Local |
|-----------|--------|-------|
| GDD inicial (fonte) | 📥 Entrada | `00-input/GDD-Toon-Defense.pdf` |
| Concept Brief | ✅ v1.0 | `01-ideation/concept-brief.md` |
| Market Analysis | ⏭️ Pulado (aceito) | — |
| Mechanics Exploration | ✅ | `01-ideation/mechanics-exploration.md` |
| Synthesis Report | ✅ | `01-ideation/synthesis-report.md` |
| Ideation Handoff | ⏳ Próximo (`*handoff`) | `01-ideation/ideation-handoff.md` |

---

*Síntese por Ivy, GD-KS Ideation Coordinator*
*Próximo: `*validate` → `*handoff` → Diana (Time de Design)*
