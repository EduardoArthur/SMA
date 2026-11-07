# Trap Builder - Ideation Handoff

> **De:** Time de Ideação (Sparky, Mira, Ivy; Marcus pulado no protótipo)
> **Para:** Time de Design (Diana e equipe)
> **Data:** 2026-11-07
> **Status:** Pronto para a Fase de Design
> **Trilha:** Standard (ideação → design → engine), escopo de protótipo, Godot 4, solo, 1–4 semanas

---

## 📋 Resumo Executivo

### O Jogo em Uma Frase
> O Lobo Mau cansou de soprar casas. Agora ele planeja armadilhas cartoon e assiste os porquinhos caírem nelas.

### Conceito Central
Tower defense **sem torres**: o jogador posiciona **armadilhas cartoon, quase todas de uso único**, num grid com **rotas bifurcadas**, usando um **orçamento fixo**. O jogo alterna entre a **Fase Blueprint** (planejamento congelado numa planta azul) e a **Fase de Ação** (execução colorida, sem interação). O objetivo é capturar os porcos que atravessam a floresta antes que escapem.

### Gancho
**"Planeje como engenheiro, assista como plateia."** Toda a habilidade está no planejamento; a recompensa é assistir a reação em cadeia.

### Público-Alvo
Jogadores casuais e mid-core de PC e mobile, fãs de *Kingdom Rush*, *Poly Bridge*, *The Incredible Machine* e *Bad Piggies*. Perfil Achiever (estrelas) + Explorer (combos).

---

## 🎯 Base do Conceito

### Gênero & Estilo
- **Gênero:** Tower Defense sem torres / Puzzle de otimização
- **Tom:** comédia pastelão, vilão carismático
- **Arte:** cartoon clássico dos anos 40–50 + modo blueprint (linhas brancas sobre azul). **Identidade própria, sem marcas de terceiros.**

### Fantasia Central
> "Em Trap Builder, você é **o Lobo Mau, que finalmente virou um gênio das engenhocas**."

**Regra de tom:** as armadilhas **nunca falham sozinhas**. O humor vem dos porcos e dos planos incompletos do jogador.

### Premissa e Ambientação: o Lobo Emboscador
Os porcos (e, depois, os patos) saem de casa e atravessam a floresta do lobo a caminho da feira ou da vila. O lobo arma emboscadas com engenhocas de catálogo. Os porcos se precavem (capacetes, veículos; pós-protótipo).
- **Spawn:** casas de palha, madeira e tijolo
- **Saída:** feira / vila
- **Mapa:** trilhas da floresta, ponte, campos

### Emoções-Chave
1. **Blueprint:** foco e tensão
2. **Transição:** expectativa ("será que funciona?")
3. **Ação:** riso e alívio

---

## 📊 Posição de Mercado

Não houve análise formal (risco aceito para o protótipo).

| Comparável | Nossa diferença |
|------------|-----------------|
| Kingdom Rush | Sem torres permanentes nem interação na onda |
| Poly Bridge / The Incredible Machine | Inimigos dinâmicos; rotas mudam com as armadilhas |
| Bad Piggies | Os porcos são as presas; o jogador constrói armadilhas |
| Orcs Must Die / Dungeon Warfare | Sem ação em tempo real; economia fechada |

**Preço e lançamento:** fora do escopo. Meta pessoal: build web no itch.io.

---

## ⚙️ Base de Mecânicas

### Core Loop
```
Analisar rotas → Comprar e posicionar armadilhas → JOGAR → Assistir → Estrelas / Ajustar → (plano preservado) → repetir
```

### Decisões Fechadas
| ID | Decisão |
|----|---------|
| D002 | Premissa **Lobo Emboscador**; os inimigos vencem ao chegar à saída |
| D003 | Bifurcações por **RNG com seed fixa**: `hash(seed_fase, id_porco, id_bifurcação)` |
| D004 | Derrota ao **ultrapassar o limite de fugas** da fase |
| D005 | Posicionamento em **tiles de rota + tiles marcados** |
| D006 | Protótipo: **Bigorna, TNT, Isca Falsa, Ventilador, Piche** |

### Armadilhas do Protótipo
| Armadilha | Tipo | Tiles | Efeito-chave |
|-----------|------|-------|--------------|
| Bigorna | Uso único | Rota | Captura 1; vira obstáculo → recálculo de rota |
| TNT | Uso único | Rota | Explosão em área |
| Isca Falsa | Uso único | Rota | Força o galho na bifurcação anterior; segura o 1º porco |
| Ventilador | Contínuo | Marcado | Empurra porcos numa direção |
| Piche | Contínuo | Rota | Desacelera |

