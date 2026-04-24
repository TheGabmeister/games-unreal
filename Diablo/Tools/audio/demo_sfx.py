"""Generate a set of Diablo-style demo sound effects to evaluate synthesis quality.

    python Tools/audio/demo_sfx.py

Outputs 16-bit 44100 Hz mono WAV files to Tools/audio/out/demo/.
"""
import os
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter, lfilter

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out", "demo")
RATE = 44100

os.makedirs(OUT_DIR, exist_ok=True)


def save(name: str, samples: np.ndarray):
    samples = np.clip(samples, -1.0, 1.0)
    pcm = (samples * 32767).astype(np.int16)
    path = os.path.join(OUT_DIR, f"{name}.wav")
    wavfile.write(path, RATE, pcm)
    print(f"  {name}.wav  ({len(pcm) / RATE:.2f}s)")


def t(duration: float) -> np.ndarray:
    return np.linspace(0.0, duration, int(RATE * duration), endpoint=False)


def noise(n: int) -> np.ndarray:
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


# ---------- 1. SWORD SWING ----------
def sword_swing():
    dur = 0.35
    n = int(RATE * dur)
    sig = highpass(noise(n), 2000) * env_adsr(n, 0.01, 0.05, 0.3, 0.15)
    sweep = pitch_sweep(dur, 800, 2500) * 0.3 * env_decay(n, 6)
    return 0.7 * (sig + sweep)


# ---------- 2. SWORD HIT (metal on flesh) ----------
def sword_hit():
    dur = 0.25
    n = int(RATE * dur)
    impact = lowpass(noise(n), 600) * env_decay(n, 10) * 0.8
    ring = np.sin(2 * np.pi * 1200 * t(dur)) * env_decay(n, 12) * 0.3
    ring += np.sin(2 * np.pi * 2400 * t(dur)) * env_decay(n, 15) * 0.15
    crack = bandpass(noise(n), 1500, 4000) * env_decay(n, 20) * 0.5
    return impact + ring + crack


# ---------- 3. MACE THUD ----------
def mace_thud():
    dur = 0.3
    n = int(RATE * dur)
    boom = pitch_sweep(dur, 120, 40) * env_decay(n, 8) * 0.9
    body = lowpass(noise(n), 300) * env_decay(n, 7) * 0.6
    click = bandpass(noise(n), 2000, 5000) * env_decay(n, 25) * 0.4
    return boom + body + click


# ---------- 4. SHIELD BLOCK ----------
def shield_block():
    dur = 0.2
    n = int(RATE * dur)
    clang = np.sin(2 * np.pi * 800 * t(dur)) * env_decay(n, 15) * 0.5
    clang += np.sin(2 * np.pi * 1600 * t(dur)) * env_decay(n, 18) * 0.3
    clang += np.sin(2 * np.pi * 3200 * t(dur)) * env_decay(n, 22) * 0.15
    transient = highpass(noise(n), 3000) * env_decay(n, 30) * 0.6
    return clang + transient


# ---------- 5. FOOTSTEP (stone floor) ----------
def footstep():
    dur = 0.15
    n = int(RATE * dur)
    tap = lowpass(noise(n), 400) * env_decay(n, 15) * 0.5
    click = bandpass(noise(n), 2000, 6000) * env_decay(n, 30) * 0.3
    return tap + click


# ---------- 6. POTION DRINK ----------
def potion_drink():
    dur = 0.6
    n = int(RATE * dur)
    gulp1_start = 0
    gulp1_n = int(0.15 * RATE)
    gulp2_start = int(0.2 * RATE)
    gulp2_n = int(0.12 * RATE)
    gulp3_start = int(0.35 * RATE)
    gulp3_n = int(0.1 * RATE)
    sig = np.zeros(n)
    for start, gn in [(gulp1_start, gulp1_n), (gulp2_start, gulp2_n), (gulp3_start, gulp3_n)]:
        freq_base = 200 + np.random.default_rng(start).integers(0, 80)
        gulp = np.sin(2 * np.pi * freq_base * t(gn / RATE))
        gulp *= bandpass(noise(gn), 150, 500) * 0.5 + 0.5
        gulp *= env_adsr(gn, 0.01, 0.03, 0.6, 0.05)
        end = min(start + gn, n)
        sig[start:end] += gulp[:end - start] * 0.5
    bubble = bandpass(noise(n), 400, 1200) * 0.1
    bubble *= env_adsr(n, 0.05, 0.1, 0.3, 0.2)
    return sig + bubble


