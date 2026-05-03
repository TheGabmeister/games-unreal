# SPEC.md — GTA: San Andreas Gameplay Mechanics

This document catalogs every major gameplay system in GTA: San Andreas (2004) for recreation in Unreal Engine 5. No implementation details — only what the game does, not how we'll build it.

---

## 1. Player Movement

### On-Foot Locomotion

| Mode | Notes |
|------|-------|
| Walk | Slow speed, analog-stick modulated |
| Run | Default speed |
| Sprint | Drains stamina; tapping sprint repeatedly is slightly faster than holding |
| Crouch | Silent movement; enables stealth and combat roll |
| Jump | ~13% faster than running; auto-climbs ledges on contact |
| Swim (surface) | Breaststroke (default) or front crawl (sprint, drains stamina) |
| Swim (dive) | Underwater movement; drains breath meter |
| Climb | Auto-latches to ledges/fences/walls when jumping toward them |
| Combat roll | While aiming + crouching, press jump + direction; brief i-frames |

### Stamina

- Internal scale: 0–1000
- Governs sprint, fast-swim, and fast-cycle duration
- Gain: +5% per 5 min running/cycling, +5% per 2.5 min swimming, +4% per 14 s gym treadmill
- Daily cap: 20% per game-day
- Permanent infinite stamina: earn $10,000 cumulative from Burglar missions

### Lung Capacity (Breath)

- Internal scale: 0–1000
- Determines underwater oxygen duration (~20 s at minimum, ~2–3 min at maximum)
- Gain: +5% per 60 s spent underwater
- Collecting all 50 Oysters grants infinite lung capacity

---

## 2. Hand-to-Hand Combat

### Fighting Styles

| Style | Gym Location | Key Feature |
|-------|-------------|-------------|
| Default | — | Basic punches/kicks; weakest |
| Boxing | Ganton Gym, LS | Uppercut–hook combo |
| Kung Fu | Cobra Martial Arts, SF | Flashy running kick |
| Muay Thai | Below the Belt, LV | Knee–elbow combo; final elbow is one-hit kill |

- Requires 35% Muscle to train
- Only one style active at a time (learning a new one replaces the previous)
- Each provides: 3–4 hit standard combo, running attack, ground attack

### Blocking and Grappling

- Hold lock-on without attacking to block; reduces incoming melee damage
- Sustained attacks break through block

### Melee Damage Scaling

- Muscle stat (0–1000) directly increases melee damage
- No separate "fighting skill" bar — improvement tied to Muscle + style selection

---

## 3. Weapon Combat

### Weapon Slots (13)

| Slot | Category | Examples |
|------|----------|---------|
| 0 | Hand | Fist, Brass Knuckles |
| 1 | Melee | Knife, Baseball Bat, Katana, Chainsaw, Golf Club, Shovel, Pool Cue, Nightstick |
| 2 | Handguns | 9mm Pistol, Silenced 9mm, Desert Eagle |
| 3 | Shotguns | Pump-Action, Sawn-Off, Combat Shotgun (SPAS-12) |
| 4 | SMGs | Tec-9, Micro SMG (Micro Uzi), SMG (MP5) |
| 5 | Assault Rifles | AK-47, M4 |
| 6 | Rifles | Country Rifle, Sniper Rifle |
| 7 | Heavy | Flamethrower, Rocket Launcher, Heat-Seeking RPG, Minigun |
| 8 | Thrown | Tear Gas, Molotov, Grenade, Satchel Charges |
| 9 | Handheld | Fire Extinguisher, Spray Can, Camera |
| 10 | Gifts | Flowers, Cane, Dildo, Vibrator |
| 11 | Special | NVG, Thermal Goggles, Parachute |
| 12 | Detonator | Satchel Charge Detonator |

### Weapon Skill Progression

Three tiers per weapon: **Poor → Gangster → Hitman**

| Tier | Unlocks |
|------|---------|
| Poor | Basic functionality; no movement while aiming |
| Gangster | Move while aiming; increased lock-on range, accuracy, fire rate, strafe speed |
| Hitman | Maximum stats; dual-wielding (for compatible weapons) |

**Dual-wield weapons at Hitman:** 9mm Pistol, Sawn-Off Shotgun, Micro SMG, Tec-9

Skill gained by landing hits on damageable targets (not walls/air).

### Lock-On / Targeting

- ~90° frontal auto-aim cone
- Locks closest hostile; reticle changes Green → Orange → Red → Black (dead)
- Rifles, snipers, heavy weapons cannot lock on
- Higher weapon skill increases lock-on range

