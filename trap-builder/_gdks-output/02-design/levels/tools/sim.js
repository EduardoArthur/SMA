// Trap Builder - simulador de referência (spec: 02-design/technical/technical-design.md v0.1)
// Simulação determinística em ticks inteiros. Serve para validar fases e como referência para o port em GDScript.
'use strict';

const TICK_RATE = 60, TU = 1200, SPAWN_INT = 60;
const RAVINE_CAP_DEFAULT = 2; // porcos por tile de barranco antes de lotar (0 = sem limite)
const SHADOW = 24, CLIMB = 120, STUN = 30, FUSE = 18, EAT = 180, PUSH = 15, FAN_RANGE = 3;
const PIGS = { pig: { speed: 30, immune: false }, helmet: { speed: 24, immune: true } };
const TRAPS = {
  anvil: { cost: 150, tile: 'R' }, tnt: { cost: 200, tile: 'R' }, bait: { cost: 75, tile: 'R' },
  fan: { cost: 125, tile: 'M' }, tar: { cost: 50, tile: 'R' },
};
const DIRS = [[1, 0], [0, 1], [-1, 0], [0, -1]];

// ---- hash de bifurcação (idêntico ao GDScript, aritmética de 64 bits) ----
const M32 = 0xFFFFFFFFn;
function forkHash(seed, pig, node) {
  let h = (BigInt(seed) * 73856093n) ^ (BigInt(pig) * 19349663n) ^ (BigInt(node) * 83492791n);
  h &= M32;
  h = ((h ^ (h >> 16n)) * 0x45d9f3bn) & M32;
  h = ((h ^ (h >> 16n)) * 0x45d9f3bn) & M32;
  h = h ^ (h >> 16n);
  return Number(h);
}

const key = (x, y) => x * 100 + y;

// ---- montagem da fase ----
function expand(points) {
  const cells = [];
  for (let i = 0; i < points.length - 1; i++) {
    let [x, y] = points[i]; const [tx, ty] = points[i + 1];
    const dx = Math.sign(tx - x), dy = Math.sign(ty - y);
    if (dx && dy) throw new Error('segmento diagonal em ' + points[i]);
    if (i === 0) cells.push([x, y]);
    while (x !== tx || y !== ty) { x += dx; y += dy; cells.push([x, y]); }
  }
  return cells;
}

function buildLevel(def) {
  const L = { def, nodes: [], edges: [], cell: new Map(), type: new Map() };
  for (const n of def.nodes) { L.nodes[n.id] = { ...n, outs: [] }; L.type.set(key(...n.cell), 'N'); L.cell.set(key(...n.cell), { k: 'n', node: n.id }); }
  for (const e of def.edges) {
    const from = L.nodes[e.from], to = L.nodes[e.to];
    const full = expand([from.cell, ...(e.via || []), to.cell]);
    const cells = full.slice(1, -1);
    if (!cells.length) throw new Error('aresta sem células ' + e.id);
    cells.forEach((c, idx) => {
      const k = key(...c);
      if (L.cell.has(k)) throw new Error(`célula repetida ${c} (aresta ${e.id})`);
      L.cell.set(k, { k: 'e', edge: e.id, idx }); L.type.set(k, 'R');
    });
    L.edges[e.id] = { id: e.id, from: e.from, to: e.to, cells };
    from.outs.push(e.id);
  }
  for (const n of L.nodes) n.outs.sort((a, b) => a - b);
  for (const c of def.marked || []) L.type.set(key(...c), 'M');
  for (const c of def.ravine || []) L.type.set(key(...c), 'V');
  // células não declaradas = BLOQUEADO ('B')
  L.routeCells = []; L.markedCells = [];
  for (const [k, t] of L.type) { if (t === 'R') L.routeCells.push(k); if (t === 'M') L.markedCells.push(k); }
  L.routeCells.sort((a, b) => a - b); L.markedCells.sort((a, b) => a - b);
  return L;
}
const typeAt = (L, x, y) => L.type.get(key(x, y)) || 'B';

