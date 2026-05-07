# Tekken 3 — Gameplay Systems Spec

PlayStation (September 1998, NA) / Arcade (March 1997, Namco System 12). Developed by Namco. This spec targets the **PlayStation version**, which is the definitive release with additional characters and modes. Arcade differences are noted where relevant.

---

## 1. Core Gameplay Systems

### 1.1 Primary Loop

3D fighting game played on an infinite arena. Two fighters face off in best-of-three rounds; the first to deplete the opponent's health bar wins the round. Matches are won by taking 2 rounds (default). No ring-outs, no walls, no floor breaks — all stages are open infinite ground.

### 1.2 Four-Button Limb System

Each button maps to a limb:

| Button | Limb | PS1 Default |
|--------|------|-------------|
| **1** | Left Punch | Square |
| **2** | Right Punch | Triangle |
| **3** | Left Kick | X |
| **4** | Right Kick | Circle |

Simultaneous presses create compound inputs:

| Input | Action |
|-------|--------|
| 1+2 | Both punches (power attacks, some throws) |
| 3+4 | Both kicks (stance transitions, special moves) |
| 1+3 | Left throw |
| 2+4 | Right throw |
| 1+2+3+4 | Ki Charge / Supercharger |

### 1.3 Hit Levels

Every attack has a hit level that determines how it interacts with blocking:

| Level | Notation | Standing Guard | Crouch Guard | Notes |
|-------|----------|----------------|--------------|-------|
| **High** | h | Blocked | Ducked entirely | Whiffs over crouching opponents |
| **Mid** | m | Blocked | **Hits** | Forces standing guard |
| **Low** | l | **Hits** | Blocked | Must crouch-block |
| **Special Mid** | sm | Blocked | Blocked | Blockable either way; can be low-parried |

### 1.4 Health and Damage

- **Health per round**: 140 HP (VS/2P modes)
- **No chip damage** on normal blocked attacks — blocking is free
- **No damage scaling in juggles** — every hit in an aerial combo deals full listed damage, enabling extremely high-damage combos
- **Counter Hit (CH)**: Landing an attack during the opponent's startup/active frames. Deals ~1.2x damage; some moves gain launch or stun properties only on CH
- **Clean Hit**: Certain moves deal +50% damage when connecting at very close range. Stacks multiplicatively with CH (up to ~1.8x)

### 1.5 Ki Charge / Supercharger

Input: 1+2+3+4. Character charges for ~55 frames (0.9s), then enters a powered state for ~120 frames (2s).

Effects while charged:
- First attack deals 1.4–1.5x damage
- First attack gains counter-hit properties
- Blocked attacks inflict guard damage (10% of normal attack damage)
- **Cannot block** while charged
- Opponent also gains 1.4–1.5x on their first attack against you

### 1.6 Round Structure

| Setting | Default | Options |
|---------|---------|---------|
| Rounds to win | 2 (best of 3) | 1, 2, or 3 rounds |
| Round timer | 40 seconds | 20, 30, 40, 50, 60 |
| Difficulty | Medium | Easy, Medium, Hard, Very Hard, Ultra Hard |

**Time Over**: Player with more remaining HP wins. Equal HP = draw (round replayed or both lose, mode-dependent).

### 1.7 No Meter / No Rage

Tekken 3 has **no super meter**, **no Rage system** (introduced in Tekken 6), and **no install/burst mechanics**. The only powered-up state is the voluntary Ki Charge (§1.5).

---

## 2. Controls & Input

### 2.1 Input Notation (Standard Tekken)

**Directional inputs:**

| Notation | Meaning | Hold variant |
|----------|---------|-------------|
| f | Forward (tap) | F (hold) |
| b | Back (tap) | B (hold) |
| d | Down (tap) | D (hold) |
| u | Up (tap) | U (hold) |
| d/f | Down-forward | D/F (hold) |
| d/b | Down-back | D/B (hold) |
| u/f | Up-forward | U/F (hold) |
| u/b | Up-back | U/B (hold) |
| N | Neutral (no direction) | — |

**Combination symbols:**

| Symbol | Meaning | Example |
|--------|---------|---------|
| + | Simultaneous press | 1+2 = both punches |
| , | Sequential / in string | 1,2 = jab then cross |
| ~ | Immediately after (very fast) | d~d/f = near-simultaneous |
| : | Just-frame (exact timing) | — |

**State abbreviations:**

| Notation | Meaning |
|----------|---------|
| WS | While Standing (rising from crouch) |
| FC | Full Crouch (holding d or d/b) |
| WR | While Running (f,f,F sustained) |
| SS | Sidestep |
| BT | Back Turned |
| CH | Counter Hit |

**Common motion abbreviations:**

| Motion | Input |
|--------|-------|
| QCF | d, d/f, f |
| QCB | d, d/b, b |
| HCF | b, d/b, d, d/f, f |
| HCB | f, d/f, d, d/b, b |
| Crouch Dash | f, N, d, d/f |

### 2.2 Default PS1 Button Map

| PS1 Button | Function |
|------------|----------|
| Square | 1 (Left Punch) |
| Triangle | 2 (Right Punch) |
| X | 3 (Left Kick) |
| Circle | 4 (Right Kick) |
| D-Pad | Movement / blocking |
| L1/L2/R1/R2 | Unassigned (configurable) |
| Start | Pause |

