# tetris — Arquitetura Godot 4

O [GDD aprovado](../02-design/gdd/main.md) substitui os documentos de Planejamento como entrada deste fluxo. O protótipo tem uma única cena de partida, `Game.tscn`. A [árvore de cenas](scene-tree-overview.md) descreve seus nós.

## Scripts e estado local

| Script | Base | Responsabilidade |
| --- | --- | --- |
| `game.gd` | `Control` | Recebe o teclado, coordena a partida e o `FallTimer`, mantém o tempo, o placar, a peça em reserva e o bloqueio de troca. Mostra `GameOver`. |
| `board_state.gd` | `RefCounted` | Mantém a grade e a peça ativa, representada por quatro células. Valida movimentos e rotações, fixa a peça e identifica e remove linhas completas. Devolve o resultado das operações ao `Game`. |
| `board_view.gd` | `Control` | Desenha a grade, as peças e o brilho da limpeza. Sinaliza o fim do brilho. |
| `hud.gd` | `Control` | Exibe o placar, a peça em reserva e a indicação de troca bloqueada. |

`Game` possui a instância de `board_state.gd`. Todo o estado pertence à cena, com zero autoloads e conexões diretas, sem `EventBus`.

## Mapa de sinais

| Emissor e sinal | Destino | Efeito |
| --- | --- | --- |
| `FallTimer.timeout` | `Game` | Tenta descer a peça uma casa. Se a descida estiver bloqueada, inicia o encaixe. |
| `Board.clear_flash_finished` | `Game` | Remove as linhas completas, atualiza o placar e coloca a próxima peça em jogo. |
| `Game.score_changed` | `HUD` | Atualiza a pontuação exibida. |
| `Game.hold_changed` | `HUD` | Atualiza a peça em reserva e o indicador de bloqueio. |

## Regras que a arquitetura deve preservar

- A queda automática começa em 1 segundo por casa e reduz o intervalo em 0,1 segundo a cada 30 segundos de jogo, até 0,2 segundo por casa aos 4 minutos.
- Limpar 1, 2, 3 ou 4 linhas no mesmo encaixe rende 100, 300, 500 ou 800 pontos. O brilho precede a remoção e o placar é atualizado quando as linhas somem.
- `←` e `→` movem uma casa ao toque e repetem após breve pausa se mantidas pressionadas. `↑` gira no sentido horário. Segurar `↓` acelera a descida. `Espaço` desce até a posição mais baixa possível e fixa a peça imediatamente.
- `C` guarda a peça e traz a próxima se a reserva estiver vazia, ou troca pela peça guardada. Giro, queda instantânea e reserva disparam uma vez por pressionamento. A reserva volta a ficar disponível quando a peça ativa é fixada.
- A partida termina quando o tabuleiro enche. A duração do brilho, a repetição lateral e a velocidade de `↓` são parâmetros de ajuste do protótipo.
