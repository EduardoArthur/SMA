// Trap Builder - gerador de áudio (02-design/audio/audio-bible.md).
// Tudo é sintetizado a partir de ondas básicas: o áudio é 100% seu, sem licença.
// Troque qualquer arquivo depois por uma versão melhor (mesmo nome).
//
// Uso (na pasta do projeto):   node tools/generate_audio.js
// Saída: audio/sfx/*.wav e audio/music/*.wav (mono, 16 bits, 44,1 kHz)
'use strict';
const fs = require('fs');
const path = require('path');

const SR = 44100;
const OUT = path.resolve(__dirname, '..', 'audio');

// ---------- utilidades ----------
let seed = 12345;
const rand = () => { seed = (seed * 1664525 + 1013904223) >>> 0; return seed / 4294967296 * 2 - 1; };
const buf = s => new Float32Array(Math.ceil(s * SR));
const midi = n => 440 * Math.pow(2, (n - 69) / 12);
const lerp = (a, b, t) => a + (b - a) * t;

function osc(wave, ph) {
  ph -= Math.floor(ph);
  switch (wave) {
    case 'sq': return ph < 0.5 ? 1 : -1;
    case 'tri': return 4 * Math.abs(ph - 0.5) - 1;
    case 'saw': return 2 * ph - 1;
    default: return Math.sin(2 * Math.PI * ph);
  }
}

// Tom com frequência f(t), envelope ataque/decaimento exponencial e filtro passa-baixa opcional.
function tone(b, start, dur, f, { wave = 'sine', vol = 0.5, attack = 0.005, decay = null, release = 0.03, lp = 0, vib = 0, vibRate = 6 } = {}) {
  const s0 = Math.floor(start * SR), n = Math.floor(dur * SR);
  let ph = 0, y = 0;
  for (let i = 0; i < n && s0 + i < b.length; i++) {
    const t = i / SR;
    const fr = (typeof f === 'function' ? f(t, t / dur) : f) * (1 + vib * Math.sin(2 * Math.PI * vibRate * t));
    ph += fr / SR;
    let env = Math.min(1, t / attack) * Math.min(1, (dur - t) / release);
    if (decay) env *= Math.exp(-t / decay);
    let x = osc(wave, ph) * vol * env;
    if (lp) { y += lp * (x - y); x = y; }
    b[s0 + i] += x;
  }
}

// Ruído filtrado: lp/hp podem ser funções do tempo normalizado (0..1).
function noise(b, start, dur, { vol = 0.5, attack = 0.002, decay = null, release = 0.02, lp = null, hp = null } = {}) {
  const s0 = Math.floor(start * SR), n = Math.floor(dur * SR);
  let low = 0, low2 = 0;
  for (let i = 0; i < n && s0 + i < b.length; i++) {
    const t = i / SR, u = t / dur;
    let x = rand();
    if (lp) { const a = typeof lp === 'function' ? lp(u) : lp; low += a * (x - low); x = low; }
    if (hp) { const a = typeof hp === 'function' ? hp(u) : hp; low2 += a * (x - low2); x = x - low2; }
    let env = Math.min(1, t / attack) * Math.min(1, (dur - t) / release);
    if (decay) env *= Math.exp(-t / decay);
    b[s0 + i] += x * vol * env;
  }
}

// Corda dedilhada (Karplus-Strong): pizzicato.
function pluck(b, start, dur, freq, vol = 0.5, damp = 0.996) {
  const period = Math.max(2, Math.round(SR / freq));
  const ring = new Float32Array(period).map(() => rand());
  const s0 = Math.floor(start * SR), n = Math.floor(dur * SR);
  let idx = 0;
  for (let i = 0; i < n && s0 + i < b.length; i++) {
    const next = (idx + 1) % period;
    const v = ring[idx];
    ring[idx] = damp * 0.5 * (ring[idx] + ring[next]);
    idx = next;
    const rel = Math.min(1, (dur - i / SR) / 0.02);
    b[s0 + i] += v * vol * rel;
  }
}

// "Piano" de desenho: senoide + harmônicos com decaimento.
function piano(b, start, dur, note, vol = 0.3) {
  const f = midi(note);
  tone(b, start, dur, f, { vol, decay: 0.45, release: 0.05 });
  tone(b, start, dur, f * 2, { vol: vol * 0.4, decay: 0.25, release: 0.05 });
  tone(b, start, dur, f * 3, { vol: vol * 0.15, decay: 0.15, release: 0.05 });
}