Buttons are fully remappable via Options > Controller Configuration.

---

## 3. Movement System

### 3.1 Basic Movement

| Action | Input | Notes |
|--------|-------|-------|
| Walk forward | Hold f | — |
| Walk backward | Hold b | Also standing guard |
| Crouch | Hold d or d/b | d/b = crouch guard |
| Jump | Tap u | Neutral jump |
| Forward jump | Tap u/f | — |
| Backward jump | Tap u/b | — |
| Forward dash | f, f | Quick burst of forward movement |
| Back dash | b, b | Quick backward retreat |

### 3.2 Sidestep (New to Tekken 3)

Tekken 3 introduced lateral movement to the series. Tap **u** to sidestep into the background or **d** to sidestep into the foreground. Hold after tapping to **sidewalk** (sustained lateral movement).

Sidesteps evade linear attacks but are beaten by tracking/homing moves and attacks with wide horizontal arcs. The 3D axis is a discrete repositioning system, not free 8-way movement.

### 3.3 Advanced Movement Techniques

**Crouch Dash**: f, N, d, d/f — a forward-advancing crouch that gives access to specialized attacks. Core to Mishima-style characters (Jin, Heihachi) but also used by others.

**Wave Dash**: Looped crouch dashes — f, N, d, d/f, f, N, d, d/f... Creates rapid forward advancement while maintaining access to crouch dash mixups (mid launchers and low sweeps). Requires a character with a Mishima-style crouch dash.

**Korean Backdash Cancel (KBD)**: b, b, d/b, N, b, d/b, N, b... Creates rapid backward movement while maintaining the ability to block between dashes. Universal technique, though harder with characters that have back-sway moves.

---

## 4. Combat System

### 4.1 Blocking

| Guard Type | Input | Blocks | Vulnerable To |
|------------|-------|--------|--------------|
| Standing | Hold b | High, Mid | Low |
| Crouching | Hold d/b | Low, Special Mid | Mid |

No chip damage on blocked attacks (exception: Ki Charge guard damage at 10%, §1.5). No proximity guard or auto-guard.

### 4.2 Throws

| Throw Type | Input | Break With | Damage |
|------------|-------|------------|--------|
| Left throw | 1+3 | 1 | ~30–35 |
| Right throw | 2+4 | 2 | ~30–35 |
| Command throws | Character-specific | Varies (1, 2, or 1+2) | Varies |
| Side throw (left) | 1+3 or 2+4 from left flank | 1 | Higher |
| Side throw (right) | 1+3 or 2+4 from right flank | 2 | Higher |
| Back throw | 1+3 or 2+4 from behind | **Unbreakable** | Highest |

Visual cue: watch the attacker's lead arm — left arm forward = break with 1, right arm forward = break with 2, both arms = break with 1+2.

**Chain throws** (King, Nina): Multi-part throws that branch into further throws. Each transition in the chain has its own break input. King has the most complex chain throw system of any character in the game, including the Rolling Death Cradle sequence.

### 4.3 Reversals and Chickening

**Reversals**: Character-specific defensive moves that catch incoming attacks and counter them. Input is typically b+1+3 (catches left-limb attacks) or b+2+4 (catches right-limb attacks).

Characters with reversals:

| Character | Reverses |
|-----------|----------|
| Jin Kazama | High/mid punches and kicks |
| Nina Williams | High/mid punches and kicks |
| Paul Phoenix | High/mid punches and kicks |
| Anna Williams | High/mid punches and kicks |
| Forest Law | High/mid punches and kicks |
| King II | Kicks only (high/mid) |

**Chickening** (new to Tekken 3): Escapes an opponent's reversal. Input: f+1+3 to chicken a left-limb reversal, f+2+4 for a right-limb reversal. Must be buffered during the attack that gets reversed. King's kick reversal **cannot be chickened**.

### 4.4 Low Parry

Input: d/f timed to an incoming low or special-mid attack. Deflects the attack and grants ~26 frames of advantage — enough for a strong punish or juggle starter. In Tekken 3, low parry availability is character-dependent (it became fully universal starting in Tekken Tag Tournament).

### 4.5 Juggle / Combo System

**Launchers** send the opponent airborne. All subsequent hits while airborne are unblockable. Tekken 3 has no Bound (Tekken 6), no Screw/Tailspin (Tekken 7), and no wall splats — juggles are purely aerial.

Combo structure:
1. **Launcher** — sends opponent airborne (hopkick, Wind God Fist, Twin Pistons, etc.)
2. **Filler** — juggle hits that connect while airborne
3. **Ender** — final hit for damage or positional advantage

Natural gravity pulls the opponent down faster with each successive hit, organically limiting combo length. **No negative damage scaling** — every hit deals full listed damage. This is a major difference from later Tekken entries and enables near-death combos.

### 4.6 Stun Types

