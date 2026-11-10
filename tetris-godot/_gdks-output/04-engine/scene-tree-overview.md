# tetris — Árvore de cenas

## `Game.tscn`

```text
Game (Control)                 game.gd
├── Background (Control)       Horizonte Synthwave
├── Board (Control)            board_view.gd
├── HUD (Control)              hud.gd
├── FallTimer (Timer)           Queda automática
└── GameOver (Control)          Fim da partida
```

`Background` apresenta o sol rosa listrado e a grade em perspectiva. `Board` desenha a moldura ciano, a grade escura, os blocos coloridos chanfrados e o brilho das linhas completas. `HUD` mostra a pontuação e a reserva. `Game` exibe `GameOver` ao encerrar a partida. `Background` e `GameOver` usam os próprios nós da cena.

O quarto script, `board_state.gd`, é um `RefCounted` local mantido por `Game`. A divisão de responsabilidades e as conexões diretas estão no [documento de arquitetura](godot-architecture.md).