function normalize(b, peak = 0.85) {
  let m = 0;
  for (const v of b) m = Math.max(m, Math.abs(v));
  if (m > 0) for (let i = 0; i < b.length; i++) b[i] = b[i] / m * peak;
  return b;
}

function writeWav(rel, b, peak) {
  normalize(b, peak);
  const data = Buffer.alloc(b.length * 2);
  for (let i = 0; i < b.length; i++) data.writeInt16LE(Math.max(-32768, Math.min(32767, Math.round(b[i] * 32767))), i * 2);
  const h = Buffer.alloc(44);
  h.write('RIFF', 0); h.writeUInt32LE(36 + data.length, 4); h.write('WAVE', 8);
  h.write('fmt ', 12); h.writeUInt32LE(16, 16); h.writeUInt16LE(1, 20); h.writeUInt16LE(1, 22);
  h.writeUInt32LE(SR, 24); h.writeUInt32LE(SR * 2, 28); h.writeUInt16LE(2, 32); h.writeUInt16LE(16, 34);
  h.write('data', 36); h.writeUInt32LE(data.length, 40);
  const file = path.join(OUT, rel);
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, Buffer.concat([h, data]));
  console.log('  ' + rel + `  (${(b.length / SR).toFixed(2)} s)`);
}

const sfx = {};

// ---------- interface ----------
sfx.sfx_place = () => { const b = buf(0.12); noise(b, 0, 0.05, { vol: 0.8, lp: 0.25, decay: 0.015 }); tone(b, 0, 0.1, 190, { vol: 0.6, decay: 0.03 }); return b; };
sfx.sfx_remove = () => { const b = buf(0.16); noise(b, 0, 0.15, { vol: 0.6, lp: u => lerp(0.05, 0.6, u), attack: 0.1, release: 0.01 }); return b; };
sfx.sfx_rotate = () => { const b = buf(0.1); noise(b, 0, 0.012, { vol: 0.8, hp: 0.3 }); noise(b, 0.05, 0.012, { vol: 0.6, hp: 0.3 }); return b; };
sfx.sfx_invalid = () => { const b = buf(0.18); tone(b, 0, 0.16, t => 140 - t * 120, { wave: 'sq', vol: 0.4, lp: 0.15, decay: 0.08 }); return b; };
sfx.sfx_play = () => {
  const b = buf(1.0); noise(b, 0, 0.06, { vol: 0.7, lp: 0.2, decay: 0.02 });
  for (const [f, v] of [[1200, 0.4], [2410, 0.2], [3630, 0.1]]) tone(b, 0.07, 0.9, f, { vol: v, decay: 0.3 });
  return b;
};
sfx.sfx_transition = () => { const b = buf(0.7); noise(b, 0, 0.7, { vol: 0.7, lp: u => 0.03 + 0.4 * Math.sin(Math.PI * u), attack: 0.25, release: 0.3 }); return b; };
sfx.sfx_star = () => { const b = buf(0.6); tone(b, 0, 0.55, midi(84), { vol: 0.4, decay: 0.2 }); tone(b, 0.0, 0.55, midi(91), { vol: 0.2, decay: 0.15 }); return b; };

