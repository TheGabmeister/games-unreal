"""Generate level-up sound effect for M6 progression.

    python Tools/audio/levelup_sfx.py

Outputs 16-bit 44100 Hz mono WAV to Tools/audio/out/.
"""
import os
import numpy as np
from scipy.io import wavfile

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


def env_decay(n, speed=5.0):
    return np.exp(-speed * np.linspace(0, 1, n))


def levelup():
    """Ascending three-note chime with shimmer — triumphant D1 feel."""
    dur = 1.2
    n = int(RATE * dur)
    sig = np.zeros(n)

    # Three ascending notes: C5, E5, G5 (major chord arpeggio)
    notes = [(523.25, 0.0), (659.25, 0.15), (783.99, 0.30)]

    for freq, onset in notes:
        start = int(onset * RATE)
        note_dur = dur - onset
        note_n = int(note_dur * RATE)
        ts = t(note_dur)

        tone = np.sin(2 * np.pi * freq * ts) * 0.4
        tone += np.sin(2 * np.pi * freq * 2 * ts) * 0.15
        tone += np.sin(2 * np.pi * freq * 3 * ts) * 0.05
        tone *= env_decay(note_n, 3.0)

        end = min(start + note_n, n)
        sig[start:end] += tone[:end - start]

    # Shimmer: high-frequency sparkle
    shimmer_n = int(0.8 * RATE)
    shimmer_start = int(0.3 * RATE)
    shimmer_ts = t(0.8)
    shimmer = np.sin(2 * np.pi * 3135.96 * shimmer_ts) * 0.08
    shimmer += np.sin(2 * np.pi * 4186.01 * shimmer_ts) * 0.04
    shimmer *= env_decay(shimmer_n, 4.0)
    end = min(shimmer_start + shimmer_n, n)
    sig[shimmer_start:end] += shimmer[:end - shimmer_start]

    return sig * 0.8


print("Generating level-up SFX...")
save("LevelUp", levelup())
print(f"\nDone — 1 file in {OUT_DIR}")
