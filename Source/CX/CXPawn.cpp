// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "PlayerState/CXPlayerState.h"
#include "GameState/CXGameState.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// 스켈레탈 메시는 기본값이 false라서, 켜야 아이템과 겹침 이벤트가 생김
	GetMesh()->SetGenerateOverlapEvents(true);

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		// attack
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::Attack);
		}
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

	// Waiting/Finished: 스로틀, 조향 0 + 핸드브레이크 고정. Playing이 되면 잠금 해제
	if (IsLocallyControlled())
	{
		if (IsMatchPlaying() == false)
		{
			ChaosVehicleMovement->SetThrottleInput(0.0f);
			ChaosVehicleMovement->SetSteeringInput(0.0f);
			ChaosVehicleMovement->SetHandbrakeInput(true);
			bMatchHandbrakeLocked = true;
		}
		else if (bMatchHandbrakeLocked)
		{
			ChaosVehicleMovement->SetHandbrakeInput(false);
			bMatchHandbrakeLocked = false;
		}
	}
}

bool ACXPawn::IsMatchPlaying() const
{
	const ACXGameState* CXGameState = GetWorld() ? GetWorld()->GetGameState<ACXGameState>() : nullptr;
	return IsValid(CXGameState) && CXGameState->MatchState == ECXMatchState::Playing;
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	// get the input magnitude for steering
	float SteeringValue = IsMatchPlaying() ? Value.Get<float>() : 0.0f;

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	// get the input magnitude for the throttle
	float ThrottleValue = IsMatchPlaying() ? Value.Get<float>() : 0.0f;

	// 최대 속도(기본 + 버프)를 넘었으면 더 가속하지 않음. GetForwardSpeed는 cm/s → km/h는 0.036
	float MaxSpeed = BaseMaxSpeed;
	if (const ACXPlayerState* CXPlayerState = GetPlayerState<ACXPlayerState>())
	{
		MaxSpeed += CXPlayerState->GetBonusMaxSpeed();
	}

	const float CurrentSpeed = ChaosVehicleMovement->GetForwardSpeed() * 0.036f;
	if (ThrottleValue > 0.0f && CurrentSpeed >= MaxSpeed)
	{
		ThrottleValue = 0.0f;
	}

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	// 게임 상태 잠금 중엔 손으로 풀 수 없음
	if (bMatchHandbrakeLocked)
	{
		return;
	}

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (IsMatchPlaying() == false)
	{
		return;
	}

	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::Attack(const FInputActionValue& Value)
{
	if (IsMatchPlaying() == false)
	{
		return;
	}

	ServerAttack();
}

void ACXPawn::ServerAttack_Implementation()
{
	// 클라이언트를 믿지 않고 서버에서도 한 번 더 검사
	if (IsMatchPlaying() == false)
	{
		return;
	}

	// 앞 범퍼 중앙에서 앞쪽으로 라인트레이스
	const FVector TraceStart = GetActorTransform().TransformPosition(AttackTraceStartOffset);
	const FVector TraceEnd = TraceStart + GetActorForwardVector() * AttackRange;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehicleAttack), false, this);

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, TraceStart, TraceEnd, ObjectQueryParams, QueryParams);

	ACXPawn* HitVehicle = bHit ? Cast<ACXPawn>(HitResult.GetActor()) : nullptr;
	if (IsValid(HitVehicle))
	{
		UGameplayStatics::ApplyDamage(HitVehicle, 1.0f, GetController(), this, UDamageType::StaticClass());
	}

	MulticastPlayAttackEffect(TraceStart, bHit ? FVector(HitResult.ImpactPoint) : TraceEnd, IsValid(HitVehicle));
}

void ACXPawn::MulticastPlayAttackEffect_Implementation(const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd, bool bHit)
{
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 5.0f);

	if (bHit && IsValid(AttackHitParticle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, AttackHitParticle, TraceEnd);
	}
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority() == false || IsValid(EventInstigator) == false)
	{
		return ActualDamage;
	}

	ACXPlayerState* VictimPlayerState = GetPlayerState<ACXPlayerState>();
	ACXPlayerState* AttackerPlayerState = EventInstigator->GetPlayerState<ACXPlayerState>();
	if (IsValid(VictimPlayerState) == false || IsValid(AttackerPlayerState) == false)
	{
		return ActualDamage;
	}

	// 피해 차량이 5원 미만이면 아무것도 안 함
	if (VictimPlayerState->GetCoins() < StealCoinAmount)
	{
		return ActualDamage;
	}

	VictimPlayerState->RemoveCoins(StealCoinAmount);
	AttackerPlayerState->AddCoins(StealCoinAmount);

	return ActualDamage;
}

#undef LOCTEXT_NAMESPACE