// ---------- armadilhas ----------
sfx.sfx_anvil_whistle = () => { const b = buf(0.42); tone(b, 0, 0.4, (t, u) => lerp(1800, 520, u), { vol: 0.5, vib: 0.02, vibRate: 9, attack: 0.03 }); return b; };
sfx.sfx_anvil_clang = () => {
  const b = buf(1.1);
  noise(b, 0, 0.12, { vol: 0.9, lp: 0.12, decay: 0.03 });
  tone(b, 0, 0.15, t => 120 - t * 300, { vol: 0.9, decay: 0.05 });
  for (const [f, v, d] of [[420, 0.35, 0.5], [1130, 0.3, 0.35], [1890, 0.22, 0.25], [2650, 0.15, 0.18]]) tone(b, 0, 1.05, f, { vol: v, decay: d });
  return b;
};
sfx.sfx_tnt_fuse = () => { const b = buf(0.32); noise(b, 0, 0.3, { vol: 0.5, hp: 0.5, attack: 0.02 }); for (let i = 0; i < 6; i++) noise(b, 0.04 * i + 0.02, 0.008, { vol: 0.8, hp: 0.4 }); return b; };
sfx.sfx_tnt_boom = () => {
  const b = buf(1.2);
  noise(b, 0, 1.15, { vol: 1.0, lp: u => lerp(0.25, 0.02, u), decay: 0.35 });
  tone(b, 0, 0.6, (t, u) => lerp(110, 35, u), { vol: 0.8, decay: 0.2 });
  return b;
};
sfx.sfx_bait_eat = () => { const b = buf(0.45); for (const s of [0, 0.22]) { noise(b, s, 0.12, { vol: 0.7, lp: 0.18, decay: 0.05 }); tone(b, s, 0.1, 260, { wave: 'sq', vol: 0.15, lp: 0.1, decay: 0.04 }); } return b; };
sfx.sfx_fan_loop = () => { const b = buf(1.0); tone(b, 0, 1.0, 120, { wave: 'saw', vol: 0.35, lp: 0.05, attack: 0.001, release: 0.001 }); noise(b, 0, 1.0, { vol: 0.25, lp: 0.08, attack: 0.001, release: 0.001 }); return b; };
sfx.sfx_push = () => { const b = buf(0.28); noise(b, 0, 0.26, { vol: 0.7, lp: u => lerp(0.05, 0.35, Math.sin(Math.PI * u)), attack: 0.06, release: 0.1 }); return b; };
sfx.sfx_ravine_fall = () => { const b = buf(0.95); tone(b, 0, 0.7, (t, u) => lerp(1300, 220, u), { vol: 0.45, vib: 0.015, vibRate: 7 }); noise(b, 0.75, 0.15, { vol: 0.4, lp: 0.1, decay: 0.04 }); return b; };
sfx.sfx_tar_step = () => { const b = buf(0.25); noise(b, 0, 0.22, { vol: 0.7, lp: u => 0.04 + 0.1 * Math.abs(Math.sin(u * 14)), decay: 0.1 }); return b; };

// ---------- porcos ----------
sfx.sfx_pig_oink = () => {
  const b = buf(0.32);
  tone(b, 0, 0.14, (t, u) => lerp(320, 250, u), { wave: 'sq', vol: 0.5, lp: 0.12, vib: 0.08, vibRate: 30 });
  tone(b, 0.15, 0.15, (t, u) => lerp(300, 220, u), { wave: 'sq', vol: 0.5, lp: 0.12, vib: 0.08, vibRate: 30 });
  return b;
};
sfx.sfx_pig_turn = () => { const b = buf(0.3); tone(b, 0, 0.28, (t, u) => lerp(260, 520, u * u), { wave: 'sq', vol: 0.45, lp: 0.14, vib: 0.05, vibRate: 25 }); return b; };
sfx.sfx_capture_pop = () => {
  const b = buf(0.45);
  tone(b, 0, 0.07, (t, u) => lerp(500, 1300, u), { vol: 0.6, decay: 0.05 });
  tone(b, 0.08, 0.35, midi(88), { wave: 'sq', vol: 0.18, lp: 0.3, decay: 0.12 });
  tone(b, 0.14, 0.3, midi(93), { wave: 'sq', vol: 0.18, lp: 0.3, decay: 0.12 });
  return b;
};
sfx.sfx_escape = () => {
  const b = buf(1.1);
  for (let i = 0; i < 4; i++) tone(b, i * 0.09, 0.08, midi(76 + (i % 2) * 3), { wave: 'tri', vol: 0.35 });
  for (const s of [0.45, 0.72]) tone(b, s, 0.22, 330, { wave: 'sq', vol: 0.4, lp: 0.08, vib: 0.01 });
  return b;
};

// ---------- lobo ----------
sfx.wolf_lick = () => { const b = buf(0.4); noise(b, 0, 0.35, { vol: 0.6, lp: u => lerp(0.04, 0.3, u), attack: 0.05 }); return b; };
sfx.wolf_cheer = () => { const b = buf(0.9); tone(b, 0, 0.85, (t, u) => 400 + 300 * Math.sin(Math.PI * u), { wave: 'tri', vol: 0.5, vib: 0.03, vibRate: 5, attack: 0.08, release: 0.2 }); return b; };
sfx.wolf_angry = () => { const b = buf(0.6); tone(b, 0, 0.55, 85, { wave: 'saw', vol: 0.6, lp: 0.06, vib: 0.15, vibRate: 22, attack: 0.04, release: 0.1 }); noise(b, 0, 0.55, { vol: 0.3, lp: 0.05 }); return b; };
sfx.wolf_feast = () => { const b = buf(1.4); tone(b, 0, 1.3, (t, u) => 350 + 450 * Math.sin(Math.PI * Math.min(1, u * 1.5)), { wave: 'tri', vol: 0.5, vib: 0.04, vibRate: 5, attack: 0.1, release: 0.3 }); return b; };
sfx.wolf_sad = () => { const b = buf(1.2); tone(b, 0, 1.15, (t, u) => lerp(600, 260, u), { wave: 'tri', vol: 0.45, vib: 0.04, vibRate: 4, attack: 0.1, release: 0.3 }); return b; };

