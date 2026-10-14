// 3198


#include "AuraGameplayTags.h"

#include "GameplayTagsManager.h"

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary.Armor"), FString("Reduces damage taken,improves Block Chance "));
}

FAuraGameplayTags FAuraGameplayTags::GameplayTags;
