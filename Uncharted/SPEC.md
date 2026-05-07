# Uncharted: Drake's Fortune — Gameplay Systems Spec

PlayStation 3, 2007. Developed by Naughty Dog. Single-player third-person action-adventure.

---

## 1. Core Gameplay Systems

### 1.1 Primary Gameplay Loop

Drake's Fortune alternates between three core pillars in a linear chapter structure:

1. **Cover-based combat** — Third-person gunfights against waves of enemies using a snap-to-cover system, two-weapon loadout, grenades, and melee combos.
2. **Traversal / platforming** — Climbing, jumping, shimmying, rope-swinging, and vine-climbing across ruins, cliffs, and structures. Navigation is guided by environmental cues and companion dialogue rather than waypoints.
3. **Puzzles** — Environmental puzzles solved by referencing Drake's journal. Symbol alignment, statue rotation, and switch-order puzzles.

Vehicle sections (jet ski, jeep turret) break up the pacing at specific chapters.

### 1.2 Combat System

**Weapon loadout**: Drake carries exactly two weapons — one sidearm (pistol slot) and one long gun (rifle/shotgun slot) — plus up to 4 Mk-NDI grenades. Picking up a weapon in an occupied slot replaces the current weapon.

**Aiming modes**:
- **Aimed fire** (L1 hold + R1): Over-the-shoulder view with crosshair. Crosshair tightens with sustained aim.
- **Blind fire** (R1 without L1): Hip fire with reduced accuracy but no exposure. Works both in and out of cover.
- **Headshots**: Instant kill on most enemies with any weapon.

**Cover system**:
- Circle to snap to any solid environmental surface (walls, crates, pillars, vehicles). Plants and vegetation do not function as cover.
- **Pop-and-shoot**: L1 to lean out from cover, R1 to fire.
- **Blind fire from cover**: R1 without L1 — sprays over/around cover with no reticle.
- **Vault**: X to climb over low cover.
- **Roll/dodge**: Circle + direction away from cover.
- **Grenade throw from cover**: L2.
- **Cover degradation**: Sustained enemy fire chips away and destroys cover, forcing repositioning. Some cover (stone walls, thick pillars) is indestructible.
- **No cover-to-cover transition**: To move between cover points, Drake must exit cover, run to the new position, and re-enter cover. Dedicated cover-to-cover dashes were introduced in Uncharted 2.

**Melee combat**:

| Combo | Input | Hits | Effect |
|---|---|---|---|
| Basic Combo | Square ×5 | 5 | Standard damage |
| Brutal Combo | Square → Triangle (timed) → Square (timed) | 3 | High damage; enemy drops **2× ammo** on death |
| Steel Fist | Shoot to weaken → Square | 1 | One-hit melee kill on weakened enemy |
| Stealth Kill | Square from behind (undetected) | 1 | Silent instant kill |
| Contextual Melee | Square near surfaces | Varies | Drake pins enemies against walls, slams heads into surfaces |

Each Brutal Combo button press must be timed to land after the previous blow connects. Brutal Combos cannot kill Descendants (§7.3).

**Grenades**:
- Thrown with L2. Aiming arc adjusted by Sixaxis tilt (PS3) or analog stick (remaster).
- Red glow indicator on the ground shows landing position.
- Max carry: 4. Found at fixed environmental locations.
- Primary uses: flush enemies from cover, destroy destructible cover, area denial.
- Drake cannot throw back enemy grenades. Grenade throw-back was introduced in Uncharted 3.

**Enemy grenades**:
- Pirates and mercenaries throw grenades to flush Drake from cover, especially if he stays in one position too long.
- Warning system: enemy voice lines ("Incoming!") + red glow on the grenade itself. No HUD grenade indicator.
- Shooting an enemy mid-throw causes them to drop the grenade at their feet, damaging themselves and nearby allies.

**Hanging combat**:
- Drake can shoot while hanging from ledges, but only **sidearms** (one-handed weapons). Long guns cannot be used while hanging.
- Drake can aim and throw grenades while hanging.
- No melee from ledges — the pull-down mechanic was introduced in Uncharted 2.
- Two trophies track ledge kills: "Hangman" (20 gunfire kills) and "Grenade Hangman" (10 grenade kills).