# ---------- 7. GOLD PICKUP ----------
def gold_pickup():
    dur = 0.4
    n = int(RATE * dur)
    sig = np.zeros(n)
    freqs = [2200, 2800, 3500, 4200]
    for i, f in enumerate(freqs):
        start = int(i * 0.06 * RATE)
        clink_dur = 0.12
        cn = int(clink_dur * RATE)
        ts = t(clink_dur)
        clink = np.sin(2 * np.pi * f * ts) * env_decay(cn, 18) * 0.35
        clink += np.sin(2 * np.pi * f * 1.5 * ts) * env_decay(cn, 22) * 0.15
        end = min(start + cn, n)
        sig[start:end] += clink[:end - start]
    return sig


# ---------- 8. DOOR OPEN (heavy stone) ----------
def door_open():
    dur = 1.2
    n = int(RATE * dur)
    rumble = lowpass(noise(n), 200) * 0.4
    rumble *= env_adsr(n, 0.1, 0.2, 0.7, 0.4)
    grind = bandpass(noise(n), 300, 1500) * 0.25
    grind *= env_adsr(n, 0.15, 0.1, 0.5, 0.3)
    creak_mod = 0.5 + 0.5 * np.sin(2 * np.pi * 3 * t(dur))
    grind *= creak_mod
    boom = pitch_sweep(0.3, 80, 30) * env_decay(int(0.3 * RATE), 6) * 0.5
    sig = rumble + grind
    sig[-int(0.3 * RATE):] += boom * 0.7
    return sig


# ---------- 9. FIREBOLT CAST ----------
def firebolt_cast():
    dur = 0.5
    n = int(RATE * dur)
    whoosh = bandpass(noise(n), 500, 3000) * env_adsr(n, 0.02, 0.1, 0.4, 0.2) * 0.5
    crackle = highpass(noise(n), 4000) * 0.15
    rng = np.random.default_rng(99)
    mask = np.zeros(n)
    for _ in range(40):
        pos = rng.integers(0, n)
        width = rng.integers(20, 200)
        end = min(pos + width, n)
        mask[pos:end] = 1.0
    crackle *= mask * env_decay(n, 4)
    tone = pitch_sweep(dur, 600, 1200) * env_adsr(n, 0.01, 0.05, 0.3, 0.2) * 0.3
    return whoosh + crackle + tone


# ---------- 10. FIREBALL EXPLOSION ----------
def fireball_explosion():
    dur = 0.8
    n = int(RATE * dur)
    boom = pitch_sweep(0.2, 200, 40)
    boom_n = len(boom)
    boom *= env_decay(boom_n, 6) * 0.9
    roar = bandpass(noise(n), 100, 800) * env_adsr(n, 0.01, 0.15, 0.5, 0.3) * 0.6
    crackle = highpass(noise(n), 3000) * 0.2
    crackle *= env_adsr(n, 0.02, 0.2, 0.2, 0.3)
    sig = roar + crackle
    sig[:boom_n] += boom
    return sig


