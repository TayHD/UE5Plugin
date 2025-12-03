// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FC_DynamicIconComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicIconSigniture, UTextureRenderTarget2D*, DynamicIcon);



UCLASS(ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_DynamicIconComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_DynamicIconComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings|DynamicIcon")
	UTextureRenderTarget2D* DynamicIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon", meta=(ToolTip="This can cause performance issues, use with caution"))
	int DynamicIconSizeX = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon", meta=(ToolTip="This can cause performance issues, use with caution"))
	int DynamicIconSizeY = 256;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	FVector LocalOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	float LocalRotationOffset = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	float FOVMultiplier = 1.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	float Distance = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	bool bAutoOrient = true;

	// can only be changed if bAutoOrient is false,
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon", meta=(EditCondition="!bAutoOrient"))
	bool bUseXForward = true;

	// can only be changed if bAutoOrient is false,
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon", meta=(EditCondition="!bAutoOrient"))
	bool bFlipHorizontal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	TEnumAsByte<ETextureRenderTargetFormat> RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon")
	TEnumAsByte<ESceneCaptureSource> CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|DynamicIcon", meta=(ToolTip="When enabled, we create a point light to light the icon"))
	bool IconLighting = true;

	UPROPERTY(BlueprintAssignable)
	FDynamicIconSigniture OnDynamicIconGenerated;

	

	

};
