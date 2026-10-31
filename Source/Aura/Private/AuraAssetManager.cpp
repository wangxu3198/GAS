// 3198


#include "AuraAssetManager.h"
#include "AbilitySystemGlobals.h"
#include "AuraGameplayTags.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	return *Cast<UAuraAssetManager>(GEngine->AssetManager);
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	FAuraGameplayTags::InitializeNativeGameplayTags();
	
	/*
	 * 注：从 Unreal Engine 5.3 开始，UAbilitySystemGlobals::Get().InitGlobalData()
	 * 已经不再需要由用户代码手动调用了，引擎会在内部自动完成初始化。
	 */
	//初始化全局信息，包含TargetData相关（InitTargetDataScriptStructCache）--使用目标数据必须步骤
	UAbilitySystemGlobals::Get().InitGlobalData();
}
