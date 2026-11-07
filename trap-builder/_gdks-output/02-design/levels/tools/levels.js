// Definição das fases do protótipo (v0.1, validadas com solve.js).
// Grid 18x10 (x 0..17, y 0..9). Nós: S spawn, F bifurcação, J junção, X saída.
// Arestas: from -> to; 'via' = cantos do caminho (segmentos ortogonais).
// Células não declaradas = BLOQUEADO. ravine = BARRANCO (lota com 2 porcos). marked = MARCADO (ventilador).
'use strict';

const L1 = {
  id: 1, name: 'O Atalho', w: 18, h: 10, seed: 1,
  budget: 450, par: 300, escapeLimit: 1,
  traps: ['anvil'], pigs: Array(2).fill('pig'),
  nodes: [
    { id: 0, kind: 'S', cell: [0, 4] }, { id: 1, kind: 'F', cell: [3, 4] },
    { id: 2, kind: 'J', cell: [8, 4] }, { id: 3, kind: 'X', cell: [12, 4] },
  ],
  edges: [
    { id: 0, from: 0, to: 1 },
    { id: 1, from: 1, to: 2 },                         // atalho (4 tiles)
    { id: 2, from: 1, to: 2, via: [[3, 1], [8, 1]] },  // volta longa (10 tiles)
    { id: 3, from: 2, to: 3 },
  ],
  refs: { '★★★ 300': [{ type: 'anvil', cell: [5, 4] }, { type: 'anvil', cell: [3, 2] }],
          '★ 150': [{ type: 'anvil', cell: [2, 4] }] },
};

const L2 = {
  id: 2, name: 'Fila Indiana', w: 18, h: 10, seed: 2,
  budget: 550, par: 400, escapeLimit: 1,
  traps: ['anvil', 'tar', 'tnt'], pigs: Array(5).fill('pig'),
  nodes: [
    { id: 0, kind: 'S', cell: [0, 5] }, { id: 1, kind: 'F', cell: [3, 5] },
    { id: 2, kind: 'J', cell: [9, 5] }, { id: 3, kind: 'X', cell: [15, 5] },
  ],
  edges: [
    { id: 0, from: 0, to: 1 },
    { id: 1, from: 1, to: 2 },                                                       // reta curta
    { id: 2, from: 1, to: 2, via: [[3, 1], [5, 1], [5, 3], [7, 3], [7, 1], [9, 1]] }, // zigue-zague longo
    { id: 3, from: 2, to: 3, via: [[10, 5], [10, 8], [12, 8], [12, 5]] },            // U final (todos passam)
  ],
  refs: { '★★★ 400': [{ type: 'tnt', cell: [9, 4] }, { type: 'tar', cell: [10, 5] }, { type: 'anvil', cell: [10, 8] }] },
};

const L3 = {
  id: 3, name: 'Cheiro de Lavagem', w: 18, h: 10, seed: 3,
  budget: 600, par: 475, escapeLimit: 1,
  traps: ['bait', 'tar', 'tnt'], pigs: Array(6).fill('pig'),
  nodes: [
    { id: 0, kind: 'S', cell: [0, 4] }, { id: 1, kind: 'F', cell: [2, 4] },
    { id: 2, kind: 'J', cell: [7, 4] }, { id: 3, kind: 'F', cell: [9, 4] },
    { id: 4, kind: 'J', cell: [14, 4] }, { id: 5, kind: 'X', cell: [16, 4] },
  ],
  edges: [
    { id: 0, from: 0, to: 1 },
    { id: 1, from: 1, to: 2, via: [[2, 1], [7, 1]] },
    { id: 2, from: 1, to: 2, via: [[2, 8], [5, 8], [5, 6], [7, 6]] },
    { id: 3, from: 2, to: 3 },
    { id: 4, from: 3, to: 4 },
    { id: 5, from: 3, to: 4, via: [[9, 1], [14, 1]] },
    { id: 6, from: 4, to: 5 },
  ],
  refs: { '★★★ 475': [{ type: 'bait', cell: [2, 1] }, { type: 'tnt', cell: [2, 5] }, { type: 'tnt', cell: [3, 1] }] },
};

const L4 = {
  id: 4, name: 'Ventania', w: 18, h: 10, seed: 4,
  budget: 450, par: 300, escapeLimit: 1,
  traps: ['fan', 'bait', 'tar'], pigs: Array(4).fill('pig'),
  nodes: [
    { id: 0, kind: 'S', cell: [0, 5] }, { id: 1, kind: 'F', cell: [3, 5] },
    { id: 2, kind: 'J', cell: [12, 5] }, { id: 3, kind: 'X', cell: [15, 5] },
  ],
  edges: [
    { id: 0, from: 0, to: 1 },
    { id: 1, from: 1, to: 2, via: [[3, 2], [12, 2]] },
    { id: 2, from: 1, to: 2, via: [[3, 8], [12, 8]] },
    { id: 3, from: 2, to: 3 },
  ],
  ravine: [[6, 1], [9, 1], [7, 9]], marked: [[6, 3], [9, 3], [7, 7]],
  refs: { '★★★ 250 (mínimo)': [{ type: 'fan', cell: [6, 3], dir: [0, -1] }, { type: 'fan', cell: [9, 3], dir: [0, -1] }] },
};

const L5 = {
  id: 5, name: 'O Banquete', w: 18, h: 10, seed: 5,
  budget: 900, par: 750, escapeLimit: 2,
  traps: ['anvil', 'tnt', 'bait', 'fan', 'tar'], pigs: Array(10).fill('pig'),
  nodes: [
    { id: 0, kind: 'S', cell: [0, 5] }, { id: 1, kind: 'F', cell: [3, 5] },
    { id: 2, kind: 'J', cell: [10, 5] }, { id: 3, kind: 'F', cell: [12, 5] },
    { id: 4, kind: 'J', cell: [15, 5] }, { id: 5, kind: 'X', cell: [17, 5] },
  ],
  edges: [
    { id: 0, from: 0, to: 1 },
    { id: 1, from: 1, to: 2, via: [[3, 2], [10, 2]] },
    { id: 2, from: 1, to: 2, via: [[3, 8], [6, 8], [6, 6], [8, 6], [8, 8], [10, 8]] },
    { id: 3, from: 2, to: 3 },
    { id: 4, from: 3, to: 4 },
    { id: 5, from: 3, to: 4, via: [[12, 2], [15, 2]] },
    { id: 6, from: 4, to: 5 },
  ],
  ravine: [[5, 1], [6, 1], [7, 1], [13, 6], [14, 6]], marked: [[6, 3], [13, 4]],
  refs: {
    '★★★ 725 (melhor encontrado)': [
      { type: 'fan', cell: [6, 3], dir: [0, -1] }, { type: 'bait', cell: [10, 3] },
      { type: 'tnt', cell: [12, 4] }, { type: 'fan', cell: [13, 4], dir: [0, 1] }, { type: 'tnt', cell: [11, 5] }],
    '★★★ 750 (com bigorna)': [
      { type: 'bait', cell: [12, 2] }, { type: 'tnt', cell: [15, 4] }, { type: 'tnt', cell: [16, 5] },
      { type: 'fan', cell: [6, 3], dir: [0, -1] }, { type: 'anvil', cell: [7, 6] }],
  },
};

module.exports = { levels: [L1, L2, L3, L4, L5] };