// ---------- música ----------
function musicBlueprint() {
  // 104 BPM, pizzicato "o vilão planeja": Am - F - C - E (2 compassos cada), 8 compassos
  const bpm = 104, beat = 60 / bpm, bars = 8, b = buf(bars * 4 * beat);
  const chords = [[57, 60, 64], [53, 57, 60], [48, 52, 55], [52, 56, 59]];
  for (let bar = 0; bar < bars; bar++) {
    const ch = chords[Math.floor(bar / 2) % 4], t0 = bar * 4 * beat;
    pluck(b, t0, beat * 1.5, midi(ch[0] - 12), 0.55, 0.995);
    pluck(b, t0 + 2 * beat, beat * 1.5, midi(ch[0] - 12 + 7), 0.45, 0.995);
    const arp = [0, 1, 2, 1, 2, 1, 0, 2];
    for (let k = 0; k < 8; k++) pluck(b, t0 + k * beat / 2, beat * 0.6, midi(ch[arp[k]] + 12), 0.28, 0.993);
  }
  return b;
}
function musicAction() {
  // 152 BPM, ragtime de perseguição: baixo oom-pah + acordes + melodia sincopada
  const bpm = 152, beat = 60 / bpm, bars = 8, b = buf(bars * 4 * beat);
  const prog = [[48, [64, 67, 72]], [48, [64, 67, 72]], [53, [65, 69, 72]], [53, [65, 69, 72]], [55, [62, 65, 71]], [55, [62, 65, 71]], [48, [64, 67, 72]], [55, [62, 65, 71]]];
  const melody = [76, 0, 79, 76, 0, 72, 74, 76, 77, 0, 81, 77, 0, 74, 76, 77, 79, 0, 83, 79, 77, 74, 76, 79, 72, 0, 76, 79, 74, 0, 71, 74];
  for (let bar = 0; bar < bars; bar++) {
    const [bass, chord] = prog[bar], t0 = bar * 4 * beat;
    for (let k = 0; k < 4; k++) {
      if (k % 2 === 0) piano(b, t0 + k * beat, beat * 0.9, (k === 0 ? bass : bass + 7) - 12, 0.4);
      else for (const n of chord) piano(b, t0 + k * beat, beat * 0.5, n - 12, 0.12);
    }
    for (let k = 0; k < 4; k++) {
      const n = melody[(bar * 4 + k) % melody.length];
      const sync = (k % 2 === 1) ? beat * 0.5 : 0;
      if (n) piano(b, t0 + k * beat + sync, beat * 0.7, n, 0.22);
    }
  }
  return b;
}
function stingerWin() {
  const b = buf(2.4), notes = [60, 64, 67, 72];
  notes.forEach((n, i) => tone(b, i * 0.14, 0.5, midi(n), { wave: 'saw', vol: 0.3, lp: 0.12, decay: 0.4 }));
  for (const n of [72, 76, 79]) tone(b, 0.6, 1.7, midi(n), { wave: 'saw', vol: 0.25, lp: 0.1, vib: 0.006, attack: 0.03, release: 0.6 });
  return b;
}
function stingerLose() {
  const b = buf(2.8), notes = [58, 57, 56];
  notes.forEach((n, i) => tone(b, i * 0.5, 0.45, midi(n - 12), { wave: 'saw', vol: 0.5, lp: 0.07, attack: 0.04, release: 0.1 }));
  tone(b, 1.5, 1.2, midi(55 - 12), { wave: 'saw', vol: 0.5, lp: 0.07, vib: 0.03, vibRate: 5, attack: 0.04, release: 0.4 });
  return b;
}

console.log('Gerando áudio em ' + OUT);
for (const [id, fn] of Object.entries(sfx)) writeWav(`sfx/${id}.wav`, fn(), 0.8);
writeWav('music/music_blueprint.wav', musicBlueprint(), 0.7);
writeWav('music/music_action.wav', musicAction(), 0.7);
writeWav('music/stinger_win.wav', stingerWin(), 0.75);
writeWav('music/stinger_lose.wav', stingerLose(), 0.75);
console.log('OK');
