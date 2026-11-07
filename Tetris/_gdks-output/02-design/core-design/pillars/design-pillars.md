# Neon Stack - Design Pillars

> **Autora:** Diana (Game Design Director) 🏛️
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Fonte:** [GDD main.md](../../gdd/main.md) §2 (este documento expande e operacionaliza os pilares)

---

## Core Fantasy

> "Em Neon Stack, você é **o maestro de luz que transforma caos em ordem, em ritmo**."

- **Poder:** mental/criativo. Leitura espacial e decisão rápida.
- **Papel:** o organizador sob pressão.
- **Emoção perseguida:** maestria e flow.

---

## Os 3 pilares

### 1. Controle impecável 🎮
**O jogador sempre sente que o erro foi dele, nunca do jogo.**

| Isso significa | Isso NÃO significa |
|----------------|--------------------|
| Input processado no mesmo frame; DAS/ARR ajustáveis | Ajuda automática ou "assistência" escondida |
| Lock delay, hold, ghost, saco de 7 e chutes de rotação | Regras novas ou exceções obscuras |
| Feedback imediato de cada ação | Efeitos que atrasam o input |

**Pergunta-filtro:** *"Isso pode fazer o jogador sentir que o jogo o traiu?"*
**Métrica:** em playtest, nenhuma reclamação de "a peça não foi para onde eu mandei".

### 2. Cada ação brilha ✨
**Toda ação relevante tem uma resposta visual e sonora proporcional à sua importância.**

| Isso significa | Isso NÃO significa |
|----------------|--------------------|
| Hierarquia clara: mover < girar < travar < limpar < Quad | Tudo explodindo o tempo todo |
| SFX afinados, partículas, pulsos, shake em momentos-chave | Efeitos aleatórios sem relação com a jogada |
| Recompensa estética na progressão (paletas, música abrindo) | Unlocks, moedas ou cosméticos |

**Pergunta-filtro:** *"O jogador sentiu essa jogada? A resposta é proporcional?"*
**Métrica:** testers descrevem limpar 4 linhas como "satisfatório" sem serem induzidos.

### 3. Flow acima de tudo 🌊
**Nada quebra o ritmo nem a leitura do poço.**

| Isso significa | Isso NÃO significa |
|----------------|--------------------|
| Pausas de animação ≤ 250 ms; nenhum popup durante o jogo | Telas de tutorial interrompendo a partida |
| Legibilidade acima do espetáculo (peças com cor fixa, fundo escurecido) | FX que encobrem o poço |
| Música e velocidade crescendo juntas | Picos de dificuldade arbitrários |
| Opções de reduzir FX/shake | Um único nível de intensidade "para todos" |

**Pergunta-filtro:** *"Isso quebra o transe ou atrapalha a leitura do poço?"*
**Métrica:** sessões de 5-12 min; > 50% de "jogar de novo" imediato.

---

## Anti-pilares

O jogo **não** terá: narrativa, unlocks/economia, mudança nas regras clássicas por causa do ritmo, multiplayer, monetização.

---

## Teste dos pilares (decisões já tomadas)

| Decisão | P1 Controle | P2 Brilho | P3 Flow | Resultado |
|---------|:-----------:|:---------:|:-------:|-----------|
| Lock delay 0,5 s / 15 resets | ✅ | – | ✅ | Mantido |
| Saco de 7 | ✅ | – | ✅ | Mantido |
| Animação de limpeza 250 ms | – | ✅ | ✅ (limite) | Mantido no teto |
| Pausa cobre o poço | ✅ (justiça) | – | – | Mantido |
| Cores de peça fixas entre paletas | ✅ | ✅ | ✅ | Mantido |
| Música abre com o nível | – | ✅ | ✅ | Mantido |
| Motion blur | ❌ (leitura) | ✅ | ❌ | **Cortado** |
| Seleção de nível inicial | – | – | – | Adiado (fora do MVP) |
| Música em stems | – | ✅ | ✅ | Adiado (custo) |

**Regra de desempate:** em conflito, **P1 > P3 > P2**. Controle primeiro, depois flow, depois espetáculo.

---

*Pilares por Diana, GD-KS Game Design Director.*
