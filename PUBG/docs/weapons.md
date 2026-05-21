# PUBG: Battlegrounds — Weapon Data Tables

Companion to [SPEC.md](../SPEC.md) §6. All values are community-datamined (patch ~41.1, 2026). KRAFTON does not publish official stats; minor source discrepancies are noted where they exist.

---

## Damage Model

### Damage Formula

```
Final Damage = Base Damage × Hit Area Multiplier × Weapon Class Multiplier × Range Modifier × (1 − Armor Reduction)
```

### Hit Area Multipliers

| Body Zone | Multiplier |
|-----------|-----------|
| Head | 2.0× |
| Neck | 1.5× |
| Clavicles | 1.0× |
| Upper Chest | 1.0× |
| Lower Chest | 1.0× |
| Stomach | 1.0× |
| Pelvis | 0.9× |
| Upper Limb (arms/thighs) | 0.6× |
| Lower Limb (forearms/shins) | 0.5× |
| Hands / Feet | 0.3× |

### Weapon Class Area Multipliers (stacked on top of hit area)

| Weapon Class | Head | Torso | Upper Limbs | Lower Limbs |
|-------------|------|-------|-------------|-------------|
| Sniper Rifles | 2.50× | 1.10× | 0.95× | 0.95× |
| DMRs | 2.35× | 1.05× | 0.95× | 0.95× |
| Assault Rifles | 2.35× | 1.00× | 0.95× | 0.95× |
| LMGs | 2.35× | 1.00× | 0.95× | 0.95× |
| SMGs | 1.80× | 1.05× | 1.30× | 1.30× |
| Pistols | 2.10× | 1.00× | 0.95× | 0.95× |
| Shotguns | 1.50× | 1.00× | 1.20× | 1.20× |

Effective headshot multiplier = Hit Area Head (2.0) × Class multiplier. Example: AR headshot = 2.0 × 2.35 = 4.7× base damage. SR headshot = 2.0 × 2.50 = 5.0× base damage.

---

## Armor

### Helmets

| Item | Damage Reduction | Durability | Coverage |
|------|-----------------|------------|----------|
| Level 1 Helmet | 30% | 80 | Top, back |
| Level 2 Helmet | 40% | 150 | Top, back, sides |
| Level 3 Helmet (Spetsnaz) | 55% | 230 | Full head + face |

Level 3 Helmet is crate-exclusive. Only helmet protecting the face.

### Vests

| Item | Damage Reduction | Durability | Coverage |
|------|-----------------|------------|----------|
| Level 1 Vest | 30% | 150 | Chest, back |
| Level 2 Vest (Police) | 40% | 200 | Chest, back |
| Level 3 Vest (Military) | 55% | 250 | Chest, back |

- Armor does NOT protect arms, legs, hands, or feet.
- Durability does NOT affect reduction percentage — a vest at 1 durability blocks the same % as a fresh one.
- Durability lost per hit = damage absorbed.
- Frag grenades are mitigated by vests.

---

## Assault Rifles

| Weapon | Ammo | Base Dmg | RPM | Bullet Vel (m/s) | Mag (Base/Ext) | Spawn | Fire Modes |
|--------|------|----------|-----|-------------------|----------------|-------|------------|
| AKM | 7.62mm | 48 | 600 | 710 | 30/40 | World | Single, Auto |
| Beryl M762 | 7.62mm | 44 | 698 | 735 | 30/40 | World | Single, Burst (3), Auto |
| ACE32 | 7.62mm | 43 | 682 | 715 | 30/40 | World | Single, Auto |
| Groza | 7.62mm | 47 | 750 | 710 | 30/40 | Crate | Single, Auto |
| Mk47 Mutant | 7.62mm | 49 | 800 | 774 | 30/40 | World | Single, Burst (2) |
| M16A4 | 5.56mm | 43 | 800 | 904 | 30/40 | World | Single, Burst (3) |
| M416 | 5.56mm | 40 | 698 | 875 | 30/40 | World | Single, Auto |
| SCAR-L | 5.56mm | 42 | 652 | 865 | 30/40 | World | Single, Auto |
| QBZ95 | 5.56mm | 42 | 652 | 924 | 30/40 | World | Single, Auto |
| AUG A3 | 5.56mm | 41 | 700 | 884 | 30/40 | Crate | Single, Auto |
| G36C | 5.56mm | 41 | 698 | 875 | 30/40 | World (Vikendi) | Single, Auto |
| K2 | 5.56mm | 41 | 698 | 874 | 30/40 | World | Single, Auto |
| FAMAS | 5.56mm | 39 | 900 | 920 | 30/40 | World | Single, Burst (3), Auto |

