# Neon Stack - Level Documentation

> **Autor:** Leo (Level Designer) 🗺️
> **Data:** 2026-11-07 · **Versão:** 1.0
> **Base:** [GDD main.md](../gdd/main.md) §5, §7 · [progression-system.md](../core-design/progression/progression-system.md) · [art-bible.md](../art/art-bible.md) §2, §4.3, §5 · [audio-bible.md](../audio/audio-bible.md) §3.2

Neon Stack tem **uma única arena** (o poço) e uma progressão contínua de **níveis de velocidade**. Este documento descreve a arena (o "level" físico), os mapas do UE5 e a tabela de **estágios** que une gravidade, paleta e áudio.

---

## 1. Mapas (UE5)

| Mapa | Conteúdo | GameMode |
|------|----------|----------|
| `L_MainMenu` | Fundo synthwave (céu, sol, chão rolando), câmera fixa, `WBP_MainMenu` | `BP_MenuGameMode` |
| `L_Game` | Arena do poço + fundo synthwave + câmeras + post process, `WBP_HUD` | `BP_NeonStackGameMode` (classe C++ base) |

- As telas de pausa, opções e resultados são **widgets** sobre o `L_Game`; não há mapas separados.
- Opcional: `L_Game` pode servir também de menu (menu sobre o fundo) para evitar loading; decisão do Engine Team.

---

## 2. Arena: layout da cena `L_Game`

### 2.1 Vista de cima (eixos UE: X = profundidade, Y = horizontal, Z = vertical)
```
        X (para longe da câmera)
        ▲
  +5000 │      ☀ Sol (plano, Z ≈ 600)
        │  ═══ Montanhas wireframe (opcional) ═══
  +1000 │  ░░░░░ Chão synthwave (plano 8000×8000, Z = −600, rola em −X) ░░░░░
        │
     0  │        ┌─────────────┐  ← Poço (BP_Board): 10×22 células, 100 uu/célula
        │        │  ISM blocos │     origem (0,0,0) = canto inferior esquerdo
        │        └─────────────┘
  −1500 │            📷 CameraActor ortográfica (olha +X)
        └──────────────────────────────────► Y
```

### 2.2 Medidas
| Item | Valor |
|------|-------|
| Tamanho da célula | 100 uu (unidades do UE) |
| Poço | 1000 uu (Y) × 2000 uu visíveis (Z); +200 uu ocultos acima |
| Posição da célula (x, y) lógica | Mundo `(0, x·100 + 50, y·100 + 50)` |
| Câmera do poço | Ortográfica, `OrthoWidth` ≈ 4400 uu (altura visível ≈ 2475 uu; o poço ocupa ~80% da tela), centrada em `(−1500, 500, 1000)` |
| Câmera do fundo | Mesma câmera (o fundo é feito de planos atrás do poço) **ou** captura separada; recomendado: mesma câmera, com planos posicionados para parecer perspectiva |
| Moldura do poço | Plano `M_WellBackground` atrás dos blocos (X = +60), borda emissiva 2 px |

> Com câmera ortográfica não há perspectiva real; o "horizonte synthwave" é obtido com **materiais** (a grade do chão desenhada em perspectiva no próprio material do plano de fundo). É a opção mais simples para iniciante.

### 2.3 Elementos da cena
| Ator | Função |
|------|--------|
| `BP_Board` (filho da classe C++) | Lógica + ISMs dos blocos fixos, peça ativa e ghost |
| `BP_Background` | Planos do céu, sol, chão; recebe paleta e velocidade de rolagem |
| `BP_AudioDirector` | Música (MetaSound) + SFX por eventos |
| `PP_NeonStack` | Post Process Volume (unbound) |
| `BP_FXManager` | Spawna Niagara, shakes e flashes por eventos |

---

## 3. Estágios de progressão

O "nível" do jogo é a velocidade. Os níveis se agrupam em **estágios** (blocos de 5) que mudam o ambiente.

| Estágio | Níveis | Linhas totais | Gravidade (s/linha) | Paleta | Rolagem do chão (uu/s × fator) | Música (cutoff / volume) | Intenção |
|---------|--------|---------------|---------------------|--------|-------------------------------|--------------------------|----------|
| **1. Sunset** | 1-5 | 0-49 | 1,00 → 0,36 | 0 Sunset | 0,20 → 0,40 | 2,5 → ~6,3 kHz / −6 → −3,3 dB | Aprender, aquecer |
| **2. Midnight** | 6-10 | 50-99 | 0,26 → 0,06 | 1 Midnight | 0,45 → 0,65 | ~7,9 → 20 kHz / −2,7 → 0 dB | Pressão crescente |
| **3. Toxic** | 11-15 | 100-149 | 0,05 (piso) | 2 Toxic | 0,70 → 0,90 | aberto / 0 dB | Transe, maestria |
| **4. Inferno** | 16-20 | 150-199 | 0,05 | 3 Inferno | 0,95 → 1,00 | aberto / 0 dB | Resistência |
| ciclo | 21+ | 200+ | 0,05 | volta a 0 | 1,00 | aberto | Infinito |

- Rolagem: `0.2 + 0.05 × (nível − 1)`, máx. 1,0 (art-bible §5).
- Cutoff da música: curva log do nível 1 (2,5 kHz) ao 10 (20 kHz) (audio-bible §3.2).

### 3.1 Transição de estágio (níveis 6, 11, 16, 21)
| t | Evento |
|---|--------|
| 0 ms | `OnLevelUp` → texto "LEVEL X" + `SFX_LevelUp` + `NS_LevelUpSweep` |
| 0-1000 ms | Lerp da paleta do fundo, grade do poço e acento de UI |
| 0-1000 ms | Rolagem do chão interpola para a nova velocidade |
| — | **O jogo não pausa** (P3: flow) |

Nos level ups **sem** troca de estágio, só o texto, o SFX e o sweep.

---

## 4. Ritmo esperado por estágio

| Jogador | Estágio onde costuma terminar | Duração |
|---------|-------------------------------|---------|
| Iniciante | 2 (Midnight, níveis 6-8) | 5-8 min |
| Intermediário | 2-3 | 8-12 min |
| Avançado | 3-4 (ou platô infinito) | 10-15+ min |

**Objetivo de design:** todo jogador vê **pelo menos 2 paletas** numa partida típica (recompensa estética visível já na primeira sessão).

---

## 5. Checklist da arena

- [ ] O poço inteiro + HUD cabem em 16:9 a 1280×720 e 1920×1080
- [ ] Blocos alinhados à grade lógica (sem meio-pixel / sem tremer)
- [ ] O fundo nunca passa à frente do poço (ordem de profundidade)
- [ ] As transições de estágio não causam queda de FPS
- [ ] A zona oculta (linhas 20-21) não aparece; peças nascendo ali ficam cortadas pela moldura

---

*Documentação de níveis por Leo, GD-KS Level Designer.*
