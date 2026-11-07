// Trap Builder - gerador de arte vetorial (02-design/art/art-bible.md).
// Formas simples, contorno grosso #2B1B17, cores chapadas. Sem texto (o importador
// SVG do Godot tem suporte limitado a fontes). Retoque à vontade no Inkscape.
//
// Uso (na pasta do projeto):   node tools/generate_art.js
// Saída: art/pigs, art/wolf, art/traps (*.svg, desenhados a 128 px; o jogo usa a 64 px)
'use strict';
const fs = require('fs');
const path = require('path');

const OUT = path.resolve(__dirname, '..', 'art');
const INK = '#2b1b17';
const W = 6; // contorno a 128 px (= 3 px no jogo)

const svg = (w, h, body) => `<svg xmlns="http://www.w3.org/2000/svg" width="${w}" height="${h}" viewBox="0 0 ${w} ${h}">\n${body}\n</svg>\n`;
const st = (fill, stroke = INK, sw = W) => `fill="${fill}" stroke="${stroke}" stroke-width="${sw}" stroke-linejoin="round" stroke-linecap="round"`;
const circle = (cx, cy, r, fill, extra = '') => `<circle cx="${cx}" cy="${cy}" r="${r}" ${fill} ${extra}/>`;
const ellipse = (cx, cy, rx, ry, fill, extra = '') => `<ellipse cx="${cx}" cy="${cy}" rx="${rx}" ry="${ry}" ${fill} ${extra}/>`;
const rect = (x, y, w, h, r, fill) => `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${r}" ${fill}/>`;
const pathEl = (d, fill) => `<path d="${d}" ${fill}/>`;
const line = (x1, y1, x2, y2, color = INK, sw = W) => `<line x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke="${color}" stroke-width="${sw}" stroke-linecap="round"/>`;

function write(rel, content) {
  const file = path.join(OUT, rel);
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, content);
  console.log('  art/' + rel);
}

// ---------- porco (perfil, olhando para a direita) ----------
function pig(walk) {
  const legs = walk ? [[34, 92, -6], [48, 92, 6], [72, 92, -6], [86, 92, 6]] : [[34, 92, 0], [48, 92, 0], [72, 92, 0], [86, 92, 0]];
  return svg(128, 128, [
    ...legs.map(([x, y, dx]) => rect(x - 6 + dx, y, 12, 22, 5, st('#e77f9b'))),
    pathEl('M18 66 q-12 -6 -6 -16 q6 -8 10 2', st('none')),                         // rabinho
    ellipse(60, 70, 44, 32, st('#f4a6b8')),                                           // corpo
    pathEl('M74 46 L82 22 L94 46 Z', st('#e77f9b')),                                  // orelha
    ellipse(104, 70, 13, 11, st('#e77f9b')),                                          // focinho
    circle(101, 68, 2.5, `fill="${INK}"`), circle(108, 68, 2.5, `fill="${INK}"`),
    circle(86, 58, 6, `fill="#ffffff" stroke="${INK}" stroke-width="3"`),             // olho esperto
    circle(88, 58, 3, `fill="${INK}"`),
    ellipse(84, 78, 7, 4, `fill="#ff8fa8"`),                                          // bochecha
  ].join('\n'));
}

