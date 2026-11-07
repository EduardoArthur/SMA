# Neon Stack - Art Bible

> **Autora:** Aurora (Art Director) 🎨
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [GDD main.md](../gdd/main.md) (pilares, HUD §7.5, feedback §6) · [progression-system.md](../core-design/progression/progression-system.md) §5

---

## 1. Visão visual

**"Luz organizada sobre escuridão."** Um poço de vidro escuro flutua sobre um horizonte synthwave; as peças são cubos com bordas de neon que acendem, pulsam e se desintegram em luz.

### Regras de ouro
1. **Legibilidade vence beleza** (pilar 3). O poço precisa ser lido em 0,1 s no nível 11.
2. **As peças nunca mudam de cor.** As paletas por nível mudam só o fundo, a grade e os acentos; o jogador nunca reaprende as peças.
3. **Contraste por camadas:** fundo (escuro, pouco saturado) < grade (média) < peças (máximo brilho) < VFX (picos curtos).
4. **O brilho é recompensa:** o glow cresce com a importância do evento (tabela §6 do GDD).

### Referências
- Estética synthwave/outrun: sol listrado, chão quadriculado em perspectiva, céu em gradiente magenta → roxo → azul-noite.
- Interfaces retrofuturistas dos anos 80 (vetores luminosos, scanlines sutis).
- **Evitar:** a aparência de qualquer jogo de blocos específico (paleta, logo, molduras). Neon Stack tem identidade própria.

---

## 2. Câmera e composição

- **Câmera ortográfica** frontal sobre o poço, sem inclinação, para preservar a grade lógica.
- O fundo é uma cena 3D separada vista em **perspectiva** (sol + chão), renderizada atrás. Opção simples: duas câmeras, ou o fundo como plano com material animado.
- Resolução de referência **1920×1080 (16:9)**; o poço ocupa ~80% da altura da tela.
- Tamanho da célula de referência: **48 px** (poço = 480×960 px).

---

## 3. Blocos

### 3.1 Forma
- **Cubo levemente chanfrado** (bevel ~6% da aresta), 1 unidade = 1 célula.
- Profundidade visível mínima (a câmera é ortográfica; o chanfro dá volume via iluminação).

### 3.2 Material `M_Block` (um só material, variação por instância)
| Camada | Descrição |
|--------|-----------|
| Núcleo | Vidro escuro: cor da peça × 0,15, levemente translúcido (fake: opaco escuro + fresnel) |
| Borda | Faixa emissiva nas arestas (máscara por UV/textura de borda), **cor da peça × EmissiveIntensity** |
| Padrão | Símbolo sutil no centro do núcleo (§4.2), emissivo a 35% |
| Pulso | Parâmetro `Pulse` (0-1) soma emissivo; usado no travar/limpar |

- Renderizar com **Instanced Static Mesh** (um ISM para os blocos fixos e um para a peça ativa/ghost).
- Dados por instância (`PerInstanceCustomData`): `R, G, B, PatternIndex, Brightness, Pulse, Alpha` (7 floats; ver cpp-specs/02-gameplay §5).
- 🔧 `EmissiveIntensity` base: **6** (borda); com bloom, resulta em glow moderado.

### 3.3 Estados
| Estado | Visual |
|--------|--------|
| Peça ativa | Borda emissiva 100% |
| Bloco fixo | Borda emissiva 70% (ativa se destaca) |
| Ghost | Só a borda, cor da peça, **alpha 30%**, sem núcleo, sem padrão |
| Travando (lock delay) | Borda pisca suavemente (onda de 6 Hz, ±15%) nos últimos 0,2 s |
| Travou | `Pulse` 1 → 0 em 150 ms |
| Linha limpando | Blocos ficam brancos (100% emissivo) por 60 ms → somem e as partículas tomam o lugar |
| Game over | Blocos apagam linha por linha do topo para o fundo (emissivo → 0, núcleo cinza) |

---

## 4. Cores

### 4.1 Peças (constantes em todas as paletas)
| Peça | Nome | Hex | Padrão interno |
|------|------|-----|----------------|
| I | Ciano elétrico | `#2DE2FF` | linha horizontal `—` |
| O | Âmbar | `#FFC23D` | quadrado vazado `□` |
| T | Magenta | `#FF3DCB` | ponto `•` |
| S | Lima | `#9CFF4A` | diagonal `/` |
| Z | Coral | `#FF5E5B` | diagonal `\` |
| J | Violeta | `#8E6BFF` | cruz `+` |
| L | Azul royal | `#3D7BFF` | xis `×` |

- **Acessibilidade:** os padrões ficam **sempre ligados** e são discretos (35% emissivo). Pares de risco para daltonismo (lima × coral, violeta × azul) diferem pelo padrão e pela luminância.
- Validar com um simulador de daltonismo (deuteranopia, protanopia, tritanopia) antes de fechar (checklist §10).