// ---- simulação ----
// plan: [{type, cell:[x,y], dir?:[dx,dy]}]
function simulate(L, plan, opts = {}) {
  const def = L.def;
  const log = opts.log ? [] : null;
  const edgeBlocked = new Array(L.edges.length).fill(false);
  const traps = plan.map((p, id) => ({ id, ...p, k: key(...p.cell), triggered: false, fallen: false, consumed: false, timer: 0, victim: null, eater: null, current: null }));
  const trapAt = new Map(); for (const t of traps) trapAt.set(t.k, t);
  for (const t of traps) if (t.type === 'fan') {
    t.current = new Set(); let [x, y] = t.cell;
    for (let i = 0; i < FAN_RANGE; i++) {
      x += t.dir[0]; y += t.dir[1]; const ty = typeAt(L, x, y);
      if (ty === 'B' || ty === 'M') break; t.current.add(key(x, y));
    }
  }
  const pigs = def.pigs.map((type, id) => ({ id, type, speed: PIGS[type].speed, immune: PIGS[type].immune, st: 'pending', timer: 0, loc: null, sub: 0, pushedBy: new Set() }));
  const spawnNode = def.nodes.find(n => n.kind === 'S').id;
  let nextSpawn = 0, nextSpawnTick = 0, captured = 0, escaped = 0, tick = 0;
  const escapes = [];
  const ravineCap = def.ravineCap === undefined ? RAVINE_CAP_DEFAULT : def.ravineCap;
  const ravineFill = new Map();

  const cellOf = p => p.loc.k === 'n' ? L.nodes[p.loc.node].cell : L.edges[p.loc.edge].cells[p.loc.idx];
  const active = p => p.st !== 'pending' && p.st !== 'cap' && p.st !== 'esc';
  const occupied = (k, self) => pigs.some(q => q !== self && active(q) && key(...cellOf(q)) === k);
  const capture = (p, cause) => { if (!active(p)) return; p.st = 'cap'; captured++; log && log.push(`t${tick} porco ${p.id} capturado (${cause})`); };
  const escape = p => { p.st = 'esc'; escaped++; escapes.push(p.id); log && log.push(`t${tick} porco ${p.id} FUGIU`); };
  const baitActiveOn = eid => L.edges[eid].cells.some(c => { const t = trapAt.get(key(...c)); return t && t.type === 'bait' && !t.consumed; });

  function chooseEdge(p, nodeId) {
    const outs = L.nodes[nodeId].outs;
    const avail = outs.filter(e => !edgeBlocked[e]);
    const baited = avail.filter(baitActiveOn);
    if (baited.length) return baited[0];
    const opts2 = avail.length ? avail : outs;
    return opts2[forkHash(def.seed, p.id, nodeId) % opts2.length];
  }

  function enter(p, entered) {
    const [x, y] = cellOf(p);
    if (p.loc.k === 'n' && L.nodes[p.loc.node].kind === 'X') { escape(p); return; }
    entered.push(p);
  }

  function processEntered(entered) {
    for (let i = 0; i < entered.length; i++) {
      const p = entered[i];
      if (!active(p) || (p.st !== 'walk' && p.st !== 'back')) continue;
      const k = key(...cellOf(p)); const t = trapAt.get(k);
      if (t && !t.consumed) {
        if (t.type === 'anvil') {
          if (t.fallen) { p.st = 'climb'; p.timer = CLIMB; }
          else if (!t.triggered) { t.triggered = true; t.timer = SHADOW; t.victim = p; p.st = 'look'; }
        } else if (t.type === 'tnt' && !t.triggered) { t.triggered = true; t.timer = FUSE; }
        else if (t.type === 'bait' && !t.eater) { t.eater = p; p.st = 'eat'; p.timer = EAT; }
      }
      if (p.st !== 'walk' && p.st !== 'back') continue;
      for (const f of traps) {
        if (f.type !== 'fan' || !f.current.has(k) || p.pushedBy.has(f.id)) continue;
        p.pushedBy.add(f.id);
        const [x, y] = cellOf(p); const dx = x + f.dir[0], dy = y + f.dir[1]; const ty = typeAt(L, dx, dy); const dk = key(dx, dy);
        if (ty === 'V' && (!ravineCap || (ravineFill.get(dk) || 0) < ravineCap)) { ravineFill.set(dk, (ravineFill.get(dk) || 0) + 1); capture(p, 'barranco'); break; }
        const at = trapAt.get(dk);
        if ((ty === 'R' || ty === 'N') && !occupied(dk, p) && !(at && at.type === 'anvil' && at.fallen)) {
          const c = L.cell.get(dk); p.loc = c.k === 'n' ? { k: 'n', node: c.node } : { k: 'e', edge: c.edge, idx: c.idx };
          p.sub = 0; p.st = 'push'; p.timer = PUSH;
          log && log.push(`t${tick} porco ${p.id} empurrado para ${dx},${dy}`);
          if (c.k === 'n' && L.nodes[c.node].kind === 'X') { escape(p); break; }
          // o efeito de chegada é aplicado ao fim do empurrão (ver timers)
          p.arrivePending = true;
          break;
        }
      }
    }
  }

  const MAX = opts.maxTicks || 20000;
  for (tick = 0; tick < MAX; tick++) {
    // 1. spawn
    if (nextSpawn < pigs.length && tick >= nextSpawnTick) {
      const sk = key(...L.nodes[spawnNode].cell);
      if (!occupied(sk, null)) {
        const p = pigs[nextSpawn++]; p.st = 'walk'; p.loc = { k: 'n', node: spawnNode }; p.sub = 0;
        nextSpawnTick = tick + SPAWN_INT;
      }
    }
    const entered = [];
    // 2. timers: armadilhas, depois porcos
    for (const t of traps) {
      if (t.type === 'anvil' && t.triggered && !t.fallen) {
        if (--t.timer <= 0) {
          t.fallen = true; const v = t.victim;
          if (v && active(v)) { if (v.immune) { v.st = 'stun'; v.timer = STUN; } else capture(v, 'bigorna'); }
          const c = L.cell.get(t.k); edgeBlocked[c.edge] = true;
          log && log.push(`t${tick} bigorna cai em ${t.cell}; aresta ${c.edge} bloqueada`);
          for (const q of pigs) if (active(q) && q.loc.k === 'e' && q.loc.edge === c.edge && q.loc.idx < c.idx && (q.st === 'walk' || q.st === 'push')) { q.st = 'back'; q.sub = 0; }
        }
      } else if (t.type === 'tnt' && t.triggered && !t.consumed) {
        if (--t.timer <= 0) {
          t.consumed = true;
          for (const q of pigs) if (active(q)) { const [x, y] = cellOf(q); if (Math.max(Math.abs(x - t.cell[0]), Math.abs(y - t.cell[1])) <= 1) capture(q, 'tnt'); }
        }
      }
    }
    for (const p of pigs) {
      if (!active(p)) continue;
      if (p.st === 'eat' && --p.timer <= 0) { const t = trapAt.get(key(...cellOf(p))); t.consumed = true; p.st = 'walk'; }
      else if (p.st === 'climb' && --p.timer <= 0) { p.st = 'walk'; }
      else if (p.st === 'stun' && --p.timer <= 0) { p.st = 'walk'; }
      else if (p.st === 'push' && --p.timer <= 0) { p.st = 'walk'; if (p.arrivePending) { p.arrivePending = false; entered.push(p); } }
    }
    // 3. movimento (ordem de id)
    for (const p of pigs) {
      if (!active(p) || (p.st !== 'walk' && p.st !== 'back')) continue;
      const here = key(...cellOf(p)); const t = trapAt.get(here);
      let sp = p.speed; if (t && t.type === 'tar') sp = Math.floor(sp / 2);
      p.sub += sp; if (p.sub < TU) continue;
      if (p.st === 'back') {
        p.sub -= TU; const e = L.edges[p.loc.edge];
        if (p.loc.idx > 0) p.loc = { k: 'e', edge: e.id, idx: p.loc.idx - 1 };
        else { p.loc = { k: 'n', node: e.from }; p.st = 'walk'; }
        enter(p, entered); continue;
      }
      let next;
      if (p.loc.k === 'n') { const eid = chooseEdge(p, p.loc.node); next = { k: 'e', edge: eid, idx: 0 }; }
      else { const e = L.edges[p.loc.edge]; next = p.loc.idx < e.cells.length - 1 ? { k: 'e', edge: e.id, idx: p.loc.idx + 1 } : { k: 'n', node: e.to }; }
      const nc = next.k === 'n' ? L.nodes[next.node].cell : L.edges[next.edge].cells[next.idx];
      if (occupied(key(...nc), p)) { p.sub = TU - 1; continue; }
      p.sub -= TU; p.loc = next; enter(p, entered);
    }
    // 4/5. gatilhos e ventiladores
    processEntered(entered);
    // 6. fim
    if (nextSpawn === pigs.length && captured + escaped === pigs.length) break;
  }
  const deadlock = captured + escaped < pigs.length;
  const cost = plan.reduce((s, p) => s + TRAPS[p.type].cost, 0);
  const fail = escaped > def.escapeLimit || deadlock;
  const stars = fail ? 0 : escaped === 0 ? (cost <= def.par ? 3 : 2) : 1;
  return { captured, escaped, escapes, ticks: tick, deadlock, cost, stars, log };
}

