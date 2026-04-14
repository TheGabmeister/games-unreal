# Quake — HUD Layout

The HUD is pure Slate (no UMG, no BP). `AQuakeHUD` constructs `SQuakeHUDOverlay` in `BeginPlay` and adds it to the player's viewport via `GameViewport->AddViewportWidgetForPlayer`. The overlay caches a weak `APlayerController` pointer and re-resolves the pawn on every paint so it survives death/respawn/teleport.

## Wireframe (1080p reference)

```
+------------------------------------------------------------------+
|  Speed:   412                                       [SILVER][GOLD]|  Debug speedo (top-left) — Phase 1 diagnostic.
|  Z vel:    10                                                    |  Remove once strafe-jump tuning is final.
|  Mode :  Falling                     Kills    12 / 20            |  Stats strip (top-right).
|                                      Secrets   2 /  3            |
|                                      Time    01:42               |
|                                                                  |
|                          QUAD  18                                |  Powerup timer (top-center, visible only while active).
|                                                                  |
|                                                                  |
|                              .                                   |  Crosshair (center).
|                                                                  |
|                                                                  |
|                                                                  |
|                                                                  |
|  Shotgun                                                         |  Weapon name (bottom-left).
|  HP 87          AR 100                             15            |  HP / armor (L) + ammo (R).
+------------------------------------------------------------------+
```

**Level-end screen.** Full-viewport dark overlay (70% alpha) with centered vertical stack: "Level Complete" title + Kills / Secrets / Time / Deaths lines. Shown for `StatsDisplaySeconds` (default 5) via `AQuakeHUD::ShowLevelEndStats`.

**Transient message.** `AHUD::DrawHUD` paints a centered yellow line near the top for the `ShowMessage` duration (replaces any prior — "latest wins," no queue). Used by `_Message`, `_Secret` ("A secret area!"), locked doors ("You need the silver key."), gated exits ("All enemies must be cleared before exiting.").

## Elements

| Element          | Position        | Color                                      | Visibility            |
|------------------|-----------------|--------------------------------------------|-----------------------|
| HP               | Bottom-left     | `(1.0, 0.85, 0.4)` amber                    | Always                |
| Armor            | Bottom-left, right of HP | Green / yellow / red tinted by absorption tier | When `Armor > 0` |
| Silver key       | Bottom-left (above weapon name) | `(0.8, 0.8, 0.85)` light gray   | When held             |
| Gold key         | Bottom-left (above weapon name) | `(1.0, 0.84, 0.0)` gold          | When held             |
| Current weapon   | Bottom-left (above HP)     | `(0.8, 0.8, 0.8)` gray         | Always when equipped  |
| Ammo             | Bottom-right    | `(0.6, 0.9, 1.0)` cyan                      | When weapon has ammo type |
| Quad timer       | Top-center      | `(0.4, 0.6, 1.0)` blue                      | While Quad active, `ceil(remaining)` |
| Stats strip      | Top-right       | White, smaller font                         | Always                |
| Debug speedo     | Top-left        | White                                       | Always in dev builds (remove post-Phase 1) |
| Transient message | Top-center, below Quad | Yellow                              | While message active  |
| Level-end screen | Full viewport   | Dark background, amber title + white body   | After exit trigger    |
| Crosshair        | Center          | White dot                                   | Always (v2 polish)    |

## Data Sources

The overlay reads from three owners every paint — no explicit refresh needed.

| Field          | Source                                                          |
|----------------|-----------------------------------------------------------------|
| HP             | `AQuakeCharacter::GetHealth()`                                   |
| Armor value    | `UQuakeInventoryComponent::GetArmor()` (on the player pawn)      |
| Armor tier     | `UQuakeInventoryComponent::GetArmorAbsorption()` → color via threshold |
| Weapon name    | `AQuakeCharacter::CurrentWeapon->DisplayName`                    |
| Ammo           | `Character->GetAmmo(Weapon->AmmoType)`                           |
| Keys           | `AQuakePlayerState::HasKey(EQuakeKeyColor)`                      |
| Powerup timer  | `AQuakePlayerState::GetPowerupRemaining(EQuakePowerup)`          |
| Kills / Total  | `AQuakePlayerState::Kills` / `AQuakeGameMode::KillsTotal`        |
| Secrets / Total | `AQuakePlayerState::Secrets` / `AQuakeGameMode::SecretsTotal`   |
| Time           | `AQuakePlayerState::GetTimeElapsed()` (from world time)          |
| Deaths         | `AQuakePlayerState::Deaths` (end-screen only)                    |

All reads go through resolved weak pointers; the widget is stateless.

## Post-Process Effects

Via `UMaterialInstanceDynamic` applied to the level's `PostProcessVolume` (one per map, Unbound, low priority). Not part of the Slate overlay.

| Effect               | Driver                                 | Behavior                                |
|----------------------|----------------------------------------|-----------------------------------------|
| Damage flash         | `AQuakeCharacter::TriggerDamageFlash(intensity)` on `TakeDamage` (non-`bSuppressesPain` types) | Red flash; intensity = `damage / 50` clamped to [0,1] |
| Quad tint            | While `HasPowerup(Quad)`                | Blue tint                               |
| Pentagram tint       | While `HasPowerup(Pentagram)` (v2)     | Red tint                                |
| Ring tint            | While `HasPowerup(Ring)` (v2)          | Yellow transparency                     |
| Biosuit tint         | While `HasPowerup(Biosuit)` (v2)       | Green tint                              |
| Pickup flash         | On pickup overlap                       | Brief gold tint                         |

## v1 vs v2

**v1 ships:** HP, armor, current weapon name + ammo, silver/gold keys, Quad timer, stats strip, transient message, level-end screen, crosshair (simple), debug speedo.

**Deferred to v2+:**
- Weapon bar (icons for all 8 slots, highlight active) — needs icon assets.
- Powerup icons (currently text labels).
- Key icons (currently `[SILVER]` / `[GOLD]` text).
- Pentagram / Ring / Biosuit timers (powerups deferred past v1).
