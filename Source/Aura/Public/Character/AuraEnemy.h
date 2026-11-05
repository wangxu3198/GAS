
// 3198

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	AAuraEnemy();
	
	/* --<IEnemyInterface>-- */
	virtual void UnHighlightActor() override;
	virtual void HighlightActor() override;
	/* <IEnemyInterface> */
	
	/* --<ICombatInterface>-- */
	virtual int32 GetPlayerLevel() override;
	/* <ICombatInterface> */
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
protected:
	/* --<AActor>-- */
	virtual void BeginPlay() override;
	/* <AActor> */
	
	/* --<AAuraCharacterBase>-- */
	virtual void InitAbilityActorInfo() override;
	/* <AAuraCharacterBase> */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true") ,Category = "Character Class Defaults")
	int32 Level = 1;
};
