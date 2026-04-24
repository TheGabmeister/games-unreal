#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DiabloAnimInstance.generated.h"

UCLASS(Abstract)
class DIABLO_API UDiabloAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed = 0.f;
};
