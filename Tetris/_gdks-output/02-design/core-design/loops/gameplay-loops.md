# Neon Stack - Gameplay Loops

> **Autor:** Core Loop Specialist 🔄 · apoio: Theo, Diana
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [mechanics-exploration.md](../../../01-ideation/mechanics-exploration.md) · [core-mechanics.md](../mechanics/core-mechanics.md) · [progression-system.md](../progression/progression-system.md)

Três loops aninhados: o **micro** (segundos) alimenta o **core** (por peça), que alimenta o **sessão/meta** (minutos e partidas).

---

## 1. Micro loop: manipular a peça (0,1-1 s)

```
[ver peça + ghost] → [mover/girar] → [ghost atualiza + tick/whoosh] → [ajustar de novo]
```

| Elemento | Detalhe |
|----------|---------|
| Ação | ←/→, girar, soft drop |
| Feedback | Ghost reposiciona no mesmo frame; SFX tick/whoosh; squash de 80 ms ao girar |
| Decisão | "Já está no lugar certo?" |
| Pilar | P1 (controle impecável) |

---

## 2. Core loop: uma peça (1-4 s)

```
┌───────────────┐   ┌────────────────┐   ┌──────────────────┐   ┌───────────────────┐
│ PLANEJAR      │──►│ POSICIONAR     │──►│ TRAVAR           │──►│ RESOLVER          │
│ preview ×3,   │   │ micro loop     │   │ hard drop ou     │   │ limpar linhas?    │
│ hold?         │   │                │   │ lock delay       │   │ pontos, combo     │
└───────────────┘   └────────────────┘   └──────────────────┘   └─────────┬─────────┘
        ▲                                                                  │
        └──────────────────── próxima peça (spawn) ◄───────────────────────┘
```

| Etapa | Feedback | Recompensa | Decisão-chave |
|-------|----------|-----------|---------------|
| Planejar | Preview e hold visíveis | – | Usar hold? Onde a próxima vai caber? |
| Posicionar | Ghost, SFX | – | Rotação e coluna |
| Travar | Pulso emissivo, thump (ou rastro + impacto no hard drop) | +1/+2 por linha de drop | Hard drop (rápido, arriscado) ou esperar o lock (ajuste fino)? |
| Resolver | Partículas, chime afinado, pontos flutuantes | 100-800 × nível + combo | Limpar já (seguro) ou guardar coluna para Quad (risco)? |

**Por que repetir:** cada peça é um mini-puzzle novo com resposta imediata; o combo e o Quad criam o desejo de "só mais uma boa jogada".

---

## 3. Feedback cycles

### Positivo (recompensa → mais desempenho)
- **Combo:** limpezas seguidas → mais pontos, música mais brilhante, escala sobe → incentivo a manter o poço "limpável".
- **Quad:** guardar coluna → recompensa máxima (800×) + o maior espetáculo.

### Negativo (equilíbrio natural)
- **Velocidade:** mais linhas → nível sobe → gravidade acelera → o jogo pressiona quem está indo bem.
- **Risco do Quad:** guardar coluna faz o poço subir; se a peça I demorar (no máximo ~12 peças com o saco de 7), o risco aumenta.

### Tensão × alívio
```
tensão ▲          ╱╲         ╱╲    ╱╲
       │    ╱╲   ╱  ╲  ╱╲   ╱  ╲  ╱  ╲ ...
       │   ╱  ╲_╱    ╲╱  ╲_╱    ╲╱
       └──────────────────────────────► tempo
           poço sobe → Quad/combo → alívio
```
O ritmo ideal (P3) alterna 10-30 s de tensão crescente com picos curtos de alívio.

---

## 4. Session loop: uma partida (3-15 min)

```
[Início calmo  ] → [Aquecimento    ] → [Pressão       ] → [Transe/clímax ] → [Game over ] → [Resultados]
 nível 1-2          nível 3-5           nível 6-9          nível 10+          tape stop      recorde?
 música abafada     paleta Sunset       paleta Midnight    paleta Toxic+      silêncio       "jogar de novo"
```

| Fase | Duração (intermediário) | Sensação | Sinais audiovisuais |
|------|-------------------------|----------|---------------------|
| Início | 0-1,5 min | Conforto, aprender o ritmo | Filtro fechado, chão lento |
| Aquecimento | 1,5-3 min | Confiança | Música abrindo |
| Pressão | 3-6,5 min | Tensão, foco | Troca de paleta, chão acelera |
| Transe | 6,5 min+ | Flow ou pânico | Música aberta e cheia |
| Fim | – | Frustração curta → vontade de revanche | Power-down, resultados com recorde |

---

## 5. Meta loop: entre partidas

```
[ver recorde] → [jogar] → [game over] → [comparar com o recorde] → [jogar de novo]
                   ▲                                                    │
                   └────────────────────────────────────────────────────┘
```

| Motivador | Implementação |
|-----------|---------------|
| Superar a si mesmo | Recorde na tela inicial, no HUD e nos resultados |
| Ver a próxima paleta | Paletas a cada 5 níveis (curiosidade estética) |
| Melhorar a técnica | Profundidade emergente: hold estratégico, Quad, lock delay em alta velocidade |
| Reinício sem atrito | "Jogar de novo" é o botão padrão nos resultados; Reiniciar na pausa |

**Sem** meta-progressão (unlocks/moedas), por decisão dos anti-pilares.

---

## 6. Riscos de loop

| Risco | Sinal no playtest | Ajuste |
|-------|-------------------|--------|
| Início lento entedia | Testers aceleram com soft drop o tempo todo no nível 1 | Começar com gravidade do nível 2 ou `LinesPerLevel` menor no início |
| Paredão entre os níveis 7-10 | Mortes concentradas ali | Suavizar a curva (progression-system §3, obs. 2) |
| Combo raramente acontece | Combo médio < 1 | Rever o bônus ou a visibilidade do contador |
| Atrito para reiniciar | Testers hesitam nos resultados | "Jogar de novo" focado por padrão (Enter) |

---

*Loops por Core Loop Specialist, GD-KS.*
