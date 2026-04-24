#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Attack.generated.h"

UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Melee Attack Trace"))
class DIABLO_API UAnimNotify_Attack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float TraceRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float TraceLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float Damage = 10.f;
};
