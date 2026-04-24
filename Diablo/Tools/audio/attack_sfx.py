"""Generate sword attack sound effects for M2 melee combat.

    python Tools/audio/attack_sfx.py

Outputs 16-bit 44100 Hz mono WAV files to Tools/audio/out/.
"""
import os
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter, lfilter

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
RATE = 44100

os.makedirs(OUT_DIR, exist_ok=True)


def save(name, samples):
    samples = np.clip(samples, -1.0, 1.0)
    pcm = (samples * 32767).astype(np.int16)
    path = os.path.join(OUT_DIR, f"{name}.wav")
    wavfile.write(path, RATE, pcm)
    print(f"  {name}.wav  ({len(pcm) / RATE:.2f}s)")


def t(duration):
    return np.linspace(0.0, duration, int(RATE * duration), endpoint=False)


def noise(n):
    return np.random.default_rng(42).uniform(-1.0, 1.0, n)


def lowpass(sig, cutoff, order=4):
    b, a = butter(order, cutoff / (RATE / 2), btype="low")
    return lfilter(b, a, sig)


def highpass(sig, cutoff, order=4):
    b, a = butter(order, cutoff / (RATE / 2), btype="high")
    return lfilter(b, a, sig)


def bandpass(sig, low, high, order=4):
    b, a = butter(order, [low / (RATE / 2), high / (RATE / 2)], btype="band")
    return lfilter(b, a, sig)


def env_adsr(n, attack, decay, sustain_level, release):
    e = np.ones(n)
    a_s = int(attack * RATE)
    d_s = int(decay * RATE)
    r_s = int(release * RATE)
    s_s = n - a_s - d_s - r_s
    if s_s < 0:
        s_s = 0
        r_s = max(0, n - a_s - d_s)
    idx = 0
    e[idx:idx + a_s] = np.linspace(0, 1, a_s)
    idx += a_s
    e[idx:idx + d_s] = np.linspace(1, sustain_level, d_s)
    idx += d_s
    e[idx:idx + s_s] = sustain_level
    idx += s_s
    e[idx:idx + r_s] = np.linspace(sustain_level, 0, r_s)
    return e


def env_decay(n, speed=5.0):
    return np.exp(-speed * np.linspace(0, 1, n))


def pitch_sweep(duration, f_start, f_end):
    ts = t(duration)
    freqs = np.linspace(f_start, f_end, len(ts))
    phase = np.cumsum(freqs / RATE) * 2 * np.pi
    return np.sin(phase)


def sword_swing():
    dur = 0.35
    n = int(RATE * dur)
    sig = highpass(noise(n), 2000) * env_adsr(n, 0.01, 0.05, 0.3, 0.15)
    sweep = pitch_sweep(dur, 800, 2500) * 0.3 * env_decay(n, 6)
    return 0.7 * (sig + sweep)


def sword_hit():
    dur = 0.25
    n = int(RATE * dur)
    impact = lowpass(noise(n), 600) * env_decay(n, 10) * 0.8
    ring = np.sin(2 * np.pi * 1200 * t(dur)) * env_decay(n, 12) * 0.3
    ring += np.sin(2 * np.pi * 2400 * t(dur)) * env_decay(n, 15) * 0.15
    crack = bandpass(noise(n), 1500, 4000) * env_decay(n, 20) * 0.5
    return impact + ring + crack


print("Generating attack SFX...")
save("SwordSwing", sword_swing())
save("SwordHit", sword_hit())
print(f"\nDone — 2 files in {OUT_DIR}")
