"""
Procedural music generator for UE5 placeholder audio.

Generates MIDI files and renders them to WAV via FluidSynth.
MIDI composition uses only Python standard library.

Usage:
    python generate_music.py [output_dir] [track_filter]

    output_dir    — directory for output files (default: current directory)
    track_filter  — comma-separated list of track names (default: all)

Output: MUS_<Name>.mid + MUS_<Name>.wav per composition.

Requires:
    FluidSynth + SoundFont for WAV rendering.
    FFmpeg (optional) for loudness normalization.
"""

import os
import struct
import subprocess
import sys

FLUIDSYNTH = r"D:\fluidsynth-v2.5.4-win10-x64-cpp11\bin\fluidsynth.exe"
SOUNDFONT = r"D:\GeneralUser-GS\GeneralUser-GS.sf2"
FFMPEG = r"D:\ffmpeg-8.1-essentials_build\bin\ffmpeg.exe"

TICKS_PER_BEAT = 480
BAR = TICKS_PER_BEAT * 4  # 4/4 time


# ---------------------------------------------------------------------------
# MIDI file writer
# ---------------------------------------------------------------------------

def vlq(value):
    result = [value & 0x7F]
    value >>= 7
    while value:
        result.append((value & 0x7F) | 0x80)
        value >>= 7
    return bytes(reversed(result))


def events_to_track(events):
    events.sort(key=lambda e: e[0])
    data = b""
    prev = 0
    for tick, ev in events:
        data += vlq(tick - prev) + ev
        prev = tick
    data += vlq(0) + b"\xFF\x2F\x00"
    return b"MTrk" + struct.pack(">I", len(data)) + data


def make_midi(tracks, tpb=TICKS_PER_BEAT):
    header = b"MThd" + struct.pack(">IhhH", 6, 1, len(tracks), tpb)
    return header + b"".join(tracks)


# ---------------------------------------------------------------------------
# Event helpers
# ---------------------------------------------------------------------------

NOTE_MAP = {
    "C": 0, "C#": 1, "Db": 1, "D": 2, "D#": 3, "Eb": 3,
    "E": 4, "F": 5, "F#": 6, "Gb": 6, "G": 7, "G#": 8, "Ab": 8,
    "A": 9, "A#": 10, "Bb": 10, "B": 11,
}


def n(name, octave):
    return 12 * (octave + 1) + NOTE_MAP[name]


def note_on_bytes(ch, pitch, vel):
    return bytes([0x90 | ch, pitch, vel])


def note_off_bytes(ch, pitch):
    return bytes([0x80 | ch, pitch, 0])


def add_note(events, ch, bar, beat, pitch, dur, vel=100):
    start = (bar - 1) * BAR + int((beat - 1) * TICKS_PER_BEAT)
    end = start + int(dur * TICKS_PER_BEAT)
    events.append((start, note_on_bytes(ch, pitch, vel)))
    events.append((end, note_off_bytes(ch, pitch)))


def add_chord(events, ch, bar, beat, pitches, dur, vel=100):
    for p in pitches:
        add_note(events, ch, bar, beat, p, dur, vel)


def cc(ch, ctrl, val):
    return bytes([0xB0 | ch, ctrl, val])


def pc(ch, prog):
    return bytes([0xC0 | ch, prog])


def meta_tempo(bpm):
    uspb = int(60_000_000 / bpm)
    return b"\xFF\x51\x03" + uspb.to_bytes(3, "big")


# ---------------------------------------------------------------------------
# BattleDark — dark fantasy RPG battle music
# Key: D minor | Tempo: 144 BPM | 12 bars (20 seconds) | Loopable
#
# Progression:
#   A (1-4):  Dm    | Dm/C  | Bb    | A
#   B (5-8):  Dm    | F     | Bb    | A
#   C (9-12): Gm    | Bb    | Dm    | A
#
# Instruments:
#   Ch 0  Timpani (prog 47)         Ch 5  French Horn (prog 60)
#   Ch 1  Contrabass (prog 43)      Ch 6  Cello ostinato (prog 42)
#   Ch 2  String Ensemble (prog 48) Ch 7  Celesta (prog 8)
#   Ch 3  Brass Section (prog 61)   Ch 9  Drums (GM percussion)
#   Ch 4  Choir Aahs (prog 52)
# ---------------------------------------------------------------------------

