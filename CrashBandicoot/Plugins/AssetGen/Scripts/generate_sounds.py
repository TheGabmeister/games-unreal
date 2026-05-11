"""
Procedural sound effect generator for UE5 placeholder audio.

Generates .wav files (16-bit PCM, 44100 Hz, mono) for common game SFX.
Uses only Python standard library — no external dependencies.

Usage:
    python generate_sounds.py [output_dir] [sound_filter]

    output_dir   — directory for output files (default: current directory)
    sound_filter — comma-separated list of sound names to generate (default: all)

Output files are named SFX_<Name>.wav.
"""

import math
import os
import random
import struct
import sys
import wave

SAMPLE_RATE = 44100


# ---------------------------------------------------------------------------
# Synthesis primitives
# ---------------------------------------------------------------------------

def sine(freq, duration, sample_rate=SAMPLE_RATE):
    """Generate a sine wave at a fixed frequency."""
    n_samples = int(sample_rate * duration)
    return [math.sin(2 * math.pi * freq * i / sample_rate) for i in range(n_samples)]


def sine_sweep(freq_start, freq_end, duration, sample_rate=SAMPLE_RATE):
    """Generate a sine wave that sweeps linearly from freq_start to freq_end."""
    n_samples = int(sample_rate * duration)
    samples = []
    phase = 0.0
    for i in range(n_samples):
        t = i / n_samples
        freq = freq_start + (freq_end - freq_start) * t
        phase += 2 * math.pi * freq / sample_rate
        samples.append(math.sin(phase))
    return samples


def noise(duration, sample_rate=SAMPLE_RATE):
    """Generate white noise."""
    n_samples = int(sample_rate * duration)
    return [random.uniform(-1, 1) for _ in range(n_samples)]


def silence(duration, sample_rate=SAMPLE_RATE):
    """Generate silence."""
    n_samples = int(sample_rate * duration)
    return [0.0] * n_samples


# ---------------------------------------------------------------------------
# Envelope shapers
# ---------------------------------------------------------------------------

def envelope_linear(samples, attack=0.01, decay=0.0, sustain=1.0, release=0.1):
    """Apply an ADSR-style linear envelope. Decay here means time to go from 1.0 to sustain level."""
    n = len(samples)
    attack_samples = int(SAMPLE_RATE * attack)
    decay_samples = int(SAMPLE_RATE * decay)
    release_samples = int(SAMPLE_RATE * release)
    sustain_samples = max(0, n - attack_samples - decay_samples - release_samples)

    out = []
    for i in range(n):
        if i < attack_samples:
            gain = i / max(attack_samples, 1)
        elif i < attack_samples + decay_samples:
            t = (i - attack_samples) / max(decay_samples, 1)
            gain = 1.0 - (1.0 - sustain) * t
        elif i < attack_samples + decay_samples + sustain_samples:
            gain = sustain
        else:
            t = (i - attack_samples - decay_samples - sustain_samples) / max(release_samples, 1)
            gain = sustain * (1.0 - t)
        out.append(samples[i] * gain)
    return out


def envelope_decay(samples, power=2.0):
    """Apply a simple exponential decay envelope (starts at 1, ends near 0)."""
    n = len(samples)
    return [samples[i] * ((1.0 - i / n) ** power) for i in range(n)]


# ---------------------------------------------------------------------------
# Mixing utilities
# ---------------------------------------------------------------------------

def mix(layers):
    """Mix multiple sample lists together (summed, then normalized)."""
    max_len = max(len(layer) for layer in layers)
    mixed = [0.0] * max_len
    for layer in layers:
        for i, s in enumerate(layer):
            mixed[i] += s
    peak = max(abs(s) for s in mixed) or 1.0
    return [s / peak for s in mixed]


def concat(parts):
    """Concatenate sample lists sequentially."""
    out = []
    for part in parts:
        out.extend(part)
    return out


def amplify(samples, gain):
    """Scale samples by a gain factor."""
    return [s * gain for s in samples]


# ---------------------------------------------------------------------------
# Sound definitions
# ---------------------------------------------------------------------------