### 1.3 Health System

No visible health bar. Damage is communicated through screen effects:

- **Color desaturation**: Screen progressively fades toward grayscale as Drake takes damage.
- **Red screen edges**: Blood-like splatter at screen edges indicates damage severity and direction of incoming fire.
- **Heartbeat audio**: Audible heartbeat at low health; sound becomes muffled.
- **Regeneration**: Health fully regenerates after several seconds without taking damage. Screen color returns to normal.

Naughty Dog's design fiction: the system represents Drake's "luck running out" — bullets are near-misses, and only the final hit actually connects.

### 1.4 Companion AI

Elena and Sully fight alongside Drake in specific chapters:

- Companions actively fire at enemies and can score kills.
- **Elena can die** under sustained fire (notably Chapter 13, the Sanctuary courtyard), resulting in **game over**. She is highly resilient but not invincible.
- During jet ski sections, the player directly controls Elena's aiming/firing while Drake drives.
- Companions provide contextual combat dialogue and naturally guide Drake toward objectives between fights.

### 1.5 Ammo Economy

Ammo is scarce by design. Primary sources:
- Fallen enemies drop ammo for their weapon type. Pickup requires pressing Triangle (not auto-collect).
- Brutal Combo kills drop **2× ammo**.
- Fixed ammo pickups in the environment.

No ammo purchases, crafting, or resupply stations.

---

## 2. Controls & Input

### 2.1 PS3 DualShock 3 / Sixaxis

**On foot**:

| Input | Action |
|---|---|
| Left Stick | Move |
| Right Stick | Camera / Look |
| X | Jump / Climb up |
| Circle | Take cover / Roll-dodge / Drop from ledge |
| Square | Melee / Interact |
| Triangle | Reload / Pick up weapon / Pick up ammo |
| L1 (hold) | Aim weapon |
| R1 | Fire weapon |
| L2 (hold) | Throw grenade (shows arc) |
| D-Pad Left | Select sidearm |
| D-Pad Right | Select long gun |
| D-Pad Down | Select grenades |
| L3 | Switch aiming shoulder |
| Select | Open Drake's journal |
| Start | Pause menu |
| Sixaxis tilt (up/down) | Adjust grenade arc height |
| Sixaxis tilt (left/right) | Balance on logs/narrow surfaces |

**Jet ski** (Chapters 8, 9, 12):

| Input | Action |
|---|---|
| Left Stick | Steer |
| R1 | Accelerate |
| L1 | Aim Elena's weapon |
| R1 (while aiming) | Fire Elena's weapon |

Cannot steer and aim simultaneously — must stop the jet ski to fire.

**Jeep turret** (Chapter 7):

| Input | Action |
|---|---|
| Left Stick / Right Stick | Aim turret |
| R1 | Fire machine gun |
| L1 | Fire grenade launcher |

Both fire modes have infinite ammo. Reticle shrinks with short bursts.

### 2.2 Sixaxis Motion Controls (PS3 Only)

- **Grenade arc**: Tilting the controller up/down adjusts throw trajectory height.
- **Log balancing**: Tilting left/right balances Drake on narrow surfaces. Mandatory — no analog alternative on PS3.

The PS4 Nathan Drake Collection removed mandatory Sixaxis controls entirely: log balancing remapped to analog sticks, grenade arc defaults to analog with Sixaxis optional.

---

## 3. World Structure

### 3.1 Chapter Layout

22 linear chapters with no backtracking or open-world exploration. Each chapter is a self-contained level segment.