---

## DMRs (Designated Marksman Rifles)

| Weapon | Ammo | Base Dmg | RPM | Bullet Vel (m/s) | Mag (Base/Ext) | Spawn | Fire Modes |
|--------|------|----------|-----|-------------------|----------------|-------|------------|
| Mini 14 | 5.56mm | 46 | 600 | 990 | 20/30 | World | Single |
| QBU | 5.56mm | 48 | 600 | 945 | 10/20 | World (Sanhok) | Single |
| Mk12 | 5.56mm | 50 | 600 | 896 | 20/30 | World | Single, Auto |
| SKS | 7.62mm | 53 | 600 | 800 | 10/20 | World | Single |
| SLR | 7.62mm | 57 | 600 | 840 | 10/20 | World | Single |
| Dragunov (SVD) | 7.62mm | 60 | 182 | 826 | 10/20 | World | Single |
| Mk14 EBR | 7.62mm | 61 | 667 | 853 | 10/20 | Crate | Single, Auto |
| VSS Vintorez | 9mm | 41 | 698 | 330 | 10/20 | World | Single, Auto |

VSS has a built-in suppressor and 4× PSO-1 scope.

---

## Sniper Rifles

| Weapon | Ammo | Base Dmg | Cycle Time (s) | Bullet Vel (m/s) | Mag (Base/Ext) | Spawn | Zeroing |
|--------|------|----------|----------------|-------------------|----------------|-------|---------|
| Win94 | .45 ACP | 66 | 0.60 | 760 | 8/— | World (Miramar) | 100–500m |
| Kar98k | 7.62mm | 79 | 1.90 | 777 | 5/— | World | 100–500m |
| Mosin-Nagant | 7.62mm | 79 | 1.90 | 777 | 5/— | World | 100–500m |
| M24 | 7.62mm | 79 | 1.80 | 811 | 5/7 | World | 100–600m |
| AWM | .300 Magnum | 120 | 1.85 | 945 | 5/— | Crate | 100–600m |
| Lynx AMR | .50 Cal | 118 | 0.70 | 1196 | 5/— | Crate | — |

The AWM one-shots through any helmet. The Lynx AMR can damage vehicles and penetrate some cover.

---

## SMGs (Submachine Guns)

| Weapon | Ammo | Base Dmg | RPM | Bullet Vel (m/s) | Mag (Base/Ext) | Spawn | Fire Modes |
|--------|------|----------|-----|-------------------|----------------|-------|------------|
| Micro UZI | 9mm | 26 | 1250 | 350 | 25/35 | World | Single, Auto |
| Vector | 9mm | 31 | 1111 | 376 | 19/33 | World | Single, Auto |
| MP9 | 9mm | 31 | 1000 | 395 | 19/33 | World | Single, Auto |
| JS9 | 9mm | 32 | 900 | 399 | 30/40 | World | Single, Auto |
| MP5K | 9mm | 33 | 900 | 376 | 30/40 | World (Vikendi) | Single, Auto |
| P90 | 5.7mm | 35 | 1000 | 707 | 50/— | Crate | Single, Auto |
| PP-19 Bizon | 9mm | 36 | 698 | 376 | 53/— | World | Single, Auto |
| UMP45 | .45 ACP | 41 | 667 | 358 | 25/35 | World | Single, Burst (3), Auto |
| Tommy Gun | .45 ACP | 40 | 750 | 279 | 30/50 | World | Single, Auto |

---

## Shotguns

| Weapon | Ammo | Dmg/Pellet | Pellets | Total Dmg | RPM | Bullet Vel (m/s) | Mag | Spawn |
|--------|------|-----------|---------|-----------|-----|-------------------|-----|-------|
| S686 | 12 Gauge | 23 | 9 | 207 | 600 | 356 | 2 | World |
| S1897 | 12 Gauge | 23 | 9 | 207 | 103 | 356 | 5 | World |
| S12K | 12 Gauge | 19 | 9 | 171 | 375 | 356 | 5 (8) | World |
| DBS | 12 Gauge | 24 | 9 | 216 | 375 | 356 | 14 | Crate |
| Sawed-Off | 12 Gauge | 21 | 8 | 168 | 240 | 356 | 2 | World |
| O12 | 12 Gauge Slug | 100 | 1 (slug) | 100 | 480 | 625 | 5 | World |