| Stun | Description | Escapable? |
|------|-------------|------------|
| **Double-Over Stun** | Opponent grabs stomach, slowly falls | Some are inescapable on CH |
| **Crumple Stun** | Fast collapse to the ground | Guarantees follow-up |
| **Drop-Knee Stun** | Opponent drops to one knee | Guarantees follow-up |
| **Counter Hit Stun** | Move-specific stun on CH only | Varies by move |

### 4.7 Ground Game / Okizeme

When knocked down, the defender has recovery options:

| Option | Input | Notes |
|--------|-------|-------|
| Tech roll (background) | 1 on ground contact | Cannot tech if hit from the side or face-down |
| Tech roll (foreground) | 2 on ground contact | Same restriction |
| Back roll | Hold b while airborne | Rolls backward |
| Getup mid kick | 4 while grounded | Mid hit level |
| Getup low kick | 3 while grounded | Low hit level |
| Quick getup kick | d+3 while grounded | Faster, shorter range |
| Spring kick | 3+4 while grounded | Not all characters |
| Stay down | Hold d | Remain grounded |
| Side roll | 1 or 2 while grounded | Roll laterally |

Some moves can hit grounded opponents (OTG / ground hits), creating okizeme pressure.

### 4.8 Frame Data Conventions

The game runs at **60 FPS**. All frame data is measured in 60ths of a second.

| Concept | Typical Values |
|---------|---------------|
| Fastest jab (1) | i10 (10-frame startup) |
| Generic throw | i12–14 |
| Standard launcher (hopkick) | ~i15 |
| Punishable on block | -10 or worse |
| Launch-punishable on block | -15 or worse |
| Positive on block | Attacker recovers first |

Detailed per-move frame data for Tekken 3 specifically is scarce — most community frame data resources cover Tekken 5 onward. The conventions above are consistent with the series from Tekken 3 forward.

---

## 5. Playable Characters

### 5.1 Full Roster (23 Characters)

#### Default Characters (10)

| Character | Fighting Style | Origin | Archetype | New/Returning |
|-----------|---------------|--------|-----------|---------------|
| **Jin Kazama** | Mishima-Style Karate + Kazama Self-Defense | Japan | Mishima rushdown / well-rounded | New (replaces Kazuya) |
| **Ling Xiaoyu** | Baguazhang + Piguaquan | China | Evasive / mixup | New |
| **Hwoarang** | Taekwondo | South Korea | Stance-based rushdown | New (replaces Baek) |
| **Eddy Gordo** | Capoeira | Brazil | Evasive / mixup | New |
| **Forest Law** | Jeet Kune Do | USA | Rushdown / well-rounded | New (replaces Marshall Law) |
| **Paul Phoenix** | Judo-based hybrid | USA | Power / explosive | Returning |
| **Nina Williams** | Assassination Arts (Aikido + Koppojutsu) | Ireland | Rushdown / pressure | Returning |
| **King II** | Pro Wrestling | Mexico | Grappler / mixup | New (replaces King I) |
| **Yoshimitsu** | Manji Ninjutsu | Japan | Trickster / unorthodox | Returning |
| **Lei Wulong** | Five-Form Kung Fu + Drunken Fist | China | Stance-heavy / trickster | Returning |

#### Unlockable Characters (13)

| Character | Fighting Style | Origin | Unlock Condition |
|-----------|---------------|--------|-----------------|
| **Kuma II** | Kuma Shinken (Bear Fighting) | Japan | Beat Arcade 1x |
| **Julia Chang** | Xinyi Liuhe Quan + Baji Quan | USA | Beat Arcade 2x |
| **Gun Jack** | Brute Force | N/A (Robot) | Beat Arcade 3x |
| **Mokujin** | Mimicry (random each round) | N/A (Wooden dummy) | Beat Arcade 4x |
| **Anna Williams** | Assassination Arts (Aikido emphasis) | Ireland | Beat Arcade 5x |
| **Bryan Fury** | Kickboxing | USA | Beat Arcade 6x |
| **Heihachi Mishima** | Mishima-Style Fighting Karate | Japan | Beat Arcade 7x |
| **Ogre** | Composite (absorbed techniques) | N/A (Ancient entity) | Beat Arcade 8x |
| **True Ogre** | Composite + monstrous abilities | N/A (Ogre transformed) | Beat Arcade 9x |
| **Tiger Jackson** | Capoeira (identical to Eddy) | N/A (Eddy alt) | Beat Arcade 16x, then select Eddy + press Start |
| **Panda** | Kuma Shinken (identical to Kuma) | N/A (Kuma alt) | Highlight Kuma, press Circle |
| **Dr. Bosconovitch** | Panic Fighting (borrowed moves) | Russia | Complete Tekken Force 4x, defeat him |
| **Gon** | Original (unique moveset) | N/A (Licensed manga guest) | Defeat Gon in Tekken Ball, or enter "GON" as Survival high score initials |

Each Arcade Mode completion must be with a **different character** to count toward the unlock progression.

### 5.2 Character Details

#### Jin Kazama
Protagonist. Combines Mishima crouch dash offense (f,N,d,d/f) with Kazama-style mid-range tools.

