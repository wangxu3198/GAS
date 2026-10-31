// 3198


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UAuraProjectileSpell::SpawnProjectile()
{
	//服务器端生成客户端复制
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		
		//生成前需要设置一些东西
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		
		//TODO:Set the Projectile Rotation
		
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass, 
			SpawnTransform, 
			GetOwningActorFromActorInfo(), 
			Cast<APawn>(GetAvatarActorFromActorInfo()), 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		//TODO:Give the Projectile a Gameplay Effect Spec for causing Damage.
		
		Projectile->FinishSpawning(SpawnTransform);
			
	}
}
