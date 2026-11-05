// 3198

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;
class USkeletalMeshComponent;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAuraCharacterBase();
	
	/* --<IAbilitySystemInterface>-- */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	/* <IAbilitySystemInterface> */
	
	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; };
	
	/* --<ICombatInterface>-- */
	virtual int32 GetPlayerLevel() override;
	/* <ICombatInterface> */

protected:
	// Called when the game starts or when spawned
	/* --<AActor>-- */
	virtual void BeginPlay() override;
	/* <AActor> */
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;
	
	/* --<ICombatInterface>-- */
	virtual FVector GetCombatSocketLocation() override;
	/* <ICombatInterface> */
	
	virtual void InitAbilityActorInfo();
	
	//敌人类里进行构造
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;
	
	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> DefaultAttributes, const float Level) const;
	void InitializeDefaultAttributes() const;

	//服务器端添加技能
	void AddCharacterAbilities();
private:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};
