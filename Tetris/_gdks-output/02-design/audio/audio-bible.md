# Neon Stack - Audio Bible

> **Autor:** Anton (Audio Director) 🎵
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [GDD main.md](../gdd/main.md) §6-7 · [core-mechanics.md](../core-design/mechanics/core-mechanics.md) §10 (eventos) · [art-bible.md](../art/art-bible.md) §6

---

## 1. Identidade sonora

**"Cada encaixe é uma nota."** O jogo soa como uma faixa synthwave que o jogador toca sem perceber: os SFX são curtos, sintéticos e **afinados na escala da música**, então limpar linhas e encadear combos soa musical, não ruidoso.

### Princípios
1. **Resposta instantânea:** todo SFX de input toca no mesmo frame do evento (latência < 20 ms; sem fade-in).
2. **Hierarquia clara:** mover < girar < travar < limpar < Quad. Quanto mais importante, mais grave/cheio e mais longo.
3. **Silêncio também é ferramenta:** a gravidade movendo a peça **não** faz som; o game over corta a música.
4. **A música apoia, não compete:** ela fica abaixo dos SFX de jogo (mixagem §6).

---

## 2. Fontes (protótipo)

| Tipo | Fonte | Licença |
|------|-------|---------|
| SFX | Packs **CC0**: Kenney (Interface/Digital/Sci-fi Sounds), freesound.org (filtrar por CC0) | CC0, sem atribuição obrigatória |
| Música | Faixa synthwave **royalty-free** em loop (~110-120 BPM) de biblioteca com licença que permita uso em jogo | Conferir e **registrar a licença** de cada arquivo |

- Guardar um `CREDITS.md` / planilha com **arquivo, autor, link e licença** de cada asset (mesmo os CC0).
- Evitar qualquer música ou som associado a jogos de blocos conhecidos.
- Formato de importação: WAV 48 kHz 16-bit; o UE5 comprime na build.

---

## 3. Música

### 3.1 Direção
- Synthwave instrumental, **tonalidade menor** (preferência: **Lá menor**), 110-120 BPM, loop limpo de 1:30-3:00.
- Instrumentação típica: bateria eletrônica, baixo pulsante, pad largo, lead/arpejo.
- **Escolha da faixa:** confirmar a tonalidade (de ouvido ou com uma ferramenta de detecção de tom) e registrar `MusicKey` e `MusicBPM` no DataAsset de áudio.

### 3.2 Reatividade: faixa única + filtro (MVP)
Uma faixa só, modulada por parâmetros em tempo real (MetaSound ou Submix Effect):

| Parâmetro | Controlado por | Mapeamento 🔧 |
|-----------|----------------|---------------|
| **Low-pass cutoff** | Nível | Nível 1: 2,5 kHz → nível 10+: 20 kHz (aberto). Curva log, interpolação de 1 s no level up |
| **Brilho extra (high-shelf)** | Combo | +1,5 dB por combo, máx. +6 dB; volta a 0 em 1,5 s quando o combo zera |
| **Volume da música** | Nível | −6 dB no nível 1 → 0 dB no nível 10 (a música "cresce" com a tensão) |
| **Duck (sidechain)** | Quad | −4 dB por 400 ms para o acorde do Quad aparecer |

| Evento | Música |
|--------|--------|
| Tela inicial | Mesma faixa, cutoff a 1,2 kHz (abafada), volume −8 dB |
| Início da partida | Filtro abre até o valor do nível 1 em 1,5 s |
| Pausa | Cutoff cai para 600 Hz e volume −10 dB em 300 ms (sem parar a faixa) |
| Retomar | Volta ao estado anterior em 300 ms |
| Game over | Efeito de "tape stop" (pitch cai até parar em 1,2 s), depois silêncio |
| Resultados | Silêncio ou pad ambiente bem baixo |

> **Evolução futura (fora do MVP):** stems por faixa de nível, se conseguir música separada em camadas. A coluna "camadas musicais" do [progression-system.md](../core-design/progression/progression-system.md) §5 passa a significar **abertura de filtro/volume** no MVP.