---

## LMGs (Light Machine Guns)

| Weapon | Ammo | Base Dmg | RPM | Bullet Vel (m/s) | Mag | Spawn |
|--------|------|----------|-----|-------------------|-----|-------|
| DP-28 | 7.62mm | 52 | 550 | 836 | 47 | World |
| M249 | 5.56mm | 41 | 800 | 909 | 75 (150) | World/Crate |
| MG3 | 7.62mm | 42 | 660/990 | 816 | 75 | Crate |

MG3 has two selectable fire rates (660 RPM and 990 RPM).

---

## Pistols

| Weapon | Ammo | Base Dmg | RPM | Bullet Vel (m/s) | Mag (Base/Ext) | Fire Modes |
|--------|------|----------|-----|-------------------|----------------|------------|
| P92 | 9mm | 34 | 1200 | 380 | 15/20 | Single |
| P18C | 9mm | 23 | 1000 | 375 | 17/25 | Single, Auto |
| Skorpion | 9mm | 22 | 857 | 350 | 20/40 | Single, Auto |
| P1911 | .45 ACP | 42 | 1091 | 250 | 7/12 | Single |
| R1895 | 7.62mm | 64 | 300 | 330 | 7/— | Single |
| R45 | .45 ACP | 65 | 240 | 330 | 6/— | Single |
| Deagle | .45 ACP | 62 | 300 | 450 | 7/10 | Single |

---

## Crossbow

| Stat | Value |
|------|-------|
| Ammo | Bolts |
| Base Damage | 105 |
| Bolt Velocity | 160 m/s |
| Reload Time | ~3.5s |
| Magazine | 1 (single bolt) |
| Attachments | Red Dot, Holo, Quiver |

One-shot headshot kill through any helmet.

---

## Melee Weapons

| Weapon | Body Damage | Head Damage | Notes |
|--------|------------|-------------|-------|
| Pan | 80 | 200 | Blocks bullets when holstered on back |
| Machete | 60 | 150 | |
| Crowbar | 60 | 90 | |
| Sickle | 60 | 150 | |
| Punch (fist) | 18 | ~38 | Jump-punch = 38 |

---

## Throwables

| Throwable | Damage | Radius | Fuse Time | Cookable | Duration | Weight |
|-----------|--------|--------|-----------|----------|----------|--------|
| Frag Grenade | 225 max | 10m lethal | 5s | Yes | Instant | 18 |
| Smoke Grenade | 0 | 10m obscure | 1s after landing | No | ~15s | 16 |
| Stun Grenade | 0 (blind/deaf) | 5m strong / 20m max | 2.5s fuse | Yes | 1–5.5s effect | 14 |
| Molotov Cocktail | 12.5 dps (ground) + 10 dps (burn) | 10m fire spread | Impact | No | 10s fire | 12 |

- Frag: 20% less damage to prone targets; mitigated by vests.
- Stun: duration scales with distance from blast.
- Molotov: ignited players burn for 4s after leaving fire.
- Throw time: 1.30s (grenades), 0.85s (Molotov). Max throw distance: ~100m.

---

## Ammo Types

| Ammo | Weapons | Weight/Round |
|------|---------|-------------|
| 5.56mm | M416, SCAR-L, M16A4, QBZ, AUG, G36C, K2, FAMAS, Mini14, QBU, Mk12, M249 | 0.5 |
| 7.62mm | AKM, Beryl, ACE32, Groza, Mk47, Kar98k, Mosin, M24, SKS, SLR, Dragunov, Mk14, DP-28, MG3, R1895 | 0.7 |
| 9mm | Micro UZI, Vector, MP9, JS9, MP5K, PP-19, P92, P18C, Skorpion, VSS | 0.375 |
| .45 ACP | UMP45, Tommy Gun, P1911, Deagle, R45, Win94 | 0.4 |
| .300 Magnum | AWM | 1.0 |
| .50 Cal | Lynx AMR | — |
| 5.7mm | P90 | — |
| 12 Gauge | S686, S1897, S12K, DBS, Sawed-Off | 1.25 |
| 12 Gauge Slug | O12 | — |
| Bolts | Crossbow | — |

---

## Ballistics

All weapons fire physical projectiles with travel time and gravity drop (no hitscan). Each projectile has per-weapon velocity, gravity, and a drag coefficient that governs velocity loss over distance.

### Bullet Drop Example (M416, 875 m/s, zeroed 100m)