def make_jump():
    """Quick upward frequency sweep with a punchy attack — classic platformer jump."""
    sweep = sine_sweep(180, 800, 0.18)
    shaped = envelope_decay(sweep, power=2.5)
    # Add a subtle low thud at the start
    thud = envelope_decay(sine(90, 0.06), power=4.0)
    thud_padded = thud + [0.0] * (len(shaped) - len(thud))
    return mix([shaped, amplify(thud_padded, 0.4)])


def make_coin():
    """Two quick ascending tones — classic coin/pickup chime."""
    tone1 = envelope_decay(sine(987, 0.07), power=1.5)   # B5
    gap = silence(0.03)
    tone2 = envelope_decay(sine(1319, 0.12), power=1.5)  # E6
    return concat([tone1, gap, tone2])


def make_hit():
    """Noise burst + low-frequency punch — impact/damage sound."""
    # Noise burst for the crack
    crack = envelope_decay(noise(0.08), power=3.0)
    # Low sine thud for body
    thud = envelope_decay(sine(60, 0.15), power=2.0)
    # Pad crack to match thud length
    crack_padded = crack + [0.0] * (len(thud) - len(crack))
    return mix([amplify(crack_padded, 0.7), thud])


def make_spin_whoosh():
    """Filtered noise sweep — air rushing past during a 360-degree spin."""
    duration = 0.45
    n_samples = int(SAMPLE_RATE * duration)
    raw = noise(duration)

    # Band-pass sweep: center frequency rises then falls over the duration,
    # simulating a rotating sound source approaching then receding.
    out = []
    for i in range(n_samples):
        t = i / n_samples
        # Bell-curve center frequency: peaks at mid-spin
        center = 400 + 1800 * math.sin(math.pi * t)
        bandwidth = 0.3
        # Simple single-pole resonance approximation
        w = 2 * math.pi * center / SAMPLE_RATE
        r = 1.0 - bandwidth
        if i >= 2:
            filtered = 2 * r * math.cos(w) * (out[-1] if out else 0) \
                     - r * r * (out[-2] if len(out) >= 2 else 0) \
                     + (1 - r) * raw[i]
        else:
            filtered = raw[i] * (1 - r)
        out.append(filtered)

    # Normalize
    peak = max(abs(s) for s in out) or 1.0
    out = [s / peak for s in out]

    # Add a subtle tonal whoosh underneath
    tonal = sine_sweep(300, 600, duration * 0.5)
    tonal += sine_sweep(600, 200, duration * 0.5)
    tonal = envelope_linear(tonal, attack=0.05, decay=0.0, sustain=0.6, release=0.15)

    # Shape the noise with a bell envelope (loud in middle, quiet at edges)
    for i in range(len(out)):
        t = i / len(out)
        bell = math.sin(math.pi * t) ** 1.5
        out[i] *= bell

    # Pad tonal to match length
    tonal_padded = tonal + [0.0] * max(0, len(out) - len(tonal))
    tonal_padded = tonal_padded[:len(out)]

    return mix([out, amplify(tonal_padded, 0.25)])


def make_crate_break():
    """Wood smash — sharp crack layered with resonant mid-frequency fragments."""
    random.seed(10)
    crack = envelope_decay(noise(0.06), power=4.0)
    # Wood resonance fragments at different frequencies
    frag1 = envelope_decay(sine(320, 0.12), power=3.0)
    frag2 = envelope_decay(sine(480, 0.09), power=3.5)
    frag3 = envelope_decay(sine(680, 0.07), power=4.0)
    # Low thud for weight
    thud = envelope_decay(sine(80, 0.15), power=2.5)
    # Trailing debris noise
    debris = envelope_decay(noise(0.25), power=2.0)
    debris = amplify(debris, 0.3)
    max_len = max(len(crack), len(frag1), len(frag2), len(frag3), len(thud), len(debris))
    def pad(s): return s + [0.0] * (max_len - len(s))
    return mix([pad(crack), amplify(pad(frag1), 0.5), amplify(pad(frag2), 0.4),
                amplify(pad(frag3), 0.3), pad(thud), pad(debris)])


