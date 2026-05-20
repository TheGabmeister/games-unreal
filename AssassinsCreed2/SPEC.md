# Assassin's Creed II — Gameplay Systems Spec

PlayStation 3 / Xbox 360 / PC, 2009. Ubisoft Montreal. Includes DLC sequences (Battle of Forli, Bonfire of the Vanities).

---

## 1. Core Gameplay Systems

### 1.1 Primary Gameplay Loop

Open-world action-adventure structured as DNA memory sequences. The player relives the genetic memories of Ezio Auditore da Firenze across Renaissance Italy (1476–1499) through the Animus 2.0. The loop cycles between:

1. **Story memories** — narrative missions with objectives, cutscenes, and scripted encounters
2. **Free-roam exploration** — parkour traversal across Italian cities, looting treasure, synchronizing viewpoints, discovering collectibles
3. **Combat encounters** — melee-focused third-person combat against guards and Templars
4. **Social stealth** — blending with crowds, hiring factions, managing notoriety
5. **Economic investment** — renovating the Villa Auditore in Monteriggioni to generate passive income
6. **Side content** — assassination contracts, races, Assassin Tombs, glyph puzzles

### 1.2 Profile System

All actions operate on a **low profile / high profile** axis controlled by a held trigger.

- **Low profile** (default): inconspicuous actions — walking, gentle push, stealth assassination, pickpocketing. Does not attract guard attention
- **High profile** (hold RT / R2): athletic and aggressive actions — sprinting, free-running, overt assassination, combat attacks, grab/shove. Draws guard attention immediately

### 1.3 Combat System

Third-person melee combat entered when guards become hostile. Ezio auto-locks to the nearest threat; manual target lock focuses a specific enemy.

**Attack / Combo Kill**: Chain timed strikes — landing 4 consecutive hits with correct timing produces a combo kill finisher on the 4th strike. Long weapons (spears, halberds) trigger the finisher on the 3rd hit. A blue visual trail on each swing confirms correct timing.

**Counter-Kill**: Hold block, then press attack when an enemy strikes. Ezio parries and delivers a lethal riposte. The timing window varies by weapon:
- Heavy weapons (hammers/maces): most forgiving window
- Swords: forgiving window
- Daggers: tighter window
- Hidden Blade: tightest window, but every successful counter is an **instant kill** on any non-boss enemy

Brutes and Seekers are immune to counter-kills from swords and daggers — they must be countered with hidden blades or heavy weapons, or defeated via dodge-attacks (see §7.1).

**Disarm**: Counter timing while unarmed (block + grab as enemy attacks). Strips the weapon from the enemy; Ezio can immediately use it against them. The only way to acquire long weapons (spears, halberds).

**Grab / Throw**: Press grab near an enemy. Once grabbed, Ezio can throw them into walls, off ledges, into water, or into other enemies.

**Sand Throw**: Unarmed only. Hold and release attack to fling dirt, blinding/staggering nearby enemies momentarily.

**Dodge / Sidestep**: Legs button in combat stance performs a lateral sidestep. In defensive stance (holding block), performs a safer dodge that enables a guaranteed follow-up strike.

**Taunt**: Head button. Provokes a targeted enemy, weakening their defenses and making them easier to disarm.

**Counter-Steal**: Hold block as an enemy attacks, then press interact. Parries the blow and simultaneously pickpockets the enemy for florins, throwing knives, or smoke bombs.

**Sweep Kick**: Stuns a grounded enemy for ~4 seconds.

### 1.4 Free-Running / Parkour

Context-sensitive traversal system. In high profile, Ezio automatically vaults low walls, leaps gaps, and scales surfaces.

| Action | Input | Notes |
|---|---|---|
| Walk / Fast Walk | Low profile + legs | Inconspicuous movement |
| Run | High profile | Moderate speed, some guard attention |
| Sprint | High profile + legs | Maximum speed, very noticeable |
| Free-Run | Sprint near obstacles | Auto-vaults, climbs, leaps |
| Climb | High profile toward wall | Grabs handholds; hold sprint to fast-climb |
| Climb Leap | Jump while climbing | Leaps upward to a higher handhold (new to AC2) |
| Wall Eject | Away from wall + jump while climbing | Pushes off backward to reach opposite wall or beam |
| Catch Ledge | Empty hand during a fall | Grabs nearest ledge, preventing fall damage |
| Drop and Catch | Empty hand while hanging | Drops to the ledge below |
| Swing Point | Automatic mid-jump | Grabs horizontal bars/poles, maintains momentum |
| Beam Walk | Automatic on narrow beams | Low profile: careful walk. High profile: sprint across |
| Leap of Faith | High profile + legs at marked edge | Dive into hay bale/cart below. Can be performed backward |
| Swimming | Automatic in water | Surface swim and underwater dive (limited breath). New to AC2 |
| Horse Riding | Mount at stables or steal | Countryside only (not Venice). Sprint, trample, mounted combat (sword only). Dismount to climb |
| Gondola | Board at docks | Venice only. Pole-driven watercraft for canal navigation. Can stand and fight from gondola |

