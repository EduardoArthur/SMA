# tetris — Síntese de ideação

## Visão consolidada

*tetris* é um puzzle clássico de blocos para PC com teclado, em uma fase única, com blocos coloridos sobre um fundo escuro e estética arcade neon synthwave. O jogador move e gira peças para completar linhas, abrir espaço e ver a pontuação subir. A queda acelera ao longo da partida, reduzindo o tempo para decidir onde encaixar cada peça. A partida termina quando o tabuleiro enche.

O público previsto são fãs de puzzles clássicos de blocos. O escopo é uma versão jogável feita em uma semana por uma pessoa iniciante no Godot 4, com documentação mínima. A análise de mercado foi dispensada por decisão do usuário.

## Mecânicas aprovadas

- **Queda automática.** A peça começa caindo uma casa por segundo. A cada 30 segundos de jogo, o intervalo cai 0,1 segundo até o mínimo de 0,2 segundo por casa, atingido aos 4 minutos.
- **Limpeza e pontuação.** Limpar 1, 2, 3 ou 4 linhas no mesmo encaixe rende 100, 300, 500 ou 800 pontos, respectivamente. As linhas completas brilham brevemente, somem e, nesse momento, o placar é atualizado.
- **Movimento e giro.** `←` e `→` movem uma casa ao toque e repetem após uma breve pausa quando mantidas pressionadas. `↑` gira no sentido horário uma vez por pressionamento. `↓` acelera a descida enquanto estiver pressionada.
- **Queda instantânea.** `Espaço` leva a peça à posição mais baixa possível e a fixa imediatamente, uma vez por pressionamento.
- **Reserva.** `C` guarda a peça e chama a próxima se a reserva estiver vazia, ou troca a peça ativa pela guardada. O uso da reserva fica bloqueado até a peça ativa ser fixada.

## Coerência e escopo

A velocidade crescente sustenta a tensão do arcade. O brilho e a atualização do placar reforçam a satisfação de limpar linhas. A pontuação favorece limpezas múltiplas, enquanto liberar espaço cedo reduz o risco de encher o tabuleiro. A reserva permite preparar um encaixe futuro. As três perguntas sobre velocidade, pontuação e teclado no concept brief estão respondidas na exploração de mecânicas.

## Critérios propostos para o protótipo

1. Jogar uma partida completa no teclado, com movimento, giro, encaixe e fim de jogo.
2. Conferir a aceleração até o limite aprovado e os quatro valores de pontuação, com brilho antes da remoção das linhas e atualização do placar ao removê-las.
3. Verificar a queda instantânea, o primeiro uso da reserva, a troca com reserva ocupada e o bloqueio até a fixação.

## Riscos e pontos para design

- **Prazo solo de uma semana.** Validar primeiro a partida completa e então integrar pontuação, aceleração e ações auxiliares.
- **Resposta dos controles.** Ajustar em jogo a pausa e o ritmo da repetição lateral e a velocidade da descida com `↓`.
- **Legibilidade da limpeza.** Ajustar a duração do brilho para que a remoção das linhas e a atualização do placar fiquem claras mesmo nas velocidades mais altas.

## Fontes internas

- [Concept brief](concept-brief.md)
- [Exploração de mecânicas](mechanics-exploration.md)
