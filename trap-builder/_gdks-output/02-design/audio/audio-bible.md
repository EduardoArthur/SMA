# Trap Builder - Audio Bible (Mínima)

> **Autor:** Anton 🎵 (Audio Director) · **Versão:** 0.1 · **Data:** 2026-11-07
> **Escopo:** protótipo. **SFX gratuitos + música CC0**, sem custo e sem gravação.

---

## 1. Direção em Uma Frase

> **Orquestra de desenho animado em miniatura:** o Blueprint é pensativo e pontilhado (pizzicato); a Ação é perseguição saltitante (ragtime); e cada impacto tem um **som de cartoon exagerado** que vende a piada.

**Regras:**
1. **O som vende o impacto:** cada armadilha tem um SFX inconfundível, tocado no **mesmo tick** do evento.
2. **O silêncio é ferramenta:** ~300 ms de quase silêncio antes da bigorna cair (só o apito de queda).
3. **Variação sem bagunça:** pitch aleatório de ±5% nos SFX repetitivos (passos, pops). **Esse aleatório é só do áudio**: use um RNG separado e nunca o da simulação, para não quebrar o determinismo.

---

## 2. Música (2 faixas + 2 vinhetas)

| Faixa | Clima | Referência de gênero | Loop |
|-------|-------|----------------------|------|
| `music_blueprint` | Calmo, "pensando", pizzicato e madeiras | Cartoon "o vilão planeja" | Sim |
| `music_action` | Rápido, perseguição, piano ragtime | Cartoon de perseguição | Sim |
| `stinger_win` | Fanfarra curta e triunfante (2–3 s) | — | Não |
| `stinger_lose` | "Wah-wah-waaah" de trombone (2 s) | — | Não |

- **Transição:** crossfade de 0,7 s junto com a transição visual (Blueprint → Ação).
- **Fontes CC0 / CC-BY:** OpenGameArt (filtro CC0), Kevin MacLeod / incompetech (CC-BY, **exige crédito**), FreePD (domínio público).
- **Formato:** `.ogg` com loop ativado no import do Godot.

---

## 3. SFX

| ID | Evento | Descrição | Fonte sugerida |
|----|--------|-----------|----------------|
| `sfx_place` | Posicionar armadilha | "Tuc" de madeira / carimbo | Kenney (Interface/Impact) |
| `sfx_remove` | Remover | "Fwip" reverso | Kenney |
| `sfx_rotate` | Girar ventilador | Clique mecânico | Kenney |
| `sfx_invalid` | Tile inválido / sem dinheiro | "Bonk" grave curto | jsfxr |
| `sfx_play` | Botão JOGAR | Alavanca + campainha | Freesound CC0 |
| `sfx_transition` | Blueprint → Ação | Swoosh de papel | Freesound CC0 |
| `sfx_anvil_whistle` | Sombra crescendo | Apito descendente (slide whistle) | Freesound CC0 |
| `sfx_anvil_clang` | Bigorna cai | CLANG metálico + "squish" | Kenney (Impact) + jsfxr |
| `sfx_tnt_fuse` | Pavio | Chiado curto | Freesound CC0 |
| `sfx_tnt_boom` | Explosão | BOOM abafado e fofo (não realista) | jsfxr (Explosion) |
| `sfx_bait_eat` | Porco come | "Nhac nhac" | Freesound CC0 |
| `sfx_fan_loop` | Ventilador ligado | Zumbido em loop, baixo | Freesound CC0 |
| `sfx_push` | Empurrão | "Whoosh" curto | jsfxr |
| `sfx_ravine_fall` | Cai no barranco | Assobio descendente + "poc" distante | jsfxr |
| `sfx_tar_step` | Passo no piche | "Squelch" pegajoso | Freesound CC0 |
| `sfx_pig_step` | Passo normal | Trote leve (pitch ±5%) | Kenney |
| `sfx_pig_oink` | Spawn / susto | Grunhido curto | Freesound CC0 |
| `sfx_pig_turn` | Porco dá meia-volta (`VOLTANDO`) | "Hã?" de porco | Freesound CC0 |
| `sfx_capture_pop` | Captura | "Pop" + moedinha | jsfxr (Pickup) |
| `sfx_escape` | Porco foge | Risadinha + buzina "fon-fon" | Freesound CC0 |
| `sfx_wolf_react` | Reação do lobo | Uivo curto / rosnado (1 por expressão) | Freesound CC0 |
| `sfx_star` | Carimbo da estrela | "Tchan" crescente (1 → 2 → 3) | jsfxr |

> **jsfxr** (gerador online, grátis) é ótimo para sons cartoon. Salve o *preset* de cada som junto com o `.wav` para poder recriar.

---

## 4. Mixagem (Godot)

| Bus | Volume inicial | Contém |
|-----|----------------|--------|
| `Master` | 0 dB | — |
| `Music` | −10 dB | Faixas e vinhetas |
| `SFX` | −4 dB | Armadilhas, porcos, lobo |
| `UI` | −6 dB | Catálogo, botões, estrelas |

- **Prioridade:** impactos (bigorna, TNT, captura) > reações > passos. Limite de **4 passos simultâneos**; os excedentes não tocam.
- **Ducking:** a música cai −6 dB por 0,5 s em BOOM e CLANG (*Compressor* com sidechain no bus `Music`, ou um tween simples no volume).
- **Velocidade 2x:** SFX normais; pitch da música +0% (não acelerar).
- **Formatos:** SFX em `.wav` 44,1 kHz mono; música em `.ogg` estéreo.
- **Organização:** `res://audio/{music,sfx,ui}/id.ext` com os mesmos IDs da tabela.

### Integração
Um autoload `AudioDirector.gd` escuta os sinais do `GameEvents` (ver `technical-design.md` §6) e toca os sons. A simulação **nunca** chama áudio diretamente.

| Sinal | Som |
|-------|-----|
| `trap_placed` / `trap_removed` | `sfx_place` / `sfx_remove` |
| `trap_triggered(anvil)` | `sfx_anvil_whistle` → (no `pig_captured`) `sfx_anvil_clang` |
| `trap_triggered(tnt)` | `sfx_tnt_fuse` → `sfx_tnt_boom` |
| `pig_captured(cause)` | `sfx_capture_pop` (+ `sfx_ravine_fall` se `cause == ravine`) |
| `pig_escaped` | `sfx_escape` |
| `phase_changed` | crossfade de música + `sfx_transition` |
| `wave_finished` | `stinger_win` / `stinger_lose` |

---

## 5. Licenças (obrigatório)

Mantenha um arquivo `CREDITS.md` na raiz do projeto com uma linha por asset de terceiros:

| Arquivo | Autor | Fonte (URL) | Licença |
|---------|-------|-------------|---------|
| `sfx_anvil_clang.wav` | … | … | CC0 |

- **Preferir CC0** (sem obrigação). CC-BY exige crédito no jogo (tela de créditos).
- **Evitar:** CC-BY-NC (bloqueia uso comercial futuro) e qualquer som de desenho animado real (são protegidos por direitos autorais).

---

*Audio Bible por Anton, GD-KS Audio Director*
