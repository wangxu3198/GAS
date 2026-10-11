// 3198

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	AAuraEnemy();
	
	virtual void UnHighlightActor() override;
	virtual void HighlightActor() override;
	
	virtual int32 GetPlayerLevel() override;
protected:
	virtual void BeginPlay() override;
	
	virtual void InitAbilityActorInfo() override;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true") ,Category = "Character Class Defaults")
	int32 Level = 1;
};
