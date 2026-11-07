# Neon Stack (título provisório) - Ideation Handoff

> **De:** Ideation Team (Sparky, Mira, Ivy)
> **Para:** Design Team (Diana e equipe)
> **Data:** 2026-11-07
> **Status:** Pronto para a fase de Design
> **Trilha:** Standard (Ideação → Design → Engine)

---

## 📋 Executive Summary

### O jogo em uma frase
> **Neon Stack:** empilhe, limpe, brilhe. Cada linha vira uma explosão de luz no ritmo da música.

### Visão unificada
Neon Stack é um puzzle de blocos caindo para PC (teclado), voltado a jogadores casuais e veteranos de puzzles que gostam de sessões curtas. O jogador vive a fantasia de ser o maestro de luz que transforma caos em ordem, usando regras clássicas e imediatamente compreensíveis: 7 peças, linhas completas somem e a velocidade cresce.

O jogo se destaca pelo **feedback audiovisual intenso (juice)** numa estética neon/synthwave. Partículas, flashes, shakes e música em camadas transformam cada jogada num pequeno espetáculo, sem alterar as regras base. A experiência central é o **flow**. O escopo é de protótipo/MVP, feito por um dev solo iniciante em UE5, em cerca de 2 a 3 semanas dentro de uma janela de 4.

### Gancho único
Regras clássicas + juice forte + música reativa por combo, numa identidade synthwave própria.

### Público-alvo
Jogadores casuais e veteranos de puzzle (perfil Achiever), em sessões de 3 a 15 minutos.

---

## 🎯 Concept Foundation

- **Gênero:** puzzle de blocos caindo
- **Tom:** hipnótico e energético, sem narrativa
- **Direção de arte:** neon/synthwave, com blocos emissivos e bloom, grade luminosa e horizonte retrô
- **Core Fantasy:** "Em Neon Stack, você é o maestro de luz que transforma caos em ordem, em ritmo"
- **Setting:** um poço de grade neon flutuando sobre um horizonte synthwave; a paleta muda a cada ~5 níveis

### Emoções-chave
1. Flow / transe (principal)
2. Satisfação a cada encaixe e limpeza
3. Tensão crescente conforme poço e velocidade sobem

---

## 📊 Market Position

Análise de mercado **pulada** (opcional no contrato da fase), pois é um protótipo de aprendizado, sem objetivo comercial.

- **Referências:** puzzles de blocos clássicos (regras), Tetris Effect (sensação), Lumines (ritmo)
- **Posição:** projeto de portfólio/aprendizado; preço n/a
- ⚠️ **Marca:** "Tetris" é marca registrada; o jogo usa título, logo, visual e áudio próprios

---

## ⚙️ Mechanics Foundation

### Core Loop
```
[Mover/Girar/Hold] → [Soltar e travar: pulso + thump] → [Limpar linhas: partículas, pontos, combo] → [Planejar com preview] → repete
```

### Mecânicas-chave (decididas)
1. **Controle de peças:** mover, girar com chute básico (−1/+1; a peça I também tenta −2/+2), soft drop e hard drop
2. **Limpeza de linhas:** 1-4 linhas valem 100/300/500/800 × nível; combo dá bônus
3. **Progressão:** nível a cada 10 linhas; gravidade exponencial
4. **Ferramentas de controle:** saco de 7, preview ×3, hold (1×/peça), ghost piece, lock delay de 0,5 s com até 15 resets

### Sistemas
- **C++ (lógica):** Board 10×22, Piece Controller, Randomizer, Hold, Game Rules/Scoring, Input (DAS ~170 ms / ARR ~50 ms), Save (high score)
- **Blueprints (visual):** Board View, Juice/FX (Niagara, shakes, flash), HUD, áudio
- **Regra de arquitetura:** o C++ emite eventos (`OnPieceLocked`, `OnLinesCleared`, `OnLevelUp`, `OnGameOver`, `OnHoldChanged`, `OnQueueChanged`); o visual só escuta e nunca altera o estado do jogo

Detalhes completos em [mechanics-exploration.md](mechanics-exploration.md).

---

## 🔍 Revisão de consistência (Ivy)

| Verificação | Resultado |
|-------------|-----------|
| Concept brief × mechanics | ✅ Coerentes. As perguntas abertas do brief (rotação, sorteio, hold/ghost, implementação) foram resolvidas pela Mira e marcadas no brief |
| Escopo × recursos | ✅ ~2-3 semanas estimadas, dentro do prazo de 1-4 semanas, com o juice na ordem certa (depois do loop jogável) |
| Gancho × emoção | ✅ Juice reforça o flow, desde que a animação de limpeza fique abaixo de ~250 ms |
| **Tensão encontrada** | ⚠️ "Iniciante em UE5" × "C++ para a lógica" |
| Resolução | Lógica pequena e isolada; o Engine Team entrega specs passo a passo; Blueprints ficam como plano B. Risco aceito conscientemente pelo dev |

