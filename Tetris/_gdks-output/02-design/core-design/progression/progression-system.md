# Neon Stack - Progression System

> **Autor:** Theo (Technical Game Designer) 🔧 · apoio: Diana
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [GDD main.md](../../gdd/main.md) §5 · [core-mechanics.md](../mechanics/core-mechanics.md)

Neon Stack tem progressão **baseada em habilidade**: não há unlocks, moedas nem upgrades. O jogo fica mais rápido, e o jogador fica melhor. As únicas recompensas são **pontos, recorde e espetáculo audiovisual**.

---

## 1. Objetivos do jogador

| Prazo | Objetivo | Recompensa |
|-------|----------|-----------|
| Curto (segundos) | Encaixar bem, limpar linhas, manter o combo | Partículas, som, pontos flutuantes |
| Médio (minutos) | Subir de nível, sobreviver à velocidade | "LEVEL X", troca de paleta (a cada 5), camada musical |
| Longo (sessões) | Bater o recorde, chegar mais longe | "NOVO RECORDE", recorde visível na tela inicial |

---

## 2. Tipos de recompensa

| Tipo | Quando | Intensidade |
|------|--------|-------------|
| Feedback de ação | Todo encaixe/limpeza | Escala com nº de linhas (Single → Quad) |
| Combo | Limpezas consecutivas | Contador + brilho da grade crescem |
| Level up | A cada 10 linhas | Texto + sweep sonoro |
| Marco estético | Níveis 6, 11, 16… | Paleta nova (+ camada musical, se implementada) |
| Recorde | Game over com score > recorde | Destaque na tela de resultados |

**Unlocks:** nenhum. **Economia:** nenhuma (fora de escopo; ver anti-pilares no GDD).

---

## 3. Curva de velocidade (gravidade)

Fórmula: `T(n) = max(0.05, (0.8 − (n−1) × 0.007)^(n−1))` (s por linha) 🔧

| Nível | Linhas totais | s/linha | linhas/s | Tempo de queda do topo (20 linhas) |
|-------|---------------|---------|----------|------------------------------------|
| 1 | 0-9 | 1,000 | 1,0 | 20,0 s |
| 2 | 10-19 | 0,793 | 1,3 | 15,9 s |
| 3 | 20-29 | 0,618 | 1,6 | 12,4 s |
| 4 | 30-39 | 0,473 | 2,1 | 9,5 s |
| 5 | 40-49 | 0,355 | 2,8 | 7,1 s |
| 6 | 50-59 | 0,262 | 3,8 | 5,2 s |
| 7 | 60-69 | 0,190 | 5,3 | 3,8 s |
| 8 | 70-79 | 0,135 | 7,4 | 2,7 s |
| 9 | 80-89 | 0,094 | 10,7 | 1,9 s |
| 10 | 90-99 | 0,064 | 15,6 | 1,3 s |
| 11+ | 100+ | 0,050 (piso) | 20,0 | 1,0 s |

> ⚠️ **Correção:** o documento de mecânicas da Ideação citava "nível 1 ≈ 0,80 s". Pela fórmula, o nível 1 dá **1,0 s**; os demais valores estavam certos.

### Observações de balanceamento
1. **Platô no nível 11:** a fórmula bate no piso de 0,05 s, e a dificuldade para de subir. Depois disso, quem decide a partida é o jogador (lock delay vira a ferramenta principal).
   - **Opção A (MVP, padrão):** manter o platô. Simples; jogadores bons podem jogar "para sempre".
   - **Opção B (após playtest):** baixar `MinGravity` para ~0,02 s ou cair até 20G (instantâneo) a partir do nível 15, para garantir um fim.
2. **Rampa íngreme entre 7 e 10:** o tempo cai de 3,8 s para 1,3 s em 30 linhas. Se o playtest mostrar paredão, suavizar com `GravityLevelStep` menor ou trocar a fórmula por uma **tabela por nível** no DataAsset (curva editável).
3. **Lock delay é o colchão:** em alta velocidade, os 0,5 s com 15 resets mantêm o jogo justo (pilar 1).

---

## 4. Ritmo esperado (pacing)

Premissas: Single/Double são as limpezas mais comuns; ~2,5 peças por linha limpa em média.

| Jogador | Peças/min | Linhas/min | Tempo até o nível 5 | Tempo até o nível 10 | Sessão típica |
|---------|-----------|------------|---------------------|----------------------|---------------|
| Iniciante | ~20 | ~8 | ~5 min | (game over antes, ~nível 6-8) | 5-8 min |
| Intermediário | ~35 | ~14 | ~3 min | ~6,5 min | 8-12 min |
| Avançado | ~60 | ~24 | ~1,7 min | ~4 min | 10-15+ min |

✅ Encaixa na meta de **sessões de 3 a 15 minutos** do GDD.

---

## 5. Progressão estética

| Faixa de níveis | Índice de paleta | Camadas musicais (futuro; no MVP = abertura de filtro/volume, ver audio-bible §3.2) |
|-----------------|------------------|-----------------------------------|
| 1-5 | 0 | base (bateria + pad) |
| 6-10 | 1 | + baixo |
| 11-15 | 2 | + lead |
| 16-20 | 3 | + arpejo / todas |
| 21+ | volta ao 0 (ciclo) ou mantém o 3 🔧 | todas |

- `PaletteIndex = floor((Level − 1) / PaletteEveryNLevels) mod PaletteCount` 🔧
- Cores de cada paleta: **Aurora** (Art Bible). Camadas: **Anton** (Audio Bible).
- A intensidade do combo modula o brilho da grade dentro da paleta atual (não troca a paleta).

---

## 6. Recorde e persistência

`USaveGame` (slot `NeonStack_Save`):

| Campo | Tipo |
|-------|------|
| HighScore | int64 |
| HighScoreLines | int32 |
| HighScoreLevel | int32 |
| HighScoreDate | FDateTime |
| Settings (MusicVol, SfxVol, FxIntensity, bScreenShake, DAS, ARR) | struct |

- Salvar ao final da partida e ao fechar a tela de opções.
- `bNewHighScore = FinalScore > HighScore`.

---

## 7. Métricas para o playtest

| Métrica | Alvo | Ação se fora do alvo |
|---------|------|----------------------|
| Nível médio alcançado (iniciantes) | 5-8 | < 4: suavizar gravidade · > 10: acelerar |
| Duração média da sessão | 5-12 min | ajustar `LinesPerLevel` ou a curva |
| % de mortes entre os níveis 7-10 | não deve concentrar > 60% | suavizar a rampa (obs. 2) |
| "Joguei de novo" imediato | > 50% das partidas | melhorar a tela de resultados e o feedback de recorde |

---

*Spec por Theo, GD-KS Technical Game Designer.*
