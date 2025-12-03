// Fill out your copyright notice in the Description page of Project Settings.


#include "FC_DynamicIconSubsystem.h"

#include "CanvasTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UFC_DynamicIconSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Initialize current task

	
	TaskDelay = 0.01f;
	GetWorld()->GetTimerManager().SetTimer(TaskTimerHandle, this, &UFC_DynamicIconSubsystem::ExecuteTasks, TaskDelay, false);

}

void UFC_DynamicIconSubsystem::Deinitialize()
{
	TaskQueue.Empty();

	Super::Deinitialize();
}

void UFC_DynamicIconSubsystem::RequestGenerateDynamicIcon(UFC_DynamicIconComponent* DynamicIconComponent, bool bForceUpdate, bool bSkipQueue)
{
	// Add to queue
	FGenerateDynamicIconTask NewTask = FGenerateDynamicIconTask();
	NewTask.DynamicIconComponent = DynamicIconComponent;
	NewTask.bForceUpdate = bForceUpdate;
	if(!bSkipQueue)
	{
		// add to back of queue
		TaskQueue.Add(NewTask);
	}
	else
	{
		// add to front of queue
		TaskQueue.Insert(NewTask, 0);
	}

	if (CurrentTask.DynamicIconComponent == nullptr)
	{
		ExecuteNextTask();
	}
	
}

void UFC_DynamicIconSubsystem::ExecuteTasks()
{
	if(TaskQueue.Num() > 0)
	{
		ExecuteNextTask();
	}
}

void UFC_DynamicIconSubsystem::ExecuteNextTask()
{
	if(TaskQueue.Num() > 0)
	{
		CurrentTask.DynamicIconComponent = TaskQueue[0].DynamicIconComponent;
		GenerateDynamicIcon(CurrentTask);
		TaskQueue.RemoveAt(0);
		GetWorld()->GetTimerManager().SetTimer(TaskTimerHandle, this, &UFC_DynamicIconSubsystem::ExecuteTasks, TaskDelay, false);
	}
	else
	{
		CurrentTask.DynamicIconComponent = nullptr;
	}
}

void UFC_DynamicIconSubsystem::InitSceneCapture(UFC_DynamicIconComponent* DynamicIconComponent)
{
	// Create SceneCaptureComponent
	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(DynamicIconComponent->GetOwner(), TEXT("IconRenderer"));
	SceneCaptureComponent->RegisterComponent();

	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComponent->CaptureSource = DynamicIconComponent->CaptureSource;
	SceneCaptureComponent->bAlwaysPersistRenderingState = true;
	
	SceneCaptureComponent->ShowOnlyActors.Add(DynamicIconComponent->GetOwner());

	// get all the child actors and add them to the show only list
	TArray<AActor*> ChildActors;
	DynamicIconComponent->GetOwner()->GetAllChildActors(ChildActors, true);
	for (AActor* ChildActor : ChildActors)
	{
		SceneCaptureComponent->ShowOnlyActors.Add(ChildActor);
	}
	
	// Set show flags
	SceneCaptureComponent->ShowFlags.Atmosphere = false;
	SceneCaptureComponent->ShowFlags.SkyLighting = false;
	SceneCaptureComponent->ShowFlags.Lighting = true;
	SceneCaptureComponent->ShowFlags.PostProcessing = false;
	SceneCaptureComponent->AttachToComponent(DynamicIconComponent->GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);



}

void UFC_DynamicIconSubsystem::ClearTempLighting(UFC_DynamicIconComponent* DynamicIconComponent, UDirectionalLightComponent* DirectionalLight1, UDirectionalLightComponent* DirectionalLight2, TMap<UPrimitiveComponent*, FLightingChannels> LightingChannelsMap)
{
	// cleanup lighting
	if(DynamicIconComponent->IconLighting)
	{
		DirectionalLight1->DestroyComponent();
		DirectionalLight2->DestroyComponent();
		
		// reset lighting channels
		for (auto& Elem : LightingChannelsMap)
		{
			Elem.Key->SetLightingChannels(Elem.Value.bChannel0, Elem.Value.bChannel1, Elem.Value.bChannel2);
		}
	}
}

