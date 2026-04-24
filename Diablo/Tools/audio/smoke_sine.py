"""Smoke test: synthesize a 440 Hz sine for 1 second as 16-bit PCM WAV.

Verifies numpy + scipy.io.wavfile are available and that UE can import the result.

    python Tools/audio/smoke_sine.py
"""
import os
import numpy as np
from scipy.io import wavfile

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out")
OUT_WAV = os.path.join(OUT_DIR, "sine.wav")

os.makedirs(OUT_DIR, exist_ok=True)

SAMPLE_RATE = 44100
DURATION_S = 1.0
FREQ_HZ = 440.0

t = np.linspace(0.0, DURATION_S, int(SAMPLE_RATE * DURATION_S), endpoint=False)
wave = 0.5 * np.sin(2.0 * np.pi * FREQ_HZ * t)

# 5ms linear fade in/out to avoid click artifacts on loop
fade = int(0.005 * SAMPLE_RATE)
wave[:fade] *= np.linspace(0.0, 1.0, fade)
wave[-fade:] *= np.linspace(1.0, 0.0, fade)

pcm = (wave * 32767.0).astype(np.int16)
wavfile.write(OUT_WAV, SAMPLE_RATE, pcm)

print(f"[smoke_sine] wrote {OUT_WAV} ({SAMPLE_RATE} Hz, {DURATION_S}s, 16-bit PCM)")