// ---------- lobo (cabeça e ombros, 3/4 de frente) ----------
function wolf(expr) {
  const gray = '#7a7f8c', light = '#c9cdd6', eye = '#ffe45c', red = '#d63a2f';
  const parts = [];
  const hatY = expr === 'angry' ? -26 : 0;
  // ombros + gravata-borboleta
  parts.push(pathEl('M28 160 q4 -38 52 -40 q48 2 52 40 Z', st('#5c6470')));
  parts.push(pathEl('M64 122 l16 8 l-16 8 Z M96 122 l-16 8 l16 8 Z', st(red, INK, 4)));
  // orelhas
  const earDrop = expr === 'sad' ? 14 : 0;
  parts.push(pathEl(`M44 ${62 + earDrop} L38 ${22 + earDrop * 2} L66 44 Z`, st(gray)));
  parts.push(pathEl(`M116 ${62 + earDrop} L122 ${22 + earDrop * 2} L94 44 Z`, st(gray)));
  // cabeça e focinho
  parts.push(ellipse(80, 72, 42, 40, st(gray)));
  parts.push(pathEl('M56 86 q24 34 48 0 q-24 -14 -48 0 Z', st(light)));
  parts.push(ellipse(80, 84, 9, 6, st(INK, INK, 2)));
  // olhos
  const eyes = (shape) => [58, 102].map(x => shape(x)).join('\n');
  switch (expr) {
    case 'watch': // binóculo
      parts.push(rect(52, 50, 56, 22, 8, st('#4a4e57')));
      parts.push(circle(66, 61, 11, st('#8ab6d6')), circle(94, 61, 11, st('#8ab6d6')));
      break;
    case 'lick':
      parts.push(eyes(x => pathEl(`M${x - 9} 62 q9 -6 18 0`, st('none'))));
      parts.push(pathEl('M90 98 q10 12 2 20 q-8 -2 -6 -14', st('#e77f9b', INK, 3)));
      break;
    case 'cheer':
    case 'feast':
      parts.push(eyes(x => pathEl(`M${x - 9} 64 q9 -10 18 0`, st('none'))));
      parts.push(pathEl('M64 98 q16 14 32 0', st('#ffffff', INK, 3)));
      parts.push(pathEl('M64 110 L80 132 L96 110 Z', st('#ffffff', INK, 3)));          // guardanapo
      parts.push(line(26, 92, 34, 132, INK, 4), line(22, 92, 22, 104, INK, 3), line(30, 92, 30, 104, INK, 3)); // garfo
      parts.push(pathEl('M132 92 q10 10 2 22 L130 132', st('#c9cdd6', INK, 4)));        // faca
      if (expr === 'feast') parts.push(...[[20, 40], [140, 36], [30, 18], [132, 14]].map(([x, y]) => pathEl(`M${x} ${y - 7} L${x + 2} ${y - 2} L${x + 7} ${y} L${x + 2} ${y + 2} L${x} ${y + 7} L${x - 2} ${y + 2} L${x - 7} ${y} L${x - 2} ${y - 2} Z`, `fill="#ffd84d"`)));
      break;
    case 'angry':
      parts.push(eyes(x => circle(x, 64, 8, st(eye, INK, 3)) + circle(x, 65, 3, `fill="${INK}"`)));
      parts.push(line(48, 50, 70, 58, INK, 5), line(112, 50, 90, 58, INK, 5));
      parts.push(pathEl('M64 100 l6 6 l6 -6 l6 6 l6 -6 l6 6', st('#ffffff', INK, 3)));
      break;
    case 'sad':
      parts.push(eyes(x => circle(x, 66, 7, st(eye, INK, 3)) + circle(x, 69, 3, `fill="${INK}"`)));
      parts.push(line(50, 56, 68, 52, INK, 4), line(110, 56, 92, 52, INK, 4));
      parts.push(pathEl('M68 106 q12 -10 24 0', st('none', INK, 4)));
      parts.push(pathEl('M104 74 q4 8 0 12 q-4 -4 0 -12 Z', `fill="#8ab6d6"`));
      break;
  }
  // chapéu-coco
  parts.push(`<g transform="translate(0 ${hatY})">`);
  parts.push(ellipse(80, 38, 40, 8, st(INK)));
  parts.push(pathEl('M54 38 q0 -30 26 -30 q26 0 26 30 Z', st('#3a2a24')));
  parts.push(rect(54, 28, 52, 7, 2, `fill="${red}"`));
  parts.push('</g>');
  return svg(160, 160, parts.join('\n'));
}

