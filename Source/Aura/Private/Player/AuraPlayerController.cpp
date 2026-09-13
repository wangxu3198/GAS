// 3198


#include "Player/AuraPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);
	
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
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	
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
		CurrentActor = Cast<IEnemyInterface>(CursorHit.GetActor());
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