**Fall damage**: mitigated by catching ledges, tackling civilians below, or performing air assassinations on enemies below.

**Viewpoint Synchronization**: Climb marked tall structures and synchronize to reveal the surrounding map area, collectibles, and points of interest.

**Flying Machine**: Leonardo's hang-glider used in Sequence 8 and a replayable DLC mission. Controlled by tilting toward fire sources for updraft lift. Loses altitude without updrafts. Cannot gain height independently.

### 1.5 Stealth System

#### Detection Model

Each guard displays a **Suspicion Status Indicator (SSI)** — an arrow above their head:

| State | Visual | Effect |
|---|---|---|
| Unaware | Empty / white | Guard ignores Ezio |
| Suspicious | Filling yellow | Guard notices Ezio, approaches to investigate |
| Investigating | Full yellow | Guard moves toward Ezio; blending or hiding drains it back |
| Hostile | Red | Guard attacks on sight, calls reinforcements |

Higher notoriety makes the yellow SSI fill faster and from greater distance.

#### Restricted Areas

Zones marked red on the minimap where Ezio is automatically treated as trespassing. Guards inside restricted areas have a pre-filled yellow SSI — any high-profile action or prolonged presence triggers immediate hostility. Restricted areas include enemy strongholds, rooftops with archers, and mission-specific zones.

#### Chase & Escape

When guards turn hostile, Ezio enters a **chase state**. To escape:
1. Break line of sight (turn corners, climb over buildings, dive underwater)
2. Hide in a hiding spot or blend with a crowd
3. Remain hidden until the SSI drains completely (~10–15 seconds)

Guards search Ezio's last known position. If they cannot find him, they return to patrol. Archers on rooftops can maintain line of sight longer, making rooftop escapes harder in guarded areas.

#### Civilian Penalties

Killing civilians causes an immediate desynchronization warning. Killing 3 civilians within a short window triggers **full desynchronization** (mission restart). This represents Ezio's historical memory — the Animus cannot process actions Ezio never took.

#### Social Stealth

- **Blending**: Walk near a group of 3+ civilians to blend. A visual pattern appears on the ground and the group turns semi-transparent. Guards cannot detect Ezio while blended. Breaks on any high-profile action
- **Hireable factions**: see §9.1

#### Hiding Spots

| Spot | Location | Notes |
|---|---|---|
| Hay bale / cart | Ground level, below Leap of Faith points | Full concealment; enables hay bale assassination |
| Well | Streets | Full concealment; Seekers can check these |
| Bench | Streets, plazas | Sit between two NPCs; enables bench assassination |
| Rooftop shelter | Rooftops (curtained structures) | Full concealment; Seekers can check these |
| Water / diving | Canals, rivers | Diving underwater breaks line of sight |

#### Assassination Types

| Type | Trigger |
|---|---|
| Stealth (low profile) | Approach from behind, press weapon hand. Quiet |
| High profile | Weapon hand in high profile near target. Flashy, draws attention |
| Running | Sprint toward target, press weapon hand |
| Air | Jump from above toward target, press weapon hand mid-air |
| Ledge | Hanging below an enemy, press weapon hand |
| Bench | Sitting on a bench as target walks past |
| Hay bale | Hiding in hay as enemy passes |
| Double | Dual hidden blades; two targets side by side (unlocked Sequence 4) |
| Poison | Poison blade touch; target wanders ~15 sec, attacks bystanders, dies ~15 sec later |

#### Notoriety System

A percentage-based meter (0–100%) tracked **per city**.

| State | Range | Effect |
|---|---|---|
| Incognito | 0–25% | Guards ignore Ezio unless he commits crimes in their presence |
| Suspicious | 26–75% | Guards actively scan; SSI fills faster; patrols approach to investigate |
| Notorious | 76–100% | Guards attack on sight |

**Raises notoriety**: killing guards (witnessed), killing civilians, witnessed theft, high-profile actions near guards, certain story missions.

**Lowers notoriety**:

| Method | Reduction | Cost |
|---|---|---|
| Tear down wanted poster | -25% | Free (small notoriety increase if guards see you doing it) |
| Bribe herald | -50% | 500 florins (can pickpocket the herald afterward to recover the bribe, though this slightly raises notoriety) |
| Assassinate corrupt official | -75% | Free but risky |

Posters, heralds, and officials appear on the minimap when notoriety is active.

### 1.6 Health / Synchronization

Health is displayed as **synchronization squares** — a row of diamond pips on the HUD.