# Chords (octave 4 voicing)
Dm = [n("D", 4), n("F", 4), n("A", 4)]
Bb = [n("Bb", 3), n("D", 4), n("F", 4)]
C_ = [n("C", 4), n("E", 4), n("G", 4)]
F_ = [n("F", 3), n("A", 3), n("C", 4)]
Gm = [n("G", 3), n("Bb", 3), n("D", 4)]
A_ = [n("A", 3), n("C#", 4), n("E", 4)]

PROG = [Dm, Dm, Bb, A_, Dm, F_, Bb, A_, Gm, Bb, Dm, A_]

BASS = [
    n("D", 2), n("C", 2), n("Bb", 1), n("A", 1),
    n("D", 2), n("F", 2), n("Bb", 1), n("A", 1),
    n("G", 1), n("Bb", 1), n("D", 2), n("A", 1),
]


def _tempo_track():
    return events_to_track([
        (0, meta_tempo(144)),
        (0, b"\xFF\x58\x04\x04\x02\x18\x08"),
    ])


def _drums():
    ch = 9
    KICK, SNARE, HH = 36, 38, 42
    CRASH, LOW_TOM, MID_TOM, HI_TOM = 49, 41, 45, 48
    events = []

    for bar in range(1, 13):
        # Hi-hat 8ths
        for i in range(8):
            beat = 1 + i * 0.5
            add_note(events, ch, bar, beat, HH, 0.2, 75 if i % 2 == 0 else 55)

        # Kick: 1, 3, &4
        add_note(events, ch, bar, 1, KICK, 0.4, 115)
        add_note(events, ch, bar, 3, KICK, 0.4, 110)
        add_note(events, ch, bar, 4.5, KICK, 0.4, 90)

        # Snare: 2, 4
        add_note(events, ch, bar, 2, SNARE, 0.4, 105)
        add_note(events, ch, bar, 4, SNARE, 0.4, 105)

        # Crash on section starts
        if bar in (1, 5, 9):
            add_note(events, ch, bar, 1, CRASH, 1.5, 115)

        # Tom fills before sections
        if bar in (4, 8, 12):
            add_note(events, ch, bar, 3.5, HI_TOM, 0.3, 95)
            add_note(events, ch, bar, 4, MID_TOM, 0.3, 100)
            add_note(events, ch, bar, 4.5, LOW_TOM, 0.4, 105)

    return events_to_track(events)


def _timpani():
    ch = 0
    events = [(0, pc(ch, 47)), (0, cc(ch, 7, 120))]

    hits = {
        (1, 1): (n("D", 2), 125), (3, 1): (n("Bb", 1), 115),
        (4, 1): (n("A", 1), 110), (5, 1): (n("D", 2), 125),
        (7, 1): (n("Bb", 1), 115), (9, 1): (n("G", 1), 120),
        (10, 1): (n("Bb", 1), 115), (11, 1): (n("D", 2), 120),
    }
    for (bar, beat), (pitch, vel) in hits.items():
        add_note(events, ch, bar, beat, pitch, 1, vel)

    # Rolls at bar 4 and 12 (beats 3-4)
    for bar, pitch in [(4, n("A", 1)), (12, n("D", 2))]:
        for i in range(4):
            add_note(events, ch, bar, 3 + i * 0.5, pitch, 0.35, 85 + i * 8)

    return events_to_track(events)


def _bass():
    ch = 1
    events = [(0, pc(ch, 43)), (0, cc(ch, 7, 115))]

    for bar_i, root in enumerate(BASS):
        bar = bar_i + 1
        fifth = root + 7
        octave = root + 12
        pattern = [root, root, octave, root, fifth, root, octave, root]
        for i, p in enumerate(pattern):
            add_note(events, ch, bar, 1 + i * 0.5, p, 0.35,
                     105 if i % 2 == 0 else 85)

    return events_to_track(events)


def _cello_ostinato():
    ch = 6
    events = [(0, pc(ch, 42)), (0, cc(ch, 7, 90)), (0, cc(ch, 10, 45))]

    for bar_i, chord in enumerate(PROG):
        bar = bar_i + 1
        root, third, fifth = [p - 12 for p in chord]
        pattern = [root, third, fifth, third + 12, fifth, third, root, third]
        base_vel = 65 if bar <= 4 else 75 if bar <= 8 else 85
        for i, p in enumerate(pattern):
            add_note(events, ch, bar, 1 + i * 0.5, p, 0.35,
                     base_vel if i % 2 == 0 else base_vel - 10)

    return events_to_track(events)


