// Exporta levels.js -> recursos Godot (.tres) + tests/expected.json (paridade).
// Uso:  node export_levels.js           (fases + expected.json; armadilhas/porcos só se ainda não existirem)
//       node export_levels.js --force   (também sobrescreve data/traps e data/pigs)
// Fluxo: editar levels.js -> node validate.js -> node export_levels.js
'use strict';
const fs = require('fs');
const path = require('path');
const S = require('./sim.js');
const { levels } = require('./levels.js');

const ROOT = path.resolve(__dirname, '../../../..');          // pasta do projeto Godot (trap-builder)
const FORCE = process.argv.includes('--force');

const TRAP_META = {
  anvil: { name: 'Bigorna', tiles: 1, dir: false, persistent: false },
  tnt: { name: 'TNT', tiles: 1, dir: false, persistent: false },
  bait: { name: 'Isca Falsa', tiles: 1, dir: false, persistent: false },
  fan: { name: 'Ventilador', tiles: 2, dir: true, persistent: true },
  tar: { name: 'Piche', tiles: 1, dir: false, persistent: true },
};
const PIG_META = { pig: { file: 'pig', speed: 30, immune: false }, helmet: { file: 'pig_helmet', speed: 24, immune: true } };
const KIND = { S: 0, F: 1, J: 2, X: 3 };

const v2i = ([x, y]) => `Vector2i(${x}, ${y})`;
const arrV2i = list => `Array[Vector2i]([${(list || []).map(v2i).join(', ')}])`;
const str = s => JSON.stringify(s);

function write(rel, content, always = true) {
  const file = path.join(ROOT, rel);
  if (!always && fs.existsSync(file) && !FORCE) { console.log('  (mantido) ' + rel); return; }
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, content.replace(/\r\n/g, '\n'));
  console.log('  escrito  ' + rel);
}

function trapTres(id) {
  const m = TRAP_META[id];
  return `[gd_resource type="Resource" script_class="TrapData" format=3]

[ext_resource type="Script" path="res://scripts/resources/trap_data.gd" id="1_script"]

[resource]
script = ExtResource("1_script")
id = &"${id}"
display_name = ${str(m.name)}
cost = ${S.TRAPS[id].cost}
valid_tiles = ${m.tiles}
has_direction = ${m.dir}
persistent = ${m.persistent}
`;
}

function pigTres(type) {
  const m = PIG_META[type];
  return `[gd_resource type="Resource" script_class="PigData" format=3]

[ext_resource type="Script" path="res://scripts/resources/pig_data.gd" id="1_script"]

[resource]
script = ExtResource("1_script")
id = &"${m.file}"
speed = ${m.speed}
immune_anvil = ${m.immune}
`;
}

function levelTres(def) {
  const ext = [
    ['Script', 'res://scripts/resources/level_data.gd', '1_level'],
    ['Script', 'res://scripts/resources/route_node.gd', '2_node'],
    ['Script', 'res://scripts/resources/route_edge.gd', '3_edge'],
    ['Script', 'res://scripts/resources/spawn_entry.gd', '4_spawn'],
    ['Script', 'res://scripts/resources/trap_data.gd', '5_trap'],
  ];
  const pigTypes = [...new Set(def.pigs)];
  pigTypes.forEach(t => ext.push(['Resource', `res://data/pigs/${PIG_META[t].file}.tres`, `pig_${t}`]));
  def.traps.forEach(t => ext.push(['Resource', `res://data/traps/${t}.tres`, `trap_${t}`]));

  const subs = [];
  const nodes = [...def.nodes].sort((a, b) => a.id - b.id);
  const edges = [...def.edges].sort((a, b) => a.id - b.id);
  nodes.forEach(n => subs.push(`[sub_resource type="Resource" id="Node_${n.id}"]
script = ExtResource("2_node")
id = ${n.id}
cell = ${v2i(n.cell)}
kind = ${KIND[n.kind]}
`));
  edges.forEach(e => subs.push(`[sub_resource type="Resource" id="Edge_${e.id}"]
script = ExtResource("3_edge")
id = ${e.id}
from_id = ${e.from}
to_id = ${e.to}
via = ${arrV2i(e.via)}
`));
  def.pigs.forEach((t, i) => subs.push(`[sub_resource type="Resource" id="Spawn_${i}"]
script = ExtResource("4_spawn")
pig = ExtResource("pig_${t}")
`));

  const loadSteps = ext.length + subs.length + 1;
  return `[gd_resource type="Resource" script_class="LevelData" load_steps=${loadSteps} format=3]

${ext.map(([t, p, id]) => `[ext_resource type="${t}" path="${p}" id="${id}"]`).join('\n')}

${subs.join('\n')}
[resource]
script = ExtResource("1_level")
level_id = ${def.id}
title = ${str(def.name)}
route_seed = ${def.seed}
budget = ${def.budget}
par = ${def.par}
escape_limit = ${def.escapeLimit}
grid_size = Vector2i(${def.w}, ${def.h})
nodes = Array[ExtResource("2_node")]([${nodes.map(n => `SubResource("Node_${n.id}")`).join(', ')}])
edges = Array[ExtResource("3_edge")]([${edges.map(e => `SubResource("Edge_${e.id}")`).join(', ')}])
spawns = Array[ExtResource("4_spawn")]([${def.pigs.map((_, i) => `SubResource("Spawn_${i}")`).join(', ')}])
ravine = ${arrV2i(def.ravine)}
marked = ${arrV2i(def.marked)}
available_traps = Array[ExtResource("5_trap")]([${def.traps.map(t => `ExtResource("trap_${t}")`).join(', ')}])
decor_seed = ${def.id * 101}
`;
}

// ---------------------------------------------------------------------------
console.log('Projeto Godot: ' + ROOT);
for (const id of Object.keys(TRAP_META)) write(`data/traps/${id}.tres`, trapTres(id), false);
for (const t of Object.keys(PIG_META)) write(`data/pigs/${PIG_META[t].file}.tres`, pigTres(t), false);

const expected = { generated_by: 'export_levels.js', fork_hash: [], levels: [] };
for (const seed of [1, 2, 3, 4, 5, 123456]) for (let pig = 0; pig < 10; pig += 3) for (const node of [0, 1, 3, 7]) {
  expected.fork_hash.push({ seed, pig, node, value: S.forkHash(seed, pig, node) });
}
for (const def of levels) {
  const rel = `data/levels/level_${String(def.id).padStart(2, '0')}.tres`;
  write(rel, levelTres(def));
  const L = S.buildLevel(def);
  const refs = [{ name: 'vazio', plan: [] }, ...Object.entries(def.refs || {}).map(([name, plan]) => ({ name, plan }))];
  expected.levels.push({
    level_id: def.id, path: 'res://' + rel,
    refs: refs.map(({ name, plan }) => {
      const r = S.simulate(L, plan);
      return {
        name, plan: plan.map(p => ({ type: p.type, cell: p.cell, dir: p.dir || [1, 0] })),
        expect: { captured: r.captured, escaped: r.escaped, ticks: r.ticks, stars: r.stars, cost: r.cost, deadlock: r.deadlock },
      };
    }),
  });
}
write('tests/expected.json', JSON.stringify(expected, null, 2) + '\n');
console.log('OK');