**Base health**: ~5 squares. Increased by:
- **Armor**: each piece adds health squares and resistance (see §6.2)
- **Codex pages**: every 4 pages decoded by Leonardo adds 1 health square (30 pages total = ~7 extra squares)

**Damage model**: proportional — small hits or short falls partially deplete a square; heavy attacks deplete more. Partial-square damage auto-heals after a few seconds. Full-square damage requires medicine.

**Medicine**: purchased from doctors. Heals lost synchronization squares. Capacity starts at 5, upgradeable to 10 then 15 via pouch upgrades (see §6.4). Price per vial varies by source — see §12.

### 1.7 Eagle Vision

Toggle with the head button. Unlike AC1, Ezio can move freely while Eagle Vision is active.

| Color | Meaning |
|---|---|
| Red | Hostile — enemies, guards |
| Blue | Friendly — allies |
| Gold | Target — mission-critical characters; gold targets leave a lingering trail for tracking |
| White | Neutral / interactive — hiding spots, objects |

Also reveals Subject 16's glyphs on landmark buildings (see §9.3).

---

## 2. Controls & Input

### 2.1 Puppet Control System

Face buttons map to body parts rather than abstract actions. Each button's function changes based on context (combat, traversal, swimming, crowd) — the HUD displays the current action mapping in real time.

| Body Part | PS3 | Xbox 360 | Low Profile | High Profile |
|---|---|---|---|---|
| Head | Triangle | Y | Eagle Vision / Synchronize | (same) |
| Weapon Hand | Square | X | Stealth assassination | Overt attack / assassination |
| Empty Hand | Circle | B | Gentle push / interact / talk | Grab / tackle / shove |
| Legs | X | A | Fast walk / pickpocket | Sprint / free-run / jump / dodge |

### 2.2 Full Button Map

**PlayStation 3**:

| Button | Function |
|---|---|
| Left Stick | Move |
| Right Stick | Camera |
| L1 | Target Lock |
| L2 | Contextual Camera |
| R1 (hold) | Weapon Wheel |
| R2 (hold) | High Profile modifier |
| Triangle | Head |
| Square | Weapon Hand |
| Circle | Empty Hand |
| X | Legs |
| D-pad Up | Quick-select: Hidden Blade |
| D-pad Down | Quick-select: Fists |
| D-pad Left | Quick-select: Medicine |
| D-pad Right | Quick-select: Primary melee weapon |
| Select | Map |
| Start | Pause Menu / Animus Desktop |

**Xbox 360**: identical layout with LT/LB swapped for L1/L2, RB/RT for R1/R2, and Y/X/B/A for Triangle/Square/Circle/X.

---

## 3. World Structure

### 3.1 Locations

| Location | Type | Districts | Unlocked |
|---|---|---|---|
| Florence (Firenze) | Major city | San Giovanni, San Marco, Santa Maria Novella, Oltrarno (DLC) | Seq 1–4, Seq 13 |
| Monteriggioni | Hub town | — | Seq 3 |
| San Gimignano | Small city | — | Seq 3 (teased), Seq 5 (full) |
| Tuscany Countryside | Rural | — | Seq 3+ |
| Apennine Mountains | Linear pass | — | Seq 6 |
| Forli | Small city | — | Seq 6 (teased), Seq 12 (DLC) |
| Romagna Countryside | Rural | — | Seq 6+ |
| Venice (Venezia) | Major city | San Polo, San Marco, Dorsoduro, Castello, Cannaregio | Seq 7–11 |
| Vatican / Rome | Linear finale | — | Seq 14 (not free-roam) |

Venice is the only area where horses cannot be used; gondolas serve as water transport instead.

### 3.2 Fast Travel

**Travel stations** (marked with a two-arrow "Viaggio" icon) connect cities and districts. Must be discovered first (revealed by viewpoint synchronization). Small florin fee per trip. In cities: ferry stations (traghetti). In countryside: stables with horses.

### 3.3 Map Revelation

Each area starts with fog of war. Climbing and synchronizing at viewpoints (73 total, 66 without DLC) reveals the surrounding map, collectible locations, and points of interest on the minimap.

---

## 4. Playable Characters

### 4.1 Ezio Auditore da Firenze

The sole playable character in the Animus sequences. A Florentine nobleman turned Assassin, aged 17–40 across the game's timeline (1476–1499). No character customization or class system — Ezio's abilities expand through story progression and equipment.

**Ability Unlock Timeline**:

| Sequence | Ability |
|---|---|
| 1 | Fistfight, climbing, pickpocketing |
| 2 | Hidden Blade (repaired by Leonardo), stealth assassination |
| 3 | Full combat, sword/dagger/mace use, villa renovation |
| 4 | Dual Hidden Blades, double assassination |
| 5 | Poison Blade |
| 5 | Smoke bombs |
| 8–9 | Hidden Gun (pistol) |