// ---- busca exaustiva: todas as combinações com custo <= cap ----
function search(L, cap, opts = {}) {
  const def = L.def; const items = [];
  for (const k of L.routeCells) for (const ty of def.traps) if (TRAPS[ty].tile === 'R') items.push({ k, type: ty, cost: TRAPS[ty].cost });
  if (def.traps.includes('fan')) for (const k of L.markedCells) for (const d of DIRS) items.push({ k, type: 'fan', dir: d, cost: TRAPS.fan.cost });
  const minCost = Math.min(...items.map(i => i.cost));
  let sims = 0; const best = { zero: null, pass: null }; const zeroSolutions = []; const byCost = new Map();
  const chosen = []; const used = new Set();
  function rec(start, cost) {
    if (chosen.length) {
      const plan = chosen.map(i => ({ type: i.type, cell: [Math.floor(i.k / 100), i.k % 100], dir: i.dir }));
      const r = simulate(L, plan); sims++;
      if (r.escaped === 0 && !r.deadlock) {
        zeroSolutions.push({ cost, plan });
        byCost.set(cost, (byCost.get(cost) || 0) + 1);
        if (!best.zero || cost < best.zero.cost) best.zero = { cost, plan, r };
      }
      if (!r.deadlock && r.escaped <= def.escapeLimit && (!best.pass || cost < best.pass.cost)) best.pass = { cost, plan, r };
    }
    if (chosen.length >= (opts.maxTraps || 6)) return;
    for (let i = start; i < items.length; i++) {
      const it = items[i]; if (cost + it.cost > cap || used.has(it.k)) continue;
      chosen.push(it); used.add(it.k); rec(i + 1, cost + it.cost); chosen.pop(); used.delete(it.k);
    }
  }
  rec(0, 0);
  return { sims, best, zeroCount: zeroSolutions.length, byCost: [...byCost.entries()].sort((a, b) => a[0] - b[0]), zeroSolutions };
}

// ---- desenho ASCII ----
function draw(L, plan = []) {
  const W = L.def.w, H = L.def.h; const rows = [];
  const sym = { anvil: 'A', tnt: 'T', bait: 'I', fan: 'F', tar: 'p' };
  for (let y = 0; y < H; y++) {
    let s = '';
    for (let x = 0; x < W; x++) {
      const k = key(x, y); const t = L.type.get(k); const tr = plan.find(p => key(...p.cell) === k);
      if (tr) { s += sym[tr.type]; continue; }
      if (t === 'N') { const n = L.nodes.find(n => key(...n.cell) === k); s += n.kind === 'S' ? 'S' : n.kind === 'X' ? 'X' : n.kind === 'F' ? '+' : 'o'; }
      else s += t === 'R' ? '.' : t === 'M' ? 'm' : t === 'V' ? '~' : ' ';
    }
    rows.push(s.replace(/\s+$/, ''));
  }
  return rows.join('\n');
}

module.exports = { simulate, search, buildLevel, draw, forkHash, TRAPS };
