# M4 + M5 - Guia de montagem: Neon, juice e áudio

> **Autora:** Priscilla 💻 · **Data:** 2026-11-08
> **Objetivo:** o visual neon (art-bible), os efeitos (Niagara, shakes, flash) e o áudio (SFX afinados + música reativa).
> **Pré-requisito:** M3 funcionando ([m3-setup-guide.md](m3-setup-guide.md)).
> ⚠️ Código escrito **sem compilar**. Mande o log se houver erro.

O C++ faz **toda a lógica** (quando tocar, com que intensidade, que cor, que pitch). Você só cria os **assets** e os liga nos Blueprints. Tudo é opcional e incremental: um campo vazio é simplesmente ignorado. Dá para fazer um efeito de cada vez e testar.

---

## 1. O que entrou no código

| Arquivo | Função |
|---------|--------|
| `Data/NSPaletteData` | DataAsset com as 4 paletas (Sunset, Midnight, Toxic, Inferno), já com as cores da art-bible |
| `FX/NSBackgroundActor` | Escreve paleta, rolagem do chão, brilho de combo e pulso do sol na **MPC_NeonStack**; faz a transição de paleta de 1 s no level up |
| `FX/NSFXDirector` | Niagara (limpeza, Quad, rastro do hard drop, faíscas, sweep de level up), camera shakes, flash, pulso dos blocos, bloom e aberração cromática |
| `Audio/NSAudioDirector` | SFX afinados na pentatônica de Lá menor, música com low-pass/volume por nível, combo, pausa, duck no Quad, "tape stop" no game over; volumes das opções |
| `UI/NSCountdownWidget`, `NSResultsWidget` | Tocam o bip da contagem e o som de novo recorde |
| `Tetris.Build.cs`, `Tetris.uproject` | + módulo/plugin **Niagara** |

Compile com o editor fechado.

---

## 2. Material Parameter Collection: `MPC_NeonStack`

Content Browser → **Materials → Material Parameter Collection** → `Content/NeonStack/Art/Materials/MPC_NeonStack`.

| Tipo | Nome (exato) | Escrito por |
|------|--------------|-------------|
| Vector | `SkyTop`, `SkyHorizon`, `SunTop`, `SunBottom`, `GridColor`, `AccentColor` | Background |
| Scalar | `FloorOffset` | Background (rolagem acumulada) |
| Scalar | `ComboGlow` | Background (0 a 0,5) |
| Scalar | `SunPulse` | Background (0→1→0 no Quad) |
| Scalar | `Flash` | FXDirector (1→0 em 0,12 s no Quad) |

Nos materiais, use o nó **Collection Parameter** (selecione a MPC e o nome).

---

## 3. Materiais (art-bible §3-5)

Todos com **Shading Model = Unlit** (o neon vem do Emissive + bloom; não precisa de luz).

### 3.1 `M_Block` (substitui o `M_Block_Debug`)
Custom data por instância (`PerInstanceCustomData`): `0-2` RGB · `3` padrão · `4` brilho · `5` pulso · `6` alpha.

1. **Cor** = Append(CD0, CD1, CD2).
2. **Borda:** `TexCoord` → máscara de borda: `1 - (min(min(u,1-u), min(v,1-v)) > 0.08)` (use `Min`, `OneMinus` e `Step`/`If`), ou uma textura `T_BlockEdge`. Funciona por face do cubo.
3. **Núcleo** = Cor × 0,15.
4. **Padrão** (opcional, M4+): atlas `T_BlockPatterns` 4×2; UV do quadro = `(TexCoord + float2(CD3 % 4, floor(CD3 / 4))) / float2(4, 2)`; × 0,35.
5. **Emissive** = Cor × (Borda × 6 × CD4 + Padrão) + Cor × CD5 × 10 + Collection(`Flash`) × 2.
6. Ligue o Emissive; Base Color pode ficar preto no Unlit.
7. No `BP_Board` → View → `Block Material = M_Block`.

**Ghost:** `M_Ghost` = cópia com *Blend Mode = Translucent*, só a borda, **Opacity = CD6** (alpha). `Ghost Material = M_Ghost`.

### 3.2 Fundo synthwave
Crie 3 **Planes** (Static Mesh `/Engine/BasicShapes/Plane`) dentro de um **BP_Background** (pai `NSBackgroundActor`), atrás do poço (X positivo):