### 4.2 Desmond Miles

Playable in brief modern-day sections between Animus sessions. Walks freely around the Assassin hideout (a warehouse), can talk to Lucy Stillman, Shaun Hastings, and Rebecca Crane. Tests Bleeding Effect-acquired abilities (Eagle Vision, freerunning). During one session, experiences a Bleeding Effect hallucination as Altair in Acre.

---

## 5. Story & Progression

### 5.1 Memory Structure

The game is divided into **14 sequences**, each containing multiple numbered **memories** (missions). Progress is tracked on the **DNA menu** as a timeline of filled/empty squares.

| Seq | Title | Setting | Year | Key Events |
|---|---|---|---|---|
| 1 | Ignorance Is Bliss | Florence | 1476 | Tutorial. Ezio's family arrested and publicly executed |
| 2 | Escape Plans | Florence | 1476 | Flee Florence. Leonardo repairs Hidden Blade. Kill **Uberto Alberti** |
| 3 | Requiescat in Pace | Monteriggioni / San Gimignano | 1476–78 | Meet uncle Mario. Villa introduced. Kill **Vieri de' Pazzi** |
| 4 | The Pazzi Conspiracy | Florence | 1478 | Pazzi Conspiracy. Kill **Francesco de' Pazzi**. Villa renovation unlocked |
| 5 | Loose Ends | San Gimignano / Tuscany | 1478 | Kill **Antonio Maffei**, **Bernardo Baroncelli**, **Stefano da Bagnone**, **Francesco Salviati**. Mercy-kill **Jacopo de' Pazzi** |
| 6 | Rocky Road | Apennine Mountains / Romagna | 1478 | Mountain crossing. Meet Caterina Sforza |
| 7 | The Merchant of Venice | Venice — San Polo | 1481 | Thieves Guild. Kill **Emilio Barbarigo** |
| 8 | Necessity, Mother of Invention | Venice — San Marco | 1485 | Leonardo's flying machine. Kill **Carlo Grimaldi** |
| 9 | Carnevale | Venice — Dorsoduro | 1486 | Venice Carnival. Kill **Marco Barbarigo** |
| 10 | Force Majeure | Venice — Castello | 1486 | Kill **Silvio Barbarigo** and **Dante Moro** |
| 11 | Alter Egos | Venice — Cannaregio | 1486–88 | Gather Codex pages. Identify Rodrigo Borgia |
| 12 | Battle of Forli | Forli | 1488 | (DLC, Jan 2010) Defend Forli with Caterina Sforza. Apple stolen by **Checco Orsi**. Replayable flying machine mission |
| 13 | Bonfire of the Vanities | Florence — Oltrarno | 1497 | (DLC, Feb 2010) Kill **9 of Savonarola's lieutenants** in any order. Opens Oltrarno district |
| 14 | Veni, Vidi, Vici | Vatican / Rome | 1499 | Final fight with **Rodrigo Borgia** (Pope Alexander VI). Minerva's warning. Ezio spares Borgia |

Sequences 12 and 13 fill "corrupted memory" gaps visible in the base game's DNA timeline. The Ezio Collection remaster (2016) includes both DLC sequences by default.

### 5.2 Modern-Day Frame

Between Animus sessions, Desmond exits and interacts with the Assassin team in their warehouse hideout. Subject 16's blood-written cryptic messages are visible via Eagle Vision on Desmond's bedroom wall. The game ends with Abstergo locating and attacking the warehouse; Desmond fights through guards using his Bleeding Effect-acquired combat skills.

---

## 6. Items & Equipment

### 6.1 Weapons

Three purchasable categories rated 1–5 in **Damage**, **Speed**, and **Deflect**.

#### Swords

| Weapon | Dmg | Spd | Def | Cost (f) | Seq |
|---|---|---|---|---|---|
| Common Sword | 1 | 2 | 2 | ~1,000 | 3 |
| Venetian Falchion | 1 | 3 | 1 | 1,900 | 3 |
| Old Syrian Sword | 1 | 2 | 3 | 2,300 | 3 |
| Captain's Sword | 2 | 4 | 3 | 5,200 | 5 |
| Florentine Falchion | 2 | 3 | 4 | 5,200 | 5 |
| Scimitar | 3 | 3 | 5 | 11,300 | 6 |
| Milanese Sword | 3 | 5 | 3 | 11,300 | 7 |
| Schiavona | 4 | 5 | 4 | 25,000 | 8 |
| Sword of Altair | 5 | 5 | 5 | 45,000 | Monteriggioni; requires all 30 Codex pages |

#### Short Blades (Daggers)