function bush() {
  return svg(220, 90, [[40, 58, 34], [84, 44, 40], [132, 46, 38], [176, 60, 32], [108, 64, 36]]
    .map(([x, y, r]) => circle(x, y, r, st('#3f8a4a'))).join('\n') + '\n' + rect(10, 66, 200, 24, 10, `fill="#3f8a4a"`));
}

// ---------- armadilhas (ação) ----------
const traps = {
  anvil: () => svg(128, 128, [
    pathEl('M30 46 H98 Q112 46 118 34 Q108 62 90 62 L84 78 H100 V96 H28 V78 H44 L38 62 Q22 62 30 46 Z', st('#4a4e57')),
    line(36, 52, 92, 52, '#8a909c', 4),
  ].join('\n')),
  anvil_fallen: () => svg(128, 128, [
    ellipse(64, 104, 50, 10, `fill="#000000" fill-opacity="0.25"`),
    pathEl('M24 58 H96 Q112 58 118 46 Q108 74 90 74 L84 88 H102 V104 H22 V88 H40 L34 74 Q18 74 24 58 Z', st('#4a4e57')),
    line(32, 64, 90, 64, '#8a909c', 4),
    circle(16, 104, 7, `fill="#f2e3c2" fill-opacity="0.8"`), circle(112, 106, 6, `fill="#f2e3c2" fill-opacity="0.8"`),
  ].join('\n')),
  tnt: () => svg(128, 128, [
    rect(22, 30, 84, 78, 6, st('#d63a2f')),
    rect(22, 56, 84, 26, 0, st('#f2e3c2', INK, 4)),
    // letras T N T com traços
    line(36, 62, 50, 62, INK, 4), line(43, 62, 43, 76, INK, 4),
    line(56, 76, 56, 62, INK, 4), line(56, 62, 70, 76, INK, 4), line(70, 76, 70, 62, INK, 4),
    line(78, 62, 92, 62, INK, 4), line(85, 62, 85, 76, INK, 4),
    pathEl('M86 30 q6 -14 18 -16', st('none', INK, 4)),
    circle(106, 14, 5, `fill="#e8b33a"`),
  ].join('\n')),
  bait: () => svg(128, 128, [
    pathEl('M40 46 q4 -10 0 -18 M64 42 q4 -10 0 -18 M88 46 q4 -10 0 -18', st('none', '#f2e3c2', 4)),
    ellipse(64, 66, 46, 10, st('#a3c244')),
    pathEl('M18 66 Q22 104 64 104 Q106 104 110 66 Z', st('#3d7dd9')),
  ].join('\n')),
  bait_empty: () => svg(128, 128, [
    ellipse(64, 66, 46, 10, st('#2b5ea6')),
    pathEl('M18 66 Q22 104 64 104 Q106 104 110 66 Z', st('#3d7dd9')),
  ].join('\n')),
  fan_base: () => svg(128, 128, [
    rect(24, 24, 80, 80, 12, st('#5c6470')),
    circle(64, 64, 34, st('#9aa3ad')),
    circle(64, 64, 26, `fill="none" stroke="${INK}" stroke-width="2"`),
  ].join('\n')),
  fan_blades: () => svg(128, 128, [0, 120, 240].map(a =>
    `<g transform="rotate(${a} 64 64)">${pathEl('M64 64 Q58 44 64 34 Q76 44 64 64 Z', st('#f2f6ff', INK, 3))}</g>`).join('\n') +
    '\n' + circle(64, 64, 6, `fill="${INK}"`)),
  tar: () => svg(128, 128, [
    pathEl('M22 74 Q18 54 42 52 Q56 40 76 50 Q104 46 108 70 Q112 92 84 94 Q64 104 44 96 Q20 96 22 74 Z', st('#1e1a24')),
    ellipse(54, 66, 10, 4, `fill="#4a4458"`), circle(82, 78, 5, `fill="none" stroke="#4a4458" stroke-width="3"`),
  ].join('\n')),
};