void UFC_DynamicIconSubsystem::GenerateDynamicIcon(FGenerateDynamicIconTask &Task)	
{
	if(Task.DynamicIconComponent->DynamicIcon != nullptr)
	{
		if(!Task.bForceUpdate)
		{
			return;
		}
	}

	
	UFC_DynamicIconComponent* DynamicIconComponent = Task.DynamicIconComponent;
	UE_LOG(LogTemp, Warning, TEXT("Generating dynamic icon for %s"), *DynamicIconComponent->GetOwner()->GetName());

	
	InitSceneCapture(DynamicIconComponent);

	FRotator WorldRotation = DynamicIconComponent->GetOwner()->GetActorRotation();
	DynamicIconComponent->GetOwner()->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));

	


	
	
	
	FBox BoundingBox = DynamicIconComponent->GetOwner()->GetComponentsBoundingBox(false, true);

	bool bUseXForward = DynamicIconComponent->bUseXForward;
	bool bFlipHorizontal = DynamicIconComponent->bFlipHorizontal;
	
	FBoxSphereBounds RootBounds = DynamicIconComponent->GetOwner()->GetComponentsBoundingBox(false, true);
	if(DynamicIconComponent->bAutoOrient) {
		RootBounds.BoxExtent.X < RootBounds.BoxExtent.Y ? bUseXForward = true : bUseXForward = false;
		FMath::Max(RootBounds.BoxExtent.X, RootBounds.BoxExtent.Y) < RootBounds.BoxExtent.Z ? bFlipHorizontal = true : bFlipHorizontal = false;
		if(DynamicIconComponent->DynamicIconSizeY > DynamicIconComponent->DynamicIconSizeX) {
			bFlipHorizontal = !bFlipHorizontal;
		}
	}

	// Get the relative center of the object
	FVector Center = BoundingBox.GetCenter();
	Center = UKismetMathLibrary::InverseTransformLocation(DynamicIconComponent->GetOwner()->GetActorTransform(), Center);
	
	FVector Extent = DynamicIconComponent->GetOwner()->GetComponentsBoundingBox(false, true).GetExtent();
	float Radius = Extent.Size();
	
	// Set ortho width
	SceneCaptureComponent->OrthoWidth = (Radius * 3 * FMath::Clamp(((float)DynamicIconComponent->DynamicIconSizeX / (float)DynamicIconComponent->DynamicIconSizeY), 0.5f, 1.0f))  * DynamicIconComponent->FOVMultiplier;
	
	// Location calculation
	float Distance = DynamicIconComponent->Distance;

	FVector LocalDirection;
	
	SceneCaptureComponent->SetRelativeLocation(Center);

	if(bUseXForward)
	{
		LocalDirection = FVector(1.0f, 0.0f, 0.0f);
	}
	else
	{
		LocalDirection = FVector(0.0f, 1.0f, 0.0f);
	}

	// Do the offsets
	SceneCaptureComponent->AddLocalOffset(LocalDirection * Distance);
	SceneCaptureComponent->AddLocalOffset(DynamicIconComponent->LocalOffset);

	// Use the rotation offset to rotate the relation location
	SceneCaptureComponent->SetRelativeLocation(UKismetMathLibrary::RotateAngleAxis(SceneCaptureComponent->GetRelativeLocation(), DynamicIconComponent->LocalRotationOffset, FVector(0.0f, 0.0f, 1.0f)));

	// Now in RELATIVE space, we look at the center of the object
	SceneCaptureComponent->SetRelativeRotation(UKismetMathLibrary::FindLookAtRotation(SceneCaptureComponent->GetRelativeLocation(), Center));
	if(bFlipHorizontal)
	{
		SceneCaptureComponent->AddLocalRotation(FRotator(0.0f, 0.0f, 90.0f));
	}

	// reset world rotation
	DynamicIconComponent->GetOwner()->SetActorRotation(WorldRotation);
	
	

	

	UDirectionalLightComponent* DirectionalLight = nullptr;
	UDirectionalLightComponent* DirectionalLight1 = nullptr;
	TMap<UPrimitiveComponent*, FLightingChannels> LightingChannelsMap;
	
	if(DynamicIconComponent->IconLighting)
	{
		// Add a point light to the scene capture component
		DirectionalLight = NewObject<UDirectionalLightComponent>(SceneCaptureComponent, TEXT("DirectionalLight1"));
		DirectionalLight->RegisterComponent();
		DirectionalLight->SetWorldLocation(SceneCaptureComponent->GetComponentLocation());
		DirectionalLight->SetWorldRotation(SceneCaptureComponent->GetComponentRotation());
		DirectionalLight->AddLocalRotation(FRotator(-45.0f, 0.0f, 0.0f));
		DirectionalLight->SetAtmosphereSunLight(false);
		DirectionalLight->CastShadows = false;
		DirectionalLight->SetLightingChannels(false, true, false);
		
		// Add a point light to the scene capture component
        DirectionalLight1 = NewObject<UDirectionalLightComponent>(SceneCaptureComponent, TEXT("DirectionalLight2"));
        DirectionalLight1->RegisterComponent();
        DirectionalLight1->SetWorldLocation(SceneCaptureComponent->GetComponentLocation());
        DirectionalLight1->SetWorldRotation(SceneCaptureComponent->GetComponentRotation());
        DirectionalLight1->AddLocalRotation(FRotator(45.0f, 0.0f, -45.0f));
        DirectionalLight1->SetAtmosphereSunLight(false);
        DirectionalLight1->SetIntensity(5.0f);
        DirectionalLight1->CastShadows = false;
        DirectionalLight1->SetLightingChannels(false, true, false);
		
		

		// On the Actor we're generating the icon for, lets find all the primitives and save their current lighting channels into a TMap
		
		TArray<UPrimitiveComponent*> Primitives = TArray<UPrimitiveComponent*>();
		DynamicIconComponent->GetOwner()->GetComponents(Primitives, true);
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			LightingChannelsMap.Add(Primitive, Primitive->LightingChannels);
			Primitive->SetLightingChannels(false, true, false);
		}
		
	}
	
	UTextureRenderTarget2D* Texture = UKismetRenderingLibrary::CreateRenderTarget2D(this, DynamicIconComponent->DynamicIconSizeX, DynamicIconComponent->DynamicIconSizeY, DynamicIconComponent->RenderTargetFormat, FLinearColor::Black, true);

	
	SceneCaptureComponent->TextureTarget = Texture;
	
	/// CAPTURE THE SCENE
	SceneCaptureComponent->CaptureScene();
	DynamicIconComponent->DynamicIcon = SceneCaptureComponent->TextureTarget;
	DynamicIconComponent->OnDynamicIconGenerated.Broadcast(DynamicIconComponent->DynamicIcon);
	/// CAPTURE THE SCENE

	
	// cleanup
	SceneCaptureComponent->DestroyComponent();
	ClearTempLighting(DynamicIconComponent, DirectionalLight, DirectionalLight1, LightingChannelsMap);
}