| Weapon | Dmg | Spd | Def | Cost (f) | Seq |
|---|---|---|---|---|---|
| Dagger | 1 | 1 | 2 | Free | 3 |
| Knife | 1 | 2 | 3 | 1,300 | 3 |
| Stiletto | 2 | 4 | 1 | 2,200 | 4 |
| Channeled Cinquedea | 3 | 3 | 4 | 4,100 | 5 |
| Metal Cestus | 3 | 5 | 1 | 2,000 | 6 |
| Sultan's Knife | 3 | 5 | 3 | 4,300 | 6 |
| Butcher's Knife | 4 | 3 | 5 | 6,400 | 7 |
| Notched Cinquedea | 5 | 5 | 4 | 9,300 | 8 |

The **Metal Cestus** also permanently increases unarmed damage when purchased.

#### Hammers / Maces

| Weapon | Dmg | Spd | Def | Cost (f) | Seq |
|---|---|---|---|---|---|
| Mercenario War Hammer | 1 | 1 | 3 | 1,900 | 3 |
| Maul | 2 | 1 | 2 | 3,600 | 4 |
| Flanged Mace | 3 | 3 | 3 | 10,500 | 6 |
| Cavalieri Mace | 4 | 2 | 2 | 20,800 | 7 |
| Condottiero War Hammer | 5 | 3 | 4 | 30,600 | Collect 50 feathers |

#### Long Weapons (Not Purchasable)

Spears, halberds, pikes. Obtained only by disarming Seekers or picking up from the ground. Longest melee reach. Max combo chain: 3 hits. Dropped automatically when sprinting or free-running.

#### Special Weapons (Leonardo's Inventions)

Unlocked via Codex pages brought to Leonardo da Vinci.

| Weapon | Unlocked | Notes |
|---|---|---|
| Hidden Blade | Seq 2 | Repaired by Leonardo. Tightest counter window, always instant kill |
| Dual Hidden Blades | Seq 4 | Enables double assassinations |
| Poison Blade | Seq 5 | Hollow needle. Victim wanders ~15 sec, attacks bystanders, dies ~15 sec later. Silent. 175f per vial |
| Hidden Gun (Pistol) | Seq 8–9 | 6-round capacity (not upgradeable). ~3 sec reload. One-shot kill. Very loud — alerts all nearby guards. Holding aim improves accuracy. Aiming at enemies lowers their morale |

#### Ranged / Thrown

| Item | Range / Radius | Capacity | Cost | Notes |
|---|---|---|---|---|
| Throwing Knives | ~20 m | 5 → 10 → 15 (pouch upgrades) | 50f each | One-hit kill early game; 2–3 later. Charged throw targets up to 5 enemies |
| Smoke Bombs | 4 m blast, 15 m alert | 3 (fixed) | 350f each | 8 sec duration. Dazes all enemies in cloud |

#### Fists (Unarmed)

Always available. Enables disarm, grab/throw, sand throw, counter-steal. Combo kills possible but tighter timing with no visual indicator.

### 6.2 Armor

Four purchasable sets plus one special set. Each set has 4 pieces: **Chest Guard**, **Vambraces**, **Greaves**, **Spaulders**. Each piece adds **Health** (sync squares) and **Resistance** (damage absorption).

#### Leather Armor (Tier 1)

| Piece | Health | Resist | Cost (f) |
|---|---|---|---|
| Spaulders | 1 | 0 | 2,300 |
| Chest Guard | 2 | 0 | 4,370 |
| Vambraces | 1 | 0 | 1,100 |
| Greaves | 1 | 0 | 1,140 |
| **Set Total** | **5** | **0** | **8,910** |

#### Helmschmied Armor (Tier 2)

| Piece | Health | Resist | Cost (f) |
|---|---|---|---|
| Spaulders | 2 | 1 | 6,200 |
| Chest Guard | 3 | 2 | 10,800 |
| Vambraces | 1 | 1 | 5,100 |
| Greaves | 2 | 1 | 4,940 |
| **Set Total** | **8** | **5** | **27,040** |

#### Metal Armor (Tier 3)

| Piece | Health | Resist | Cost (f) |
|---|---|---|---|
| Pauldrons | 3 | 2 | 12,000 |
| Chest Guard | 4 | 3 | 17,200 |
| Vambraces | 2 | 2 | 6,300 |
| Greaves | 2 | 2 | 7,200 |
| **Set Total** | **11** | **9** | **42,700** |

#### Missaglias Armor (Tier 4)

| Piece | Health | Resist | Cost (f) |
|---|---|---|---|
| Pauldrons | 4 | 3 | 21,300 |
| Chest Guard | 5 | 4 | 27,900 |
| Vambraces | 3 | 3 | 12,000 |
| Greaves | 3 | 3 | 14,600 |
| **Set Total** | **15** | **13** | **75,800** |

