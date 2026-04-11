#include "QuakeDamageType_Explosive.h"

UQuakeDamageType_Explosive::UQuakeDamageType_Explosive()
{
	// SPEC section 1.5 damage type table:
	//   SelfDamageScale = 0.5 — rocket jumps are survivable
	//   KnockbackScale  = 4.0 — big launch impulse
	SelfDamageScale = 0.5f;
	KnockbackScale = 4.0f;
}
