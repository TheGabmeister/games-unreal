#include "DiabloDungeonGenerator.h"

#include "Diablo.h"
#include "DiabloEnemy.h"
#include "DiabloGameInstance.h"
#include "DiabloGameMode.h"
#include "DungeonStairs.h"
#include "TilePalette.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"

ADiabloDungeonGenerator::ADiabloDungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	StairsClass = ADungeonStairs::StaticClass();
}

void ADiabloDungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
	Generate();
}

void ADiabloDungeonGenerator::Generate()
{
	if (bGenerated)
	{
		return;
	}
	bGenerated = true;

	ResolveFloorSettings();

	int32 Seed = 0;
	if (ADiabloGameMode* GameMode = GetWorld()->GetAuthGameMode<ADiabloGameMode>())
	{
		Seed = GameMode->GetSeedForDungeonFloor(DungeonFloorName);
	}
	if (Seed == 0)
	{
		Seed = FMath::Rand();
	}

	FRandomStream Stream(Seed);
	BuildLayout(Stream);
	SpawnTiles();
	SpawnGameplayActors(Stream);
	RefreshNavigation();

	UE_LOG(LogDiablo, Display, TEXT("Generated %s with seed %d (%d rooms, %d spawned actors)"),
		*DungeonFloorName.ToString(), Seed, Rooms.Num(), SpawnedActors.Num());
}

void ADiabloDungeonGenerator::BuildLayout(FRandomStream& Stream)
{
	Grid.Init(EDungeonTileType::Empty, GridWidth * GridHeight);
	Rooms.Reset();

	const int32 StartW = 6;
	const int32 StartH = 6;
	const int32 StartX = (GridWidth - StartW) / 2;
	const int32 StartY = (GridHeight - StartH) / 2;

	FGridRoom StartRoom{ StartX, StartY, StartW, StartH };
	CarveRect(StartRoom.X, StartRoom.Y, StartRoom.W, StartRoom.H);
	Rooms.Add(StartRoom);

	const int32 MaxAttempts = TargetRoomCount * 40;
	for (int32 Attempt = 0; Attempt < MaxAttempts && Rooms.Num() < TargetRoomCount; ++Attempt)
	{
		const FGridRoom& BaseRoom = Rooms[Stream.RandRange(0, Rooms.Num() - 1)];
		TryBudRoom(BaseRoom, Stream);
	}

	AddBorderWalls();
}

bool ADiabloDungeonGenerator::TryBudRoom(const FGridRoom& BaseRoom, FRandomStream& Stream)
{
	const int32 Side = Stream.RandRange(0, 3);
	const int32 CorridorLength = Stream.RandRange(2, 6);
	const int32 CorridorWidth = 2;
	const int32 NewW = Stream.RandRange(4, 8);
	const int32 NewH = Stream.RandRange(4, 8);

	int32 CorridorX = 0;
	int32 CorridorY = 0;
	int32 CorridorW = 0;
	int32 CorridorH = 0;
	int32 RoomX = 0;
	int32 RoomY = 0;

	if (Side == 0)
	{
		const int32 DoorX = Stream.RandRange(BaseRoom.X + 1, BaseRoom.X + BaseRoom.W - 2);
		CorridorX = DoorX;
		CorridorY = BaseRoom.Y - CorridorLength;
		CorridorW = CorridorWidth;
		CorridorH = CorridorLength;
		RoomX = DoorX - NewW / 2;
		RoomY = CorridorY - NewH;
	}
	else if (Side == 1)
	{
		const int32 DoorY = Stream.RandRange(BaseRoom.Y + 1, BaseRoom.Y + BaseRoom.H - 2);
		CorridorX = BaseRoom.X + BaseRoom.W;
		CorridorY = DoorY;
		CorridorW = CorridorLength;
		CorridorH = CorridorWidth;
		RoomX = CorridorX + CorridorLength;
		RoomY = DoorY - NewH / 2;
	}
	else if (Side == 2)
	{
		const int32 DoorX = Stream.RandRange(BaseRoom.X + 1, BaseRoom.X + BaseRoom.W - 2);
		CorridorX = DoorX;
		CorridorY = BaseRoom.Y + BaseRoom.H;
		CorridorW = CorridorWidth;
		CorridorH = CorridorLength;
		RoomX = DoorX - NewW / 2;
		RoomY = CorridorY + CorridorLength;
	}
	else
	{
		const int32 DoorY = Stream.RandRange(BaseRoom.Y + 1, BaseRoom.Y + BaseRoom.H - 2);
		CorridorX = BaseRoom.X - CorridorLength;
		CorridorY = DoorY;
		CorridorW = CorridorLength;
		CorridorH = CorridorWidth;
		RoomX = CorridorX - NewW;
		RoomY = DoorY - NewH / 2;
	}

	if (!CanCarveRect(CorridorX, CorridorY, CorridorW, CorridorH, 0) ||
		!CanCarveRect(RoomX, RoomY, NewW, NewH, 1))
	{
		return false;
	}

	CarveRect(CorridorX, CorridorY, CorridorW, CorridorH);
	CarveRect(RoomX, RoomY, NewW, NewH);
	Rooms.Add(FGridRoom{ RoomX, RoomY, NewW, NewH });
	return true;
}