| Plano | Material | Ideia |
|-------|----------|-------|
| Céu (bem grande, X ≈ +4000) | `M_SynthSky` | Lerp(`SkyHorizon`, `SkyTop`, TexCoord.V invertido) |
| Sol (X ≈ +3500, centralizado) | `M_SynthSun` | Disco (distância ao centro da UV < 0,5) × gradiente `SunBottom`→`SunTop`; faixas horizontais na metade de baixo (`frac(V × 12) > 0,4`); escala/brilho × (1 + `SunPulse` × 0,05) |
| Chão (abaixo do poço, X ≈ +1000) | `M_SynthFloor` | Grade: `frac(UV × 20 + float2(0, FloorOffset))` → linhas finas (< 0,03) × `GridColor` × (1 + `ComboGlow`) |

> Com câmera ortográfica não há perspectiva real; para o chão parecer um horizonte, desenhe as linhas verticais convergindo no material (multiplique U por `1/(V + 0,2)` em torno do centro) ou incline o plano levemente. Comece simples.

No `BP_Background`: `Collection = MPC_NeonStack`, `Palettes = DA_Palettes` (crie: Data Asset → `NSPaletteData`; as 4 paletas já vêm preenchidas).

### 3.3 Poço
Plano `M_WellBackground` atrás dos blocos (X = +60), cor `#05040A`, com a grade das células a 8% em `GridColor` (art-bible §4.3).

Coloque **um BP_Background em cada mapa** (`L_Game` e `L_MainMenu`): no menu ele usa a paleta 0 com rolagem lenta.

---

## 4. Pós-processamento (art-bible §7)

Em `L_Game` (e no menu): **Post Process Volume** com *Infinite Extent (Unbound)* ligado.
- Exposure: *Metering Mode* = Manual, *Exposure Compensation* ≈ 0 (ajuste até o neon ficar bonito).
- Bloom: *Method* Standard, *Threshold* 1,0 (o C++ controla a intensidade).
- Motion Blur: *Amount* 0.
- Vignette 0,4; Film Grain 0,05.

---

## 5. Efeitos: `BP_FXDirector` (pai `NSFXDirector`)

Arraste para o `L_Game` e preencha:
- `Collection = MPC_NeonStack`, `Post Process =` o volume do mapa.
- Camera shakes: crie 2 Blueprints **Camera Shake Base** com *Root Shake Pattern* = **Perlin Noise**:
  - `CS_HardDrop`: duração 0,08 s, amplitude de localização Y/Z ≈ 2.
  - `CS_Quad`: duração 0,25 s, amplitude ≈ 8, *Blend Out* curto.
- Niagara: crie os sistemas abaixo (comece pelo template **Simple Sprite Burst** / **Fountain**). Em cada um, adicione **User Parameters**: `Intensity` (float), `Length` (float), `Color` (Linear Color). O C++ preenche:

| Campo | Sistema | Uso dos parâmetros |
|-------|---------|--------------------|
| `Line Clear FX` | `NS_LineClear` | Spawn Burst = 50 × `User.Intensity`; caixa de spawn com largura = `User.Length` (a linha inteira); velocidade lateral; vida 0,4 s; cor branca/da paleta |
| `Quad Burst FX` | `NS_QuadBurst` | Anel expandindo + partículas grandes; burst × `Intensity` |
| `Hard Drop Trail FX` | `NS_HardDropTrail` | Ribbon ou sprites esticados verticais com altura = `User.Length`; vida 0,15 s |
| `Lock Spark FX` | `NS_LockSpark` | 4-8 faíscas × `Intensity`; cor = `User.Color` (cor da peça) |
| `Level Up Sweep FX` | `NS_LevelUpSweep` | Linha de luz subindo `User.Length` em 0,5 s |

Use sprites **aditivos** e material emissivo. Orçamento: ~1.500 partículas no pior caso.

> O pulso dos blocos ao travar e o "flash branco" da linha antes de sumir já estão no código (`PulseCells`/`FlashRows`) e funcionam assim que o `M_Block` lê CD4/CD5.

---

## 6. Áudio: `BP_AudioDirector` (pai `NSAudioDirector`)

### 6.1 Mixagem
1. Crie 2 **Sound Classes**: `SC_Music` e `SC_SFX` (Sounds → Classes → Sound Class).
2. Crie 1 **Sound Mix**: `SMix_Volumes` (vazio; o C++ preenche as classes).
3. No `BP_AudioDirector`: `Volume Mix = SMix_Volumes`, `Music Class = SC_Music`, `Sfx Class = SC_SFX`.

### 6.2 Arquivos
- **SFX:** packs CC0 (ex.: Kenney *Interface Sounds* / *Digital Audio* / *Sci-fi Sounds*). Importe WAV.
- **Tonais** (`SfxClear`, `SfxQuad`, `SfxComboTick`, `SfxCountdown`): escolha sons de nota (chime, pluck, blip) e, se possível, **afinados em Lá (A)**. Se estiverem em outra nota, ajuste `Root Offset Semitones` (ex.: o som está em Dó → `-3`).
- **Música:** synthwave royalty-free em loop, 110-120 BPM. No asset do Sound Wave marque **Looping**. Se a música estiver em outro tom que não Lá menor, use `Root Offset Semitones` para afinar os SFX com ela (Dó menor = `+3`, Mi menor = `+7`, etc.).
- Registre **arquivo, autor, link e licença** num `CREDITS.md` na raiz do projeto.