#### Armor of Altair (Special)

| Piece | Health | Resist |
|---|---|---|
| Pauldrons | 4 | 4 |
| Chest Guard | 6 | 5 |
| Vambraces | 2 | 3 |
| Greaves | 3 | 3 |
| **Set Total** | **15** | **15** |

- **Free** — obtained by collecting all 6 Assassin Seals from Assassin Tombs
- **Cannot be broken** — never needs repair (unique advantage over Missaglias)
- Stored in the Sanctuary beneath the Villa Auditore

#### Armor Degradation

All purchasable armor degrades with damage. Every ~20 damage squares taken removes 1 resistance from each equipped piece. Broken armor provides no benefit. Repaired at blacksmiths (cost scales with tier). Armor of Altair is exempt.

### 6.3 Capes

| Cape | Effect | Source |
|---|---|---|
| Plain Cape | Cosmetic only | Tailor |
| Medici Cape | Notoriety always 0 in Florence / Tuscany | Seq 6 reward |
| Venetian Cape | Notoriety always 0 in Venice / Forli | Seq 11 reward |
| Auditore Cape | Notoriety always 100% outside Monteriggioni | Collect all 100 feathers |

### 6.4 Pouches & Capacity

All pouches purchased from tailors. Each upgrade adds +5 capacity.

| Item | Small | Medium (3,000f) | Large (6,000f) |
|---|---|---|---|
| Medicine | 5 | 10 | 15 |
| Throwing Knives | 5 | 10 | 15 |
| Poison Vials | 5 | 10 | 15 |

Smoke bombs (3) and pistol ammo (6) have fixed capacities with no upgrades available.

---

## 7. Enemies & Opponents

### 7.1 Guard Types

| Type | Weapons | Armor | Behavior |
|---|---|---|---|
| Militia / Regular | Swords, knives, maces | Light | Standard combat; easy to counter |
| Elite Guards | Better swords | Medium | Tougher, better timing |
| Brutes | Heavy two-handed (axes, war hammers) | Heavy plate | Slow; immune to counter-kills from small/medium weapons — must use hidden blade counter, dodge, or heavy weapons |
| Agiles | Daggers | Light / none | Fast; dodge player attacks; chase across rooftops via free-running |
| Seekers | Spears, halberds | Medium-heavy | Most aware guard type — detect Ezio walking behind them; search hiding spots (hay bales, wells, rooftop shelters); deployed at checkpoints |
| Archers | Crossbows / bows | Light | Stationed on rooftops; warn Ezio to descend; shoot on sight if he stays; weak in melee |
| Papal Guards | Various | Heavy | Elite Vatican guards in late sequences |

### 7.2 Boss: Rodrigo Borgia (Sequence 14)

Two-phase final encounter:

1. **Staff vs. Apple**: Rodrigo wields the Papal Staff (Staff of Eden) — Dmg 5, Spd 5, Def 5. Ezio wields the Apple of Eden for ranged energy blasts and area stuns. Standard combat with counter-kill opportunities. Ezio briefly acquires the Papal Staff mid-fight but cannot retain it afterward
2. **Fistfight**: After disarming each other of the Pieces of Eden, the fight becomes a bare-knuckle brawl inside the Vatican vault. Grab/throw and combo mechanics. Ezio spares Rodrigo after defeating him

---

## 8. Economy

### 8.1 Currency

**Florins (f)** — the single currency for all transactions.

### 8.2 Income Sources

| Source | Notes |
|---|---|
| Villa income | Primary passive income. ~10,000f per 20-minute cycle when fully upgraded. Deposited in villa treasure chest (max capacity ~60,000f) |
| Pickpocketing | Walk into civilians and hold pickpocket button. Small amounts (~50–200f per target) |
| Looting | Search dead/unconscious enemies. Yields florins and consumables |
| Mission rewards | Story memories and side missions grant florins |
| Treasure chests | 330 total across the game world |
| Assassination contracts | 30 contracts, each pays florins |

### 8.3 Shops

| Shop | Sells | Notes |
|---|---|---|
| Blacksmith | Weapons, armor, ammo (knives, bullets), armor repair | 22 purchasable weapons. Repair cost scales with armor tier |
| Doctor | Medicine, poison vials | Healing and poison supply. Prices vary by source (see §12) |
| Tailor | Pouches, dyes | Pouch upgrades (§6.4). Dyes recolor Ezio's robes and cape — cosmetic only, ~500–2,000f per palette |
| Art Merchant | Paintings, treasure maps | 30 paintings for villa gallery. Treasure maps reveal chest locations per district |

### 8.4 Villa Renovation

Unlocked at Sequence 4. Ezio speaks with the **architect** in Monteriggioni to purchase renovations. Sister Claudia tracks progress via a ledger.