| Ch. | Name | Primary Gameplay | Treasures |
|---|---|---|---|
| 1 | Ambushed | Tutorial combat on a boat | 0 |
| 2 | The Search for El Dorado | Jungle traversal, platforming, light combat | 6 |
| 3 | A Surprising Find | U-boat exploration, swimming through flooded compartments | 2 |
| 4 | Plane-wrecked | Jungle combat and platforming | 10 |
| 5 | The Fortress | Climbing through a colonial fortress, puzzles | 4 |
| 6 | Unlocking the Past | Puzzle-heavy: journal-based symbol/statue puzzles | 4 |
| 7 | Out of the Frying Pan | Jeep turret combat escape | 0 |
| 8 | The Drowned City | Jet ski through flooded ruins | 2 |
| 9 | To the Tower | Jet ski continuation + on-foot climbing | 1 |
| 10 | The Customs House | Combat-heavy with mixed platforming | 2 |
| 11 | Trapped | Combat arena with spike trap hazards | 3 |
| 12 | Heading Upriver | Jet ski upstream with barrel hazards | 0 |
| 13 | Sanctuary? | Monastery exploration, bell puzzle, symbol alignment | 7 |
| 14 | Going Underground | Underground climbing through dark passages | 6 |
| 15 | On the Trail of the Treasure | Symbol alignment puzzles, combat | 6 |
| 16 | The Treasure Vault | Discovery of the vault, platforming and combat | 3 |
| 17 | The Heart of the Vault | Descendants introduced; survival arena with Eddy Raja | 0 |
| 18 | The Bunker | WWII German bunker exploration, Descendant combat | 1 |
| 19 | Unwelcome Guests | Three-way fights: mercenaries vs. Descendants vs. Drake | 1 |
| 20 | Race to the Rescue | Chase sequence to rescue Elena | 1 |
| 21 | Gold and Bones | Heavy combat, mixed enemy types | 2 |
| 22 | Showdown | Final boss fight against Navarro on a cargo ship | 0 |

**Enemy faction progression**:
- Chapters 1–12: Pirates exclusively
- Chapters 13–17: Transition to mercenaries (pirate overlap in Ch. 17)
- Chapters 17–21: Descendants introduced; three-way battles
- Chapter 22: Mercenaries + Navarro boss

### 3.2 Navigation

No minimap, compass, or objective markers. Players navigate via:
- Environmental visual cues (climbable surfaces indicated by texture — moss, brick patterns, handholds)
- Companion dialogue that naturally guides toward objectives
- Drake's journal for puzzle hints
- Linear level design with clear forward paths

### 3.3 Environmental Hazards

| Hazard | Description |
|---|---|
| Explosive barrels | Red barrels that detonate on gunfire or contact; damage Drake and enemies |
| Spike traps | Trip-wire activated spike walls in ruins; dodge-roll to avoid; can kill enemies |
| Collapsing scaffolding | Wooden platforms that break under weight; timed traversal |
| Crumbling ledges | Handholds that deteriorate after several seconds; must keep moving |
| Falling | Long falls are fatal; no fall damage threshold — short falls safe, long falls kill |
| Water currents | Strong currents in river sections push the jet ski into obstacles |
| Floating explosive barrels | Red/white barrels on water; instant kill on jet ski contact |

---

## 4. Traversal & Platforming

### 4.1 Core Movement

- **Walk/run**: Left stick controls speed via analog deflection. Partial tilt walks, full tilt runs. **No sprint button** — there is no dedicated sprint or stamina system.
- **Climbing**: X to jump upward between handholds. Drake climbs walls, pillars, ruins, vines, and cliff faces. Climbable surfaces are visually distinguished by texture (moss, brick patterns, handholds) rather than explicit markers.
- **Ledge grab**: Automatic when jumping toward a climbable surface.
- **Shimmying**: Move left/right along ledges, windowsills, and handholds while hanging.
- **Rope/chain swinging**: Drake grabs ropes, chains, and hanging fixtures. Swing using left stick, jump off at arc's peak with X.
- **Vine climbing**: Functionally identical to wall climbing; vines act as vertical/horizontal handholds on jungle surfaces.
- **Drop down**: Circle to release from a ledge.
- **Rolling/dodge**: Circle + direction on ground performs a combat roll.
- **Log balancing**: Walking across narrow surfaces. Sixaxis tilt on PS3, analog stick on remaster.
- **Traversal prompts**: Early chapters display contextual "Press X" button prompts near climbable surfaces. These fade as the game assumes player familiarity.

### 4.2 Swimming

- **Surface swimming**: Left stick to direct; contextual speed.
- **Underwater swimming**: Limited breath mechanic. Drake dives and swims through flooded passages (e.g., U-boat interior, Chapter 3).
- **No underwater combat**.
- **Late-game water hazard**: In Chapters 17–21 (Descendant territory), falling into water is **instant death** — a Descendant drags Drake under in a scripted kill. Water transitions from safe traversal to kill zone as the game progresses.
- **Wet clothing**: Drake's clothes get wet realistically — only portions that contact water become drenched, and they gradually dry over time.