---

## 4. SFX afinados

### 4.1 Escala
- Escala da música: **Lá menor pentatônica** (A, C, D, E, G): sem notas "erradas" em nenhuma combinação.
- Os samples tonais (chime, acorde) são importados **afinados em Lá (A)**. Se a música estiver em outro tom, ajustar `RootOffsetSemitones` no DataAsset.
- Afinação via **pitch multiplier**: `pitch = 2^(semitons / 12)`.

| Grau | Nota (em Lá) | Semitons | Pitch |
|------|--------------|----------|-------|
| 1 | A | 0 | 1,000 |
| 2 | C | +3 | 1,189 |
| 3 | D | +5 | 1,335 |
| 4 | E | +7 | 1,498 |
| 5 | G | +10 | 1,782 |
| 6 | A (8ª) | +12 | 2,000 |
| 7 | C (8ª) | +15 | 2,378 |
| 8 | D (8ª) | +17 | 2,670 |

### 4.2 Regras de afinação
- **Limpeza de linhas:** Single = grau 1, Double = grau 2, Triple = grau 3 (sobe com o nº de linhas).
- **Combo:** cada combo sobe 1 grau a partir do som da limpeza: `grau = base(k) + combo`, limitado ao grau 8. Um combo longo "sobe a escala".
- **Quad:** acorde (A + C + E + A8) tocado como sample único afinado em Lá, + sub grave.
- **Pitch variation** nos não tonais (tick, whoosh, thump): ±3% aleatório para não cansar.

---

## 5. Catálogo de SFX

| Id | Evento (C++) | Descrição | Duração | Tonal? | Volume rel. | Simultâneos |
|----|--------------|-----------|---------|--------|-------------|-------------|
| `SFX_Move` | `OnPieceMoved` (não por gravidade) | Tick digital curto, agudo e seco | 30 ms | não | −18 dB | 2 (corta o mais antigo) |
| `SFX_Rotate` | `OnPieceRotated` | Whoosh sintético curto | 80 ms | não | −14 dB | 2 |
| `SFX_RotateFail` | rotação cancelada | Clique abafado bem baixo (opcional) | 40 ms | não | −24 dB | 1 |
| `SFX_SoftDrop` | soft drop, 1 por linha | Tick bem baixo e grave (opcional; testar se cansa) | 20 ms | não | −26 dB | 1 |
| `SFX_HardDrop` | `OnPieceHardDropped` | Impacto: whoosh descendente + thump forte | 150 ms | não | −8 dB | 1 |
| `SFX_Lock` | `OnPieceLocked` (sem hard drop) | Thump grave e macio, "encaixe" | 120 ms | não | −12 dB | 1 |
| `SFX_LockWarn` | últimos 0,2 s do lock delay | Pulso baixo sutil (combina com a borda piscando) | 200 ms | não | −22 dB | 1 |
| `SFX_Hold` | `OnHoldChanged` (troca ok) | "Swish" + click de travamento | 120 ms | não | −14 dB | 1 |
| `SFX_HoldDenied` | hold bloqueado | Buzz curto e baixo | 80 ms | não | −20 dB | 1 |
| `SFX_Clear` | `OnLinesCleared` k=1-3 | Chime/pluck brilhante + "shatter" granular | 400 ms | **sim** (§4.2) | −8 dB | 2 |
| `SFX_Quad` | `OnLinesCleared` k=4 | Acorde cheio + sub grave + shatter maior | 900 ms | **sim** | −4 dB | 1 |
| `SFX_ComboTick` | combo ≥ 1 (junto ao Clear) | Camada de "sparkle" que fica mais densa com o combo | 300 ms | **sim** | −14 dB | 1 |
| `SFX_LevelUp` | `OnLevelUp` | Sweep ascendente (riser curto) + acorde de 2 notas | 700 ms | **sim** | −8 dB | 1 |
| `SFX_GameOver` | `OnGameOver` | Descida grave + desligamento elétrico ("power down") | 1,5 s | não | −6 dB | 1 |
| `SFX_NewRecord` | resultados, recorde | Arpejo ascendente A-C-E-A | 1,0 s | **sim** | −8 dB | 1 |
| `SFX_UI_Move` | navegação de menu | Tick suave | 30 ms | não | −18 dB | 1 |
| `SFX_UI_Confirm` | confirmar | Blip ascendente | 100 ms | **sim** | −14 dB | 1 |
| `SFX_UI_Back` | voltar / pausa | Blip descendente | 100 ms | **sim** | −14 dB | 1 |
| `SFX_Countdown` | contagem 3-2-1 | Bip (3 e 2 em A, 1 em A8) | 100 ms | **sim** | −12 dB | 1 |