| Move | Input | Properties |
|------|-------|-----------|
| Wind God Fist | f,N,d,d/f+2 | Launcher, hits high |
| Electric Wind God Fist (EWGF) | f,N,d~d/f+2 (just-frame) | Faster recovery, safe on block |
| Thunder God Fist | f,N,d,D/F+1 | Mid uppercut launcher |
| Hell Sweep | f,N,d,d/f+4 | Low from crouch dash |
| Twin Pistons | d/f+1,2 | Mid-mid juggle starter |
| Flash Punch Combo | 1,1,2 | Fast high string |
| White Heron | 1+4,2,4 | Multi-hit string |

**Unique mechanics**: Crouch dash access; EWGF requires just-frame input (d/f and 2 pressed on the same frame). First character in the series to have the EWGF.

#### Ling Xiaoyu
Extremely evasive stance-based fighter with a small hitbox.

| Move | Input | Properties |
|------|-------|-----------|
| Art of Phoenix (AOP) | d+1+2 | Very low stance, ducks highs and some mids |
| Rain Dance (RDS) | b+3+4 | Back-turned stance with full forward mobility |
| Storming Flower | RDS 2,1 | Power attack from back-turn |
| California Roll | u/f+3+4 | Evasive rolling attack |
| Raccoon Swing | f+1+2 | Mid |
| Bayonet | d/f+1 | Key mid poke |
| Mistrust | d+2,1 | Low-mid |

**Unique mechanics**: Art of Phoenix is a crouching stance that evades many attacks. Rain Dance is a unique back-turned state with forward movement and sidestep access.

#### Hwoarang
Relentless stance-based kick pressure with four distinct stances.

| Stance | Abbreviation | Access |
|--------|-------------|--------|
| Left Foot Forward | LFF | Default stance |
| Right Foot Forward | RFF | Via stance transition moves |
| Left Flamingo | LF | Via specific moves from LFF |
| Right Flamingo | RF | Via specific moves from RFF |

| Move | Input | Properties |
|------|-------|-----------|
| Hunting Hawk | u/f+3,4,3 | Jumping triple kick |
| Dynamite Heel | d/b+4 | Low |
| Sky Rocket | f,N,d,d/f+4 | Launcher |
| Machine Gun Kicks | LF 3,3,3 | Rapid kicks from Left Flamingo |
| Backlash | b+3 | Spinning kick |

**Unique mechanics**: Moves transition between stances, creating flowing offense. The player must track which stance each move leaves them in.

#### Eddy Gordo
Acrobatic, evasive capoeira fighter known for being effective even with simple inputs.

| Move | Input | Properties |
|------|-------|-----------|
| Handstand | Via various moves | Inverted stance with unique attacks |
| Negativa | From Handstand (idle) | Low stance, evades highs |
| Satellite Moon | 3~4 | Launcher |
| Slippery Kicks | d/b+3,3 | Low-low from relaxed |

**Unique mechanics**: Constant stance shifting between Handstand and Negativa. Attacks naturally evade due to acrobatic animations. Tiger Jackson shares this complete moveset.

#### Forest Law
Fast, aggressive Jeet Kune Do fighter with strong strings and dragon moves.

| Move | Input | Properties |
|------|-------|-----------|
| Junkyard Kick | b+2,3,4 | Mid-low-mid — iconic string |
| Dragon's Tail | d/b+4 | Fast low sweep |
| Somersault | u/f+4 | Launcher |
| Frogman | d/b+3 | Sliding low |
| Dragon Storm | 1,2,3 | Fast punches into kick |
| Triple Fang | d+1,3,3 | Low-mid-mid |

#### Paul Phoenix
Explosive power character built around high single-hit damage.

| Move | Input | Properties |
|------|-------|-----------|
| Phoenix Smasher (Death Fist) | QCF+2 | Devastating mid; enormous CH damage |
| Demolition Man | d+4,2,1+2 | Low-mid string |
| Tile Splitter to Death Fist | d+1,2 | Low-mid |
| Falling Leaf | d/f+2 | Mid launcher |
| Flash Elbow | f+1+2 | Power mid |
| Burning Fist | d+1+2 | Unblockable (slow) |

#### Nina Williams
Fast, aggressive fighter with chain throws and frame traps.

| Move | Input | Properties |
|------|-------|-----------|
| Blonde Bomb | QCF+2 | Mid punch |
| Divine Cannon | u/f+4,3,4 | Hopkick string |
| Bad Habit | d/b+3+4 | Low |
| Ivory Cutter | d/f+3,2 | Mid string |
| Chain Throws | Multiple starters | Multi-part grab sequences |

**Unique mechanics**: Multi-part chain throws similar to King but with fewer branches. Reversal capability for punches and kicks.

#### King II
Grappler defined by the most complex throw system in fighting games of the era.

| Move | Input | Properties |
|------|-------|-----------|
| Jaguar Driver | QCF+1+2 | Chain throw starter, branches into multiple follow-ups |
| Shining Wizard | f,f,f+2+4 (vs crouching) | Iconic running knee throw |
| Giant Swing | f,HCF+1 | Powerful throw |
| Tombstone Piledriver | d/b,f+2+4 | High-damage throw |
| Achilles Hold | d/b+1+3 or d/b+2+4 | Leg grab → STF / Scorpion Deathlock |
| Rolling Death Cradle | Multi-part chain | One of the longest chain sequences |
| Ali Kicks | 3,3,3 | Mid kick string |

