# Neon Stack - Quickstart (setup automático)

> **Data:** 2026-11-08 · Substitui a montagem manual dos guias M1/M3/M4 para chegar ao jogo jogável.
> ⚠️ Código e script escritos **sem compilar/rodar** (não há Unreal no ambiente onde foram gerados). Mande o log se algo falhar.

## O que mudou

| Antes | Agora |
|-------|-------|
| Criar à mão DA, Input Actions, IMCs, BPs, WBPs, mapas | **Um script Python** cria os assets e os mapas |
| 7 Widget Blueprints obrigatórios | **UI padrão em C++** (HUD, contagem, pausa, resultados, opções, menu). WBPs viram opcionais para estilizar |
| `BP_Board`, `BP_NSPlayerController`, `BP_*GameMode` obrigatórios | As **classes C++ vão direto no mapa**; campos vazios carregam assets padrão por caminho (`Source/Tetris/NSDefaultAssets.h`) |
| Camera Shake Blueprints | **Tremor procedural** no `NSFXDirector` (os shakes BP continuam opcionais) |

## Passo a passo

1. **Feche o editor** e compile (*Development Editor / Win64*). O `.uproject` agora habilita Niagara, Python e Editor Scripting Utilities.
2. Abra o projeto. (O mapa inicial ainda não existe; ignore o aviso.)
3. **Tools → Execute Python Script…** → `Scripts/setup_neon_stack.py`.
   - No **Output Log** aparece `[NeonStack] Setup concluído: N itens criados, M avisos.`
   - Se houver avisos, copie as linhas `[NeonStack]` e me mande.
4. Abra `Content/NeonStack/Maps/L_MainMenu` e aperte **Play**.
   - Enter em JOGAR → contagem → jogo. Esc/P pausa. Game over → resultados.
5. Rode os testes: **Tools → Session Frontend → Automation**, filtro `NeonStack` (12 testes).

## O que o script cria

| Pasta | Assets |
|-------|--------|
| `NeonStack/Data` | `DA_GameConfig`, `DA_Palettes` |
| `NeonStack/Input` | `IA_MoveLeft/Right`, `IA_SoftDrop`, `IA_HardDrop`, `IA_RotateCW/CCW`, `IA_Hold`, `IA_Pause`, `IMC_Gameplay`, `IMC_UI` |
| `NeonStack/Art/Materials` | `MPC_NeonStack`, `M_Block` (borda neon + pulso + flash), `M_Ghost`, `M_SynthSky` (gradiente + estrelas), `M_SynthSun` (sol listrado), `M_SynthFloor` (grade em perspectiva rolando), `M_Well` |
| `NeonStack/Maps` | `L_MainMenu` e `L_Game`: câmera ortográfica, post process (exposição manual, bloom), céu/sol/chão; no jogo também Board, Background, FXDirector, AudioDirector |

Rodar de novo é seguro: nada existente é sobrescrito. Para refazer um asset, apague-o e rode o script outra vez.

## Continua manual (opcional)

- **Sons e música** (audio-bible): importar e ligar no ator `AudioDirector` dos dois mapas, mais Sound Classes/Mix ([m4-m5-setup-guide.md](m4-m5-setup-guide.md) §6). Sem sons, o jogo roda mudo.
- **Niagara** (m4-m5 §5): sem eles, o feedback visual fica no pulso/flash dos blocos, no flash de tela e no tremor.
- **Estilo da UI:** criar WBPs com a classe C++ como pai ([m3-setup-guide.md](m3-setup-guide.md)) para trocar fontes/cores. Depois, apontar o WBP no ator/GameMode (via Blueprint filho) ou manter o padrão.

## Problemas comuns

| Sintoma | Causa / correção |
|---------|------------------|
| `Execute Python Script` não aparece | Plugins *Python Editor Script Plugin* e *Editor Scripting Utilities* desligados: Edit → Plugins |
| `AttributeError: module 'unreal' has no attribute 'NSGameConfig'` | O C++ não compilou/carregou; compile de novo com o editor fechado |
| Tela preta no Play | Mapa errado (abra `L_MainMenu`/`L_Game`) ou a câmera não está com *Auto Activate for Player 0* |
| Tudo branco/estourado | Ajuste *Exposure Compensation* no `PostProcess` do mapa (negativo escurece) |
| Blocos sem cor | `M_Block` não compilou: abra o material e veja o erro do nó Custom |
| Chão/sol de cabeça para baixo | Os materiais usam posição do mundo; se mover os planos, ajuste os números no nó Custom |