### 4.3 Vehicle Sections

**Jet ski** (Chapters 8, 9, 12): Drake drives, Elena rides as passenger/gunner. Elena uses M79 grenade launcher with infinite ammo (Chapters 8–9) or 92FS-9mm (Chapter 12). Steering and shooting are mutually exclusive — must stop to fire. No reverse capability.

**Jeep turret** (Chapter 7): Elena drives, Drake mans the rear-mounted AGS-17 turret. Machine gun (R1) and grenade launcher (L1), both infinite ammo. On-rails; player controls aiming/firing only.

---

## 5. Story & Progression

### 5.1 Structure

Linear 22-chapter story with no branching, no dialogue choices, and a single ending. Progression is strictly sequential — no chapter skipping on first playthrough.

**Chapter select** unlocks after completing the game, allowing replay of any individual chapter with current unlocks and difficulty selection.

### 5.2 Post-Game Content

- **Chapter select**: Replay any chapter.
- **Higher difficulties**: Crushing unlocked after completing Hard (PS3). All difficulties available from the start in the remaster.
- **Bonus rewards**: Medal points (PS3) or treasure/completion-based unlocks (remaster) open costumes, render modes, gameplay tweaks, and behind-the-scenes content (§9).
- **No New Game+**: No carry-over of weapons or stats between playthroughs. Treasure collection and medal progress persist across all saves.

---

## 6. Items & Equipment

### 6.1 Weapons

Drake carries one sidearm + one long gun. No upgrades, attachments, or customization. Weapons are swapped by pressing Triangle near a ground weapon — the old weapon is dropped in its place.

**Sidearms**:

| Weapon | Basis | Power | Speed | Magazine | Max Ammo | Notes |
|---|---|---|---|---|---|---|
| PM - 9mm | Makarov PM | 5/10 | 6/10 | 8 | 40 | Starting pistol. Low recoil, reliable but weak. |
| 92FS - 9mm | Beretta 92FS | 6/10 | 7/10 | 15 | 60 | Best all-around sidearm. 3–4 body shots or 1 headshot to kill. |
| Micro - 9mm | Micro Uzi | 6/10 | 10/10 | 20 | 60 | Full-auto machine pistol. Highest sidearm fire rate. |
| Wes - 44 | S&W Model 629 | 9/10 | 5/10 | 6 | 12 | Sully's revolver. One-shots most unarmored enemies. |
| Desert - 5 | Desert Eagle | 9/10 | 6/10 | 7 | 13 | Superior to Wes-44 in fire rate, accuracy, and reload. Rare ammo. |

**Long guns**:

| Weapon | Basis | Power | Speed | Magazine | Max Ammo | Notes |
|---|---|---|---|---|---|---|
| AK-47 | AK-47 | 7/10 | 8/10 | 30 | 90 | Standard pirate rifle. Accuracy degrades during sustained auto. |
| M4 | Colt Model 653 | 7/10 | 9/10 | 32 | 96 | Standard mercenary rifle. Superior accuracy and range to AK-47. |
| MP40 | MP40 | 7/10 | 8/10 | 35 | 105 | Found in bunker chapters. Practically no recoil. |
| Moss - 12 | Mossberg 500 | 7/10 | 3/10 | 6 | 18 | Pump-action. One-shot at close range, useless at distance. |
| SAS - 12 | SPAS-12 | 8/10 | 4/10 | 8 | 21 | Semi-auto shotgun. Superior to Moss-12 in every stat. |
| Dragon Sniper | Dragunov SVD | 10/10 | 3/10 | 5 | 10 | Scoped. One-shot kill on most enemies. Extremely rare ammo. |
| M79 | M79 | 10/10 | 1/10 | 1 | 3 | Grenade launcher. Near-flat trajectory. One-shots groups. Very slow reload. |

**Grenades**:

| Item | Basis | Max Carry | Notes |
|---|---|---|---|
| Mk-NDI | M67 frag | 4 | "NDI" = Naughty Dog Inc. Red glow shows landing spot. |