bool ADiabloDungeonGenerator::CanCarveRect(int32 X, int32 Y, int32 W, int32 H, int32 Margin) const
{
	if (X - Margin < 1 || Y - Margin < 1 ||
		X + W + Margin >= GridWidth - 1 || Y + H + Margin >= GridHeight - 1)
	{
		return false;
	}

	for (int32 TileY = Y - Margin; TileY < Y + H + Margin; ++TileY)
	{
		for (int32 TileX = X - Margin; TileX < X + W + Margin; ++TileX)
		{
			if (Grid[GetIndex(TileX, TileY)] != EDungeonTileType::Empty)
			{
				return false;
			}
		}
	}

	return true;
}

void ADiabloDungeonGenerator::CarveRect(int32 X, int32 Y, int32 W, int32 H)
{
	for (int32 TileY = Y; TileY < Y + H; ++TileY)
	{
		for (int32 TileX = X; TileX < X + W; ++TileX)
		{
			if (IsInBounds(TileX, TileY))
			{
				Grid[GetIndex(TileX, TileY)] = EDungeonTileType::Floor;
			}
		}
	}
}

void ADiabloDungeonGenerator::AddBorderWalls()
{
	TArray<EDungeonTileType> WithWalls = Grid;

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			if (Grid[GetIndex(X, Y)] != EDungeonTileType::Empty)
			{
				continue;
			}

			bool bTouchesFloor = false;
			for (int32 OffsetY = -1; OffsetY <= 1 && !bTouchesFloor; ++OffsetY)
			{
				for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
				{
					if (OffsetX == 0 && OffsetY == 0)
					{
						continue;
					}

					const int32 NeighborX = X + OffsetX;
					const int32 NeighborY = Y + OffsetY;
					if (IsInBounds(NeighborX, NeighborY) &&
						Grid[GetIndex(NeighborX, NeighborY)] == EDungeonTileType::Floor)
					{
						bTouchesFloor = true;
						break;
					}
				}
			}

			if (bTouchesFloor)
			{
				WithWalls[GetIndex(X, Y)] = EDungeonTileType::Wall;
			}
		}
	}

	Grid = MoveTemp(WithWalls);
}

void ADiabloDungeonGenerator::SpawnTiles()
{
	const float TileSize = GetTileSize();
	const FVector DefaultFloorScale(TileSize / 100.f, TileSize / 100.f, 0.1f);
	const FVector DefaultWallScale(TileSize / 100.f, TileSize / 100.f, 2.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			const EDungeonTileType TileType = Grid[GetIndex(X, Y)];
			if (TileType == EDungeonTileType::Empty)
			{
				continue;
			}

			const FTilePaletteEntry* Entry = TilePalette ? TilePalette->FindEntry(TileType) : nullptr;
			UClass* ActorClass = Entry && Entry->ActorClass ? Entry->ActorClass.Get() : ADungeonTile::StaticClass();
			const FVector Scale = Entry ? Entry->Scale :
				(TileType == EDungeonTileType::Wall ? DefaultWallScale : DefaultFloorScale);
			const FVector Offset = Entry ? Entry->LocationOffset :
				(TileType == EDungeonTileType::Wall ? FVector(0.f, 0.f, 100.f) : FVector(0.f, 0.f, -5.f));

			AActor* TileActor = GetWorld()->SpawnActor<AActor>(
				ActorClass, FTransform(GetTileWorldLocation(X, Y) + Offset), SpawnParams);
			if (!TileActor)
			{
				continue;
			}

			TileActor->SetActorScale3D(Scale);
			if (ADungeonTile* DungeonTile = Cast<ADungeonTile>(TileActor))
			{
				DungeonTile->SetTileType(TileType);
				if (Entry && Entry->Material)
				{
					DungeonTile->MeshComponent->SetMaterial(0, Entry->Material);
				}
			}
			SpawnedActors.Add(TileActor);
		}
	}
}