**Unique mechanics**: Extensive chain throw trees where each transition has its own break input. Kick-only reversal that cannot be chickened.

#### Yoshimitsu
Unorthodox trickster with self-damage mechanics, healing, and teleportation.

| Move | Input | Properties |
|------|-------|-----------|
| Meditation / Indian Sit | d+3+4 | Heals +5 HP per bounce; can teleport f/b |
| Flash | 1+4 | Parry/counter |
| Harakiri | d/b+1+4 | Stabs self; damages opponent behind |
| Spinning Sword Slice | b+1,1 | Double slash |
| Shark Attack | d/f+3,1 | Knee into uppercut |
| Sword Spin | u+1+2 | Unblockable helicopter |
| Bad Breath | d+1+2 | Poison cloud |

**Unique mechanics**: Only character with a weapon (sword). Can heal via Meditation. Suicide attacks damage both fighters. Teleportation from Sit stance.

#### Lei Wulong
The most stance-rich character with 5 animal forms plus drunken and lying positions.

| Animal Stance | Access | Role |
|---------------|--------|------|
| Snake | Via Rush Punches | Low profile, hopping attacks |
| Dragon | Via Rush Punches / transitions | Includes health-restoring throw |
| Tiger | Via SSL from Snake | Overhead stuns |
| Panther | Via SSR from Snake | Low attacks |
| Crane | Via SSR from Snake | Combos and punches |

| Move | Input | Properties |
|------|-------|-----------|
| Rush Punches | 1,2,1,2,1 | Five-hit string; each punch can transition to an animal stance |
| Play Dead | d+3+4 | Lie on ground; access ground attacks |
| Razor Rush | 4,3 | Kick combination |

**Unique mechanics**: Five animal stances plus Drunken Walk and Play Dead create massive option diversity. Transitions flow: Snake → Dragon → Tiger (via SSL); Snake → Panther → Crane (via SSR).

#### Heihachi Mishima
Archetypal Mishima with straightforward power offense.

