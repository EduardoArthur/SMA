# Neon Stack - Design Handoff

> **De:** Design Team (Diana, Theo, Aurora, Anton, Leo, Core Loop Specialist)
> **Coordenação:** Dylan (Design Coordinator) 📋
> **Para:** Engine Team (Ulysses, Priscilla, Simon, Bella, Eric)
> **Data:** 2026-11-07 · **Status:** Design concluído, pronto para a implementação

---

## 1. Pacote de documentos

| Documento | Responsável | Palavras | Conteúdo |
|-----------|-------------|----------|----------|
| [gdd/main.md](gdd/main.md) | Diana | ~2.400 | Fonte de verdade: pilares, regras, pontuação, telas, opções, features F1-F15 |
| [core-design/pillars/design-pillars.md](core-design/pillars/design-pillars.md) | Diana | ~590 | Pilares, filtros, desempate P1 > P3 > P2 |
| [core-design/mechanics/core-mechanics.md](core-design/mechanics/core-mechanics.md) | Theo | ~2.240 | **Spec de implementação**: coordenadas, peças, chutes, DAS/ARR, lock delay, estados, eventos, DataAsset, 12 testes |
| [core-design/loops/gameplay-loops.md](core-design/loops/gameplay-loops.md) | Core Loop | ~800 | Micro, core, sessão e meta loops |
| [core-design/progression/progression-system.md](core-design/progression/progression-system.md) | Theo | ~970 | Curva de gravidade, ritmo, paletas, SaveGame, métricas |
| [art/art-bible.md](art/art-bible.md) | Aurora | ~2.070 | Blocos/material, cores, paletas, fundo, VFX, pós-processamento, UI, assets |
| [audio/audio-bible.md](audio/audio-bible.md) | Anton | ~1.680 | Música reativa, SFX afinados, catálogo, mixagem, MetaSounds |
| [level-design/levels.md](level-design/levels.md) | Leo | ~940 | Mapas, layout da arena em uu, câmera, estágios |

Narrativa: **não se aplica** (anti-pilar; o preset solo-indie não exige).

---

## 2. Revisão de consistência (Dylan)

| Verificação | Resultado |
|-------------|-----------|
| Regras GDD × core-mechanics | ✅ Idênticas; o GDD remete à spec para os detalhes |
| Gravidade (Ideação × progressão) | ✅ Corrigido: nível 1 = 1,0 s em todos os docs |
| Música (GDD F15 × progressão × audio) | ✅ Alinhados: faixa única + filtro no MVP; stems como evolução futura |
| Cores das peças × paletas | ✅ Peças fixas; só o ambiente muda (art-bible §4) |
| Eventos C++ × Art/Audio | ✅ Todos os VFX/SFX mapeados para os eventos de core-mechanics §10 |
| Valores de câmera/HUD | ✅ Poço ~80% da altura (art-bible §2 = levels §2.2) |
| Pilares × decisões | ✅ Matriz em design-pillars; motion blur cortado |
| Perguntas bloqueadoras | ✅ Nenhuma (título final em aberto, não bloqueia) |

---

## 3. Ordem de construção recomendada

Derivada das prioridades e dependências do GDD §8.

| Marco | Features | Resultado verificável | Estimativa |
|-------|----------|-----------------------|-----------|
| **M1: Núcleo jogável (C++)** | F1 Board · F2 Peças/rotação · F3 Gravidade/lock · F4 Game over | Partida completa com cubos crus; testes T1-T6, T9, T10 passam | 3-5 dias |
| **M2: Controle justo** | F5 Saco de 7 + preview · F6 Hold · F7 Ghost · F8 DAS/ARR | Nível 10 parece justo; testes T7, T8 passam | 2-4 dias |
| **M3: Jogo completo** | F9 Pontuação/níveis · F10 Save · F11 Telas + HUD | Fluxo inicial → jogo → resultados; testes T11, T12 passam | 2-3 dias |
| **M4: Neon & juice** | F12 Visual neon · F13 VFX · F14 SFX | Checklists de art-bible §10 e audio-bible §8 | 3-5 dias |
| **M5: Música reativa** | F15 | Filtro/volume reagem ao nível e ao combo | 1-2 dias |

**Total:** ~11-19 dias de trabalho, dentro do prazo de 4 semanas.

---

## 4. Decisões técnicas para o Engine Team confirmar

1. Lógica em **classe C++ testável** (sem rendering) + `ABoard` fino que faz o tick e emite delegates *(recomendado em core-mechanics §10)*.
2. Renderização com **Instanced Static Mesh** + `PerInstanceCustomData` *(art-bible §3.2)*.
3. **Câmera ortográfica** única; fundo com materiais *(levels §2.2)*.
4. `UNeonStackConfig`, `DA_Palettes`, `DA_Audio` como DataAssets; `USaveGame` para o recorde e as opções.
5. Enhanced Input para teclado; DAS/ARR na lógica, não no Input Action.
6. Testes da lógica: Automation Tests do UE5 (12 casos de core-mechanics §13).

---

## 5. ⚠️ Atenção: trilha Standard × contrato do Engine

A trilha **Standard** pula a fase de Planejamento (Ideação → Design → Engine). Porém, o contrato `phase-03-to-04` do GD-KS exige `roadmap.md`, `epics.md`, `stories.md` e `sprint-plan.md` antes do Engine.

Opções:
- **A.** Seguir direto para o Engine usando a ordem de construção da §3 como roadmap (fiel à trilha Standard).
- **B.** Rodar um Planejamento leve (Sam/Peter) só para gerar os 4 documentos a partir da §3, satisfazendo o contrato.

---

## 6. Assinaturas

- [x] **Diana** (Game Design Director): GDD e pilares
- [x] **Dylan** (Design Coordinator): revisão de consistência e handoff

**Fase de Design concluída! 🎉**

*Handoff por Dylan, GD-KS Design Coordinator.*