void ADiabloDungeonGenerator::SpawnGameplayActors(FRandomStream& Stream)
{
	if (Rooms.Num() == 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UClass* StairsToSpawn = StairsClass ? StairsClass.Get() : ADungeonStairs::StaticClass();

	const FGridRoom& StartRoom = Rooms[0];
	const FIntPoint StartCenter = StartRoom.Center();
	ADungeonStairs* UpStairs = GetWorld()->SpawnActor<ADungeonStairs>(
		StairsToSpawn, FTransform(GetTileWorldLocation(StartCenter.X, StartCenter.Y, 50.f)), SpawnParams);
	if (UpStairs)
	{
		UpStairs->TargetLevelName = ReturnLevelName;
		UpStairs->TargetFloorIndex = FloorIndex > 1 ? FloorIndex - 1 : 0;
		UpStairs->SetActorScale3D(FVector(1.5f, 1.5f, 0.5f));
		SpawnedActors.Add(UpStairs);
	}

	if (FloorIndex < MAX_FLOOR && Rooms.Num() > 1)
	{
		const FGridRoom& ExitRoom = Rooms[Rooms.Num() - 1];
		const FIntPoint ExitCenter = ExitRoom.Center();
		ADungeonStairs* DownStairs = GetWorld()->SpawnActor<ADungeonStairs>(
			StairsToSpawn, FTransform(GetTileWorldLocation(ExitCenter.X, ExitCenter.Y, 50.f)), SpawnParams);
		if (DownStairs)
		{
			DownStairs->TargetLevelName = FName("Lvl_Dungeon");
			DownStairs->TargetFloorIndex = FloorIndex + 1;
			DownStairs->bIsDownStairs = true;
			DownStairs->SetActorScale3D(FVector(1.5f, 1.5f, 0.5f));
			SpawnedActors.Add(DownStairs);
		}
	}

	const int32 PotionRoomIdx = Rooms.Num() > 2 ? Rooms.Num() - 2 : Rooms.Num() - 1;
	if (HealingPotionClass && Rooms.Num() > 1)
	{
		const FGridRoom& PotionRoom = Rooms[PotionRoomIdx];
		const FIntPoint PotionCenter = PotionRoom.Center();
		AActor* Potion = GetWorld()->SpawnActor<AActor>(
			HealingPotionClass, FTransform(GetTileWorldLocation(PotionCenter.X, PotionCenter.Y, 100.f)), SpawnParams);
		if (Potion)
		{
			SpawnedActors.Add(Potion);
		}
	}

	if (!EnemyClass || Rooms.Num() <= 1)
	{
		return;
	}

	TArray<int32> CandidateRooms;
	for (int32 RoomIndex = 1; RoomIndex < Rooms.Num(); ++RoomIndex)
	{
		CandidateRooms.Add(RoomIndex);
	}

	for (int32 i = 0; i < CandidateRooms.Num(); ++i)
	{
		const int32 SwapIndex = Stream.RandRange(i, CandidateRooms.Num() - 1);
		CandidateRooms.Swap(i, SwapIndex);
	}

	const int32 EnemyCount = FMath::Min(TargetEnemyCount, CandidateRooms.Num());
	const EDiabloEnemyArchetype ArchetypeCycle[] = {
		EDiabloEnemyArchetype::MeleeGrunt,
		EDiabloEnemyArchetype::FastMelee,
		EDiabloEnemyArchetype::RangedArcher,
		EDiabloEnemyArchetype::FallenCoward,
		EDiabloEnemyArchetype::Spellcaster,
		EDiabloEnemyArchetype::Summoner,
	};

	for (int32 i = 0; i < EnemyCount; ++i)
	{
		const FGridRoom& Room = Rooms[CandidateRooms[i]];
		const int32 SpawnX = Stream.RandRange(Room.X + 1, Room.X + Room.W - 2);
		const int32 SpawnY = Stream.RandRange(Room.Y + 1, Room.Y + Room.H - 2);
		ADiabloEnemy* Enemy = GetWorld()->SpawnActor<ADiabloEnemy>(
			EnemyClass, FTransform(GetTileWorldLocation(SpawnX, SpawnY, 100.f)), SpawnParams);
		if (Enemy)
		{
			Enemy->ConfigureArchetype(ArchetypeCycle[i % UE_ARRAY_COUNT(ArchetypeCycle)]);
			Enemy->SummonClass = EnemyClass;
			ApplyFloorScaling(Enemy);
			SpawnedActors.Add(Enemy);
		}
	}
}

FName ADiabloDungeonGenerator::BiomeNameForFloor(int32 Floor)
{
	if (Floor <= 4)  return FName("Cathedral");
	if (Floor <= 8)  return FName("Catacombs");
	if (Floor <= 12) return FName("Caves");
	return FName("Hell");
}

int32 ADiabloDungeonGenerator::BiomeLocalFloor(int32 Floor)
{
	return ((Floor - 1) % 4) + 1;
}

void ADiabloDungeonGenerator::ResolveFloorSettings()
{
	FloorIndex = 1;
	if (UDiabloGameInstance* GI = Cast<UDiabloGameInstance>(GetGameInstance()))
	{
		FloorIndex = FMath::Clamp(GI->CurrentFloorIndex, 1, MAX_FLOOR);
	}

	const FName BiomeName = BiomeNameForFloor(FloorIndex);
	const int32 LocalFloor = BiomeLocalFloor(FloorIndex);

	DungeonFloorName = *FString::Printf(TEXT("%s_L%d"), *BiomeName.ToString(), LocalFloor);

	const FString PalettePath = FString::Printf(TEXT("/Game/Dungeons/TP_%s.TP_%s"),
		*BiomeName.ToString(), *BiomeName.ToString());
	UTilePalette* LoadedPalette = LoadObject<UTilePalette>(nullptr, *PalettePath);
	if (LoadedPalette)
	{
		TilePalette = LoadedPalette;
	}
	else
	{
		UE_LOG(LogDiablo, Warning, TEXT("Failed to load palette: %s — using default"), *PalettePath);
	}

	TargetRoomCount = FMath::Clamp(15 + (FloorIndex - 1) / 3, 15, 20);
	TargetEnemyCount = FMath::Clamp(8 + (FloorIndex - 1), 8, 23);
	ReturnLevelName = (FloorIndex <= 1) ? FName("Lvl_Diablo") : FName("Lvl_Dungeon");

	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		if (UDirectionalLightComponent* LC = It->FindComponentByClass<UDirectionalLightComponent>())
		{
			if (FloorIndex <= 4)
			{
				LC->SetIntensity(2.f);
				LC->SetLightColor(FLinearColor(1.f, 0.9f, 0.8f));
			}
			else if (FloorIndex <= 8)
			{
				LC->SetIntensity(1.5f);
				LC->SetLightColor(FLinearColor(0.7f, 0.7f, 0.9f));
			}
			else if (FloorIndex <= 12)
			{
				LC->SetIntensity(1.2f);
				LC->SetLightColor(FLinearColor(0.8f, 0.6f, 0.4f));
			}
			else
			{
				LC->SetIntensity(3.f);
				LC->SetLightColor(FLinearColor(1.f, 0.3f, 0.1f));
			}
			break;
		}
	}

	UE_LOG(LogDiablo, Display, TEXT("Floor %d: %s (biome: %s, rooms: %d, enemies: %d)"),
		FloorIndex, *DungeonFloorName.ToString(), *BiomeName.ToString(),
		TargetRoomCount, TargetEnemyCount);
}