# ---------- 11. LIGHTNING BOLT ----------
def lightning_bolt():
    dur = 0.6
    n = int(RATE * dur)
    rng = np.random.default_rng(77)
    sig = np.zeros(n)
    for _ in range(6):
        start = rng.integers(0, n // 2)
        zap_dur = rng.integers(int(0.02 * RATE), int(0.08 * RATE))
        end = min(start + zap_dur, n)
        zap_n = end - start
        zap = highpass(noise(zap_n), 2000) * 0.7
        zap += np.sin(2 * np.pi * rng.integers(1500, 4000) * np.linspace(0, zap_n / RATE, zap_n, endpoint=False)) * 0.3
        zap *= env_decay(zap_n, 8)
        sig[start:end] += zap
    rumble = lowpass(noise(n), 300) * env_adsr(n, 0.01, 0.1, 0.3, 0.3) * 0.4
    return sig + rumble


# ---------- 12. HEALING SPELL ----------
def healing_spell():
    dur = 1.0
    n = int(RATE * dur)
    ts = t(dur)
    shimmer = np.zeros(n)
    for f in [800, 1000, 1200, 1500, 1800]:
        shimmer += np.sin(2 * np.pi * f * ts) * 0.1
    shimmer *= env_adsr(n, 0.1, 0.2, 0.6, 0.3)
    vibrato = 1.0 + 0.02 * np.sin(2 * np.pi * 6 * ts)
    shimmer *= vibrato
    chime = np.sin(2 * np.pi * 2400 * ts) * env_decay(n, 3) * 0.15
    soft_noise = bandpass(noise(n), 2000, 6000) * 0.05 * env_adsr(n, 0.2, 0.1, 0.3, 0.3)
    return shimmer + chime + soft_noise


# ---------- 13. MONSTER GROWL ----------
def monster_growl():
    dur = 0.8
    n = int(RATE * dur)
    ts = t(dur)
    base = np.sin(2 * np.pi * 80 * ts)
    base += np.sin(2 * np.pi * 120 * ts) * 0.6
    base += np.sin(2 * np.pi * 160 * ts) * 0.3
    vibrato = 1.0 + 0.15 * np.sin(2 * np.pi * 5 * ts)
    base *= vibrato
    base *= env_adsr(n, 0.1, 0.15, 0.7, 0.25)
    rumble = lowpass(noise(n), 250) * 0.3 * env_adsr(n, 0.05, 0.1, 0.5, 0.2)
    return 0.7 * (base + rumble)


# ---------- 14. SKELETON RATTLE ----------
def skeleton_rattle():
    dur = 0.5
    n = int(RATE * dur)
    sig = np.zeros(n)
    rng = np.random.default_rng(55)
    for i in range(8):
        pos = int(i * 0.055 * RATE) + rng.integers(0, int(0.02 * RATE))
        click_n = rng.integers(int(0.008 * RATE), int(0.025 * RATE))
        if pos + click_n > n:
            break
        freq = rng.integers(1500, 4000)
        click_ts = np.linspace(0, click_n / RATE, click_n, endpoint=False)
        click = np.sin(2 * np.pi * freq * click_ts) * env_decay(click_n, 20)
        click += highpass(noise(click_n), 3000) * 0.3 * env_decay(click_n, 25)
        sig[pos:pos + click_n] += click * 0.5
    return sig


# ---------- 15. LEVEL UP ----------
def level_up():
    dur = 1.5
    n = int(RATE * dur)
    ts = t(dur)
    chord = np.zeros(n)
    notes = [523.25, 659.25, 783.99, 1046.50]
    for i, f in enumerate(notes):
        start = int(i * 0.15 * RATE)
        note_n = n - start
        note_ts = t(note_n / RATE)
        tone = np.sin(2 * np.pi * f * note_ts) * env_adsr(note_n, 0.02, 0.1, 0.6, 0.4) * 0.25
        chord[start:start + note_n] += tone
    shimmer = bandpass(noise(n), 3000, 8000) * 0.08 * env_adsr(n, 0.3, 0.2, 0.3, 0.5)
    sweep = pitch_sweep(dur, 400, 1200) * env_adsr(n, 0.05, 0.3, 0.2, 0.5) * 0.15
    return chord + shimmer + sweep


# ---------- 16. ENEMY DEATH ----------
def enemy_death():
    dur = 0.6
    n = int(RATE * dur)
    ts = t(dur)
    groan = pitch_sweep(dur, 200, 80) * env_adsr(n, 0.02, 0.1, 0.5, 0.3) * 0.5
    groan *= 1.0 + 0.1 * np.sin(2 * np.pi * 7 * ts)
    thud = lowpass(noise(n), 200) * env_decay(n, 8) * 0.4
    return groan + thud


# ---------- Generate all ----------
print("Generating Diablo demo SFX...")
save("01_sword_swing", sword_swing())
save("02_sword_hit", sword_hit())
save("03_mace_thud", mace_thud())
save("04_shield_block", shield_block())
save("05_footstep", footstep())
save("06_potion_drink", potion_drink())
save("07_gold_pickup", gold_pickup())
save("08_door_open", door_open())
save("09_firebolt_cast", firebolt_cast())
save("10_fireball_explosion", fireball_explosion())
save("11_lightning_bolt", lightning_bolt())
save("12_healing_spell", healing_spell())
save("13_monster_growl", monster_growl())
save("14_skeleton_rattle", skeleton_rattle())
save("15_level_up", level_up())
save("16_enemy_death", enemy_death())
print(f"\nDone — {16} files in {OUT_DIR}")
