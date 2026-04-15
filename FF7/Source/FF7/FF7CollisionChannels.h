// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * Named aliases for project-defined custom collision channels.
 *
 * Order of constants here MUST match the order of +DefaultChannelResponses
 * entries under [/Script/Engine.CollisionProfile] in Config/DefaultEngine.ini.
 * Adding a new channel: append a new constant below AND a new ini entry in
 * the same position. Never reorder — doing so silently remaps existing code
 * to the wrong channel.
 */
namespace FF7
{
	constexpr ECollisionChannel ECC_Interact = ECC_GameTraceChannel1;
}