def make_bounce_crate():
    """Spring/boing — oscillating pitch sweep with rubbery bounce character."""
    # Fast sine sweep up then oscillating decay
    n_samples = int(SAMPLE_RATE * 0.35)
    samples = []
    phase = 0.0
    for i in range(n_samples):
        t = i / n_samples
        # Frequency bounces: starts high, oscillates with decay
        freq = 400 + 600 * math.exp(-t * 6) * math.sin(t * 25)
        phase += 2 * math.pi * freq / SAMPLE_RATE
        samples.append(math.sin(phase))
    shaped = envelope_decay(samples, power=1.8)
    # Add a short attack pop
    pop = envelope_decay(sine(800, 0.02), power=5.0)
    pop_padded = pop + [0.0] * (len(shaped) - len(pop))
    return mix([shaped, amplify(pop_padded, 0.3)])


def make_wumpa_collect():
    """Quick cheerful ding — bright ascending two-tone chime."""
    tone1 = envelope_decay(sine(1175, 0.06), power=1.5)  # D6
    gap = silence(0.02)
    tone2 = envelope_decay(sine(1568, 0.10), power=1.5)  # G6
    return concat([tone1, gap, tone2])


def make_extra_life():
    """1-up jingle — triumphant ascending notes in a major arpeggio."""
    notes = [
        (523, 0.08),   # C5
        (659, 0.08),   # E5
        (784, 0.08),   # G5
        (1047, 0.15),  # C6 (held longer)
    ]
    parts = []
    for freq, dur in notes:
        tone = envelope_linear(sine(freq, dur), attack=0.005, decay=0.02, sustain=0.8, release=0.03)
        parts.append(tone)
        parts.append(silence(0.02))
    # Final shimmer on the last note
    shimmer = envelope_decay(sine(1047 * 2, 0.2), power=2.0)
    parts.append(amplify(shimmer, 0.3))
    return concat(parts)


def make_akuaku_pickup():
    """Stylized mystical tone for mask pickup — resonant low formants with rising shimmer."""
    # Low resonant "ooh" approximation via layered sines at formant frequencies
    dur = 0.5
    f1 = envelope_linear(sine(180, dur), attack=0.03, decay=0.1, sustain=0.7, release=0.15)
    f2 = envelope_linear(sine(360, dur), attack=0.05, decay=0.1, sustain=0.5, release=0.15)
    f3 = envelope_linear(sine(540, dur), attack=0.08, decay=0.1, sustain=0.3, release=0.15)
    # Rising shimmer
    shimmer = sine_sweep(800, 2400, dur)
    shimmer = envelope_linear(shimmer, attack=0.1, decay=0.0, sustain=0.2, release=0.2)
    return mix([f1, amplify(f2, 0.6), amplify(f3, 0.3), amplify(shimmer, 0.15)])


def make_akuaku_hit():
    """Mask shatter — high-frequency crack with descending resonant fragments."""
    # Sharp initial crack
    crack = envelope_decay(noise(0.04), power=5.0)
    # Descending glass-like tones
    tone1 = envelope_decay(sine_sweep(2400, 800, 0.2), power=2.5)
    tone2 = envelope_decay(sine_sweep(1800, 400, 0.25), power=2.0)
    # Low shatter thud
    thud = envelope_decay(sine(120, 0.12), power=3.0)
    max_len = max(len(crack), len(tone1), len(tone2), len(thud))
    def pad(s): return s + [0.0] * (max_len - len(s))
    return mix([pad(crack), amplify(pad(tone1), 0.5), amplify(pad(tone2), 0.4), pad(thud)])