**Income model**: every 20 minutes of real playtime, ~10% of each renovation's cost is deposited into the villa chest.

**Renovable facilities**:
- **Shops** (blacksmith, doctor, tailor, art merchant) — 3 upgrade tiers each. Higher tiers unlock better inventory and provide 5% / 10% / 15% discounts
- **Infrastructure** — bank, church, mine, well, barracks, brothel, thieves guild. Each reopened building increases income and may reveal hidden treasure chests

**Total renovation cost**: ~86,000–91,000 florins.

**Villa value** also increased by displaying paintings, weapons, armor, Codex pages, feathers, Assassin Seals, and statuettes.

**Combat training ring**: located in the villa courtyard. Uncle Mario serves as instructor during early sequences, teaching combat techniques (counter-kills, dodge, disarm, sand throw). The ring remains available for free-form practice throughout the game.

---

## 9. Side Content & Collectibles

### 9.1 Hireable Factions

| Faction | Cost | Role |
|---|---|---|
| Courtesans | ~150f | Distract/lure guards; form a blend group around Ezio |
| Mercenaries | ~150f | Fight guards directly; draw combat attention. Cannot free-run |
| Thieves | ~150f | Lure guards away by provoking chases; can free-run across rooftops |

### 9.2 Side Missions

| Type | Count | Description |
|---|---|---|
| Assassination Contracts | 30 | Via pigeon coops on rooftops. Locate and kill target(s), sometimes with restrictions |
| Races | 6 | Sprint through checkpoint markers within a time limit |
| Beat-Up Events | 5 | Fistfight only — pummel a target until they submit |
| Courier Missions | 6 | Deliver item between locations within a time limit |

### 9.3 Assassin Tombs

6 extended platforming/parkour challenge dungeons. Each contains the sarcophagus of a legendary Assassin and yields one **Assassin Seal**. Collecting all 6 unlocks the Armor of Altair.

| Tomb | Assassin | Location |
|---|---|---|
| Santa Maria Novella | Darius (killer of Xerxes I) | Florence |
| Santa Maria del Fiore | Iltani (killed Alexander the Great) | Florence |
| Torre Grossa | Wei Yu (killed Qin Shi Huang) | San Gimignano |
| Rocca di Ravaldino | Qulan Gal (Mongol-era Assassin) | Forli |
| Basilica di San Marco | Amunet (killed Cleopatra) | Venice |
| Santa Maria della Visitazione | Leonius (killed Caligula) | Venice |

### 9.4 Templar Lairs

3 additional platforming challenge dungeons (DLC / special editions). Similar to Assassin Tombs but reward large florin payouts rather than seals.

### 9.5 Glyph Puzzles ("The Truth")

20 glyphs painted on landmark buildings by Subject 16 (Clay Kaczmarek), visible only with Eagle Vision. Each triggers a code-breaking or image-selection puzzle. Solved in fixed numerical order regardless of discovery order. Completing all 20 unlocks "The Truth" — a video showing Adam and Eve fleeing a Precursor facility with an Apple of Eden.

Distribution: 5 Florence, 1 Monteriggioni, 5 Tuscany, 2 Romagna, 7 Venice.

### 9.6 Collectibles Summary

| Collectible | Count | Reward |
|---|---|---|
| Feathers | 100 | 50: Condottiero War Hammer purchasable. 100: Auditore Cape |
| Codex Pages | 30 | Weapon upgrades; every 4 decoded = +1 health square; all 30 required for Seq 14 |
| Glyphs | 20 | "The Truth" video |
| Viewpoints | 73 (66 without DLC) | Map revelation |
| Treasure Chests | 330 | Florins |
| Statuettes | 8 | Villa value. Roman gods: Mars, Venus, Pluto, Neptune, Jupiter, Minerva, Diana, Apollo. Every pair = 2,000f bonus |
| Paintings | 30 | Villa gallery display. 8 are by Leonardo da Vinci |
| Assassin Seals | 6 | Armor of Altair |

---

## 10. UI & HUD

### 10.1 HUD Layout

**Top-left**: Synchronization (health) bar — row of diamond pips with Ezio's avatar. Screen flashes blue/red at critical health. Notoriety indicator — a diamond icon that fills from empty (incognito) through partial (suspicious) to full (notorious).

**Top-right**: Puppet HUD — gamepad diagram showing four face buttons with body-part icons and context-sensitive action labels that update in real time based on Ezio's state.

**Bottom-left**: Equipped weapon/tool icon. Current florin count.

**Bottom-right**: Circular minimap showing streets, buildings, water, restricted zones (red), enemy positions (red dots), mission markers, shop icons, viewpoints, faction group locations, pigeon coops, and collectibles. Notoriety-reduction targets (posters, heralds, officials) appear when notoriety is active. Fog of war cleared by viewpoint synchronization.