**Emplaced / vehicle weapons**:

| Weapon | Context | Notes |
|---|---|---|
| AGS-17 turret | Chapter 7 jeep | Machine gun + grenade launcher, infinite ammo |
| Elena's M79 | Jet ski (Ch. 8–9) | Infinite ammo, no reload |
| Elena's 92FS | Jet ski (Ch. 12) | Cover fire during river navigation |

Power/speed ratings are relative scales from community guides. The game does not expose internal damage values.

### 6.2 Collectibles

**61 treasures** across 22 chapters (60 standard + 1 Strange Relic):

- Treasures appear as small objects with a pulsating sparkle effect.
- Often hidden in off-path locations, behind objects, on ledges, or in dark corners.
- Pickup is saved immediately — persists even if Drake dies before the next checkpoint.
- A counter briefly appears on pickup.

**Strange Relic**: Found in Chapter 5. A Precursor Orb (Jak and Daxter Easter egg). Has its own trophy ("Relic Finder") but does not count toward Fortune Hunter treasure milestones.

---

## 7. Enemies & Opponents

### 7.1 Eddy Raja's Pirates (Chapters 1–17)

Ragtag fighters in civilian clothing with no body armor.

**Weapons used**: PM-9mm, Micro-9mm, AK-47, Moss-12, M79.

**Behavior**: Attack in large groups with less tactical awareness. Less likely to flank or coordinate suppression. Use speedboats and jeeps.

**Variants by weapon**:
- AK-47 riflemen (most common)
- Moss-12 shotgunners (close-range rushers)
- Micro-9mm users (aggressive, mobile)
- M79 grenadiers (rare, area denial; identifiable by bush hats and bright red shirts)
- PM-9mm pistol users (weakest)

### 7.2 Navarro's Mercenaries (Chapters 13–22)

Professional soldiers in dark military clothing with body armor. Better trained and equipped than pirates.

**Weapons used**: 92FS-9mm, Desert-5 (with laser sights), M4, SAS-12, Dragon Sniper, M79.

**Behavior**: Aggressive flanking from multiple angles. Use laser sights (visible red line — can be spotted and evaded). After Chapter 19, some wear helmets that protect against headshots.

**Variants by weapon**:
- M4 riflemen (standard, well-armored)
- Desert-5 snipers (laser sight, high damage, can one-shot on Crushing)
- SAS-12 shotgunners (aggressive pushers)
- Dragon Sniper snipers (extreme range)
- M79 grenadiers (area denial, one-shot potential)
- Helmeted variants (post-Ch. 19, require body shots or precise headshots)

### 7.3 Descendants (Chapters 17–21)

Mutated descendants of Spanish colonists infected by El Dorado's mutagenic plague. Feral, pale-skinned humanoids.

**Behavior**:
- **Melee only** — no firearms. Charge and claw at extreme speed.
- **Pack attacks** — swarm in groups from multiple directions.
- **Attack all factions** — hostile to Drake, pirates, and mercenaries, creating three-way battles.
- **Higher health** than standard human enemies.
- **Immune to Brutal Combos** — forces firearm use.
- **Best counters**: Shotguns (SAS-12), blind fire while backing away, headshots.

### 7.4 Three-Way Battles (Chapters 19–21)

Descendants, mercenaries, and Drake are all hostile to each other simultaneously. Tactical opportunity: let Descendants and mercenaries fight each other before engaging survivors.

### 7.5 Boss Encounters

**Eddy Raja — Chapter 17 "The Heart of the Vault"**:
Not a traditional boss fight. Drake and Eddy form a temporary alliance, fighting back-to-back against swarming Descendants in a survival arena. Eddy's death is scripted via cutscene (killed by a Descendant).

**Atoq Navarro — Chapter 22 "Showdown"**:
Takes place on a cargo ship.

| Phase | Description |
|---|---|
| 1 — Firefight | Drake uses his carried weapons against waves of mercenaries. Navarro suppresses with a modified SAS-12 (laser sight, semi-auto), firing 3–4 shots then reloading. |
| 2 — Chase | Drake pursues Navarro up stairs. Falling crates require dodge-rolls (Circle QTEs). During this phase, Drake **loses his weapons** when Navarro detonates explosive barrels. |
| 3 — Unarmed Advance | Drake must close the distance unarmed, timing movement between cover during Navarro's reload windows. Navarro's fire destroys cover sequentially. Tighter timing on higher difficulties. |
| 4 — QTE Melee | Close-range QTE sequence. Button prompts to disarm Navarro (matching Brutal Combo inputs). Final QTE triggers ending cutscene. |