def make_akuaku_invincible():
    """Power-up fanfare — bright ascending arpeggio with sustained shimmer."""
    notes = [
        (523, 0.06),   # C5
        (659, 0.06),   # E5
        (784, 0.06),   # G5
        (1047, 0.06),  # C6
        (1319, 0.06),  # E6
        (1568, 0.20),  # G6 (held)
    ]
    parts = []
    for freq, dur in notes:
        tone = envelope_linear(sine(freq, dur), attack=0.003, decay=0.01, sustain=0.9, release=0.02)
        parts.append(tone)
    # Sustained power chord
    chord_dur = 0.4
    c1 = envelope_linear(sine(1047, chord_dur), attack=0.01, decay=0.1, sustain=0.6, release=0.15)
    c2 = envelope_linear(sine(1319, chord_dur), attack=0.01, decay=0.1, sustain=0.5, release=0.15)
    c3 = envelope_linear(sine(1568, chord_dur), attack=0.01, decay=0.1, sustain=0.5, release=0.15)
    chord = mix([c1, amplify(c2, 0.8), amplify(c3, 0.7)])
    parts.append(chord)
    return concat(parts)


def make_tnt_tick():
    """Sharp countdown tick — short percussive click."""
    click = envelope_decay(sine(1200, 0.03), power=6.0)
    # Add a subtle low knock
    knock = envelope_decay(sine(200, 0.04), power=4.0)
    knock_padded = knock + [0.0] * max(0, len(click) - len(knock))
    click_padded = click + [0.0] * max(0, len(knock) - len(click))
    return mix([click_padded, amplify(knock_padded, 0.4)])


def make_tnt_explosion():
    """Heavy blast — deep bass boom with layered noise and rumbling tail."""
    # Initial transient
    transient = envelope_decay(noise(0.03), power=6.0)
    # Deep bass boom
    boom = envelope_decay(sine(40, 0.5), power=1.5)
    # Mid rumble
    rumble_noise = noise(0.6)
    rumble = envelope_decay(rumble_noise, power=1.8)
    # Filtered crackle (mid frequency noise)
    n_samples = int(SAMPLE_RATE * 0.4)
    crackle = noise(0.4)
    for i in range(2, n_samples):
        crackle[i] = crackle[i] * 0.3 + crackle[i-1] * 0.5 + crackle[i-2] * 0.2
    crackle = envelope_decay(crackle, power=2.5)
    # Sub-bass punch
    sub = envelope_decay(sine(25, 0.3), power=2.0)
    max_len = max(len(transient), len(boom), len(rumble), len(crackle), len(sub))
    def pad(s): return s + [0.0] * (max_len - len(s))
    return mix([pad(transient), pad(boom), amplify(pad(rumble), 0.6),
                amplify(pad(crackle), 0.4), amplify(pad(sub), 0.7)])


def make_game_over():
    """Sad descending jingle — minor key falling phrase."""
    notes = [
        (784, 0.20),   # G5
        (740, 0.20),   # F#5
        (659, 0.20),   # E5
        (587, 0.30),   # D5
        (523, 0.50),   # C5 (held, sad resolution)
    ]
    parts = []
    for freq, dur in notes:
        tone = envelope_linear(sine(freq, dur), attack=0.01, decay=0.05, sustain=0.7, release=0.08)
        parts.append(tone)
        parts.append(silence(0.03))
    # Low octave doubling on final note
    low_end = envelope_linear(sine(262, 0.5), attack=0.02, decay=0.1, sustain=0.5, release=0.15)
    # Pad to align with end
    total_before = sum(len(p) for p in parts)
    low_padded = [0.0] * (total_before - len(low_end)) + low_end
    result = concat(parts)
    if len(low_padded) < len(result):
        low_padded += [0.0] * (len(result) - len(low_padded))
    return mix([result, amplify(low_padded[:len(result)], 0.5)])