void ADiabloDungeonGenerator::ApplyFloorScaling(ADiabloEnemy* Enemy) const
{
	if (!Enemy) return;

	const float Mult = 1.f + (FloorIndex - 1) * 0.2f;
	Enemy->Stats.MaxHP = FMath::RoundToFloat(Enemy->Stats.MaxHP * Mult);
	Enemy->Stats.HP = Enemy->Stats.MaxHP;
	Enemy->MonsterLevel = FloorIndex;
	Enemy->MonsterAC = 5.f + (FloorIndex - 1) * 3.f;
	Enemy->XPReward = FMath::RoundToInt64(static_cast<double>(Enemy->XPReward) * Mult);
	Enemy->ProjectileDamage *= Mult;
}

void ADiabloDungeonGenerator::RefreshNavigation()
{
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->Build();
	}
}

int32 ADiabloDungeonGenerator::GetIndex(int32 X, int32 Y) const
{
	return Y * GridWidth + X;
}

bool ADiabloDungeonGenerator::IsInBounds(int32 X, int32 Y) const
{
	return X >= 0 && Y >= 0 && X < GridWidth && Y < GridHeight;
}

FVector ADiabloDungeonGenerator::GetTileWorldLocation(int32 X, int32 Y, float Z) const
{
	const float TileSize = GetTileSize();
	return GetActorLocation() + FVector(
		(X - GridWidth * 0.5f + 0.5f) * TileSize,
		(Y - GridHeight * 0.5f + 0.5f) * TileSize,
		Z);
}

float ADiabloDungeonGenerator::GetTileSize() const
{
	return TilePalette ? TilePalette->TileSize : 300.f;
}
