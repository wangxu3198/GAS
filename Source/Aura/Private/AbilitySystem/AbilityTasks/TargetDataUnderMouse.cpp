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
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate)
		{
			//当前这个 AbilityTask 正在等待远程玩家（客户端）的数据，暂时挂起。数据到了后台会自动广播，这个函数只是等待
			SetWaitingOnRemotePlayerData();
		}
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
	 * 把“客户端这次预测的编号”临时设为服务器当前的预测编号，让服务器在窗口内产出的权威结果能被客户端精确识别为“对同一次预测的答复”，从而完成对账与回滚。
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

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	// 告知ASC该目标数据已消费完毕，主动释放其本地存储的缓存（Map），避免内存占用或数据复用。
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
