

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CBAnimInstance.generated.h"

UCLASS()
class CB_API UCBAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	float VerticalVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	bool bIsSpinning;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	bool bIsMoving;

private:
	TWeakObjectPtr<class ACBPlayerCharacter> PlayerCharacter;
};
