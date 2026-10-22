// 3198


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
	SetShowMouseCursor(true);
	DefaultMouseCursor = EMouseCursor::Default;
	
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
	/*
	 * 因为C++的模板实参推导机制，所以不用<>
	 * 前提条件：在C++中，通常编译器可以自动推导模板类型。但在以下两种情况下，必须手动（显式）指定类型：
	 * 
	 * 1. 类型未出现在参数列表：模板参数没有作为函数参数传入（编译器无从推导）。
	 * 2. 存在二义性：编译器无法唯一确定要使用的类型。
	 * 
	 * 代码示例解析：
	 * 定义了一个模板函数 template<typename T> T GetDefaultValue()。
	 * 因为该函数的参数列表 () 是空的，没有传入任何参数，编译器无法推导 T 是什么类型。
	 * 因此，在调用时必须显式指定（如 GetDefaultValue<int>()），否则会报错。
	 */
	AuraInputComponent->BindAbilityActions(InputConfig, this, &AAuraPlayerController::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagHeld);
	
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
	
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = CurrentActor;
	
	if (CursorHit.GetActor()->Implements<UEnemyInterface>())
	{
		//Cast 返回的是 IEnemyInterface*（原生接口指针），不能直接赋值给 TScriptInterface。
		CurrentActor = TScriptInterface<IEnemyInterface>(CursorHit.GetActor());
		
	}else
	{
		CurrentActor = nullptr;
	}
	
	/**
	* Line trace from cursor. There are several scenarios:
	* A. LastActor is null && ThisActor is null
	 *      - Do nothing
	 * B. LastActor is null && ThisActor is valid
	 *      - Highlight ThisActor
	 * C. LastActor is valid && ThisActor is null
	 *      - UnHighlight LastActor
	 * D. Both actors are valid, but LastActor != ThisActor
	 *      - UnHighlight LastActor, and Highlight ThisActor
	 * E. Both actors are valid, and are the same actor
	 *      - Do nothing
	 */
	if (LastActor == nullptr && CurrentActor == nullptr)
	{
		// A. Do nothing
	}
	else if (LastActor == nullptr && CurrentActor != nullptr)
	{
		// B. Highlight ThisActor (CurrentActor)
		CurrentActor->HighlightActor();
	}
	else if (LastActor != nullptr && CurrentActor == nullptr)
	{
		// C. UnHighlight LastActor
		LastActor->UnHighlightActor();
	}
	else if (LastActor != nullptr && CurrentActor != nullptr && LastActor != CurrentActor)
	{
		// D. UnHighlight LastActor, and Highlight ThisActor (CurrentActor)
		LastActor->UnHighlightActor();
		CurrentActor->HighlightActor();
	}
	else if (LastActor == CurrentActor)
	{
		// E. Both actors are valid, and are the same actor - Do nothing
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	
}
