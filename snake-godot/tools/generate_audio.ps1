# Generates simple placeholder sounds and music for Snake (Audio Bible §3-4).
# Everything is synthesized from basic waveforms, so you own all of it.
# Replace any file later with a jsfxr / BeepBox version; same names.
#
# Run from the project folder:
#   powershell -ExecutionPolicy Bypass -File tools/generate_audio.ps1
#
# Output: assets/audio/sfx/*.wav and assets/audio/music/*.wav

param([string]$OutDir = (Join-Path $PSScriptRoot "..\assets\audio"))

$code = @"
using System;
using System.IO;

public static class Synth
{
    public static int SR = 44100;
    static Random rng = new Random(1);

    // ---------- building blocks ----------

    static float[] Buf(double seconds) { return new float[(int)(seconds * SR)]; }

    static double Midi(double note) { return 440.0 * Math.Pow(2.0, (note - 69.0) / 12.0); }

    static double Osc(string wave, double phase, double duty)
    {
        phase -= Math.Floor(phase);
        if (wave == "sq") return phase < duty ? 1.0 : -1.0;
        if (wave == "tri") return 4.0 * Math.Abs(phase - 0.5) - 1.0;
        return Math.Sin(2.0 * Math.PI * phase);
    }

    // A tone with a frequency curve freq(t) and a simple attack/release envelope.
    static void Tone(float[] b, double start, double dur, Func<double, double> freq, string wave,
                     double vol, double attack, double release, double duty)
    {
        int s0 = (int)(start * SR), n = (int)(dur * SR);
        double phase = 0.0;
        for (int i = 0; i < n && s0 + i < b.Length; i++)
        {
            double t = (double)i / SR;
            phase += freq(t) / SR;
            double env = Math.Min(1.0, t / Math.Max(attack, 1e-4)) * Math.Min(1.0, (dur - t) / Math.Max(release, 1e-4));
            b[s0 + i] += (float)(Osc(wave, phase, duty) * vol * env);
        }
    }

    static void Note(float[] b, double start, double dur, double midi, string wave, double vol, double duty)
    {
        double f = Midi(midi);
        Tone(b, start, dur, t => f, wave, vol, 0.005, Math.Min(0.05, dur * 0.5), duty);
    }

    // Filtered noise. cutoff(x) is a one-pole coefficient (0..1) over normalized time x.
    static void Noise(float[] b, double start, double dur, double vol, double attack, double decayPow,
                      Func<double, double> cutoff, bool highpass)
    {
        int s0 = (int)(start * SR), n = (int)(dur * SR);
        double lp = 0.0;
        for (int i = 0; i < n && s0 + i < b.Length; i++)
        {
            double t = (double)i / SR;
            double x = rng.NextDouble() * 2.0 - 1.0;
            lp += cutoff(t / dur) * (x - lp);
            double v = highpass ? x - lp : lp;
            double env = Math.Min(1.0, t / Math.Max(attack, 1e-4)) * Math.Pow(Math.Max(0.0, 1.0 - t / dur), decayPow);
            b[s0 + i] += (float)(v * vol * env);
        }
    }

    static void Kick(float[] b, double start, double vol)
    {
        Tone(b, start, 0.14, t => 150.0 * Math.Pow(0.3, t / 0.14), "sine", vol, 0.001, 0.12, 0.5);
    }

    static void Snare(float[] b, double start, double vol)
    {
        Noise(b, start, 0.13, vol, 0.001, 2.0, x => 0.55, false);
        Tone(b, start, 0.08, t => 190.0, "tri", vol * 0.4, 0.001, 0.07, 0.5);
    }

    static void Hat(float[] b, double start, double vol)
    {
        Noise(b, start, 0.035, vol, 0.001, 2.0, x => 0.25, true);
    }

    static void Write(string path, float[] b, double peakDb)
    {
        float max = 1e-6f;
        foreach (float v in b) max = Math.Max(max, Math.Abs(v));
        double gain = Math.Pow(10.0, peakDb / 20.0) / max;
        using (var w = new BinaryWriter(File.Create(path)))
        {
            w.Write(new char[] { 'R', 'I', 'F', 'F' }); w.Write(36 + b.Length * 2);
            w.Write(new char[] { 'W', 'A', 'V', 'E' }); w.Write(new char[] { 'f', 'm', 't', ' ' });
            w.Write(16); w.Write((short)1); w.Write((short)1); w.Write(SR); w.Write(SR * 2);
            w.Write((short)2); w.Write((short)16);
            w.Write(new char[] { 'd', 'a', 't', 'a' }); w.Write(b.Length * 2);
            foreach (float v in b)
                w.Write((short)Math.Round(Math.Max(-1.0, Math.Min(1.0, v * gain)) * 32767.0));
        }
    }

    // ---------- sound effects ----------