| Distance | Drop | Travel Time |
|----------|------|-------------|
| 100m | zeroed | 114 ms |
| 200m | 25 cm | 228 ms |
| 300m | 57 cm | 341 ms |
| 500m | 158 cm | 568 ms |
| 1000m | 633 cm | 1136 ms |

### Effective Range by Category

| Weapon Class | Effective Range | Drag Coefficient |
|-------------|-----------------|-----------------|
| SMGs | 150–250m | 0.65–0.68 |
| Shotguns | 50–80m | varies |
| ARs (5.56) | 400–500m | 0.81–0.95 |
| ARs (7.62) | 400–500m | 0.71–0.90 |
| DMRs | 500–1000m | 0.71–0.96 |
| Sniper Rifles | 500–650m | 0.80–0.98 |
| LMGs | 400–500m | 0.75–0.90 |

Higher drag coefficient = less velocity loss = longer effective range.

---

## Attachments

### Muzzle

| Attachment | Vert Recoil | Horiz Recoil | Pattern Scale | Muzzle Flash | Sound | Fits |
|-----------|------------|-------------|--------------|-------------|-------|------|
| Compensator (AR/SMG) | −15% (AR) / −25% (SMG) | −10% (AR) / −20% (SMG) | −25% | — | — | ARs, SMGs |
| Compensator (SR/DMR) | −20% | −20% | −25% | — | — | SRs, DMRs |
| Flash Hider (AR/SMG) | −10% | −10% | −10% | Eliminated | — | ARs, SMGs |
| Flash Hider (SR/DMR) | −10% | −10% | −10% | Eliminated | — | SRs, DMRs |
| Suppressor (AR/SMG) | — | — | — | Eliminated | Greatly reduced | ARs, SMGs |
| Suppressor (SR/DMR) | — | — | — | Eliminated | Greatly reduced | SRs, DMRs |
| Suppressor (Pistol) | — | — | — | Eliminated | Greatly reduced | Pistols |
| Duckbill | — | — | — | — | — | S1897, S12K |
| Choke | — | — | — | — | — | S1897, S12K |

Duckbill widens horizontal pellet spread and narrows vertical. Choke tightens overall spread.

### Grips

| Grip | Vert Recoil | Horiz Recoil | Recoil Recovery | ADS Speed |
|------|------------|-------------|-----------------|-----------|
| Vertical Foregrip | −25 to −30% | Minor | — | — |
| Angled Foregrip | −15% | −20% | — | +10% |
| Half Grip | −30% | −20% | Slower | — |
| Thumb Grip | −15% | — | +10% | +15% |
| Light Grip | — | −15% | +20% | — |
| Laser Sight | — | — | — | — |

Laser Sight reduces hipfire spread instead of ADS recoil.

### Magazines

| Magazine | Effect |
|---------|--------|
| Extended Mag | +10 rounds (AR/DMR/SMG/Pistol), +2–3 rounds (SR) |
| Quickdraw Mag | −30% reload (AR/DMR/SR), −60% reload (SMG/Pistol) |
| Ext. Quickdraw Mag | Both effects combined |

### Stocks

| Stock | Effect | Fits |
|-------|--------|------|
| Tactical Stock | −20% pattern, −10% vert recoil, +10% ADS | M416, Vector, MP5K, Mk47, M16A4, M249 |
| Cheek Pad | −20% kick, −15% vert recoil, −10% sway | AWM, Kar98k, M24, Mk14, SKS, SLR, VSS |
| Bullet Loops | +20–30% reload speed | Kar98k, S1897, Win94, S686 |
| Stock (UZI) | −20% horiz recoil | Micro UZI, Skorpion |

### Scopes / Sights

| Sight | Magnification | Zeroing | Notes |
|-------|--------------|---------|-------|
| Red Dot Sight | 1× | No | Universal |
| Holographic Sight | 1× | No | Circle reticle |
| Canted Sight | 1× | No | Secondary slot; quick-toggle |
| 2× Aimpoint | 2× | No | |
| 3× Scope | 3× | No | |
| 4× ACOG | 4× | 100–400m | Range-finding reticle |
| 6× Scope | 3–6× variable | 100–600m | Adjustable zoom |
| 8× CQBSS | 8× | 100–1000m | DMRs, SRs only |
| 15× PM II | 15× | 100–1000m | Crate only; SRs, DMRs only |

Weapons marked "Crate" in the Spawn column are only available from care packages (§1.4 in SPEC.md). M249 has alternated between world-spawn and crate-exclusive across patches.
