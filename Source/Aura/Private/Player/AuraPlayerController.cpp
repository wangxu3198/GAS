// 3198


#include "Player/AuraPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction, 1.f);
		
		const float DistanceToDestination = (LocationOnSpline - CacheDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
	AutoRun();
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
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
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
	
	if (LastActor != CurrentActor)
	{
		if (CurrentActor)
			CurrentActor->HighlightActor();
		if (LastActor)
			LastActor->UnHighlightActor();
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (!AuraAbilitySystemComponent)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = CurrentActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		//调用时机可能过早，所以判断一下
		if (GetASC()){GetASC()->AbilityInputTagReleased(InputTag);}
		return;
	}
	
	if (GetASC()){GetASC()->AbilityInputTagReleased(InputTag);}

	if (!bTargeting && !bShiftKeyDown)
	{
		APawn* ControllerPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControllerPawn)
		{	
			/*
			 * 因为 UE 默认把寻路当成「服务器专用功能」，客户端世界里连 NavigationSystem 都不会被创建
			 * Allow Client Side Navigation可解决
			 */
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControllerPawn->GetActorLocation(), CacheDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				CacheDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()){GetASC()->AbilityInputTagHeld(InputTag);}
		return;
	}
	
	if (bTargeting || bShiftKeyDown)
	{
		if (GetASC()){GetASC()->AbilityInputTagHeld(InputTag);}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		if (CursorHit.bBlockingHit){CacheDestination = CursorHit.ImpactPoint;}
		
		/**
		 * 移动代码故意不放在上面的 if (GetHitResultUnderCursor) 里：
		 * 射线检测成功时只负责更新 CacheDestination，移动用的是上一次有效的目标点。
		 * 若某帧射线打不到任何东西（鼠标悬在 UI 上、移出屏幕边缘、被遮挡），
		 * 玩家仍应朝最后一个有效目标继续走；放进 if 里那一帧会停止移动，表现会一顿一顿。
		 */
		if (APawn* ControllerPawn = GetPawn<APawn>())
		{
			const FVector WorldDirection = (CacheDestination - ControllerPawn->GetActorLocation()).GetSafeNormal();
			ControllerPawn->AddMovementInput(WorldDirection);
		}
	}
	
	
}
