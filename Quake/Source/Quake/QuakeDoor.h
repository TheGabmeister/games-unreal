#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeActivatable.h"
#include "QuakeDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EQuakeKeyColor : uint8
{
	None   UMETA(DisplayName = "None"),
	Silver UMETA(DisplayName = "Silver"),
	Gold   UMETA(DisplayName = "Gold")
};

/**
 * Phase 8 door per SPEC section 5.4. Moves a UStaticMeshComponent between
 * closed and open positions under tick-driven linear interpolation. A
 * UTimelineComponent was considered but rejected — curves require .uasset
 * authoring, which would pull gameplay data out of C++ and into an editor
 * artifact, violating the "C++ first" rule. Linear interp in Tick is
 * deterministic, needs no asset, and matches original Quake's door feel.
 *
 * States: Closed → Opening → Open → Closing → Closed.
 *
 * Safety rules (SPEC 5.4):
 *   - While any ACharacter overlaps BlockingZone, the door will not *start*
 *     closing (the auto-close timer retries after a short delay).
 *   - Once closing, the moving mesh uses sweep=true and calls
 *     HandleCrushHit on any ACharacter caught in the path — 10000 damage
 *     with UQuakeDamageType_Telefrag. The door keeps moving (the victim is
 *     already gibbed; stopping would just trap the corpse).
 *
 * Keys: RequiredKey is exposed as EQuakeKeyColor. For Phase 8 the key check
 * is a stub — if RequiredKey != None the door is always treated as locked
 * (logs + AddOnScreenDebugMessage). Phase 10 (Keys + Powerups + HUD) wires
 * the real PlayerState key query and the proper HUD message.
 *
 * IQuakeActivatable: Activate(Instigator) opens the door (checking the key
 * gate). Buttons and triggers hold a TObjectPtr<AActor> picker to this door.
 */
UCLASS()
class QUAKE_API AQuakeDoor : public AActor, public IQuakeActivatable
{
	GENERATED_BODY()

public:
	AQuakeDoor();

	// --- Components ---

	/** Moving door mesh. Asset slot filled in the BP subclass. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	/** Overlap-only volume aligned with the swept path — used to defer
	 *  closing while a pawn is inside. Sized in the BP subclass. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UBoxComponent> BlockingZone;

	// --- Tuning (SPEC 5.4) ---

	/** How far the door slides. Direction is OpenAxis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (ClampMin = "0.0"))
	float OpenDistance = 128.f;

	/** Local-space direction the door moves when opening. Normalized on use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	FVector OpenAxis = FVector(0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (ClampMin = "0.0"))
	float OpenSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (ClampMin = "0.0"))
	float CloseSpeed = 200.f;

	/** Seconds from reaching Open to starting Close. 0 = stay open forever. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (ClampMin = "0.0"))
	float AutoCloseDelay = 4.f;

	/** Damage applied to anything the door crushes while closing. SPEC: 10000. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (ClampMin = "0.0"))
	float CrushDamage = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	EQuakeKeyColor RequiredKey = EQuakeKeyColor::None;

	// --- IQuakeActivatable ---

	virtual void Activate(AActor* InInstigator) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnDoorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	enum class EState : uint8 { Closed, Opening, Open, Closing };

	EState State = EState::Closed;

	/** Mesh relative location when Closed (captured in BeginPlay). */
	FVector ClosedRelativeLoc = FVector::ZeroVector;

	/** Mesh relative location when fully Open (ClosedRelativeLoc + OpenAxis * OpenDistance). */
	FVector OpenRelativeLoc = FVector::ZeroVector;

	/** Timer that transitions Open → Closing after AutoCloseDelay. */
	FTimerHandle CloseTimerHandle;

	void TryStartClosing();
	bool IsBlockingZoneOccupied() const;
	bool CanOpenFor(AActor* InInstigator) const;
};