### Drive-By

- Driver: fire SMGs out left/right window only
- Passenger: wider aiming freedom with pistols, SMGs, shotguns, rifles
- Motorcycles: one-handed SMG with more flexible aiming

---

## 4. Stealth

- Crouch to enter stealth mode; minimap blip turns blue (hidden)
- **Knife stealth kill:** approach from behind while crouched + locked on; instant silent kill (throat-slit animation)
- Silenced 9mm: quieter but not perfectly silent at close range
- Detection is line-of-sight based; no explicit noise meter in general gameplay (noise meter only appears during Burglar missions)

---

## 5. Vehicles

### Types (212 total)

| Category | Count |
|----------|-------|
| Cars (all types) | ~100+ |
| Motorcycles | 10 |
| Bicycles | 3 (BMX, Mountain Bike, Bike) |
| Boats | 11 |
| Planes (fixed-wing) | 11 |
| Helicopters | 9 |
| RC Vehicles | 6 |
| Trains | 3 |
| Trailers | 6 |

### Vehicle Health & Damage

- Health: 0–1000
- Above 650: no visible effect
- 550–650: white smoke
- 390–550: grey smoke
- 250–390: black smoke + clunking
- Below 250: engine fire → explosion in seconds
- 0: instant destruction

**Visual deformation:** doors/hoods/trunks dangle or detach; bumpers separate; headlights break. Chassis mesh does not soft-deform.

### Driving Physics Features

| Feature | Details |
|---------|---------|
| Hydraulics | Lowriders only; bounce direction via right stick; $1,500 install |
| Nitrous Oxide | 2x/5x/10x tanks; boosts acceleration not top speed; auto-refills over time |
| Tire puncture | Shot out or spike-stripped; one tire on strip pops ALL tires |
| Vehicle fire | Below 250 HP; external fire can ignite without instant explosion |

### Vehicle Customization (Mod Garages)

| Garage | Specialization | Modifications |
|--------|---------------|---------------|
| TransFender (3 locations) | ~65 car models | Paint, wheels, exhausts, hoods, vents, spoilers, skirts, fog lamps, nitrous, hydraulics, bass |
| Loco Low Co. (1 location) | 8 lowriders | All TransFender options + lowrider-specific body mods |
| Wheel Arch Angels (1 location) | 6 tuner cars | Alien/X-Flow body kits + standard mods |

### Driving Skill

- Gained by driving 4-wheeled vehicles (not motorcycles/bikes)
- Improves: airborne control, reduced skidding, wheelie ability
- At 20%: stadium events unlock

### Flying Skill

- Gained by flying aircraft or completing Pilot School
- Reduces turbulence/shaking; improves responsiveness
- Pilot School rewards: Rustler (Bronze), Stunt Plane (Silver), Hunter (Gold)

### Cycling Skill

- Gained by riding bicycles or gym exercise bike
- Improves: bunny-hop height, top speed, less chance of falling off
- Bunny-hop exploit: fire-button immediately after release launches abnormally high

### Special Vehicles

