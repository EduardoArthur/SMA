// Solver de fases: exaustivo até K armadilhas + busca local para planos maiores.
'use strict';
const S = require('./sim.js');
const DIRS = [[1, 0], [0, 1], [-1, 0], [0, -1]];

function items(L) {
  const out = [];
  for (const k of L.routeCells) for (const ty of L.def.traps) if (S.TRAPS[ty].tile === 'R') out.push({ k, type: ty });
  if (L.def.traps.includes('fan')) for (const k of L.markedCells) for (const d of DIRS) out.push({ k, type: 'fan', dir: d });
  return out;
}
const toPlan = sel => sel.map(i => ({ type: i.type, cell: [Math.floor(i.k / 100), i.k % 100], dir: i.dir }));
const costOf = sel => sel.reduce((s, i) => s + S.TRAPS[i.type].cost, 0);

// score: menor é melhor. Fugas pesam muito; depois custo.
function score(L, sel) {
  const r = S.simulate(L, toPlan(sel));
  return { s: (r.deadlock ? 99 : r.escaped) * 10000 + costOf(sel), r };
}

function exhaustive(L, K, cap) {
  const its = items(L); let best = null, bestPass = null, sims = 0; const zeroByCost = new Map();
  const sel = [], used = new Set();
  (function rec(start, cost) {
    if (sel.length) {
      const r = S.simulate(L, toPlan(sel)); sims++;
      if (!r.deadlock && r.escaped === 0) { zeroByCost.set(cost, (zeroByCost.get(cost) || 0) + 1); if (!best || cost < best.cost) best = { cost, plan: toPlan(sel) }; }
      if (!r.deadlock && r.escaped <= L.def.escapeLimit && (!bestPass || cost < bestPass.cost)) bestPass = { cost, plan: toPlan(sel), escaped: r.escaped };
    }
    if (sel.length >= K) return;
    for (let i = start; i < its.length; i++) {
      const it = its[i]; const c = cost + S.TRAPS[it.type].cost;
      if (c > cap || used.has(it.k)) continue;
      sel.push(it); used.add(it.k); rec(i + 1, c); sel.pop(); used.delete(it.k);
    }
  })(0, 0);
  return { sims, best, bestPass, zeroByCost: [...zeroByCost].sort((a, b) => a[0] - b[0]) };
}

// gerador pseudo-aleatório determinístico (para reprodutibilidade da busca)
function rng(seed) { let s = seed >>> 0; return () => { s ^= s << 13; s >>>= 0; s ^= s >> 17; s ^= s << 5; s >>>= 0; return s / 4294967296; }; }

function local(L, { restarts = 60, steps = 400, seed = 7, budget } = {}) {
  const its = items(L); const R = rng(seed); budget = budget || L.def.budget;
  let globalBest = null;
  const pick = () => its[Math.floor(R() * its.length)];
  for (let r = 0; r < restarts; r++) {
    let sel = [];
    while (true) { const it = pick(); if (sel.some(s => s.k === it.k)) continue; if (costOf([...sel, it]) > budget) break; sel.push(it); if (R() < 0.25) break; }
    let cur = score(L, sel);
    for (let st = 0; st < steps; st++) {
      const cand = sel.slice(); const op = R();
      if (op < 0.3 && cand.length) cand.splice(Math.floor(R() * cand.length), 1);
      else if (op < 0.6 || !cand.length) { const it = pick(); if (!cand.some(s => s.k === it.k)) cand.push(it); }
      else { const i = Math.floor(R() * cand.length); const it = pick(); if (!cand.some((s, j) => j !== i && s.k === it.k)) cand[i] = it; }
      if (costOf(cand) > budget || !cand.length) continue;
      const sc = score(L, cand);
      if (sc.s <= cur.s) { sel = cand; cur = sc; }
    }
    if (cur.r.escaped === 0 && !cur.r.deadlock && (!globalBest || costOf(sel) < globalBest.cost)) globalBest = { cost: costOf(sel), plan: toPlan(sel) };
  }
  return globalBest;
}

function report(def, opts = {}) {
  const L = S.buildLevel(def);
  console.log(`\n=== Fase ${def.id}: ${def.name} | orç ${def.budget} par ${def.par} fugas ${def.escapeLimit} | ${def.pigs.length} porcos | seed ${def.seed}`);
  console.log(S.draw(L));
  console.log('tiles de rota:', L.routeCells.length, '| marcados:', L.markedCells.length);
  if (opts.refs) for (const [name, plan] of Object.entries(opts.refs)) {
    const r = S.simulate(L, plan);
    console.log(`ref "${name}": custo ${r.cost} | capt ${r.captured} fugas ${r.escaped} ${r.deadlock ? 'DEADLOCK' : ''} | ★${r.stars}`);
  }
  const ex = exhaustive(L, opts.K || 3, opts.cap || def.budget);
  console.log(`exaustivo (≤${opts.K || 3} armadilhas, ${ex.sims} sims): melhor 0 fugas = ${ex.best ? ex.best.cost : '—'} ${ex.best ? JSON.stringify(ex.best.plan) : ''}`);
  console.log(`  passa (fugas ≤ limite) mais barato = ${ex.bestPass ? ex.bestPass.cost + ' (' + ex.bestPass.escaped + ' fugas)' : '—'} | soluções 0 fugas por custo: ${JSON.stringify(ex.zeroByCost.slice(0, 8))}`);
  if (opts.local !== false) {
    const lb = local(L, opts.localOpts || {});
    console.log(`busca local: melhor 0 fugas = ${lb ? lb.cost + ' ' + JSON.stringify(lb.plan) : '—'}`);
  }
  return L;
}

module.exports = { exhaustive, local, report, items };