---

## 🏆 Success Criteria

### Criativos
- [ ] Uma partida completa jogável do início ao game over
- [ ] Controle justo no nível 10 (erros são do jogador, não do jogo)
- [ ] Limpar 4 linhas "dá vontade de fazer de novo"
- [ ] Um playtester descreve a experiência como hipnótica / "entrei na zona"

### Pessoais (principal objetivo do projeto)
- [ ] Aprender o fluxo C++ ↔ Blueprints no UE5 (UCLASS, UPROPERTY, delegates)
- [ ] Aprender Niagara, materiais emissivos/post-process e Enhanced Input
- [ ] Concluir um projeto UE5 do início ao fim
- [ ] Peça de portfólio

### Comerciais
n/a

---

## ⚠️ Known Risks

### Alta prioridade
| Risco | Impacto | Mitigação |
|-------|---------|-----------|
| Curva de C++ no UE5 | Alto | Lógica isolada e pequena; specs detalhadas; plano B em BP |
| Excesso de efeitos prejudica a leitura | Alto | Intensidade de FX configurável; legibilidade em primeiro lugar; playtest em nível alto |
| Input com timing ruim | Alto | Tempo acumulado por delta; DAS/ARR expostos |

### Média prioridade
| Risco | Impacto | Mitigação |
|-------|---------|-----------|
| Curva de velocidade íngreme | Médio | Tabela de gravidade ajustável |
| Música reativa estoura o prazo | Médio | Marcada como opcional (Prioridade 5) |

### Riscos aceitos
| Risco | Por que aceito |
|-------|----------------|
| Sem análise de mercado | Protótipo de aprendizado, não comercial |
| Título indefinido | Não bloqueia Design nem Engine |

---

## ❓ Open Questions for Design Team

### Alta prioridade
1. Layout de tela: posição do poço, preview, hold, placar/nível/linhas (wireframe do HUD)
2. Paleta neon das 7 peças, legível para daltônicos (forma/brilho, não só cor)

### Média prioridade
1. Pausa: poço visível ou coberto?
2. Tela inicial: só "Jogar" + recorde, ou com seleção de nível inicial?
3. Opções mínimas: volume, intensidade de FX/shake?

### Para o Engine Team
1. Renderização: Instanced Static Mesh + câmera ortográfica (recomendado) vs. Paper2D/UMG
2. Estrutura: classe C++ de lógica pura + Actor fino de view (recomendado)

**Bloqueadores:** nenhum.

---

## 🎨 Design Team Brief

### Prioridades imediatas (trilha Standard, documentação padrão)
1. **GDD enxuto** (Diana): regras como fonte de verdade, com todos os números ajustáveis
2. **Specs técnicas e tabelas de dados** (Theo): gravidade por nível, pontuação, DAS/ARR, lock delay, offsets de rotação das 7 peças
3. **Art Bible curta** (Aurora): paleta neon, materiais emissivos, referências synthwave, HUD
4. **Audio Bible curta** (Anton): lista de ~8 SFX + plano de stems musicais

### Pode ser pulado neste projeto
- Narrativa (Nina): o jogo não tem história
- Level design (Leo): há uma tela só; a progressão é por velocidade/paleta (cabe no GDD)

### Ponto de partida recomendado
`@game-design-director` (Diana) → `*create-gdd`

---

## 📁 Document References

| Documento | Status | Local |
|-----------|--------|-------|
| Concept Brief | ✅ Completo | `01-ideation/concept-brief.md` |
| Market Analysis | ⏭️ Pulado (opcional) | n/a |
| Mechanics Exploration | ✅ Completo | `01-ideation/mechanics-exploration.md` |
| Ideation Handoff | ✅ Este documento | `01-ideation/ideation-handoff.md` |

---

## 👋 Meet Your Design Team

| Agente | Papel | Uso neste projeto |
|--------|-------|-------------------|
| **Diana** 🎨 | Game Design Director | GDD (principal) |
| **Theo** 📐 | Technical Game Designer | Specs e tabelas (essencial para o C++) |
| **Aurora** 🎨 | Art Director | Art Bible neon |
| **Anton** 🎵 | Audio Director | SFX + música |
| **Dylan** 📋 | Design Coordinator | Handoff para o Engine |
| Leo 🗺️ / Nina 📖 | Level / Narrative | Não necessários |

---

## ✅ Handoff Checklist

- [x] Conceito claramente definido
- [x] Gancho identificado
- [ ] Pesquisa de mercado (pulada: opcional)
- [x] Público-alvo definido
- [x] Mecânicas centrais projetadas
- [x] Escopo avaliado
- [x] Perguntas abertas documentadas (sem bloqueadores)
- [x] Documentos linkados

---

**Fase de Ideação concluída! 🎉**

*Gerado por GD-KS Ideation Team: Ivy, Ideation Coordinator*