def make_arrow_launch():
    """Upward whoosh — focused noise sweep from low to high with wind character."""
    duration = 0.3
    n_samples = int(SAMPLE_RATE * duration)
    raw = noise(duration)
    # Rising band-pass filter
    out = []
    for i in range(n_samples):
        t = i / n_samples
        center = 200 + 3000 * t * t  # Accelerating sweep up
        bandwidth = 0.25
        w = 2 * math.pi * center / SAMPLE_RATE
        r = 1.0 - bandwidth
        if i >= 2:
            filtered = 2 * r * math.cos(w) * (out[-1] if out else 0) \
                     - r * r * (out[-2] if len(out) >= 2 else 0) \
                     + (1 - r) * raw[i]
        else:
            filtered = raw[i] * (1 - r)
        out.append(filtered)
    peak = max(abs(s) for s in out) or 1.0
    out = [s / peak for s in out]
    # Shape: ramp up then cut
    for i in range(len(out)):
        t = i / len(out)
        env = min(t * 3, 1.0) * ((1.0 - t) ** 0.5)
        out[i] *= env
    # Add tonal sweep underneath
    tonal = sine_sweep(200, 1200, duration)
    tonal = envelope_linear(tonal, attack=0.02, sustain=0.4, release=0.05)
    tonal_padded = tonal + [0.0] * max(0, len(out) - len(tonal))
    return mix([out, amplify(tonal_padded[:len(out)], 0.2)])


def make_enemy_defeat():
    """Poof/pop when enemy dies."""
    pop = noise(0.08)
    pop = envelope_linear(pop, attack=0.005, sustain=0.3, release=0.3)
    tone = sine_sweep(800, 200, 0.15)
    tone = envelope_linear(tone, attack=0.01, sustain=0.3, release=0.3)
    return mix([pop, amplify(tone, 0.6)])


def make_enemy_hit():
    """Hit sound when enemy takes damage but survives."""
    thud = noise(0.06)
    thud = envelope_linear(thud, attack=0.005, sustain=0.2, release=0.4)
    tone = sine(300, 0.1)
    tone = envelope_linear(tone, attack=0.01, sustain=0.3, release=0.3)
    return mix([thud, amplify(tone, 0.5)])


def make_shield_block():
    """Metallic clank when spin bounces off shield."""
    clank = sine(1800, 0.12)
    clank2 = sine(2400, 0.08)
    env = envelope_linear(mix([clank, amplify(clank2, 0.7)]), attack=0.002, sustain=0.1, release=0.5)
    n = noise(0.04)
    n = envelope_linear(n, attack=0.001, sustain=0.1, release=0.5)
    return mix([env, amplify(n, 0.3)])


SOUNDS = {
    "Jump": make_jump,
    "Coin": make_coin,
    "Hit": make_hit,
    "SpinWhoosh": make_spin_whoosh,
    "CrateBreak": make_crate_break,
    "BounceCrate": make_bounce_crate,
    "WumpaCollect": make_wumpa_collect,
    "ExtraLife": make_extra_life,
    "AkuAkuPickup": make_akuaku_pickup,
    "AkuAkuHit": make_akuaku_hit,
    "AkuAkuInvincible": make_akuaku_invincible,
    "TNTTick": make_tnt_tick,
    "TNTExplosion": make_tnt_explosion,
    "GameOver": make_game_over,
    "ArrowLaunch": make_arrow_launch,
    "EnemyDefeat": make_enemy_defeat,
    "EnemyHit": make_enemy_hit,
    "ShieldBlock": make_shield_block,
}


# ---------------------------------------------------------------------------
# WAV export
# ---------------------------------------------------------------------------

def write_wav(filepath, samples, sample_rate=SAMPLE_RATE):
    """Write samples (float -1..1) to a 16-bit mono WAV file."""
    with wave.open(filepath, "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        max_val = 32767
        data = b"".join(
            struct.pack("<h", max(-max_val, min(max_val, int(s * max_val))))
            for s in samples
        )
        wf.writeframes(data)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = sys.argv[1:]
    output_dir = args[0] if args else "."
    sound_filter = args[1].split(",") if len(args) > 1 else None

    os.makedirs(output_dir, exist_ok=True)

    for name, generator in SOUNDS.items():
        if sound_filter and name not in sound_filter:
            continue
        samples = generator()
        filename = f"SFX_{name}.wav"
        filepath = os.path.join(output_dir, filename)
        write_wav(filepath, samples)
        print(f"  Written: {filepath} ({len(samples)} samples, {len(samples)/SAMPLE_RATE:.2f}s)")

    print("Done.")


if __name__ == "__main__":
    main()