def _strings():
    ch = 2
    events = [(0, pc(ch, 48)), (0, cc(ch, 7, 95))]

    for bar_i, chord in enumerate(PROG):
        bar = bar_i + 1
        vel = 70 if bar <= 4 else 85 if bar <= 8 else 100
        add_chord(events, ch, bar, 1, chord, 3.9, vel)
        add_chord(events, ch, bar, 1, [p + 12 for p in chord], 3.9, vel - 15)

    # Tremolo strings in climax (bars 9-12): switch to prog 44
    events.append(((9 - 1) * BAR, pc(ch, 44)))
    events.append(((9 - 1) * BAR, cc(ch, 7, 100)))

    return events_to_track(events)


def _brass():
    ch = 3
    events = [(0, pc(ch, 61)), (0, cc(ch, 7, 108))]

    D5, E5, F5, G5 = n("D", 5), n("E", 5), n("F", 5), n("G", 5)
    C5, Bb4, A4, Cs5 = n("C", 5), n("Bb", 4), n("A", 4), n("C#", 5)

    # Section A: stabs
    for bar in range(1, 5):
        ch_notes = [p + 12 for p in PROG[bar - 1]]
        add_chord(events, ch, bar, 1, ch_notes, 0.7, 112)
        add_chord(events, ch, bar, 3, ch_notes, 0.7, 95)

    # Section B: melody
    melody_b = [
        (5, 1, D5, 1), (5, 2, F5, 1), (5, 3, E5, 1), (5, 4, D5, 1),
        (6, 1, F5, 2), (6, 3, E5, 1), (6, 4, C5, 1),
        (7, 1, D5, 1), (7, 2, Bb4, 1), (7, 3, D5, 1), (7, 4, F5, 1),
        (8, 1, E5, 3), (8, 4, Cs5, 1),
    ]
    for bar, beat, pitch, dur in melody_b:
        add_note(events, ch, bar, beat, pitch, dur, 108)

    # Section C: melody variation (higher energy)
    melody_c = [
        (9, 1, Bb4, 1), (9, 2, D5, 1), (9, 3, G5, 1), (9, 4, F5, 1),
        (10, 1, D5, 1), (10, 2, Bb4, 1), (10, 3, F5, 1), (10, 4, D5, 1),
        (11, 1, D5, 2), (11, 3, C5, 1), (11, 4, A4, 1),
        (12, 1, A4, 2), (12, 3, Cs5, 1), (12, 4, E5, 1),
    ]
    for bar, beat, pitch, dur in melody_c:
        add_note(events, ch, bar, beat, pitch, dur, 115)

    return events_to_track(events)


def _horn():
    ch = 5
    events = [(0, pc(ch, 60)), (0, cc(ch, 7, 95)), (0, cc(ch, 10, 80))]

    A3, Bb3, C4, D4, E4 = n("A", 3), n("Bb", 3), n("C", 4), n("D", 4), n("E", 4)
    Cs4, G3 = n("C#", 4), n("G", 3)

    counter = [
        (5, 1, A3, 2, 85), (5, 3, D4, 2, 85),
        (6, 1, C4, 2, 85), (6, 3, A3, 2, 80),
        (7, 1, Bb3, 2, 85), (7, 3, D4, 2, 90),
        (8, 1, Cs4, 3.9, 90),
        (9, 1, G3, 2, 95), (9, 3, Bb3, 2, 95),
        (10, 1, Bb3, 2, 95), (10, 3, D4, 2, 95),
        (11, 1, D4, 3.9, 100),
        (12, 1, A3, 2, 100), (12, 3, E4, 2, 105),
    ]
    for bar, beat, pitch, dur, vel in counter:
        add_note(events, ch, bar, beat, pitch, dur, vel)

    return events_to_track(events)


def _choir():
    ch = 4
    events = [(0, pc(ch, 52)), (0, cc(ch, 7, 85))]

    roots = [
        None, None, n("Bb", 3), n("A", 3),
        n("D", 4), n("F", 3), n("Bb", 3), n("A", 3),
        n("G", 3), n("Bb", 3), n("D", 4), n("A", 3),
    ]
    for bar_i, pitch in enumerate(roots):
        if pitch is None:
            continue
        bar = bar_i + 1
        vel = 50 if bar <= 4 else 60 if bar <= 8 else 72
        add_note(events, ch, bar, 1, pitch, 3.9, vel)
        add_note(events, ch, bar, 1, pitch + 7, 3.9, vel - 10)

    return events_to_track(events)