### 10.2 HUD States

- **Exploration**: full HUD — health, minimap, puppet display, equipment
- **Combat**: target lock indicator on focused enemy; camera pulls back to show arena
- **Stealth / blend**: SSI arrows above guards; blend visual pattern on ground around civilian groups
- **Cutscene / memory corridor**: HUD hidden. White void "memory corridor" appears after major assassinations for target's dying words
- **Swimming**: breath meter appears during underwater diving

### 10.3 In-Game Indicators

- **SSI arrows**: per-guard awareness state (white → yellow → red)
- **Waypoint markers**: directional indicators toward active objectives
- **Interaction prompts**: context-sensitive button icons near interactable objects/NPCs
- **Autosave icon**: spinning Animus symbol during save

### 10.4 Menu Screens

- **Animus Desktop** (pause): Resume, Map, DNA, Database, Options, Statistics, Quit
- **Map**: full overhead map with icons for all points of interest, custom waypoint placement, fast travel
- **DNA**: memory sequence timeline — completed memories as filled squares
- **Database**: Shaun Hastings' historical entries on locations, people, events. Snarky commentary
- **Weapon Wheel** (hold R1/RB): radial selector for all available weapons and tools. D-pad shortcuts mapped per §2.2

---

## 11. Engine & Presentation Systems

### 11.1 Dialogue System

Conversations are fully voiced with cinematic camera angles. No dialogue choices — all interactions are scripted. **Memory corridor**: after major assassinations, a white void appears where Ezio and the dying target exchange final words before the target dies.

### 11.2 Save System

- **Autosave**: triggers at memory completion, viewpoint synchronization, treasure/collectible acquisition, purchases, and area transitions. Spinning Animus icon indicates save in progress
- **Manual save**: via Pause Menu > Quit Session, which saves current state and exits to Animus Desktop
- **Death/desynchronization**: restarts from the last checkpoint within the current memory

### 11.3 Camera

Third-person at all times. Right stick for manual rotation. Player movement is relative to camera angle. Camera auto-adjusts in narrow spaces. Target lock focuses and orbits a specific enemy. Climbing direction is camera-relative — handhold selection depends on camera orientation.

### 11.4 Audio System

- Dynamic music intensity scales with player state: calm exploration, suspicious investigation, open combat, chase
- Each city has distinct ambient soundscapes (Venetian canals, Florentine market bustle, countryside birdsong)
- Jesper Kyd composed the score, blending Renaissance-era instruments with electronic textures
- "Ezio's Family" serves as the main character theme, recurring across the Ezio trilogy
- Italian voice acting for ambient NPC dialogue; English for principal characters with Italian accent

---

## 12. Open Questions / Unverified

- **Exact counter-kill frame windows** per weapon category — community consensus is hidden blade < dagger < sword < heavy, but precise frame counts are not publicly documented
- **Armor resistance formula** — the relationship between resistance circles and actual damage reduction percentage is not clearly documented. Per-piece resistance values for mid-tier sets (Metal, Helmschmied) may vary from the values listed; set totals are more reliably sourced
- **Medicine and poison vial prices** — sources report medicine at 75f, 150f, or 175f per vial. Likely reflects base price vs. Monteriggioni discount tiers. Poison vials similarly reported as 175f or 250f. Exact base prices and discount math need verification
- **Common Sword price** — listed as ~1,000f but the exact value did not surface across multiple sources
- **Villa income formula precision** — "~10% of renovation cost per 20 minutes" is approximate; the exact multiplier and whether it compounds or is flat per building is unclear
- **Throwing knife damage scaling** — early game one-hit kills, later requires 2–3 knives. Whether this scales with enemy health, armor, or a fixed sequence threshold is unconfirmed
- **Pouch upgrade tiers and availability sequences** — throwing knife belt may go to 20 capacity rather than 15; sources conflict on exact medium/large pouch availability sequences

---

## 13. References

### Wikis
- Assassin's Creed Wiki (Fandom) — weapons, armor, abilities, locations, characters, collectibles, DLC
- StrategyWiki — controls, equipment, side missions, collection guides, Assassin Tombs
- Wikipedia — overview, release info, DLC dates

### Guides & FAQs
- GameFAQs — guard types, counter mechanics, weapon lists, pouch upgrades, villa renovation
- Gamepressure — weapon collection, armor collection, city maps
- GamerSyndrome — ranged weapons guide
- SuperCheats — controls, general guide
- TrueAchievements / TrueTrophies — achievement guides, villa walkthrough

### Community Sources
- The Hidden Blade (fan site) — notoriety system, moves/skills list, counter-kill tips
- AlteredGamer — combat and fighting tips
- Steam community guides — memories guide, 100% checklist
- Interface design blogs — HUD analysis
