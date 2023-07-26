// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SpiderMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utilities/TraceUtils.h"
#include "Debug/DebugHelper.h"
#include "Kismet/KismetMathLibrary.h"

void USpiderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	PerformMovement(DeltaTime);
}
#pragma region SpiderMovement

TArray<FHitResult> USpiderMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape)
{
	TArray<FHitResult> OutHitResults;
	// Use the capsule trace with Relative Orientation 
	UTraceUtils::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		SpiderCapsuleTraceRadius,
		SpiderCapsuleTraceHalfHeight,
		UpdatedComponent->GetComponentRotation(),
		SpiderSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
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
void USpiderMovementComponent::PerformMovement(float DeltaTime)
{
	FVector NewLocation;
	FRotator NewRotation;

	// If a ground trace is successful pawn will continuously try to move towards the ground until collision hits
	if (TraceForCurrentGround())
	{
		NewRotation = GetRotationAlignedToSurface(GroundTraceResult.ImpactNormal);
		NewLocation = GroundTraceResult.ImpactNormal  * -1.f * GravityFactor; 		
	}

	// Check if the Pawn is near a wall then calculate Location and Rotation to move to that wall (Override Previous Location and Rotation)
	if (CanClimbToWall())
	{
		// Calculate the correct Values depending on the traced surface e.g ceilings walls etc.
		ProcessSurfaceInfo();
		
		// Todo - @hamza Use Input Vector to decide Interpolation Alpha (Or not because I will be using it for AI?)
		float ClimbAlpha = GetLastInputVector().Dot(UpdatedComponent->GetForwardVector());
		NewRotation = GetRotationAlignedToSurface(CurrentSurfaceNormal);
		NewLocation = CurrentSurfaceLocation * 0.1f;	
		
	}

	// Check if the Pawn is not near wall nor near ground, apply gravity
	if (!TraceForCurrentGround() && !CanClimbToWall())
	{
		NewLocation = UpdatedComponent->GetUpVector() * -1.f * GravityFactor;
		NewRotation = UpdatedComponent->GetComponentRotation();
	}
	
	// Todo - @hamza Get Location using a function instead of hardcoded everywhere	
	// Todo - @hamza Create A dedicated function for actual movement
	// Move the component based on the calculated Location and Rotation
	UpdatedComponent->MoveComponent(NewLocation, FQuat::Slerp(UpdatedComponent->GetComponentQuat(), NewRotation.Quaternion(), DeltaTime * 12.f), true);
}

bool USpiderMovementComponent::TraceForSurfaces()
{
	const FVector Offset = UpdatedComponent->GetForwardVector() * WallTraceStartOffset;
	const FVector Start = UpdatedComponent->GetComponentLocation() + Offset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	SpiderSurfaceTracedResults = DoCapsuleTraceMultiByObject(Start, End, bDrawDebug);
	return !SpiderSurfaceTracedResults.IsEmpty();
}

bool USpiderMovementComponent::TraceForCurrentGround()
{
	const FVector FwdOffset = (UpdatedComponent->GetForwardVector() * GroundTraceForwardOffset);
	const FVector UpOffset = (UpdatedComponent->GetUpVector() * GroundTraceDistance);
	FVector Start = UpdatedComponent->GetComponentLocation() + FwdOffset;
	Start += UpOffset;
	FVector End = UpdatedComponent->GetComponentLocation() + FwdOffset;
	End -= UpOffset;

	GroundTraceResult =  DoLineTraceSingleByObject(Start, End, bDrawDebug);
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

FRotator USpiderMovementComponent::GetRotationAlignedToSurface(FVector SurfaceNormal)
{
	const FVector newUp = SurfaceNormal;
	const FVector newForward = newUp.Cross(UpdatedComponent->GetForwardVector().Cross(UpdatedComponent->GetUpVector()));
	const FVector newRight = newUp.Cross(newForward);

	return UKismetMathLibrary::MakeRotationFromAxes(newForward, newRight, newUp);
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