- **Trains:** throttle/brake only; locked to rails; derails above ~190 km/h on curves
- **Combine Harvester:** pedestrian grinding (ejects body parts); door locks (can't be busted)
- **Parachute:** auto-equipped when exiting aircraft at altitude; manual deploy; steer + flare to land

---

## 6. RPG Stats & Progression

### All Stats

| Stat | Scale | Gained By | Lost By | Effect |
|------|-------|-----------|---------|--------|
| Respect | 0–100% | Missions, killing enemies, taking territory, spray tags | Losing territory, gang member deaths, arrest/death | Max gang recruits (1%=2, 80%=7) |
| Sex Appeal | Composite | Clothing (50%), last vehicle (50%), moderate muscle | Cheap clothes, bad vehicle, excess fat/muscle | Girlfriend eligibility |
| Stamina | 0–1000 | Running, swimming, cycling, gym treadmill | Never decreases | Sprint/swim/cycle duration |
| Muscle | 0–1000 | Weightlifting, exercise | Starvation (when fat=0) | Melee damage, respect bonus, model change at 50% |
| Fat | 0–1000 | Eating | Exercise, starvation | Reduced speed/jump; visual model change |
| Lung Capacity | 0–1000 | Underwater swimming | Never decreases | Underwater oxygen duration |
| Weapon Skills | Per-weapon | Landing hits | Never decreases | Accuracy, fire rate, dual-wield |
| Driving Skill | 0–1000 | Driving cars | Never decreases | Handling, airborne control |
| Flying Skill | 0–1000 | Flying aircraft, Pilot School | Never decreases | Turbulence reduction |
| Cycling Skill | 0–1000 | Riding bikes, gym bike | Never decreases | Bunny-hop height, bike speed |
| Gambling Skill | 0–1000 | Spending at casinos ($1/pt per $100) | Never decreases | Max bet limit |

### Body/Physique System

| State | Appearance | Gameplay |
|-------|-----------|----------|
| High Fat | Obese model, round face | Slower run, lower jump |
| High Muscle | Buff model, wide shoulders | More melee damage, walk animation change at 50% |
| Both high | Bulky build | Mix of effects |
| Both low | Skinny/default | Normal speed, low damage |

### Hunger System

- Must eat every 72 in-game hours (~48 real minutes)
- Starvation: fat consumed → then muscle → then health drains → death
- Overeating (12+ meals in 6 game-hours): CJ vomits, losing all fat gained
- Food sources: Cluckin' Bell, Burger Shot, Well Stacked Pizza, vending machines, street vendors

---

## 7. Clothing & Appearance

### Equipment Slots (7)

Torso, Legs, Shoes, Watch, Chain/Necklace, Glasses/Mask, Hat

### Clothing Stores (6)

| Store | Tier | Notes |
|-------|------|-------|
| Binco | Budget | Lowest stats |
| Sub Urban | Mid | Casual/streetwear |
| ZIP | Mid | Athletic/casual |
| ProLaps | Expensive | Designer sportswear |
| Victim | Expensive | Fashion/trendy |
| Didier Sachs | Luxury | Suits; highest stat boosts |

### Tattoos

- 7 body areas (upper/lower arms, back, chest, stomach)
- Each provides up to +3% respect OR sex appeal
- Permanent; replaceable but not removable

### Haircuts

- 7 barber shops; styles affect respect and sex appeal
- Includes facial hair options

---

## 8. Gang Territory System

### Overview

- 53 territories in Los Santos
- Three gangs: Grove Street Families (green), Ballas (purple), Los Santos Vagos (yellow)
- Unlocked after mission "Doberman"

### Taking Territory

1. Kill 3 enemy gang members on foot in their territory → war triggers
2. Survive 3 waves of attackers:
   - Wave 1: 6–8 enemies with pistols/Micro SMGs
   - Wave 2: 8–10 enemies with SMGs/AK-47s
   - Wave 3: 10–12 enemies with AK-47s
3. Victory claims the territory

### Defending

- Random attacks on GSF turf; map flashes red
- Only 1 wave to survive
- Failure loses the territory and respect

### Rewards

- +6% respect per territory held
- Money accumulates at Johnson House proportional to holdings
- 19+ territories required for final mission

### Gang Recruitment

- Recruit GSF members proportional to respect (max 7)
- Members follow, attack enemies, provide covering fire
- Can be commanded to hold or follow

---

## 9. Wanted Level System

### Escalation

| Stars | Force | Response |
|-------|-------|----------|
| 1 | Police officers | Patrol cars; attempt arrest |
| 2 | Armed police | Shoot to kill; aggressive ramming |
| 3 | + Helicopter | Roadblocks; aerial pursuit |
| 4 | SWAT | Armored Enforcers; rappel from heli |
| 5 | FBI | 4-agent SUVs; streets clear of civilians |
| 6 | Military | Rhino tanks (instant kill on collision); M4 soldiers |

### Star Availability

- 1–4: from game start
- 5: after "The Green Sabre"
- 6: after "Yay Ka-Boom-Boom"
- Entering locked regions: auto 4-star

### Losing Wanted Level

| Method | Effect |
|--------|--------|
| Pay 'n' Spray | Clears all stars |
| Police Bribes (pickups) | -1 star each |
| Safehouse save | Clears all |
| Clothing change | Clears all |
| Mod garage visit | Clears all |
| Evasion (distance/time) | Stars flash then clear |
| Wasted/Busted | Clears all (lose weapons + money) |

---

## 10. World Structure

### Geography

| Region | Based On |
|--------|---------|
| Los Santos | Los Angeles |
| San Fierro | San Francisco |
| Las Venturas | Las Vegas |
| Red County | Rural California |
| Flint County | Northern California forests |
| Whetstone | Sierra Nevada (Mount Chiliad) |
| Tierra Robada | Semi-arid central California |
| Bone County | Nevada desert (includes Area 69) |

Total map area: ~36 km²  (4x GTA Vice City)

### Zone Locks

- Start: Los Santos only
- After "The Green Sabre": + Countryside + San Fierro
- After "Yay Ka-Boom-Boom": + Desert + Las Venturas (full map)
- Penalty for early entry: automatic 4-star wanted level

### Day/Night Cycle

- 1 real second = 1 game minute
- Full day cycle = 24 real minutes
- Days of the week tracked

### Weather

| Type | Regions | Gameplay Effect |
|------|---------|-----------------|
| Sunny | All | Default |
| Cloudy | All | Most frequent |
| Rain | LS, SF, countryside | Reduced NPC/traffic density |
| Fog | Countryside, SF | Heavy visibility reduction |
| Sandstorm | Bone County / LV desert only | Near-zero visibility; blows aircraft off course; no police helis |

---

## 11. Side Activities & Minigames

### Gambling (Las Venturas)

Blackjack, Video Poker, Roulette, Slots, Horse Racing  
Gambling Skill tiers: Gambler ($1K max) → Professional ($10K) → Hi-Roller ($100K) → Whale ($1M)

### Vehicle Missions

| Mission | Vehicle | Completion | Reward |
|---------|---------|-----------|--------|
| Vigilante | Police vehicles | Level 12 | +50% max armor |
| Paramedic | Ambulance | Level 12 | +50% max health |
| Firefighter | Firetruck | Level 12 | Fireproof (on foot) |
| Taxi | Taxi/Cabbie | 50 fares | Nitrous + jump for all taxis |
| Pimping | Broadway | 10 levels | Prostitutes pay CJ |
| Freight | Train | 2 levels | $50,000 |

### Sports & Rhythm

- **Pool:** 8-ball in bars; bet money
- **Basketball:** 2-on-2 near Sweet's house
- **Dancing:** Rhythm game at clubs (directional arrows to music)
- **Lowrider Challenge:** Hydraulic bounce-to-beat at El Corona

### Burglar Missions

- Night only (20:00–06:00); use black Boxville van
- Stealth with noise meter; resident wakes if bar fills → 3-star wanted
- Reward formula: $20 × n² (n = items stolen that session)
- $10,000 cumulative earnings → infinite stamina

### Races

- ~25 street races (LS, SF, LV)
- 6 air races (checkpoints, no opponents)
- Mount Chiliad downhill (bicycle)
- Stadium events: Bloodring (demo derby), 8-Track (stock cars), Dirt Track (ATVs)
- Triathlons (multi-stage run/swim/cycle)

### Other

- BMX Challenge, NRG-500 Challenge (motorcycle)
- Ammu-Nation Shooting Range (skill progression)
- Gym workouts (stat improvement)

---

## 12. Property System

### Safehouses (37 total)

- 8 free (story-unlocked)
- 29 purchasable ($6,000–$120,000; total $879,000)
- Provide: save point, wardrobe, garage

### Asset Businesses (10)

| Asset | Cost | Daily Income |
|-------|------|--------------|
| Verdant Meadows Airfield | $80,000 | $10,000 |
| Wang Cars | $50,000 | $8,000 |
| Zero RC Shop | $30,000 | $5,000 |
| Johnson House | Free | $10,000 |
| 6 others (story-unlocked) | Free | $2,000 each |

Each asset requires completing associated missions before generating income.

---

## 13. Collectibles

| Type | Count | Region | Reward |
|------|-------|--------|--------|
| Spray Tags | 100 | Los Santos | Weapons spawn at Johnson House; gang upgrades |
| Photo Ops | 50 | San Fierro | $100,000 + weapons at Doherty Garage |
| Horseshoes | 50 | Las Venturas | $100,000 + weapons at Four Dragons; luck boost |
| Oysters | 50 | Entire map (underwater) | $100,000 + infinite lung capacity + bypass sex appeal requirements |
| Unique Stunt Jumps | 70 | Entire map | — |

---

## 14. Mission System

### Scale

- ~100 story missions (86 mandatory + 14 optional)
- Longest GTA campaign at time of release

### Structure

- Triggered by walking into colored map markers at mission-giver locations
- Each giver has a unique icon letter (S = Sweet, BS = Big Smoke, etc.)
- Failure returns to open world; must drive back to marker to retry
- Original: no mid-mission checkpoints; Definitive Edition added some

### Mission Types

On-foot combat, driving/chasing, stealth infiltration, flying, RC vehicles, escort/protect, timed objectives, scripted setpieces, multi-stage (shift between combat and driving mid-mission)

### Story Arc (4 acts)

1. **Los Santos:** CJ returns for mother's funeral; rebuilds Grove Street Families; Big Smoke & Ryder's betrayal revealed; Sweet arrested; CJ exiled
2. **Countryside & San Fierro:** Robberies with Catalina; dismantles Loco Syndicate drug operation; builds legitimate businesses; Toreno's CIA blackmail begins
3. **Desert & Las Venturas:** CIA ops from airstrip; casino heist with Woozie's Triads; establishes power
4. **Return to Los Santos:** Sweet released; reclaim Grove Street; riots erupt; Big Smoke killed in crack palace; Tenpenny dies in pursuit crash

### Phone Calls

- 7 independent call threads checking prerequisites at 90–1000 ms intervals
- Unlock markers, deliver exposition, set up next steps
- High-priority: 20 s recall; low-priority: 60 s recall
- Only received when not on mission and not in interior

---

## 15. Girlfriend System

### Six Girlfriends

| Name | Location | Body Requirement | 100% Reward |
|------|----------|-----------------|-------------|
| Denise Robinson | Ganton, LS | None (story) | Pimp Suit |
| Millie Perkins | Prickle Pine, LV | None (story) | Casino keycard (at 35%) |
| Helena Wankstein | Red County | Low fat, low muscle | Rural Clothes |
| Katie Zhan | SF | High muscle | Medic Outfit + keep weapons on death |
| Michelle Cannes | SF | Average | Racing Suit |
| Barbara Schternvart | El Quebrados | Overweight | Cop Outfit + keep weapons on arrest |

### Mechanics

- Date types: restaurant, dancing, driving
- Successful date: +5% relationship; +10% with special interaction
- At 35–50%: girlfriend's car available
- At 100%: unique outfit
- Body requirements bypassed by max sex appeal OR all 50 oysters
- Only Millie required for story (casino heist keycard)

---

## 16. Schools

| School | Location | Lessons | Bronze Reward | Gold Reward |
|--------|----------|---------|---------------|-------------|
| Driving School | Doherty, SF | 12 | Super GT | Hotknife |
| Pilot School | Verdant Meadows | 10 | Rustler + Pilot's License | Hunter |
| Boat School | Bayside Marina, SF | 5 | Marquis | Jetmax |
| Bike School | Blackfield, LV | 6 | Freeway | NRG-500 |

Medal thresholds: Bronze 70–84%, Silver 85–99%, Gold 100%

---

## 17. Import/Export

- Located at Easter Basin Docks, SF
- 3 sequential lists of 10 vehicles each (30 total)
- Payout scales with vehicle condition (damage reduces value)
- Completion bonuses: $50K / $100K / $200K per list
- Total potential earnings: ~$1.44M
- Exported vehicles become purchasable on a day-of-week rotation

---

## References

### Wikis
- [GTA Wiki (Fandom)](https://gta.fandom.com/wiki/Grand_Theft_Auto:_San_Andreas)
- [Grand Theft Wiki](https://www.grandtheftwiki.com/Grand_Theft_Auto:_San_Andreas)
- [GTAMods Wiki](https://gtamods.com/wiki/Main_Page)

### Guides & Databases
- [GTABase — San Andreas](https://www.gtabase.com/gta-san-andreas/)
- [GTA-SanAndreas.com](https://www.gta-sanandreas.com/)
- [StrategyWiki — GTA:SA](https://strategywiki.org/wiki/Grand_Theft_Auto:_San_Andreas)
- [GTA.cz English — San Andreas](https://www.gta.cz/eng/san-andreas/)

### Technical / Speedrunning
- [SDA Knowledge Base — Game Mechanics & Glitches](https://kb.speeddemosarchive.com/Grand_Theft_Auto:_San_Andreas/Game_Mechanics_and_Glitches)
- [Multi Theft Auto Wiki — Weapon Skill Levels](https://wiki.multitheftauto.com/wiki/Weapon_skill_levels)
- [GTAMods — weapon.dat](https://gtamods.com/wiki/Weapon.dat)
- [open.mp — Vehicle Health](https://open.mp/docs/scripting/resources/vehiclehealth)

### Community
- [GTAForums](https://gtaforums.com/)
- [Steam Community Guides](https://steamcommunity.com/app/12120/guides/)
- [GameFAQs — Stats FAQ](https://gamefaqs.gamespot.com/pc/924362-grand-theft-auto-san-andreas/faqs/47686)

### General
- [Wikipedia — Grand Theft Auto: San Andreas](https://en.wikipedia.org/wiki/Grand_Theft_Auto:_San_Andreas)
