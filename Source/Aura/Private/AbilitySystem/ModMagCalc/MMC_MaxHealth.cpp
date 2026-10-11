// 3198


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	//设置vigor捕获的目标
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	//设置是否快照
	VigorDef.bSnapshot = false;
	
	
	//捕捉属性加到捕捉属性数组
	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//FGameplayEffectSpec包含从源和目标捕捉的标签
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	//以下参数必须传给特定函数，用于捕获我们关注的属性时的评估(6.P132)
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float Vigor = 0.f;
	//获取捕获的属性数值
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluateParameters, Vigor);
	
	Vigor = FMath::Max<float>(Vigor, 0.f);
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetEffectContext().GetSourceObject());
	check(CombatInterface);
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();
	
	return 80.f + 2.5f *Vigor + 10.f * PlayerLevel;
}
