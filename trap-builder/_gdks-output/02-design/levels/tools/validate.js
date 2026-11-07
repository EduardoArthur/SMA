// Valida as soluções de referência de todas as fases.
// Uso:  node validate.js            -> só simula as referências (rápido)
//       node validate.js --search   -> também roda o solver (lento: minutos)
'use strict';
const S = require('./sim.js');
const { report } = require('./solve.js');
const { levels } = require('./levels.js');

let ok = true;
for (const def of levels) {
  const L = S.buildLevel(def);
  console.log(`\n=== Fase ${def.id}: ${def.name} | orçamento ${def.budget} · par ${def.par} · fugas ${def.escapeLimit} · ${def.pigs.length} porcos · seed ${def.seed}`);
  for (const [name, plan] of Object.entries(def.refs || {})) {
    const r = S.simulate(L, plan);
    const expected = (name.match(/^★+/) || ['★★★'])[0].length; // estrelas esperadas = prefixo do nome
    const pass = r.stars === expected;
    ok = ok && pass;
    console.log(`  ${pass ? '✔' : '✘'} ${name}: custo ${r.cost} | capturados ${r.captured} · fugas ${r.escaped}${r.deadlock ? ' · DEADLOCK' : ''} | ★${r.stars}`);
    console.log(S.draw(L, plan).split('\n').map(l => '    ' + l).join('\n'));
  }
  if (process.argv.includes('--search')) report(def, { K: 3, localOpts: { restarts: 40, steps: 300 } });
}
console.log(ok ? '\nTodas as referências deram as estrelas esperadas.' : '\nFALHA em alguma referência.');
process.exit(ok ? 0 : 1);