// versões blueprint: mesmo desenho, só traço branco
function blueprint(svgText) {
  return svgText
    .replace(/fill="(?!none)[^"]*"/g, 'fill="none"')
    .replace(/stroke="#[0-9a-fA-F]{6}"/g, 'stroke="#f2f6ff"')
    .replace(/fill-opacity="[^"]*"/g, '');
}

// ---------- ícone do jogo (res://icon.svg) ----------
// O Lobo Mau armando a emboscada: puxa a corda que segura a bigorna sobre o alvo.
function icon() {
  const strip = s => s.replace(/^<svg[^>]*>\n?/, '').replace(/<\/svg>\s*$/, '');
  const wolfInner = strip(wolf('lick'));
  const anvilInner = strip(traps.anvil());
  const grid = [];
  for (let i = 1; i < 6; i++) grid.push(line(i * 21.3, 0, i * 21.3, 128, '#3a6fb0', 1.5), line(0, i * 21.3, 128, i * 21.3, '#3a6fb0', 1.5));
  const pulley = [99, 17];
  const paw = [80, 50];
  return svg(128, 128, [
    '<defs><clipPath id="frame"><rect x="3" y="3" width="122" height="122" rx="22"/></clipPath></defs>',
    rect(3, 3, 122, 122, 22, st('#1f4e8c')),
    '<g clip-path="url(#frame)">',
    ...grid,
    // alvo no chão, à direita (onde a bigorna vai cair)
    ellipse(99, 113, 20, 5, `fill="#000000" fill-opacity="0.3"`),
    line(90, 107, 108, 119, '#ff6b5a', 4), line(108, 107, 90, 119, '#ff6b5a', 4),
    // viga + roldana
    line(70, 10, 126, 10, INK, 6),
    line(pulley[0], 10, pulley[0], pulley[1], INK, 3),
    circle(pulley[0], pulley[1], 7, st('#9aa3ad', INK, 4)),
    // corda: roldana -> bigorna (vertical) e roldana -> pata do lobo (diagonal)
    line(pulley[0], pulley[1] + 6, pulley[0], 50, '#e8b33a', 3),
    line(pulley[0] - 6, pulley[1] + 1, paw[0], paw[1], '#e8b33a', 3),
    // bigorna suspensa
    `<g transform="translate(72 31) scale(0.43)">${anvilInner}</g>`,
    // lobo à esquerda, maior
    `<g transform="translate(-10 36) scale(0.64)">${wolfInner}</g>`,
    // braço erguido ao lado da cabeça, pata segurando a corda
    pathEl(`M66 124 Q86 96 ${paw[0]} ${paw[1] + 6}`, st('none', INK, 11)),
    pathEl(`M66 124 Q86 96 ${paw[0]} ${paw[1] + 6}`, st('none', '#7a7f8c', 6)),
    circle(paw[0], paw[1], 7, st('#c9cdd6', INK, 3)),
    // brilho de "plano em andamento"
    pathEl('M20 14 L22 20 L28 22 L22 24 L20 30 L18 24 L12 22 L18 20 Z', `fill="#ffd84d"`),
    '</g>',
    rect(3, 3, 122, 122, 22, st('none')),
  ].join('\n'));
}

console.log('Gerando arte em ' + OUT);
fs.writeFileSync(path.resolve(__dirname, '..', 'icon.svg'), icon());
console.log('  icon.svg (ícone do projeto)');
write('pigs/pig.svg', pig(false));
write('pigs/pig_walk.svg', pig(true));
for (const e of ['watch', 'lick', 'cheer', 'angry', 'feast', 'sad']) write(`wolf/wolf_${e}.svg`, wolf(e));
write('wolf/bush.svg', bush());
for (const [id, fn] of Object.entries(traps)) {
  const s = fn();
  write(`traps/${id}.svg`, s);
  if (['anvil', 'tnt', 'bait', 'fan_base', 'tar'].includes(id)) write(`traps/${id}_bp.svg`, blueprint(s));
}
console.log('OK');