**Total do MVP:** 15 sons obrigatórios + 4 opcionais (`RotateFail`, `SoftDrop`, `LockWarn`, `ComboTick`).

---

## 6. Mixagem

### 6.1 Submixes / Sound Classes
```
Master
├── Music      (slider "Volume da música", padrão 70)
└── SFX        (slider "Volume dos SFX", padrão 80)
    ├── Gameplay   (move, rotate, lock, clear, quad…)
    └── UI         (menus, countdown)
```
- Sliders 0-100 mapeados em dB: `dB = 20·log10(v/100)` (0 = mudo).
- Os volumes são salvos no `USaveGame` (progression-system §6).

### 6.2 Níveis-alvo
| Camada | Pico aproximado |
|--------|-----------------|
| Música | −16 a −12 dBFS |
| SFX de jogo | −10 a −4 dBFS |
| Master | limitador em −1 dBFS |

- **Concurrency:** usar `Sound Concurrency` com os limites da coluna "Simultâneos" (§5); em DAS rápido, o `SFX_Move` não pode empilhar.
- Tudo **2D** (sem espacialização); opcional: pan leve do `SFX_Clear` conforme a coluna média das linhas (±20%).

---

## 7. Implementação (UE5)

| Item | Recomendação |
|------|--------------|
| Disparo | Um **Blueprint `BP_AudioDirector`** (ou componente) escuta os delegates C++ e toca os sons; a lógica de jogo não toca áudio |
| SFX simples | `Sound Wave` + `Sound Cue` com random pitch; ou `PlaySound2D` com pitch multiplier calculado |
| Afinação | Função BP `GetScalePitch(Degree)` lendo a tabela §4.1 + `RootOffsetSemitones` |
| Música | **MetaSound Source** com a faixa + filtro low-pass + high-shelf + ganho, expostos como inputs (`Cutoff`, `ShelfGain`, `Volume`); ajustar via `SetFloatParameter` |
| Tape stop | Parâmetro de pitch/playback rate no MetaSound com rampa de 1,2 s |
| Duck no Quad | Rampa de ganho no MetaSound (ou Submix com sidechain) |
| Dados | `DA_Audio` (DataAsset): referências aos sons, `MusicKey`, `MusicBPM`, `RootOffsetSemitones`, tabelas de cutoff/volume por nível |

---

## 8. Checklist de validação sonora

- [ ] Segurar ← com ARR 0 e ARR 50: o tick não vira "metralhadora" desagradável
- [ ] Single → Double → Triple → Quad soam claramente diferentes e crescentes
- [ ] Combo de 5 sobe a escala sem soar desafinado contra a música
- [ ] Música no nível 1 está audível mas discreta; no nível 10 está cheia
- [ ] SFX de jogo sempre perceptíveis sobre a música no volume padrão
- [ ] Pausa/retomar sem estalos (rampas de 300 ms)
- [ ] Sliders a 0 silenciam totalmente; a 100 sem clipping
- [ ] Todas as licenças registradas no arquivo de créditos

---

*Audio Bible por Anton, GD-KS Audio Director.*