No health bar is displayed for Navarro. Gabriel Roman is killed in a cutscene (not a player fight).

---

## 8. Puzzles

### 8.1 Drake's Journal

Accessed via Select (PS3) or Touchpad (remaster). Contains sketches, maps, and written clues from Sir Francis Drake. Required for solving nearly every puzzle. The game prompts the player to check the journal at puzzle locations.

### 8.2 Puzzle Types

**Symbol/switch order**: Four symbols correspond to four switches. The journal shows them numbered 1–4 indicating activation order.

**Statue rotation**: Statues on compass points (N/S/E/W) must be rotated to face directions shown in the journal diagram, referencing a compass painted on the floor.

**Symbol alignment**: Rotating symbols on large stone mechanisms to match patterns in the journal (e.g., shell, snake, and arrows alignment in Chapter 15).

**Bell puzzle**: Two bells must be rung simultaneously. Journal clue: "Two bells resound in perfect harmony."

**Water level**: Manipulating devices to flood rooms, raising water to reach inaccessible areas (Chapter 5).

**Observation**: Using environmental clues (murals, inscriptions) alongside journal entries to determine the correct action.

---

## 9. Rewards & Unlockables

### 9.1 Medal Points System (PS3 Original)

Medals are earned through gameplay accomplishments and award points spent on bonus content.

**Medal categories**:
- **Treasure medals**: First three give 5 pts each; subsequent give 10 pts. "Master Fortune Hunter" (60 treasures) awards 50 pts. Total from treasures: 175 pts.
- **Kill quota medals**: Kill thresholds with specific weapons (e.g., 50 Kills: AK-47).
- **Headshot medals**: Milestones at 20, 100, 250 headshots; "Headshot Expert" (5 in a row).
- **Brutal Combo medals**: "Brutal Brawler" (5 kills), "Brutal Slugger" (20 kills), "Brutal Expert" (5 in a row).
- **Steel Fist medals**: 5 kills, 10 in a row.
- **Master Ninja**: 50 stealth kills from behind.
- **Run-and-Gunner**: 20 blind fire kills.
- **Explosive medals**: "Triple Dyno-Might!" (3 enemies with 1 explosion), "Dyno-Might Master" (5 times).
- **Hangman medals**: Kill enemies while hanging (20 gunfire, 10 grenades).
- **Survivor**: Kill 75 enemies in a row without dying.

### 9.2 Unlockable Content

Accessed via Bonuses > Rewards in the pause menu. Costumes require one full completion.

