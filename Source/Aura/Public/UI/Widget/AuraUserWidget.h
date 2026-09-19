// 3198

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
protected:
	//用于初始化视觉表现（设置控件控制器时调用）
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
