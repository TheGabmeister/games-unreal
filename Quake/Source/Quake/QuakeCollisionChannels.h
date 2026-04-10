#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Quake custom collision channels.
 *
 * Indices MUST stay aligned with Config/DefaultEngine.ini
 * [/Script/Engine.CollisionProfile] section. Reordering or renaming a
 * channel rewrites the index, which silently re-maps every actor's saved
 * collision state — don't.
 *
 * Per CLAUDE.md "Architecture: Editor-Only Pieces", channels are declared
 * in DefaultEngine.ini and per-actor responses are set in C++ constructors.
 * This header is the C++-side mirror so call sites don't have to repeat the
 * raw ECC_GameTraceChannelN literal.
 *
 * See SPEC section 1.6 for the response matrix and per-system rules.
 */
namespace QuakeCollision
{
	// --- Object channels ---

	/** Pickups (overlap-only sphere). See SPEC 1.6 rule 4. */
	static constexpr ECollisionChannel ECC_Pickup     = ECC_GameTraceChannel1;

	/** Projectile actors (rocket / nail / grenade). */
	static constexpr ECollisionChannel ECC_Projectile = ECC_GameTraceChannel2;

	/** Enemy capsule AFTER the 2 s post-death channel flip. SPEC 1.6 rule 2. */
	static constexpr ECollisionChannel ECC_Corpse     = ECC_GameTraceChannel3;

	// --- Trace channels ---

	/** Hitscan (Shotgun, SSG, Thunderbolt) and the Axe melee trace. SPEC 1.6 rule 9. */
	static constexpr ECollisionChannel ECC_Weapon     = ECC_GameTraceChannel4;
}
