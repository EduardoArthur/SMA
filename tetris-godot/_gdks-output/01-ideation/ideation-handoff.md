# tetris — Handoff de Ideação para Design

**Destino:** Diana, direção de design · **Status:** preparado para revisão

## Resumo executivo

> Encaixe blocos neon, limpe linhas e marque pontos enquanto o ritmo acelera.

*tetris* é um puzzle clássico de blocos para PC com teclado. A partida tem uma fase e termina quando o tabuleiro enche. O gancho é sobreviver à queda cada vez mais rápida das peças num arcade retrô neon synthwave. O público previsto são fãs de puzzles clássicos de blocos. O projeto tem uma semana de desenvolvimento solo em Godot 4, com documentação mínima.

## Fundamentos do conceito

A experiência combina a satisfação de ver linhas sumirem e pontos subirem com a tensão de decidir cada vez mais rápido. A direção visual usa blocos coloridos em um tabuleiro escuro. A análise de mercado foi dispensada pelo usuário e consta como opcional no contrato da transição.

## Fundamentos das mecânicas

Mover e girar → encaixar → limpar linhas, liberar espaço e pontuar → escolher o próximo encaixe.

- **Velocidade.** A queda automática começa em 1 segundo por casa. A cada 30 segundos de jogo, o intervalo diminui 0,1 segundo até o mínimo de 0,2 segundo por casa.
- **Pontuação.** Limpar 1, 2, 3 ou 4 linhas no mesmo encaixe rende 100, 300, 500 ou 800 pontos. As linhas brilham brevemente antes de sumir, e o placar sobe quando elas somem.
- **Teclado.** `←` e `→` movem uma casa e repetem após breve pausa se mantidas pressionadas. `↑` gira no sentido horário. Segurar `↓` acelera a descida. `Espaço` faz a queda instantânea e fixa a peça.
- **Reserva.** `C` guarda a peça ativa e chama a próxima se a reserva estiver vazia, ou troca pela peça guardada. A reserva fica bloqueada até a peça ativa ser fixada. Giro, queda instantânea e reserva disparam uma ação por pressionamento.

## Prioridades para Design

Começar o GDD com as regras da partida e os controles. Validar primeiro uma partida completa com movimento, rotação, encaixe e fim de jogo. Em seguida, verificar limpeza, pontuação, aceleração, queda instantânea e reserva. Ajustar no protótipo a pausa e o ritmo da repetição lateral, a velocidade da descida com `↓` e a duração do brilho.

## Documentos de referência

- [Concept brief](concept-brief.md)
- [Exploração de mecânicas](mechanics-exploration.md)
- [Relatório de síntese](synthesis-report.md)
