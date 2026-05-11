#pragma once

#include "Engine/EngineTypes.h"

namespace CBCollision
{
	constexpr ECollisionChannel Player = ECC_GameTraceChannel1;
	constexpr ECollisionChannel Pickup = ECC_GameTraceChannel2;
	constexpr ECollisionChannel Enemy  = ECC_GameTraceChannel3;
}
