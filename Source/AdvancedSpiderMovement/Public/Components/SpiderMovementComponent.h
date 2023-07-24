// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "SpiderMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ADVANCEDSPIDERMOVEMENT_API USpiderMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

protected:
#pragma region OverriddenFunctions
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** Perform movement on an autonomous client */
	virtual void PerformMovement(float DeltaTime);
	// virtual void PhysWalking() override;
#pragma endregion 

private:	
#pragma region SpiderMovementTraces
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false);
#pragma endregion 
#pragma region SpiderMovementCore
	bool TraceForSurfaces();
	bool TraceForCurrentGround();
	void ProcessSurfaceInfo();
	bool CanClimbToWall() const;
	bool DoesComponentExistInTracedSurfaces(const USceneComponent* ComponentToCheck);
#pragma endregion
#pragma region SpiderMovementCoreVars
	TArray<FHitResult> SpiderSurfaceTracedResults;
	FHitResult GroundTraceResult;
	FVector CurrentSurfaceLocation;
	FVector CurrentSurfaceNormal;
	bool bWantToClimbWall;
#pragma endregion 
#pragma region SpiderMovementBPVars
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> SpiderSurfaceTraceTypes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	float SpiderCapsuleTraceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	float SpiderCapsuleTraceHalfHeight = 72.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	float WallTraceStartOffset = 30.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	float GroundTraceForwardOffset = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Movement", meta = (AllowPrivateAccess = "true"))
	float GroundTraceDistance = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AdvancedSpiderMovement | Physics", meta = (AllowPrivateAccess = "true"))
	float GravityFactor = 3.f;
	
	
#pragma endregion 
	
};