| Move | Input | Properties |
|------|-------|-----------|
| Wind God Fist | f,N,d,d/f+2 | Mid launcher (unlike Jin's high WGF) |
| Thunder God Fist | f,N,d,D/F+1 | Uppercut launcher |
| Hell Sweep | f,N,d,d/f+4 | Low from crouch dash |
| Twin Pistons | d/f+1,2 | Mid juggle starter |
| Iron Hand | b+1 | Counter-hit tool |
| Flash Punch Combo | 1,1,2 | Fast high string |

#### Bryan Fury
Counter-hit specialist with strong punishment and combo damage.

| Move | Input | Properties |
|------|-------|-----------|
| Mach Punch | f,f+2 | Fast mid |
| Snake Edge | d/f+3 | Slow low launcher |
| Mach Kick | f,f+4 | Lunging mid |
| Fisherman's Slam | QCB+2+4 | Powerful throw |
| Gatling Combo | 3,2,1,4 | Mid-mid-mid-low string |
| Orbital Heel | u/f+4 | Safe mid launcher |

#### Julia Chang
Fast poking fighter with annoying pressure and mixups.

| Move | Input | Properties |
|------|-------|-----------|
| Party Crasher | f,f+1 | Lunging mid |
| Wind Roll | f,f+3 | Advancing kick |
| Bow and Arrow | QCF+1 | Rushing palm |
| Mad Axes | d/f+2,1 | Mid string |
| Mountain Crusher | d/f+1,1 | Double mid |

#### Kuma II / Panda
Slow power characters with deceptive range. Panda is a cosmetic alternate with the identical moveset.

| Move | Input | Properties |
|------|-------|-----------|
| Megaton Claw | f+1+2 | Power mid |
| Rolling Bear | f+1+2+3+4 | Forward roll |
| Bear Sit | d+3+4 | Sit on opponent |
| Grizzly Claw | d/f+2 | Launcher |

#### Gun Jack
Extremely slow robot with devastating single-hit damage and strong throws.

| Move | Input | Properties |
|------|-------|-----------|
| Megaton Punch | d/f+2 | Heavy mid |
| Machine Gun Punches | 1,1,1,1,1,2 | Rapid punches |
| Gigaton Punch | Hold b, then f+1 | Charge unblockable |
| Pancake Press | d+1+2 (vs grounded) | Ground slam |

#### Mokujin
2,000-year-old wooden training dummy that copies a random character's complete moveset each round. The assigned style is indicated by Mokujin's stance at round start.

#### Anna Williams
Similar to Nina with Aikido emphasis. In the arcade version she was a Nina palette swap; the PS1 version gives her unique attacks, voice, and ending.

#### Ogre (Ancient Ogre)
Composite moveset absorbed from defeated martial artists across the Tekken universe: Armor King, Wang Jinrei, Bruce Irvin, Jun Kazama, Lee Chaolan, Anna Williams, Kunimitsu, and Baek Doo San. Only three truly unique moves.

#### True Ogre
Transformed beast form of Ogre. Retains all of Ancient Ogre's absorbed moves and adds monstrous unique attacks:

| Move | Input | Properties |
|------|-------|-----------|
| Hell's Flame | 1+2 | Fire breath |
| Blazing Inferno | d+1+2 | Angled fire breath |
| Owl's Hunt | 3+4 (from lying) | Teleport body slam |
| Buffalo Horn | d/f+1+2 | Launching horn thrust |
| Tail attacks | d/f+3+4, d+3+4 | Mid/low tail spins |

#### Dr. Bosconovitch (PS1 Exclusive)
Joke/gimmick character who falls down after most moves. Borrows attacks from Lei, Heihachi, Yoshimitsu, Jack, Xiaoyu, and others. His constant falling makes him extremely difficult to use competitively but uniquely entertaining.

#### Gon (PS1 Exclusive)
Licensed guest character from Masashi Tanaka's manga. Tiny dinosaur with an extremely small hitbox — most high and many mid attacks whiff entirely. Has fire breath and fart attacks. Cannot be thrown normally due to size. Will never return in future Tekken games due to licensing.

### 5.3 Competitive Tier List (Community Consensus)

Tekken 3 predates formalized FGC tier lists. The following is a best-effort reconstruction from archived community rankings. Placement varies by region (Korean and Japanese metas differed).

| Tier | Characters |
|------|-----------|
| **S** | Jin Kazama, Ling Xiaoyu |
| **A** | Nina Williams, Hwoarang, Paul Phoenix, Heihachi Mishima, Bryan Fury |
| **B** | Forest Law, King II, Lei Wulong, Julia Chang, Eddy Gordo, Yoshimitsu |
| **C** | Ogre, Gun Jack, Kuma/Panda, Anna Williams, Mokujin |
| **Non-competitive** | Dr. Bosconovitch, Gon, True Ogre (often banned) |

### 5.4 Character Lineage

Tekken 3 is set 19 years after Tekken 2. Only 6 characters returned directly; most were replaced by younger successors.

**Returning** (6): Paul, Nina, Lei, Yoshimitsu, Heihachi, Anna

**Successor replacements** (8):

| New | Replaces | Relationship |
|-----|----------|-------------|
| Jin Kazama | Kazuya Mishima | Son (Kazuya killed by Heihachi) |
| Forest Law | Marshall Law | Son (Marshall stayed home) |
| King II | King I | Orphan raised by King I (killed by Ogre) |
| Kuma II | Kuma I | Son (Kuma I died of old age) |
| Julia Chang | Michelle Chang | Adopted daughter |
| Hwoarang | Baek Doo San | Student (Baek attacked by Ogre) |
| Bryan Fury | Bruce Irvin | Same archetype, no story link |
| Gun Jack | Jack-2 | Next-gen robot, built by Jane |

**Entirely new** (no predecessor): Eddy Gordo, Ling Xiaoyu, Ogre/True Ogre, Mokujin

Many Tekken 2 characters who did not return had their moves absorbed into Ogre's composite moveset (Jun Kazama, Lee Chaolan, Wang Jinrei, Kunimitsu, Armor King, Baek Doo San, Bruce Irvin).

---

## 6. World Structure

### 6.1 Stage List (15 Stages, PS1)

All stages are **infinite arenas** — no walls, no floor breaks, no ring-outs. The Skyring stage has cosmetic ropes but no boundary mechanics.

| Stage | Associated Character(s) | Setting |
|-------|------------------------|---------|
| Tiger Dojo, Tokyo | Jin Kazama | Indoor Japanese dojo |
| Taekwondo Dojo | Hwoarang | Korean dojo |
| Punk Alley | Paul Phoenix, Bryan Fury | Urban US alley |
| Hong Kong Street | Lei Wulong | Chinese street |
| Martial Arts Dojo | Forest Law, Kuma/Panda | Chinese martial arts dojo |
| Carnival | Ling Xiaoyu | Amusement park (features Namco Wonder Eggs branding) |
| Skyring | King II | Elevated ring with sky visible below |
| Grassy Land | Eddy Gordo, Tiger Jackson | Brazilian outdoor field |
| Laboratory Courtyard | Nina, Anna, Gun Jack | Outdoor laboratory |
| Forest | Yoshimitsu, Mokujin | Serene Japanese forest |
| Mexican Temple | Heihachi, Julia Chang | Hall outside Ogre's temple ruins |
| Ogre's Temple | Ogre, True Ogre | Interior of Aztec-style temple (final boss stage) |
| Mishima Polytechnical School | Jin / Xiaoyu (alternate) | Triggers when opponent is Jin or Xiaoyu in school uniform (3rd costume) |
| Junky Mansion | Dr. Bosconovitch | Doctor's laboratory (PS1 exclusive) |
| Beach Island | Gon | Beachside (PS1 exclusive) |

### 6.2 Arcade Mode Progression

10 stages:
- **Stages 1–8**: Random opponents from the roster
- **Stage 9**: Heihachi Mishima (sub-boss). Exception: playing as Heihachi → sub-boss is Jin
- **Stage 10**: Ogre (round 1) → transforms into True Ogre (round 2). Both forms are always fought regardless of round settings

---

## 7. Game Modes

### 7.1 Arcade Mode

Standard single-player tournament (§6.2). Completing with each character unlocks their FMV ending and progresses toward new character unlocks.

### 7.2 VS Mode

Local two-player competitive fighting. Uses shared game options for difficulty, timer, and round count. Stage is randomly assigned.

### 7.3 Team Battle

Each player selects a team of 1–8 fighters. Fighters battle sequentially; the loser is eliminated, the winner continues with a small health recovery before the next fight. Last team standing wins. Available as 1P vs CPU or 2P vs 2P.

### 7.4 Time Attack

Same structure as Arcade Mode with locked settings: **2 rounds, 40-second timer, Medium difficulty**. Cannot change characters mid-run. Cannot pause. Records total completion time per character.

### 7.5 Survival Mode

Single character against endless CPU opponents. Each battle is one round. A small amount of health recovers after each victory — more time remaining on the clock yields slightly more recovery. Game ends when health hits zero. Settings are locked. Tracks number of opponents defeated.

### 7.6 Practice Mode

Free training with no time limit or health depletion. Features:
- Set CPU behavior (fight normally at selected difficulty, or repeat specific attacks)
- Set opponent recovery behavior: crouch, guard, roll, or ukemi on knockdown
- Second controller can control the training dummy
- Move list display

### 7.7 Tekken Force Mode

Side-scrolling beat-em-up minigame. Available from the start (no unlock required). PS1 exclusive.

**4 stages**, each ending with a boss fight:

| Stage | Name | Setting | Boss HP |
|-------|------|---------|---------|
| 1 | Backstreets | Urban city street | 130 |
| 2 | Badlands | Desert/rocky terrain | 150 |
| 3 | In the Dark | Underground laboratory | 170 |
| 4 | Mishima Fortress | Japanese castle | 250 (always Heihachi) |

**Enemies** (Tekken Force soldiers, named by rank):

| Rank | HP | Time Awarded on Defeat |
|------|-----|----------------------|
| Crow | 15 | +2 seconds |
| Falcon | 20 | +2 seconds |
| Hawk | 25–40 | +4–8 seconds |
| Owl | 30–40 | +6–8 seconds |

- Player starts with **130 HP**
- **Chicken** items restore 50 HP each (dropped by enemies with 40 HP or scattered in stages)
- A global timer runs throughout — defeating enemies adds time; running out of time ends the game
- Stages 1–3 boss identity depends on the selected character; Stage 4 is always Heihachi

**Completion rewards**:

| Run | Reward |
|-----|--------|
| 1st | Bronze Key |
| 2nd | Silver Key |
| 3rd | Gold Key |
| 4th | Dr. Bosconovitch appears as a secret boss — defeat to unlock |

### 7.8 Tekken Ball Mode

Volleyball-like minigame on Beach Island. Unlocked after beating Arcade Mode with all 10 default characters (or after ~550 total matches, or when True Ogre is unlocked — sources vary).

**Rules**:
- Two characters face off across a beach court divided by a center line
- Players attack the ball to send it to the opponent's side
- Hitting the ball with attacks **charges** it with damage based on the attack's power
- Ball hitting the opponent or landing on their court side deals the charged damage
- At maximum charge, the ball catches fire for massive damage
- All damage is **non-recoverable**
- Direct hits or throws on the opponent deal **no damage** — only the ball deals damage
- Unblockable moves add a "!" mark instead of damage charge
- Win by depleting the opponent's health bar

**Ball types**:

| Ball | Difficulty Label | Damage Transfer |
|------|-----------------|----------------|
| Beach Ball | Beginner | 60% of attack damage |
| Gum Ball | Expert | 80% of attack damage |
| Iron Ball | Grand Master | 100% of attack damage |

### 7.9 Theater Mode

View character FMV endings, CG movies, and listen to the soundtrack. Progressively populated as characters complete Arcade Mode. After all endings are obtained, two additional options unlock:
- **MUSIC**: BGM player with "Arrange" and "Arcade" toggle
- **DISC**: Insert Tekken 1 or Tekken 2 disc to view those games' cinematics and music

---

## 8. UI & HUD

### 8.1 Fight HUD

| Element | Position | Description |
|---------|----------|-------------|
| P1 Health Bar | Top-left | Horizontal bar depleting left-to-right |
| P2 Health Bar | Top-right | Horizontal bar depleting right-to-left |
| P1 Character Name | Below P1 health bar | — |
| P2 Character Name | Below P2 health bar | — |
| Round Timer | Top-center | Countdown from 40 (or set value) |
| Round Indicators | Below health bars | Dots/icons showing rounds won |

### 8.2 HUD States

- **Pre-round**: Character names, "Round X" announcement, "FIGHT!" prompt
- **During round**: Health bars, timer, round indicators
- **Round end**: "K.O." overlay or "Time" overlay. Winner announcement. Victory pose replay
- **Match end**: "YOU WIN" or "YOU LOSE". Winner's ending trigger (Arcade Mode) or return to character select (VS)
- **Continue screen**: Defeated character shown; countdown from 9 to 0 with announcer voiceover

### 8.3 Character Select Screen

Grid layout of character portraits. Unlocked characters appear as their portrait; locked slots are absent or silhouetted. Timer for selection in Arcade Mode. Costume selection via button press (each character has 2+ costumes).

---

## 9. Presentation Systems

### 9.1 Save System

Memory Card save on PS1. Saves:
- Unlocked characters
- Unlocked modes (Tekken Ball, Theater)
- Theater Mode endings and music
- Time Attack records
- Survival records
- Tekken Force progress (keys)
- Options settings

### 9.2 Camera Behavior

Fixed cinematic camera that tracks both fighters. Camera pans and zooms to maintain both characters in frame. No player camera control during fights. Replays use dynamic camera angles.

### 9.3 FMV Endings

Every character has a unique pre-rendered CG ending played upon completing Arcade Mode. Notable exceptions: Tiger Jackson has his own ending despite sharing Eddy's moveset; Dr. Bosconovitch shares an ending with Yoshimitsu; True Ogre shares with Ogre.

### 9.4 Victory Animations

Each character has 2+ victory poses after winning. Some are triggered by holding specific buttons during the victory replay:
- Anna has alternate butt-shaking pose (hold Circle)
- Yoshimitsu has secret poses including one where he chants the Heart Sutra
- Xiaoyu's handstand pose is disabled when wearing her school uniform

---

## 10. Arcade vs. PlayStation Differences

| Feature | Arcade (System 12) | PlayStation |
|---------|-------------------|-------------|
| Characters | ~21 (Anna = Nina palette swap) | 23 unique + 2 alts (Anna fully separate, Gon and Dr. B added) |
| Backgrounds | Full 3D environments | 2D panoramic images (hardware limitation) |
| Modes | Arcade + VS only | Arcade, VS, Team Battle, Time Attack, Survival, Practice, Tekken Force, Tekken Ball, Theater |
| Music | Original arcade soundtrack | Re-arranged + new tracks |
| Sound FX | Normal pitch | Slightly higher pitch |
| Character detail | Higher polygon count | Slightly reduced |
| Resolution | Higher | Lower |

---

## 11. Open Questions / Unverified

1. **Exact HP in Arcade mode single-player**: Multiple sources confirm 140 HP for VS mode. The arcade single-player value may differ but no source explicitly confirms a different number.

2. **Juggle scaling**: Consensus is no negative scaling (full damage per hit), but some community members debate whether hidden scaling exists. Formal scaling was definitively introduced in Tekken 6/TTT2.

3. **Counter hit multiplier**: The exact multiplier in Tekken 3 is not precisely documented. 1.2x is the standard from later games; the Ki Charge 1.4–1.5x applies only to that powered state.

4. **Throw break window**: The ~20-frame window comes from Tekken 7/8 research. Tekken 3's exact window is not publicly documented but the mechanic functions identically.

5. **Low parry universality**: Sources conflict on whether d/f low parry was universal in Tekken 3 or character-specific. Evidence leans toward character-specific, becoming universal in TTT.

6. **Per-move frame data**: Detailed Tekken 3 frame data is scarce online. Most community resources cover Tekken 5 onward. General conventions (i10 jabs, i15 launchers) appear consistent.

7. **Clean hit multiplier**: The +50% figure comes from wiki sources but exact testing data specific to Tekken 3 is limited.

8. **Tekken Ball unlock trigger**: Sources cite different conditions — completing Arcade with all 10 default characters, unlocking True Ogre, or reaching 550 total matches. These may not be mutually exclusive.

---

## 12. References

### Wikis
- [Tekken Wiki (Fandom)](https://tekken.fandom.com/wiki/Tekken_3) — character pages, move terminology, mechanics
- [Tekkenpedia](https://eng.tekkenpedia.com/) — Supercharger/Ki Charge, Tekken Ball mechanics
- [Wikipedia — Tekken 3](https://en.wikipedia.org/wiki/Tekken_3) — release info, version differences
- [StrategyWiki — Tekken 3](https://strategywiki.org/wiki/Tekken_3) — movelists

### FAQs & Guides
- [GameFAQs — Tekken 3 FAQ by tragic](https://gamefaqs.gamespot.com/arcade/563192-tekken-3/faqs/981)
- [GameFAQs — Tekken 3 Mini-FAQ by CMurdock](https://gamefaqs.gamespot.com/ps/198900-tekken-3/faqs/4257)
- [Arcade Quartermaster — Tekken 3](https://www.arcadequartermaster.com/tekken3_characters.html) — character/stage data

### Community / Technical
- [SuperCombo Wiki — Tekken 3](https://wiki.supercombo.gg/w/Tekken_3/) — character-specific data
- [infil.net Fighting Game Glossary](https://glossary.infil.net/) — Korean Backdash, movement terminology
- [SDTEKKEN Frame Data Guide](https://sdtekken.com/t5dr/frame-data-guide/) — frame data conventions (Tekken 5-era, consistent with T3)
- [IAMTEKKEN — Understanding Damage Scaling](https://iamtekken.wordpress.com/2009/12/25/understanding-damage-scaling/) — scaling mechanics history
- [1UP Infinite — History of Movement in Tekken](https://1upinfinite.org/tekken-movement-feature/) — sidestep, KBD origins
- [LearnTekken Basic Techniques](https://ampersanders.github.io/LearnTekken/basic_tech.html) — universal mechanics