### 4.2 Padrões
Uma **textura atlas 4×2** (`T_BlockPatterns`, 512×256, canal único) com os 7 símbolos + 1 vazio; o `PatternIndex` seleciona o quadro.

### 4.3 Paletas de ambiente (mudam a cada 5 níveis)
| Índice | Níveis | Nome | Céu (topo → horizonte) | Sol | Grade do chão / poço | Acento UI |
|--------|--------|------|------------------------|-----|----------------------|-----------|
| 0 | 1-5 | **Sunset** | `#12002B` → `#5A0F5E` | `#FF7A3D` → `#FF2E88` | `#FF2E88` | `#FF3DCB` |
| 1 | 6-10 | **Midnight** | `#000814` → `#0A2A5E` | `#2DE2FF` → `#3D7BFF` | `#2DE2FF` | `#2DE2FF` |
| 2 | 11-15 | **Toxic** | `#00110A` → `#0B3D2E` | `#9CFF4A` → `#1FD1A5` | `#1FD1A5` | `#9CFF4A` |
| 3 | 16-20 | **Inferno** | `#140000` → `#4A0A00` | `#FFC23D` → `#FF3B1F` | `#FF5E5B` | `#FFC23D` |
| ciclo | 21+ | volta ao 0 | | | | |

- O fundo é escurecido **atrás do poço** (vinheta retangular, 70% de escurecimento) para não competir com as peças.
- A **transição de paleta** é um lerp de 1,0 s disparado por `OnLevelUp`, com um "sweep" de luz horizontal na grade do poço.
- O **interior do poço** é sempre quase preto (`#05040A`, 90% opaco) com a grade de células em 8% de opacidade, na cor de grade da paleta.

---

## 5. Fundo synthwave

| Elemento | Spec |
|----------|------|
| Céu | Gradiente vertical (paleta) + estrelas esparsas estáticas |
| Sol | Disco com gradiente + **faixas horizontais** recortadas na metade inferior; leve glow |
| Montanhas (opcional) | Silhueta wireframe low-poly na linha do horizonte, cor de grade a 40% |
| Chão | Grade em perspectiva, linhas emissivas; **rola em direção à câmera** |
| Velocidade de rolagem | 🔧 `0.2 + 0.05 × (nível − 1)` unidades/s, máx. 1,0 (acompanha a tensão) |
| Reação ao combo | Brilho das linhas do chão +10% por combo (máx. +50%) |
| Reação ao Quad | Sol pulsa (escala 1 → 1,05 → 1, 300 ms) |

Implementação mais barata: o chão é **um plano com material** (grade procedural, panner de UV) e o sol é um plano com material; não precisa de geometria complexa.

---

## 6. VFX (Niagara)

| Efeito | Gatilho | Descrição | Duração | Escala com FX |
|--------|---------|-----------|---------|---------------|
| `NS_LineClear` | `OnLinesCleared` (por linha) | Linha vira faísca: 40-60 partículas por linha na cor dos blocos, explodindo para os lados com gravidade leve, cauda curta | 0,4 s | contagem × (0,3 / 0,6 / 1,0) |
| `NS_QuadBurst` | Count = 4 | Anel de luz expandindo do centro das linhas + partículas grandes | 0,6 s | contagem e tamanho |
| `NS_HardDropTrail` | `OnPieceHardDropped` | Rastro vertical de luz da origem ao destino, nas colunas da peça | 0,15 s | alpha |
| `NS_LockSpark` | `OnPieceLocked` | 4-8 faíscas pequenas nos blocos de baixo da peça | 0,2 s | contagem |
| `NS_LevelUpSweep` | `OnLevelUp` | Varredura horizontal de luz no poço, de baixo para cima | 0,5 s | alpha |
| Pontos flutuantes | `OnLinesCleared` | Texto (UMG ou Text Render) "+1200" sobe 60 px e some | 0,8 s | — |

### Outros efeitos (sem Niagara)
| Efeito | Spec |
|--------|------|
| Camera shake hard drop | Amplitude 2 px, 80 ms (só se o shake estiver ligado) |
| Camera shake Quad | Amplitude 8 px, 250 ms, decaimento rápido |
| Flash Quad | Overlay branco 25% → 0 em 120 ms (Baixa: desligado) |
| Squash ao girar | Escala da peça 1,0 → 0,92 → 1,0 em 80 ms |
| Combo | Contador "COMBO ×N" ao lado do poço; escala 1,3 → 1,0 e brilho na cor de acento |

### Opção "Intensidade de FX"
| Nível | Partículas | Flash | Bloom | Fundo animado |
|-------|-----------|-------|-------|---------------|
| Baixa | 30% | Off | 0,5× | rolagem lenta fixa |
| Média | 60% | 50% | 0,8× | normal |
| Alta (padrão) | 100% | 100% | 1,0× | normal |

**Orçamento:** máx. ~1.500 partículas vivas (Quad + combo na Alta). Usar sprites aditivos e GPU sim se necessário.

---

## 7. Pós-processamento

