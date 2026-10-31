// 3198


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		//TODO:We are on the server,so listen for target data.
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	/** 
	 * 创建一个预测窗口。它会生成或获取一个当前的 PredictionKey（ASC中的ScopedPredictionKey）。
	 * 在此作用域内发出的关键网络操作（如 RPC、属性修改）都会关联此 Key。
	 * 这样当数据发送到服务器时，服务器就能将其与客户端的预测流程对齐并进行校验。
	 */
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = HitResult;
	DataHandle.Add(Data);
	
	// AbilityOriginalPredictionKey：能力最初激活时的预测键
	/*
	 * CurrentPredictionKey：专门针对当前特定上下文的预测键（可通过ASC创建）
	 * AbilitySystemComponent->ScopedPredictionKey（获得一个范围预测键--是个变量要自己更新其中数值（创建作用域预测窗口））
	 * 能力系统组件总能获取当前的预测键
	 * 作用域预测窗口：是个Time窗口，在这段时间内本地的所有操作都会被预测（有作用域限制--仅限创建这个窗口时所在的范围，
	 * 因为超过时ScopedPredictionKey的析构函数将数据重置）
	 * 以下操作将带着刚才的预测 Key 发送给服务器 
	 * 服务器从包里拿出了这个 FPredictionKey。
	 * 服务器在自己的 ASC 里临时创建一个属于服务器的 FScopedPredictionWindow（注意，这里是拿着你传过来的 Key 去构造的）。
	 * 用自己的代码重新执行 GA 逻辑：服务器收到 RPC（携带 Key 和技能句柄）后，会独立地在服务器上激活同一个 GA。这一步是服务器自己的代码在跑，不是客户端代码的远程复制。
	 * 用 Key 标记服务器端产生的副作用：服务器在本地执行 GA 时，会用收到的 PredictionKey 构建一个预测窗口，把自己产生的 GameplayEffect、属性修改等副作用也绑定到这个 Key 上。
	 * 校验并反馈：服务器根据自己的权威状态（真实蓝量、距离、冷却等）判断这次激活是否合法。合法则通过，不合法则拒绝。无论结果如何，服务器都会通过 RPC 或属性复制把结果传回客户端。
	 */
	AbilitySystemComponent->ServerSetReplicatedTargetData(GetAbilitySpecHandle(), 
		GetActivationPredictionKey(), DataHandle, FGameplayTag(), 
		AbilitySystemComponent->ScopedPredictionKey);
	
	// 某些情况不应该广播能力任务委托--可确保能力仍处于激活状态（能力处于激活状态时才能进行广播）
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