### 6.3 Ligações no `BP_AudioDirector`
| Campo | Som (audio-bible §5) |
|-------|----------------------|
| Music | faixa synthwave (Looping) |
| Sfx Move / Rotate / Hard Drop / Lock | tick, whoosh, impacto, thump |
| Sfx Hold / Hold Denied | swish, buzz |
| Sfx Clear / Quad / Combo Tick | chime, acorde, sparkle (tonais) |
| Sfx Level Up / Game Over / New Record / Countdown | riser, power down, arpejo, bip |
| Sfx Lock Warn | opcional (pulso baixo) |

**Concurrency** (audio-bible §5): no asset de `SfxMove` e `SfxRotate`, em *Concurrency*, crie um `SCon_Max2` (*Max Count 2, Resolution Rule = Stop Oldest*), para segurar a seta não virar metralhadora.

### 6.4 Onde colocar
- `L_Game`: um `BP_AudioDirector`.
- `L_MainMenu`: outro `BP_AudioDirector` só com `Music` + mix. Sem board, ele entra no **modo menu** (música abafada, −8 dB).

### 6.5 Como a música reage (automático)
| Situação | Low-pass | Volume |
|----------|----------|--------|
| Menu | 1,2 kHz | −8 dB |
| Nível 1 → 10 | 2,5 kHz → 20 kHz (log) | −6 → 0 dB |
| Combo | — | +0,75 dB/combo (máx. +3), decai |
| Quad | — | −4 dB por 0,4 s (o acorde aparece) |
| Pausa | 600 Hz | −10 dB |
| Game over | — | "tape stop" (pitch 1 → 0,4 em 1,2 s) e fade |

Todos os números estão expostos no `BP_AudioDirector` para ajuste.

---

## 7. UI neon (art-bible §8), opcional mas recomendado

- Fontes **Orbitron** e **Audiowide** (Google Fonts, OFL): importe os `.ttf` → crie *Font* assets → use nos textos.
- Bordas: **Border** com brush *Rounded Box* / outline de 2 px na cor de acento.
- Ganchos já prontos nos WBP (Event Graph → Override): `OnComboShown`, `OnLevelUpShown`, `OnLinesClearedShown` (HUD), `OnNumberShown` (contagem), `OnNewHighScoreShown` (resultados). Use-os para tocar **Widget Animations** (escala 1,3 → 1,0, piscar).
- Sons de UI: nos botões, *Style → Pressed Sound / Hovered Sound*.

---

## 8. Checklist (art-bible §10 + audio-bible §8)

**Visual**
- [ ] As 7 peças distinguíveis no nível 1 e no 11 (e em escala de cinza, com os padrões)
- [ ] Ghost nunca confundido com a peça ativa
- [ ] Quad em FX Alta: o poço fica legível em < 300 ms; o bloom não funde blocos vizinhos
- [ ] Paleta muda suavemente nos níveis 6, 11, 16; FX Baixa reduz partículas, desliga o flash e trava a rolagem lenta
- [ ] Shake desligado nas opções → nenhum tremor

**Áudio**
- [ ] Segurar ← com ARR 0 e 50: o tick não incomoda
- [ ] Single → Double → Triple → Quad soam crescentes; combo de 5 sobe a escala sem desafinar contra a música
- [ ] Música abafada no nível 1, aberta no 10; pausa abafa sem estalos
- [ ] Sliders a 0 silenciam; a 100 sem clipping
- [ ] Licenças registradas

---

## 9. Erros prováveis

| Sintoma | Correção |
|---------|----------|
| `NiagaraComponent.h` não encontrado | Confirme `"Niagara"` no `Tetris.Build.cs` e o plugin no `.uproject`; regenere os project files |
| `SetVariableLinearColor` não existe | Troque por `SetColorParameter(TEXT("Color"), Color)` |
| Música sem filtro | O Sound Wave precisa tocar pelo `MusicComponent` (campo `Music`); não arraste a música solta no mapa |
| Volumes das opções não mudam nada | `Volume Mix`/`Music Class`/`Sfx Class` vazios, ou os SFX com outra Sound Class atribuída no asset |
| Parâmetros do Niagara não pegam | Nomes devem ser exatamente `Intensity`, `Length`, `Color` na seção **User Parameters** |
