# tetris — Game Design Document

## Visão e escopo

> Encaixe blocos neon, limpe linhas e marque pontos enquanto o ritmo acelera.

Puzzle clássico de blocos para PC com teclado. O protótipo tem uma fase e uma partida que dura até o tabuleiro encher. O desenvolvimento é solo, em uma semana, usando Godot 4.

## Pillars

- **Encaixes sob pressão.** A queda acelera durante a partida. O jogador precisa decidir cada vez mais rápido entre abrir espaço agora e preparar uma limpeza múltipla.
- **Recompensa visível.** A peça fica visivelmente fixa ao encaixar. Quando uma linha é completada, o brilho breve anuncia a limpeza e o placar sobe assim que a linha desaparece.
- **Horizonte Synthwave (opção A).** Sol rosa listrado e grade em perspectiva ao fundo, com detalhes em magenta e moldura ciano. O tabuleiro escuro mantém legíveis os blocos coloridos com acabamento chanfrado.

## Target Audience

Fãs de puzzles clássicos de blocos que procuram a satisfação de encaixar peças, limpar linhas e ver a pontuação subir conforme aumenta a pressão da partida.

## Core Mechanics

### Ciclo da partida

Mover e girar a peça em queda, encaixá-la na grade e escolher onde colocar a próxima. Ao fixar uma peça, as linhas horizontais completas brilham brevemente, desaparecem e liberam espaço. O placar é atualizado quando elas somem. A partida termina quando o tabuleiro enche.

### Velocidade e pontuação

A queda automática começa em **1 segundo por casa**. A cada **30 segundos de jogo**, o intervalo diminui **0,1 segundo**, até chegar a **0,2 segundo por casa** aos 4 minutos. A partir daí, o intervalo permanece nesse limite.

| Linhas limpas no mesmo encaixe | Pontos |
| --- | ---: |
| 1 | 100 |
| 2 | 300 |
| 3 | 500 |
| 4 | 800 |

### Controles

| Tecla | Comportamento |
| --- | --- |
| `←` / `→` | Move uma casa ao toque. Ao manter pressionada, repete após uma breve pausa. |
| `↑` | Gira a peça no sentido horário uma vez por pressionamento. |
| `↓` | Acelera a descida enquanto estiver pressionada. |
| `Espaço` | Leva a peça à posição mais baixa possível e a fixa imediatamente. Uma ação por pressionamento. |
| `C` | Guarda ou troca a peça uma vez por pressionamento. Após o uso, a reserva fica bloqueada até a peça ativa ser fixada. |

Com a reserva vazia, `C` guarda a peça ativa e coloca a próxima em jogo. Com a reserva ocupada, `C` troca a peça ativa pela guardada. A fixação, inclusive por queda instantânea, libera a reserva para a próxima jogada.

## Critérios de teste do protótipo

1. Jogar uma partida completa no teclado e confirmar que o tabuleiro cheio encerra a partida. Verificar o movimento de uma casa ao toque, a repetição lateral ao segurar e uma rotação por toque.
2. Conferir a queda inicial de 1 segundo por casa, a primeira redução aos 30 segundos e o limite de 0,2 segundo a partir dos 4 minutos. Segurar e soltar `↓` deve acelerar a descida e restaurar o ritmo automático, respectivamente.
3. Limpar de uma a quatro linhas no mesmo encaixe em testes separados. Em cada teste, verificar o brilho antes de as linhas desaparecerem e o valor correto somado ao placar na remoção.
4. Confirmar que `Espaço` fixa a peça na posição mais baixa possível. Testar `C` com a reserva vazia e ocupada, verificar que uma segunda tentativa antes da fixação não troca a peça e confirmar que a próxima peça pode usar a reserva.

## Ajustes de protótipo

Calibrar a duração do brilho, a pausa e o ritmo da repetição lateral e a velocidade da descida com `↓` durante os testes.

Base de referência: [handoff de Ideação](../../01-ideation/ideation-handoff.md).