    static float[] Chomp(int seed, double pitch, double bright)
    {
        rng = new Random(seed);
        var b = Buf(0.14);
        // Three quick crunchy noise bursts plus a low "thump".
        for (int k = 0; k < 3; k++)
            Noise(b, k * 0.035, 0.045, 0.9 - k * 0.2, 0.001, 2.0, x => bright - 0.2 * x, false);
        Tone(b, 0.0, 0.1, t => Math.Max(55.0, pitch - 1200.0 * t), "sine", 0.8, 0.001, 0.09, 0.5);
        return b;
    }

    static float[] Gasp()
    {
        rng = new Random(7);
        var b = Buf(0.22);
        Noise(b, 0.0, 0.22, 0.8, 0.07, 1.5, x => 0.06 + 0.45 * x, false); // breath opening up: "hhuh!"
        Tone(b, 0.0, 0.18, t => 420.0 + 500.0 * t, "sine", 0.12, 0.05, 0.08, 0.5);
        return b;
    }

    static float[] Bonk()
    {
        var b = Buf(0.17);
        Tone(b, 0.0, 0.16, t => 260.0 * Math.Pow(0.25, t / 0.16), "sq", 0.5, 0.001, 0.15, 0.5);
        Tone(b, 0.0, 0.16, t => 130.0 * Math.Pow(0.4, t / 0.16), "sine", 0.6, 0.001, 0.15, 0.5);
        return b;
    }

    static float[] SlideDown()
    {
        var b = Buf(0.72);
        Tone(b, 0.0, 0.7, t => 1500.0 * Math.Pow(300.0 / 1500.0, t / 0.7) * (1.0 + 0.02 * Math.Sin(2.0 * Math.PI * 7.0 * t)),
             "sine", 0.8, 0.01, 0.15, 0.5);
        return b;
    }

    static float[] Milestone()
    {
        var b = Buf(0.5);
        Note(b, 0.00, 0.09, 84, "sq", 0.3, 0.25);
        Note(b, 0.09, 0.09, 88, "sq", 0.3, 0.25);
        Note(b, 0.18, 0.30, 91, "sq", 0.3, 0.25);
        Note(b, 0.18, 0.30, 79, "tri", 0.3, 0.5);
        return b;
    }

    static float[] SpeedUp()
    {
        rng = new Random(11);
        var b = Buf(0.16);
        Noise(b, 0.0, 0.15, 0.7, 0.01, 1.0, x => 0.05 + 0.5 * x, true);
        return b;
    }

    static float[] UiClick()
    {
        var b = Buf(0.06);
        Tone(b, 0.0, 0.05, t => 900.0, "sine", 0.7, 0.001, 0.045, 0.5);
        Tone(b, 0.0, 0.025, t => 1800.0, "sine", 0.2, 0.001, 0.02, 0.5);
        return b;
    }

    static float[] Pop()
    {
        var b = Buf(0.09);
        Tone(b, 0.0, 0.08, t => 500.0 + 9000.0 * t, "sine", 0.8, 0.002, 0.07, 0.5);
        return b;
    }

    // ---------- music: 16 bars, 132 BPM, C major (Audio Bible §4.1) ----------

    static float[] Music()
    {
        double beat = 60.0 / 132.0, slot = beat / 2.0, bar = beat * 4.0;
        var b = Buf(bar * 16.0);
        // Chord progression C - Am - F - G: bass root and chord tones.
        int[] roots = { 48, 45, 41, 43 };
        int[][] chords = { new[] { 60, 64, 67 }, new[] { 57, 60, 64 }, new[] { 53, 57, 60 }, new[] { 55, 59, 62 } };
        // 8-bar melody on an 8th-note grid (-1 = rest).
        int[][] melody = {
            new[] { 72, -1, 76, -1, 79, 76, 72, -1 },
            new[] { 69, -1, 72, -1, 76, -1, 72, 69 },
            new[] { 65, 69, 72, -1, 77, -1, 76, 72 },
            new[] { 74, -1, 71, -1, 67, -1, -1, -1 },
            new[] { 72, 76, 79, -1, 84, -1, 79, 76 },
            new[] { 81, -1, 79, 76, 72, -1, 76, -1 },
            new[] { 77, -1, 76, -1, 74, -1, 72, 69 },
            new[] { 71, -1, 74, -1, 79, -1, -1, -1 },
        };
        for (int m = 0; m < 16; m++)
        {
            double t0 = m * bar;
            int c = m % 4;
            bool second = m >= 8; // A' section: brighter
            for (int s = 0; s < 8; s++)
            {
                double t = t0 + s * slot;
                Note(b, t, slot * 0.85, roots[c] + (s % 2 == 0 ? 0 : 12), "tri", 0.45, 0.5);   // bouncy octave bass
                if (s % 2 == 1)
                    foreach (int n in chords[c]) Note(b, t, slot * 0.5, n, "sq", 0.05, 0.125); // off-beat stabs
                int mel = melody[m % 8][s];
                if (mel >= 0)
                {
                    Note(b, t, slot * 0.9, mel, "sq", 0.2, 0.25);
                    if (second) Note(b, t, slot * 0.9, mel + 12, "sq", 0.06, 0.25);
                }
                Hat(b, t, 0.12);
                if (second) Hat(b, t + slot * 0.5, 0.07);
            }
            Kick(b, t0, 0.8); Kick(b, t0 + 2 * beat, 0.8);
            Snare(b, t0 + beat, 0.4); Snare(b, t0 + 3 * beat, 0.4);
        }
        return b;
    }

