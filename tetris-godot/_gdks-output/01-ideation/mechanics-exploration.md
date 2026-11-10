# tetris — Exploração de mecânicas

## Ciclo principal

Mover e girar a peça para encaixá-la → ver a peça fixar na grade → limpar linhas, liberar espaço e ganhar pontos → escolher o encaixe da próxima peça. A partida termina quando o tabuleiro enche.

## Core Mechanics

| Tecla | Ação | Resposta |
| --- | --- | --- |
| `←` / `→` | Mover a peça uma casa | Move ao toque. Se mantida pressionada, repete após uma breve pausa. |
| `↑` | Girar no sentido horário | Uma rotação por pressionamento. |
| `↓` | Acelerar a descida | Acelera enquanto estiver pressionada. |
| `Espaço` | Queda instantânea | Leva a peça à posição mais baixa possível e a fixa imediatamente. Uma ação por pressionamento. |
| `C` | Guardar ou trocar a peça | Uma ação por pressionamento. A reserva fica bloqueada até a peça ativa ser fixada. |

Com a reserva vazia, `C` guarda a peça ativa e coloca a próxima em jogo. Com a reserva ocupada, `C` troca a peça ativa pela guardada. Fixar a peça, inclusive com `Espaço`, libera o uso da reserva na próxima jogada.

## Mapa dos sistemas

- **Tempo e queda automática.** A peça começa caindo uma casa por segundo. A cada 30 segundos de jogo, o intervalo diminui em 0,1 segundo, até o mínimo de 0,2 segundo por casa, atingido aos 4 minutos.
- **Encaixe, limpeza e placar.** Ao fixar uma peça, a grade identifica as linhas completas. Elas brilham brevemente, somem e liberam espaço. O placar é atualizado quando as linhas somem.

| Linhas limpas no mesmo encaixe | Pontos |
| --- | ---: |
| 1 | 100 |
| 2 | 300 |
| 3 | 500 |
| 4 | 800 |

## Profundidade

Limpar uma linha imediatamente abre espaço. Esperar por uma limpeza múltipla pode render mais pontos, mas deixa o tabuleiro mais cheio. A reserva permite preparar um encaixe futuro, e o bloqueio até a próxima fixação torna importante escolher quando usá-la. A queda cada vez mais rápida reduz o tempo para decidir.

## Resposta e feedback

A peça fica visivelmente fixa na grade ao encaixar. O brilho neon breve sinaliza a limpeza antes de as linhas desaparecerem e o placar subir. A repetição lateral ajuda a atravessar a grade, enquanto giro, reserva e queda instantânea respondem uma vez por pressionamento.

## Prioridades do protótipo

1. Validar o tabuleiro, a queda, os movimentos, a rotação e o fim de jogo em uma partida completa.
2. Adicionar limpeza com brilho, atualização do placar e progressão de velocidade. Conferir os quatro valores de pontuação e o limite de queda.
3. Validar a descida acelerada, a queda instantânea e os dois casos da reserva. Conferir que uma segunda tentativa de troca só funciona depois que a peça ativa é fixada.

A duração exata do brilho, a pausa e o ritmo da repetição lateral e a velocidade da descida com `↓` ficam para ajuste no protótipo.
