// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SpiderMovementComponent.h"

#include "FrameTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Debug/DebugHelper.h"
#include "Kismet/KismetMathLibrary.h"

void USpiderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	PerformMovement(DeltaTime);
}

void USpiderMovementComponent::PerformMovement(float DeltaTime)
{
	TraceForSurfaces();
	TraceForCurrentGround();
	ProcessSurfaceInfo();
	FVector newUp;
	FVector newForward;
	FVector newRight;
	FVector NewLocation;
	FRotator NewRotation;
	if (TraceForCurrentGround())
	{
		NewLocation = GroundTraceResult.ImpactNormal  * -1.f * GravityFactor; // UKismetMathLibrary::GetDirectionUnitVector(UpdatedComponent->GetComponentLocation(), GroundTraceResult.Location);
		newUp = GroundTraceResult.ImpactNormal;
		newForward = newUp.Cross(UpdatedComponent->GetForwardVector().Cross(UpdatedComponent->GetUpVector()));
		newRight = newUp.Cross(newForward);
		NewRotation = UKismetMathLibrary::MakeRotationFromAxes(newForward, newRight, newUp);
		//UKismetMathLibrary::MakeRotFromXZ(GetOwner()->GetActorForwardVector(), GroundTraceResult.ImpactNormal);
		
	}
	if (CanClimbToWall())
	{
		// Todo - @hamza Use Input Vector to decide Interpolation Alpha
		float ClimbAlpha = GetLastInputVector().Dot(UpdatedComponent->GetForwardVector());
		NewLocation = 10.f * UKismetMathLibrary::GetDirectionUnitVector(UpdatedComponent->GetComponentLocation(), CurrentSurfaceLocation);
		newUp = CurrentSurfaceNormal;
		newForward = newUp.Cross(UpdatedComponent->GetForwardVector().Cross(UpdatedComponent->GetUpVector()));
		newRight = newUp.Cross(newForward);
		NewRotation = UKismetMathLibrary::MakeRotationFromAxes(newForward, newRight, newUp);
	}

	if (!TraceForCurrentGround() && !CanClimbToWall())
	{
		NewLocation = UpdatedComponent->GetUpVector() * -1.f * GravityFactor;
		NewRotation = UpdatedComponent->GetComponentRotation();
	}
	
	FHitResult Hit(1.f);
	// Todo - @hamza Get Location using a function instead of hardcoded everywhere
	UpdatedComponent->MoveComponent(NewLocation, NewRotation, true);
}

#pragma region SpiderMovement

TArray<FHitResult> USpiderMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape)
{
	TArray<FHitResult> OutHitResults;
	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		SpiderCapsuleTraceRadius,
		SpiderCapsuleTraceHalfHeight,
		SpiderSurfaceTraceTypes,
		false,
		TArray<AActor*>({this->GetOwner()}),
		bShowDebugShape? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		OutHitResults,
		false
		);
	return OutHitResults;
}

FHitResult USpiderMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End,
	bool bShowDebugShape)
{
	FHitResult OutHitResult;
	UKismetSystemLibrary::LineTraceSingleForObjects(
	this,
		Start,
		End,
		SpiderSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		bShowDebugShape? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		OutHitResult,
		false
	);
	return OutHitResult;
}
#pragma endregion
#pragma region SpiderMovementCore
bool USpiderMovementComponent::TraceForSurfaces()
{
	const FVector Offset = UpdatedComponent->GetForwardVector() * WallTraceStartOffset;
	const FVector Start = UpdatedComponent->GetComponentLocation() + Offset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	SpiderSurfaceTracedResults = DoCapsuleTraceMultiByObject(Start, End, true);
	return !SpiderSurfaceTracedResults.IsEmpty();
}

bool USpiderMovementComponent::TraceForCurrentGround()
{
	const FVector FwdOffset = (UpdatedComponent->GetForwardVector() * GroundTraceForwardOffset);
	const FVector UpOffset = (UpdatedComponent->GetUpVector() * GroundTraceDistance);
	FVector Start = UpdatedComponent->GetComponentLocation() + FwdOffset;
	Start += UpOffset;
	//Offset = Offset * FVector(1.f, 1.f, -1.f);
	FVector End = UpdatedComponent->GetComponentLocation() + FwdOffset;
	End -= UpOffset;

	GroundTraceResult =  DoLineTraceSingleByObject(Start, End, true);
	return GroundTraceResult.bBlockingHit;
}

void USpiderMovementComponent::ProcessSurfaceInfo()
{
	CurrentSurfaceLocation = FVector::ZeroVector;
	CurrentSurfaceNormal = FVector::ZeroVector;
	if (SpiderSurfaceTracedResults.IsEmpty())
	{
		bWantToClimbWall = false;
		return;
	}
	CurrentSurfaceLocation = FVector::ZeroVector;
	CurrentSurfaceNormal = FVector::ZeroVector;

	for (const FHitResult Hit : SpiderSurfaceTracedResults)
	{
		CurrentSurfaceLocation += Hit.ImpactPoint;
		CurrentSurfaceNormal += Hit.ImpactNormal;
	}

	CurrentSurfaceLocation /= SpiderSurfaceTracedResults.Num();
	CurrentSurfaceNormal = CurrentSurfaceNormal.GetSafeNormal();
	bWantToClimbWall = true;
}

bool USpiderMovementComponent::CanClimbToWall() const
{
	return bWantToClimbWall;
}

bool USpiderMovementComponent::DoesComponentExistInTracedSurfaces(const USceneComponent* ComponentToCheck)
{
	for (FHitResult Hit : SpiderSurfaceTracedResults)
	{
		if (Hit.GetComponent() == ComponentToCheck)
		{
			return true;
		}
	}
	return false;
}
#pragma endregion