**Costumes**:
- Naughty Dog T-Shirt Drake
- Baseball T-Shirt Drake (cheat code)
- Wetsuit Drake (with Ottsel logo — Jak and Daxter reference)
- Cartoon Drake (exaggerated proportions)
- Elena Fisher (Drake's model replaced; cutscenes unchanged)
- Victor Sullivan

**Render modes**:
- Black and White
- Sepia Tone
- Next-Gen Filter (oversaturated bloom — satirical)

**Gameplay tweaks**:
- Infinite Ammo
- One-Shot Kills
- Slow Motion / Fast Motion
- Mirror World (flips horizontally)
- Weapon Select (choose starting weapons per chapter)

**Bonus content**:
- Making-of videos (PS3 only — removed in remaster)
- Concept art galleries

### 9.3 PS4 Remaster Changes

Medal points system replaced. Rewards unlocked via treasure collection and difficulty completion. Added: Photo Mode, Speed Run Mode with chapter/full-game timers, and leaderboards.

---

## 10. UI & HUD

### 10.1 HUD Layout

Deliberately minimalist to preserve immersion:

- **Ammo counter**: Lower-right corner. Shows current magazine / total reserve. Fades when not in combat.
- **Grenade count**: Displayed alongside weapon info.
- **Weapon indicator**: Current equipped weapon icon.
- **No health bar**: Damage communicated through screen desaturation, red edge effects, and audio (§1.3).
- **No minimap or radar**.
- **No objective markers or waypoints**.

### 10.2 Reticle States

- **No persistent crosshair** when hip-firing or in cover.
- **Aimed mode** (L1 hold): Crosshair appears centered on screen.
- **Grenade arc**: Trajectory indicator when holding L2. Red glow on ground at landing point.
- **Sniper scope**: Dedicated scope view for Dragon Sniper.
- **Turret reticle**: Shrinks with short bursts for increased accuracy.

### 10.3 Damage Feedback

- **Screen desaturation**: Progressive grayscale as damage accumulates.
- **Red vignette**: Blood-like splatter at screen edges indicates severity and direction.
- **Audio**: Muffled sound, audible heartbeat at low health.
- **Recovery**: Color gradually returns as health regenerates.

### 10.4 Context UI

- **Treasure pickup notification**: Counter briefly appears on collection.
- **Journal prompt**: Game prompts player to check Drake's journal at puzzle locations.
- **QTE prompts**: Button icons appear during the Navarro boss fight (§7.5).
- **No tutorial popups** on Hard/Crushing difficulties. Shown on Easy/Normal.

---

## 11. Engine & Presentation Systems

### 11.1 Camera

- **Default**: Third-person behind and slightly above Drake. Dynamic adjustment during platforming and climbing.
- **Aimed mode** (L1): Tight over-the-shoulder view (right shoulder default).
- **Shoulder swap**: L3 toggles left/right shoulder aiming.
- **Cover mode**: Wider view of combat area with Drake pressed against surface.
- **Sniper scope**: Dedicated scope view for Dragon Sniper.
- **Cutscenes**: In-engine cinematics using the same character models as gameplay.

### 11.2 Dialogue & Narrative

- **Cutscenes**: Fully motion-captured (Nolan North as Drake, Emily Rose as Elena, Richard McGonagle as Sully). Non-interactive. Non-skippable on first playthrough (PS3).
- **In-gameplay banter**: Contextual dialogue during gameplay. Drake mutters before jumps, comments on environments, reacts to danger. Companions naturally guide toward objectives through conversation, doubling as a hint system.
- **No dialogue choices**: Entirely linear narrative.

### 11.3 Chapter Transitions

No loading screens after the initial ~30-second startup load. Chapters flow seamlessly — cutscenes blend directly into gameplay with level data streaming in the background. A brief title card with the chapter name overlays the continuing scene between chapters.

### 11.4 Save System

- **Checkpoint autosave**: Frequent automatic saves. No manual save during gameplay.
- **Instant restart**: Death returns to most recent checkpoint immediately.
- **No repeated dialogue**: Cutscene dialogue since last checkpoint does not replay on restart.
- **Treasure persistence**: Treasure pickups save immediately, even if Drake dies before the next checkpoint.
- **Save file limit**: Maximum 10 save files.
- **Chapter select**: Available after first completion.

### 11.5 Difficulty Settings

**PS3 Original — 4 levels**:

| Difficulty | Drake Damage Dealt | Damage to Drake | Notes |
|---|---|---|---|
| Easy | ~1.5× | ~0.5× | Aim assist enabled; combat tutorials shown |
| Normal | 1.0× | 1.0× | Standard experience |
| Hard | 0.5× | 2.0× | No aim assist; tutorials removed |
| Crushing | ~0.25× | ~3–4× | Drake's base health halved. Enemies more aggressive, flank more, higher health. Death in 2–3 shots. |

**PS4 Remaster — 5 levels**: Adds Explorer (very easy) and Brutal (unlocked after Crushing; 4× enemy damage, half Drake health, death in 2–3 hits).

Difficulty can be lowered mid-playthrough but may affect trophy eligibility.

### 11.6 Audio System

Composed by Greg Edmonson, performed by the Skywalker Session Orchestra.

- **Dynamic music**: Transitions between exploration (ambient, atmospheric) and combat (intense, percussive) states.
- **Instrumentation**: Orchestral strings/brass/percussion, ethnic percussion, didgeridoo, native flutes, erhu. World music influences supporting the globe-trotting adventure tone.
- **Horror shift**: Chapters 17–21 use instruments that mimic human voice with screams and wails for Descendant encounters.
- **Design approach**: Deliberately avoided repeating melodies to prevent fatigue during long sessions. More ambient than melodic in jungle/underground settings.

### 11.7 Sixaxis Motion Controls (PS3 Only)

- **Grenade arc**: Tilt up/down adjusts throw trajectory.
- **Log balancing**: Tilt left/right to balance on narrow surfaces. Mandatory on PS3.

Removed in Nathan Drake Collection (PS4): log balancing remapped to analog sticks, grenade arc defaults to analog with optional Sixaxis.

---

## 12. Trophies

48 trophies (1 Platinum, 3 Gold, 8 Silver, 36 Bronze). Added via post-launch patch in August 2008.

**Completion trophies**:

| Tier | Trophy | Requirement |
|---|---|---|
| Platinum | Platinum Trophy | Unlock all other trophies |
| Gold | Charted! - Hard | Complete on Hard |
| Gold | Charted! - Crushing | Complete on Crushing |
| Gold | Master Thief Collection | Find all 61 treasures (60 + Strange Relic) |
| Silver | Charted! - Normal | Complete on Normal |
| Silver | Master Fortune Hunter | Find 60 treasures |
| Bronze | Charted! - Easy | Complete on Easy |

**Treasure progression**: Bronze trophies at 1, 10, 20, 30, 40, 50 treasures. "Relic Finder" (Bronze) for the Strange Relic.

**Combat trophies**: Headshot milestones (20/100/250), weapon kill quotas (20–50 kills per weapon), Brutal Combo milestones, Steel Fist kills, Hangman kills (from ledges), Run-and-Gunner (blind fire), Survivor (75 kills without dying), Dyno-Might (explosive multi-kills), Master Ninja (50 stealth kills).

**PS4 remaster additions**: Time trial trophies for specific chapters, Photo Mode trophy, costume change trophy, and a separate Crushing completion trophy.

---

## 13. Version Differences

| Feature | PS3 Original (2007) | PS4 Remaster (2015, Bluepoint) |
|---|---|---|
| Resolution | 720p | 1080p |
| Frame rate | 30 fps | 60 fps |
| Sixaxis controls | Mandatory | Removed / optional |
| Aim assist | Original (sluggish) | Completely rebuilt |
| Grenade throw | Sixaxis arc | Baseball-style from later Uncharted games |
| Difficulty modes | Easy / Normal / Hard / Crushing | Adds Explorer and Brutal |
| Reward system | Medal points | Treasure / completion-based |
| Making-of videos | Included | Removed |
| Photo Mode | N/A | Added |
| Speed Run Mode | N/A | Added (chapter + full-game timers) |
| Surround sound | Standard | Enhanced |
| Motion blur | None | Added |

---

## 14. Open Questions / Unverified

- **Exact damage values**: The game does not expose internal HP or damage numbers. Power/speed ratings (X/10) from community guides are relative scales, not absolute values.
- **Headshot multiplier**: Confirmed as instant kill on most enemies with most weapons, but the exact internal multiplier is unknown.
- **Enemy health values**: Not publicly available in absolute numbers. Relative differences (pirates < mercenaries < Descendants) are well-established.
- **Regeneration timing**: Exact seconds to begin/complete health recovery not publicly documented; varies by difficulty.
- **Difficulty multipliers**: Easy and Crushing damage multipliers are approximate community estimates, not datamined values.
- **Cover degradation rates**: How many shots to destroy specific cover types is not documented.

---

## 15. References

### Wikis
- Uncharted Wiki (Fandom) — weapons, rewards, medals, trophies, controls, difficulty
- IMFDB — weapon identification and real-world basis

### Guides & Community
- GameFAQs — weapon stats, treasure guides, gameplay mechanics
- PSNProfiles / PlayStationTrophies.org — trophy lists and requirements
- Push Square — Nathan Drake Collection improvements, "luck" health system

### Developer Sources
- Game Developer (Gamasutra) — Postmortem: Naughty Dog's Uncharted: Drake's Fortune
- PlayStation Blog — Nathan Drake Collection features and modes

### Music
- VGMO — Uncharted soundtrack analysis and composer interviews