| Parâmetro | Valor inicial 🔧 |
|-----------|------------------|
| Bloom method | Standard |
| Bloom intensity | 1,2 |
| Bloom threshold | 1,0 (só o emissivo brilha) |
| Exposure | **Manual / fixa** (sem auto exposure, para evitar flutuação de brilho) |
| Vignette | 0,4 |
| Chromatic aberration | 0,1 (pico 0,6 por 150 ms no Quad) |
| Film grain | 0,05 |
| Scanlines | Opcional: overlay de UI a 4% de opacidade |
| Motion blur | **Desligado** (atrapalha a leitura) |
| Lumen / sombras | Desnecessários; usar iluminação simples ou unlit + emissivo |

---

## 8. UI / HUD

### 8.1 Tipografia (licença OFL, livres para uso)
| Uso | Fonte | Observação |
|-----|-------|-----------|
| Logo, títulos, "LEVEL X", "COMBO" | **Orbitron** (Bold/Black) | Caixa alta, espaçamento +5% |
| Números (pontos, linhas) | **Orbitron** com números tabulares; fallback **Share Tech Mono** | Os números não "pulam" de largura |
| Textos de menu | **Audiowide** | Caixa alta |

- Os textos têm **glow** (sombra externa colorida, raio 4-8 px) na cor de acento da paleta.

### 8.2 Painéis
- Molduras de **linha fina emissiva** (2 px) na cor de acento, cantos chanfrados a 45°; fundo do painel `#05040A` a 80%.
- **Hold:** caixa 4×3 células; quando bloqueado (`bCanHold = false`), a peça fica em cinza a 40%.
- **Next:** 1ª peça em tamanho 100%, as 2 seguintes a 70%.
- **Pontos/Nível/Linhas/Recorde:** rótulo pequeno (Audiowide, 18 px, 60% de opacidade) sobre o valor grande (Orbitron, 40 px).

### 8.3 Telas
| Tela | Visual |
|------|--------|
| Inicial | Logo "NEON STACK" grande com glow sobre o fundo synthwave animado; recorde abaixo; menu vertical; o item selecionado tem barra emissiva + leve pulso |
| Pausa | Poço coberto por painel opaco com "PAUSA"; o fundo continua rolando (mais lento); contagem 3-2-1 grande ao retomar |
| Opções | Sliders com trilho emissivo; valores numéricos à direita |
| Resultados | Pontuação grande; "NOVO RECORDE" em âmbar `#FFC23D` piscando; linhas e nível abaixo |

### 8.4 Logo (provisório)
"NEON STACK" em Orbitron Black, preenchimento em gradiente magenta → ciano, contorno emissivo, reflexo de grade sob o texto. Será substituído quando o título final for definido.

---

## 9. Lista de assets (produção)

| Asset | Tipo | Observação |
|-------|------|-----------|
| `SM_Block` | Static Mesh | Cubo chanfrado (pode ser feito no Modeling Mode do UE5) |
| `M_Block` | Material | Núcleo + borda + padrão + pulso via custom data |
| `T_BlockEdge` | Textura | Máscara de borda (ou procedural no material) |
| `T_BlockPatterns` | Textura | Atlas 4×2 dos símbolos |
| `M_Ghost` | Material Instance | Só a borda, translúcida |
| `M_WellBackground` | Material | Fundo do poço + grade de células |
| `M_SynthSky`, `M_SynthSun`, `M_SynthFloor` | Materiais | Parâmetros de paleta e rolagem |
| `DA_Palettes` | DataAsset | As 4 paletas (§4.3) |
| `NS_LineClear`, `NS_QuadBurst`, `NS_HardDropTrail`, `NS_LockSpark`, `NS_LevelUpSweep` | Niagara | §6 |
| `CS_HardDrop`, `CS_Quad` | Camera Shake | §6 |
| `PP_NeonStack` | Post Process Volume | §7 |
| Fontes Orbitron, Audiowide, Share Tech Mono | Font | Google Fonts (OFL) |
| `WBP_HUD`, `WBP_MainMenu`, `WBP_Pause`, `WBP_Options`, `WBP_Results` | Widgets | §8 |

Todos os assets podem ser produzidos **dentro do UE5** (materiais procedurais + Niagara), sem ferramentas externas.

---

## 10. Checklist de validação visual

- [ ] Captura do nível 1 e do nível 11 em escala de cinza: as 7 peças são distinguíveis pelo padrão
- [ ] Simulação de deuteranopia, protanopia e tritanopia: nenhum par de peças é confundível
- [ ] Ghost nunca é confundido com a peça ativa ou com blocos fixos
- [ ] Quad na intensidade Alta: o poço fica legível em < 300 ms
- [ ] Bloom não "funde" blocos vizinhos (ajustar o threshold se fundir)
- [ ] As 4 paletas de fundo mantêm o contraste com as 7 cores de peça
- [ ] HUD legível a 1280×720

---

*Art Bible por Aurora, GD-KS Art Director.*