**Combo-assinatura (o teste do conceito):** bigorna desvia → piche desacelera → isca segura → TNT pega o grupo.

### Sistemas
S1 Grid & Tiles · S2 Rotas & Bifurcações · S3 Armadilhas · S4 Porcos · S5 Fases (Blueprint↔Ação) · S6 Economia & Estrelas · S7 Catálogo. Detalhes em `mechanics-exploration.md`.

---

## 📁 Documentos

| Documento | Status | Local |
|-----------|--------|-------|
| GDD inicial (fonte) | 📥 Entrada | `00-input/GDD-Toon-Defense.pdf` |
| Concept Brief | ✅ v1.0 | `01-ideation/concept-brief.md` |
| Market Analysis | ⏭️ Pulado (aceito) | — |
| Mechanics Exploration | ✅ | `01-ideation/mechanics-exploration.md` |
| Synthesis Report | ✅ | `01-ideation/synthesis-report.md` |
| Ideation Handoff | ✅ | `01-ideation/ideation-handoff.md` |

---

## ❓ Questões em Aberto

### Alta Prioridade (Design, resolver antes das fases)
1. **G2: O lobo aparece em cena** na Ação? (assiste e reage / só no menu)
2. **G4: Regras de borda:** o ventilador empurrando para fora da rota captura o porco? A bigorna pode isolar a saída?
3. **G5: Fórmula de estrelas** (proposta: ★ passou · ★★ zero fugas · ★★★ zero fugas + gasto ≤ orçamento-par) e orçamentos por fase.

### Média Prioridade
1. **G6:** onda única ou várias ondas por fase?
2. **G3:** marca fictícia do catálogo de engenhocas (substituta da "Acme").
3. Mostrar a escolha das bifurcações depois da 1ª tentativa (UX)?

### Para Times Seguintes (Engine)
1. Movimento **determinístico, sem física**.
2. Rotas como **grafo explícito** (Resource), sem pathfinding livre.
3. Script base `Trap` para as armadilhas (custo, tiles válidos, `on_trigger`, persistência).

### Baixa / Pós-protótipo
- G1: título final (Trap Builder × Toon Defense) · G7: patos voadores · G8: análise de mercado

---

## 🎨 Brief para o Time de Design

### Prioridades Imediatas
1. **Diana:** GDD enxuto do protótipo + resolver G2, G4, G5, G6.
2. **Theo:** fichas técnicas das 5 armadilhas, regras de rota/RNG e fórmula de estrelas.
3. **Desenho das 5 fases** (rotas, bifurcações, orçamento-par):
   1. Bigorna · 2. TNT + Piche · 3. Isca · 4. Ventilador · 5. Desafio (todas)
4. **Aurora:** art bible mínima (blueprint + paleta cartoon + silhuetas do porco e do lobo + marca fictícia).
5. **Anton:** lista de SFX (apito de queda, CLANG, BOOM, squelch, zumbido, nhac, risada).

### Recomendado por Agente
| Agente | Papel | Prioridade no protótipo |
|--------|-------|-------------------------|
| **Diana** 🎨 | Game Design Director: GDD, sistemas, economia | ✅ Essencial (começar aqui) |
| **Theo** 🔧 | Technical Game Designer: specs e tabelas | ✅ Essencial |
| **Marco** ⚙️ | Mechanics Designer: detalhar as armadilhas e interações | ✅ Essencial |
| **Lucas** 🔄 | Core Loop Specialist: ritmo Blueprint ↔ Ação | 🟡 Útil |
| **Felix** ✨ | Game Feel Physicist: juice dos impactos | 🟡 Útil (depois de P2) |
| **Aurora** 🎨 | Art Director | 🟡 Mínimo |
| **Anton** 🎵 | Audio Director | 🟡 Mínimo |
| **Nina** 📖 | Narrative Designer | ⚪ Opcional (premissa + marca) |
| **Dylan** 📋 | Design Coordinator: síntese e handoff ao Engine | ✅ No fim da fase |

> Nota: o GD-KS instalado **não tem um Level Designer dedicado**. O desenho das 5 fases fica com **Diana + Marco**.

### Ponto de Partida
Carregar **Diana** (`@game-design-director`) e rodar `*gdd`.

---

## ✅ Checklist do Handoff

- [x] Conceito claramente definido
- [x] Gancho identificado
- [ ] Pesquisa de mercado (pulada, risco aceito)
- [x] Público-alvo definido
- [x] Mecânicas centrais desenhadas
- [x] Escopo avaliado (protótipo, P1 a P4)
- [x] Questões em aberto documentadas e priorizadas
- [x] Documentos referenciados

---

**Fase de Ideação concluída! 🎉**

*Gerado pelo GD-KS Ideation Team (Ivy 🎯)*