def _celesta():
    ch = 7
    events = [(0, pc(ch, 8)), (0, cc(ch, 7, 65)), (0, cc(ch, 10, 85))]

    D6, E6, F6, G6 = n("D", 6), n("E", 6), n("F", 6), n("G", 6)
    C6, Bb5, A5, Cs6 = n("C", 6), n("Bb", 5), n("A", 5), n("C#", 6)

    # Ghost the brass melody one octave up, softer
    melody = [
        (5, 1, D6, 1), (5, 2, F6, 1), (5, 3, E6, 1), (5, 4, D6, 1),
        (6, 1, F6, 2), (6, 3, E6, 1), (6, 4, C6, 1),
        (7, 1, D6, 1), (7, 2, Bb5, 1), (7, 3, D6, 1), (7, 4, F6, 1),
        (8, 1, E6, 3), (8, 4, Cs6, 1),
        (9, 1, Bb5, 1), (9, 2, D6, 1), (9, 3, G6, 1), (9, 4, F6, 1),
        (10, 1, D6, 1), (10, 2, Bb5, 1), (10, 3, F6, 1), (10, 4, D6, 1),
        (11, 1, D6, 2), (11, 3, C6, 1), (11, 4, A5, 1),
        (12, 1, A5, 2), (12, 3, Cs6, 1), (12, 4, E6, 1),
    ]
    for bar, beat, pitch, dur in melody:
        add_note(events, ch, bar, beat, pitch, dur, 55)

    return events_to_track(events)


def compose_battle_dark():
    tracks = [
        _tempo_track(),
        _drums(),
        _timpani(),
        _bass(),
        _cello_ostinato(),
        _strings(),
        _brass(),
        _horn(),
        _choir(),
        _celesta(),
    ]
    return make_midi(tracks)


# ---------------------------------------------------------------------------
# Invincibility — fast-paced power-up music
# Key: C major | Tempo: 170 BPM | 12 bars (~17s) | Loopable
#
# Progression:
#   A (1-4):  C     | G     | Am    | F
#   B (5-8):  C     | G     | F     | G
#   C (9-12): Am    | F     | C     | G
#
# Instruments:
#   Ch 0  Synth Lead (prog 80)       Ch 4  Timpani (prog 47)
#   Ch 1  Slap Bass (prog 36)        Ch 9  Drums (GM percussion)
#   Ch 2  Synth Brass (prog 62)
#   Ch 3  Marimba (prog 12)
# ---------------------------------------------------------------------------

_I_C  = [n("C", 4), n("E", 4), n("G", 4)]
_I_G  = [n("G", 3), n("B", 3), n("D", 4)]
_I_Am = [n("A", 3), n("C", 4), n("E", 4)]
_I_F  = [n("F", 3), n("A", 3), n("C", 4)]

_I_PROG = [_I_C, _I_G, _I_Am, _I_F, _I_C, _I_G, _I_F, _I_G, _I_Am, _I_F, _I_C, _I_G]
_I_BASS = [
    n("C", 2), n("G", 2), n("A", 2), n("F", 2),
    n("C", 2), n("G", 2), n("F", 2), n("G", 2),
    n("A", 2), n("F", 2), n("C", 2), n("G", 2),
]


def _inv_tempo_track():
    return events_to_track([
        (0, meta_tempo(170)),
        (0, b"\xFF\x58\x04\x04\x02\x18\x08"),
    ])


