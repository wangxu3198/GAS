// 3198

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UAuraAssetManager& Get();
protected:
	//很早被调用，用于初始化
	virtual void StartInitialLoading() override;
};
