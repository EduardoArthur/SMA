# Neon Stack (título provisório) - Concept Brief

> **Gerado por:** GD-KS Ideation Team (Sparky 💡)
> **Data:** 2026-11-07
> **Status:** Draft
> **Versão:** 1.0
> **Projeto/pasta:** Tetris (o nome de pasta pode ficar; o jogo usa título próprio, pois "Tetris" é marca registrada)

---

## 🎯 Overview

### Logline
> **Neon Stack:** empilhe, limpe, brilhe. Cada linha vira uma explosão de luz no ritmo da música.

### Elevator Pitch
Neon Stack é um puzzle de blocos caindo para PC. Peças de quatro blocos caem num poço de grade neon. O jogador gira e encaixa as peças para completar linhas, que somem com explosões de luz, tremor de câmera e música que reage aos combos. A velocidade cresce a cada nível até o jogador entrar em transe ou o poço transbordar.

Loop central: **mover/girar → encaixar → limpar linhas → explosão de luz + pontos**.

O que o torna único: as regras são clássicas, mas o feedback audiovisual é intenso e transforma cada jogada num pequeno espetáculo.

O jogador sente flow quando a velocidade sobe e ele continua limpando linha atrás de linha sem pensar.

Público: fãs de puzzles clássicos de blocos e de jogos com forte "juice" e estética synthwave.

### The Hook
**Gancho de feedback (juice):** a mecânica não muda. O que muda é como cada ação *soa e brilha*:
- Peça encaixando → pulso de luz + "thump" grave
- Linha limpa → a linha se desintegra em partículas neon (Niagara)
- 4 linhas de uma vez → flash de tela, camera shake, acorde musical
- Combos consecutivos → a música ganha camadas (stems) e a grade pulsa no BPM

---

## 🎮 Core Concept

### Genre & Style
- **Gênero principal:** Puzzle de blocos caindo (falling-block puzzle)
- **Elementos secundários:** feedback audiovisual reativo (leve influência de jogos rítmicos)
- **Direção de arte:** Neon / synthwave: fundo escuro, blocos emissivos com bloom, grade luminosa, horizonte retrô ao fundo
- **Tom:** Hipnótico, energético, sem narrativa

### Setting
- **Mundo:** Um poço de grade neon flutuando num horizonte synthwave
- **Período:** Atemporal / retrofuturista
- **Locais-chave:** Uma única tela de jogo; o fundo pode mudar de paleta a cada nível

### Themes
Ordem a partir do caos, ritmo, flow.

---

## 👤 Player Experience

### Core Fantasy
> "Em Neon Stack, você é **o maestro de luz que transforma caos em ordem, em ritmo**"

### Primary Emotions
O jogador deve sentir:
1. **Flow / transe:** a emoção principal. Ritmo hipnótico com velocidade crescente.
2. **Satisfação:** cada linha limpa é recompensada com luz e som.
3. **Tensão crescente:** o poço enche, a velocidade sobe; o transe fica frágil.

### Defining Moments

| Momento | Descrição | Emoção |
|--------|-------------|---------|
| A peça certa | O poço está alto e chega a peça reta que limpa 4 linhas: flash, shake e acorde | Alívio explosivo |
| Entrar na zona | Nível 10+, a música está com todas as camadas e o jogador joga no automático | Flow |
| Novo recorde | Game over, mas o placar brilha com "NOVO RECORDE" | Orgulho |

**Screenshot que vende:** quatro linhas se desintegrando em partículas neon, grade pulsando, horizonte synthwave ao fundo.

---

## 🔄 Gameplay Foundation

### Core Loop
```
[Mover/Girar] → [Encaixar] → [Limpar linhas] → [Explosão de luz + pontos + velocidade]
```

### Primary Mechanics
1. **Queda e controle de peças:** 7 peças de 4 blocos; mover, girar, soft drop e hard drop.
2. **Limpeza de linhas:** linha completa some; mais linhas de uma vez dão mais pontos.
3. **Progressão de velocidade:** a cada N linhas o nível sobe e a gravidade acelera.
4. **Preview e hold** (avaliar no Design): próxima peça visível; guardar uma peça.

### Player Progression
- **Curto prazo:** limpar linhas, montar combos
- **Médio prazo:** subir de nível (velocidade + mudança de paleta/música)
- **Longo prazo:** bater o recorde (high score salvo localmente)