    // ---------- jingles (Audio Bible §4.2) ----------

    static float[] JingleGameOver()
    {
        var b = Buf(2.4);
        int[] notes = { 67, 66, 65, 64 };
        double[] starts = { 0.0, 0.4, 0.8, 1.2 };
        double[] durs = { 0.35, 0.35, 0.35, 1.1 };
        for (int i = 0; i < 4; i++)
        {
            double f = Midi(notes[i]);
            bool last = i == 3;
            Func<double, double> curve = t => f * (1.0 - (last ? 0.06 : 0.03) * Math.Min(1.0, t / 0.3))
                                               * (last ? 1.0 + 0.03 * Math.Sin(2.0 * Math.PI * 5.0 * t) : 1.0);
            Tone(b, starts[i], durs[i], curve, "sq", 0.25, 0.01, 0.12, 0.5);
            Tone(b, starts[i], durs[i], t => curve(t) * 0.5, "tri", 0.3, 0.01, 0.12, 0.5);
        }
        return b;
    }

    static void Fanfare(float[] b, double start)
    {
        int[] up = { 72, 76, 79, 84 };
        for (int i = 0; i < 4; i++) Note(b, start + i * 0.13, 0.13, up[i], "sq", 0.25, 0.25);
        Note(b, start + 0.52, 0.9, 88, "sq", 0.25, 0.25);
        foreach (int n in new[] { 72, 76, 79 }) Note(b, start + 0.52, 0.9, n, "tri", 0.2, 0.5);
        Kick(b, start + 0.52, 0.7);
    }

    static float[] JingleNewBest()
    {
        var b = Buf(1.6);
        Fanfare(b, 0.0);
        return b;
    }

    static float[] JinglePerfect()
    {
        rng = new Random(23);
        var b = Buf(3.4);
        for (int i = 0; i < 16; i++) Snare(b, i * 0.05, 0.15 + 0.02 * i); // drum roll
        Fanfare(b, 0.85);
        foreach (int n in new[] { 60, 64, 67, 72, 76 }) Note(b, 1.95, 1.3, n, "tri", 0.2, 0.5);
        foreach (int n in new[] { 72, 79 }) Note(b, 1.95, 1.3, n, "sq", 0.08, 0.25);
        Kick(b, 1.95, 0.9);
        Noise(b, 1.95, 1.2, 0.3, 0.001, 2.0, x => 0.3, true); // crash
        return b;
    }

    // ---------- entry point ----------

    public static void Generate(string outDir)
    {
        string sfx = Path.Combine(outDir, "sfx"), music = Path.Combine(outDir, "music");
        Directory.CreateDirectory(sfx);
        Directory.CreateDirectory(music);

        SR = 44100;
        Write(Path.Combine(sfx, "sfx_chomp_a.wav"), Chomp(1, 170.0, 0.35), -1.0);
        Write(Path.Combine(sfx, "sfx_chomp_b.wav"), Chomp(2, 190.0, 0.42), -1.0);
        Write(Path.Combine(sfx, "sfx_chomp_c.wav"), Chomp(3, 155.0, 0.30), -1.0);
        Write(Path.Combine(sfx, "sfx_gasp.wav"), Gasp(), -1.0);
        Write(Path.Combine(sfx, "sfx_bonk.wav"), Bonk(), -1.0);
        Write(Path.Combine(sfx, "sfx_slide_down.wav"), SlideDown(), -1.0);
        Write(Path.Combine(sfx, "sfx_milestone.wav"), Milestone(), -1.0);
        Write(Path.Combine(sfx, "sfx_speed_up.wav"), SpeedUp(), -1.0);
        Write(Path.Combine(sfx, "sfx_ui_click.wav"), UiClick(), -1.0);
        Write(Path.Combine(sfx, "sfx_pop.wav"), Pop(), -1.0);

        SR = 22050; // music and jingles: half rate keeps the files small
        Write(Path.Combine(music, "music_garden_loop.wav"), Music(), -1.0);
        Write(Path.Combine(music, "jingle_game_over.wav"), JingleGameOver(), -1.0);
        Write(Path.Combine(music, "jingle_new_best.wav"), JingleNewBest(), -1.0);
        Write(Path.Combine(music, "jingle_perfect.wav"), JinglePerfect(), -1.0);
    }
}
"@

Add-Type -TypeDefinition $code -Language CSharp
$target = [System.IO.Path]::GetFullPath($OutDir)
[Synth]::Generate($target)
Get-ChildItem -Path $target -Recurse -Filter *.wav | ForEach-Object {
    "{0,-28} {1,8:N0} bytes" -f $_.Name, $_.Length
}