def _inv_drums():
    ch = 9
    KICK, SNARE, HH, OH = 36, 38, 42, 46
    CRASH = 49
    events = []
    for bar in range(1, 13):
        # Driving 16th hi-hats
        for i in range(16):
            beat = 1 + i * 0.25
            vel = 80 if i % 4 == 0 else 55 if i % 2 == 0 else 40
            add_note(events, ch, bar, beat, HH, 0.1, vel)
        # Open hi-hat on &2, &4
        add_note(events, ch, bar, 2.5, OH, 0.2, 70)
        add_note(events, ch, bar, 4.5, OH, 0.2, 70)
        # Kick: four-on-floor + extra hits
        add_note(events, ch, bar, 1, KICK, 0.3, 120)
        add_note(events, ch, bar, 2, KICK, 0.3, 110)
        add_note(events, ch, bar, 3, KICK, 0.3, 115)
        add_note(events, ch, bar, 4, KICK, 0.3, 110)
        add_note(events, ch, bar, 3.5, KICK, 0.2, 85)
        # Snare: 2, 4
        add_note(events, ch, bar, 2, SNARE, 0.3, 115)
        add_note(events, ch, bar, 4, SNARE, 0.3, 115)
        # Crash on section starts
        if bar in (1, 5, 9):
            add_note(events, ch, bar, 1, CRASH, 1.5, 120)
    return events_to_track(events)


def _inv_bass():
    ch = 1
    events = [(0, pc(ch, 36)), (0, cc(ch, 7, 120))]
    for bar_i, root in enumerate(_I_BASS):
        bar = bar_i + 1
        oct = root + 12
        fifth = root + 7
        # Driving 8th-note bass line
        pattern = [root, oct, root, fifth, root, oct, fifth, root]
        for i, p in enumerate(pattern):
            add_note(events, ch, bar, 1 + i * 0.5, p, 0.35,
                     115 if i % 2 == 0 else 90)
    return events_to_track(events)


def _inv_lead():
    ch = 0
    events = [(0, pc(ch, 80)), (0, cc(ch, 7, 110))]

    C5, D5, E5, F5, G5 = n("C", 5), n("D", 5), n("E", 5), n("F", 5), n("G", 5)
    A5, B5, C6 = n("A", 5), n("B", 5), n("C", 6)
    A4, B4 = n("A", 4), n("B", 4)

    # Section A: energetic ascending melody
    melody_a = [
        (1, 1, C5, 0.5), (1, 1.5, E5, 0.5), (1, 2, G5, 1), (1, 3, A5, 0.5), (1, 3.5, G5, 0.5), (1, 4, E5, 1),
        (2, 1, D5, 0.5), (2, 1.5, G5, 0.5), (2, 2, B5, 1), (2, 3, A5, 0.5), (2, 3.5, G5, 0.5), (2, 4, D5, 1),
        (3, 1, A4, 0.5), (3, 1.5, C5, 0.5), (3, 2, E5, 1), (3, 3, G5, 0.5), (3, 3.5, E5, 0.5), (3, 4, C5, 1),
        (4, 1, F5, 1), (4, 2, E5, 0.5), (4, 2.5, D5, 0.5), (4, 3, C5, 1.5),
    ]
    for bar, beat, pitch, dur in melody_a:
        add_note(events, ch, bar, beat, pitch, dur, 110)

    # Section B: higher energy variation
    melody_b = [
        (5, 1, C5, 0.5), (5, 1.5, E5, 0.5), (5, 2, G5, 0.5), (5, 2.5, C6, 0.5), (5, 3, B5, 1), (5, 4, G5, 1),
        (6, 1, G5, 0.5), (6, 1.5, A5, 0.5), (6, 2, B5, 1), (6, 3, A5, 0.5), (6, 3.5, G5, 0.5), (6, 4, D5, 1),
        (7, 1, F5, 0.5), (7, 1.5, A5, 0.5), (7, 2, C6, 1), (7, 3, A5, 0.5), (7, 3.5, F5, 0.5), (7, 4, C5, 1),
        (8, 1, G5, 1), (8, 2, A5, 0.5), (8, 2.5, B5, 0.5), (8, 3, C6, 2),
    ]
    for bar, beat, pitch, dur in melody_b:
        add_note(events, ch, bar, beat, pitch, dur, 115)

    # Section C: climactic peak
    melody_c = [
        (9, 1, A5, 0.5), (9, 1.5, C6, 0.5), (9, 2, A5, 1), (9, 3, G5, 0.5), (9, 3.5, E5, 0.5), (9, 4, C5, 1),
        (10, 1, F5, 0.5), (10, 1.5, A5, 0.5), (10, 2, C6, 1), (10, 3, A5, 1), (10, 4, F5, 1),
        (11, 1, C6, 1), (11, 2, B5, 0.5), (11, 2.5, A5, 0.5), (11, 3, G5, 1), (11, 4, E5, 1),
        (12, 1, G5, 1), (12, 2, A5, 0.5), (12, 2.5, B5, 0.5), (12, 3, C6, 2),
    ]
    for bar, beat, pitch, dur in melody_c:
        add_note(events, ch, bar, beat, pitch, dur, 120)

    return events_to_track(events)