---

## ⭐ Unique Selling Points

### Why This Game?
1. Regras clássicas e instantaneamente compreensíveis
2. Feedback audiovisual intenso que deixa cada jogada satisfatória
3. Estética neon/synthwave coesa, com música reativa

### Competitive Differentiation
| Jogo comparável | Nossa diferença |
|-----------------|----------------|
| Puzzles de blocos clássicos | Mesmas regras base, mas com juice e música reativa |
| Tetris Effect (referência de sensação) | Escopo muito menor; identidade synthwave própria, sem efeitos sinestésicos complexos |
| Puzzle Bobble / Lumines (referência rítmica) | Mantemos a mecânica clássica; o ritmo é só feedback, não regra |

---

## 📏 Scope Indicators

### Target Scope
- **Tempo de desenvolvimento:** protótipo, 1 a 4 semanas
- **Equipe:** solo (iniciante em UE5)
- **Plataforma:** PC Windows, teclado
- **Preço:** n/a (protótipo de aprendizado)

### Scope Level
- [x] Minimal Viable Product (MVP)
- [ ] Small (2-4 hours content)
- [ ] Medium (5-15 hours content)
- [ ] Large (20+ hours content)

### Content Estimates (Rough)
- **Níveis/áreas:** 1 tela; níveis infinitos de velocidade (paleta muda a cada ~5 níveis)
- **Mecânicas centrais:** 4 (queda/controle, limpeza, velocidade, preview/hold)
- **Peças:** 7 tipos
- **Efeitos de juice:** ~5 (encaixe, limpeza, 4 linhas, level up, game over)
- **Música:** 1 faixa em camadas (stems) + ~8 SFX

### Ordem sugerida (MVP primeiro, juice depois)
1. Grade + peças + controles + limpeza + game over (jogável sem efeitos)
2. Pontuação, níveis, velocidade, high score
3. Visual neon (materiais emissivos + bloom)
4. Juice: partículas Niagara, camera shake, SFX
5. Música reativa por camadas (se sobrar tempo)

---

## 🎯 Target Audience

### Primary Audience
Jogadores casuais e veteranos de puzzles; sessões curtas (3 a 15 min).

### Player Motivations (Bartle Types)
- [x] Achievers - Want to accomplish goals
- [ ] Explorers - Want to discover and learn
- [ ] Socializers - Want to connect with others
- [ ] Killers - Want to compete and dominate

### Games They Love
- Puzzles de blocos caindo clássicos
- Tetris Effect
- Lumines

---

## ❓ Open Questions

1. ~~Sistema de rotação~~ → **resolvido:** simples + chute básico (ver mechanics-exploration.md)
2. ~~Geração de peças~~ → **resolvido:** saco de 7
3. ~~Hold e ghost piece no MVP?~~ → **resolvido:** sim, junto com preview ×3 e lock delay
4. ~~Implementação~~ → **resolvido:** C++ para a lógica + Blueprints para visual/juice
5. Título final e logo próprios (em aberto, não bloqueia)

---

## 📋 Next Steps

- [ ] Market Analysis com Marcus (`@market-analyst`): opcional neste protótipo
- [ ] Mechanics Exploration com Mira (`@mechanics-explorer`): fechar as perguntas 1-3
- [ ] Scope Analysis com Sparky (`*scope`)
- [ ] Ideation Synthesis com Ivy (`@ideation-coordinator`) → handoff para Design

---

## 📝 Notes & Ideas

- Ideias guardadas para depois: modos Sprint (40 linhas) e Ultra (2 min); "poço vivo" com lixo subindo.
- Gancho escolhido pensando em aprendizado de UE5: Niagara, materiais emissivos, post-process (bloom), MetaSounds/áudio em camadas.

---

## Appendix: Inspiration & References

### Visual References
Synthwave/outrun (grade neon em perspectiva, sol retrô, paleta magenta/ciano/roxo).

### Gameplay References
Puzzles de blocos caindo clássicos (regras), Tetris Effect (sensação de flow).

### Audio/Music References
Synthwave instrumental, ~110-120 BPM, em camadas (bateria, baixo, lead, pad).

---

*Documento gerado pelo GD-KS (Game Development Knowledge System)*
*Próximo: Mechanics Exploration → Synthesis → Design Team*