def _inv_brass():
    ch = 2
    events = [(0, pc(ch, 62)), (0, cc(ch, 7, 100))]
    for bar_i, chord in enumerate(_I_PROG):
        bar = bar_i + 1
        vel = 90 if bar <= 4 else 100 if bar <= 8 else 110
        high = [p + 12 for p in chord]
        # Staccato stabs on 1 and 3
        add_chord(events, ch, bar, 1, high, 0.5, vel)
        add_chord(events, ch, bar, 3, high, 0.5, vel - 10)
    return events_to_track(events)


def _inv_marimba():
    ch = 3
    events = [(0, pc(ch, 12)), (0, cc(ch, 7, 80))]
    for bar_i, chord in enumerate(_I_PROG):
        bar = bar_i + 1
        root, third, fifth = chord
        # Arpeggiated 16th patterns
        pattern = [root, third, fifth, third + 12, fifth, third, root + 12, fifth]
        for i, p in enumerate(pattern):
            add_note(events, ch, bar, 1 + i * 0.5, p + 12, 0.35, 75 if i % 2 == 0 else 60)
    return events_to_track(events)


def _inv_timpani():
    ch = 4
    events = [(0, pc(ch, 47)), (0, cc(ch, 7, 105))]
    for bar_i, root in enumerate(_I_BASS):
        bar = bar_i + 1
        add_note(events, ch, bar, 1, root, 0.8, 110)
        if bar in (4, 8, 12):
            add_note(events, ch, bar, 3, root, 0.3, 90)
            add_note(events, ch, bar, 3.5, root, 0.3, 95)
            add_note(events, ch, bar, 4, root, 0.3, 100)
            add_note(events, ch, bar, 4.5, root, 0.3, 105)
    return events_to_track(events)


def compose_invincibility():
    tracks = [
        _inv_tempo_track(),
        _inv_drums(),
        _inv_bass(),
        _inv_lead(),
        _inv_brass(),
        _inv_marimba(),
        _inv_timpani(),
    ]
    return make_midi(tracks)


COMPOSITIONS = {
    #  name: (composer_fn, duration_seconds)
    "BattleDark": (compose_battle_dark, 20.0),
    "Invincibility": (compose_invincibility, 5.0),
}


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

def render_wav(midi_path, wav_path, duration=None):
    raw_wav = wav_path + ".raw.wav"

    cmd = [FLUIDSYNTH, "-ni", "-g", "0.6", "-T", "wav", "-O", "s16",
           "-F", raw_wav, "-r", "44100", SOUNDFONT, midi_path]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"  FluidSynth error: {result.stderr.strip()}")
        return False

    if os.path.exists(FFMPEG):
        af = "loudnorm"
        cmd_ff = [FFMPEG, "-y", "-i", raw_wav]
        if duration:
            cmd_ff += ["-t", str(duration)]
        cmd_ff += ["-af", af, wav_path]
        ff_result = subprocess.run(cmd_ff, capture_output=True, text=True)
        if ff_result.returncode == 0:
            os.remove(raw_wav)
        else:
            os.rename(raw_wav, wav_path)
    else:
        os.rename(raw_wav, wav_path)

    return True


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = sys.argv[1:]
    output_dir = args[0] if args else "."
    name_filter = args[1].split(",") if len(args) > 1 else None

    os.makedirs(output_dir, exist_ok=True)

    for name, (composer, duration) in COMPOSITIONS.items():
        if name_filter and name not in name_filter:
            continue

        midi_data = composer()
        mid_file = os.path.join(output_dir, f"MUS_{name}.mid")
        wav_file = os.path.join(output_dir, f"MUS_{name}.wav")

        with open(mid_file, "wb") as f:
            f.write(midi_data)
        print(f"  MIDI: {mid_file}")

        if os.path.exists(FLUIDSYNTH) and os.path.exists(SOUNDFONT):
            if render_wav(mid_file, wav_file, duration):
                print(f"  WAV:  {wav_file} ({duration}s)")
            else:
                print(f"  WAV render failed for {name}")
        else:
            print("  Skipping WAV render (FluidSynth/SoundFont not found)")

    print("Done.")


if __name__ == "__main__":
    main()
