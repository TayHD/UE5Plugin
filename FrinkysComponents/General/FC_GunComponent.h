// Copyright Frinky 2022

#pragma once

#include "CoreMinimal.h"
#include "FC_Barrel.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CheatManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "FC_GunComponent.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single,
	Burst,
	Auto,
	Manual,
};

USTRUCT(BlueprintType)
struct FRecoilStruct
{
	FRecoilStruct()
	{
		FSoftObjectPath MyAssetPath("CurveFloat'/FrinkysComponents/Curves/CT_LocationAlpha.CT_LocationAlpha'");
		LocationCurve = dynamic_cast<UCurveFloat*>(MyAssetPath.TryLoad());

		MyAssetPath = "CurveFloat'/FrinkysComponents/Curves/CT_RotationAlpha.CT_RotationAlpha'";
		RotationCurve = dynamic_cast<UCurveFloat*>(MyAssetPath.TryLoad());

		

	}
	
	GENERATED_BODY()
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Strength")
	float AnimationStrength = 1;
	
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Strength")
	float PitchStrength = 1;
	
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Strength")
	float LocationStrength = 1;
	
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Strength")
	float CameraRecoilStrength = 1;
	
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Time")
	float TimeDilation = 1;

	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Time")
	float RotationTimeDilation = 1;

	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Time")
	float LocationTimeDilation = 1;

	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere)
	float RandomMulitplier = 1;

	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere)
	float RandomMultiplierCamera = 1;

	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere)
	float MicroShakeMultiplier = 1;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere)
	float MacroShakeMultiplier = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere)
	float RotationDampening = 30;

	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere)
	float LocationDampening = 30;

	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere)
	bool InvertPitch = true;

	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere)
	bool xForward = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UCurveFloat* LocationCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UCurveFloat* RotationCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class UCameraShakeBase> MicroShake;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class UCameraShakeBase> MacroShake;
	
};

USTRUCT(BlueprintType)
struct FBulletImpactStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPhysicalMaterial* PhysMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FColor DebugColour = FColor::Red;

	// PENETRATE
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PenetrationChance = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PenetrationRicochetAngle = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PenetrationPower = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PenetrationVelocityThreshold = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BulletPenetrationVelocityReduction = 0.3;

	// RICOCHET
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RicochetChance = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RicochetAngleThreshold = 45;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RicochetVelocityThreshold = 0.1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BulletRicochetVelocityReduction = 0.1;
	
};

UCLASS(Blueprintable, BlueprintType)
class FRINKYSCOMPONENTS_API UFC_BulletImpactData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	TArray<FBulletImpactStruct> ImpactStructs;

	UPROPERTY(EditAnywhere)
	float BulletBaseVelocity = 1;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(ToolTip="Checks through the data base for the hit data, if we cant find it we fall back to the FIRST ARRAY ELEMENT"))
	FBulletImpactStruct GetDataFromHitResult(FHitResult HitResult);	
	
	
	
};

USTRUCT(BlueprintType)
struct FCustomHitData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	AActor* HitActor;
	UPROPERTY(BlueprintReadOnly)
	float HitStrength = 1;
	UPROPERTY(BlueprintReadOnly)
	FVector HitDirection;
	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;

	FCustomHitData(AActor* InHitActor, FVector InHitDirection, FHitResult InHitResult, float InHitStrength)
	{
		HitActor = InHitActor;
		HitDirection = InHitDirection;
		HitResult = InHitResult;
		HitStrength = InHitStrength;
		
	}
	FCustomHitData()
	{
		HitActor = nullptr;
		HitDirection = FVector(0.0, 0.0, 0.0);
		HitResult = FHitResult();
		HitStrength = 1;
	}
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGunMulticast);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHitMulticast, TArray<FCustomHitData>, HitData);


UCLASS(Blueprintable, ClassGroup=(FRINKYCORE), meta=(BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_GunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFC_GunComponent();

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void Serialize(FArchive& Ar) override;



public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	//Property's

	UPROPERTY()
	TArray<uint8> SaveData;	

	//Components
	UPROPERTY(BlueprintReadOnly, Replicated)
	UFC_Barrel* Barrel;

	//Sound
	UPROPERTY()
	UAudioComponent* Gunshot;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Gun|Sound") 
	float GunshotFadeTime = 0.1;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Sound") 
	USoundBase* Sound;
	
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Sound")
	USoundConcurrency* Concurrency;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Sound") 
	USoundAttenuation* Attenuation;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Sound") 
	USoundBase* SuppressorSound;
	
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Sound")
	USoundConcurrency* SuppressorConcurrency;
	
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Sound") 
	USoundAttenuation* SuppressorAttenuation;

	//Reloading

	//The ammo currently in the mag
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Replicated, Category="Gun|Reloading")
	int Ammo;
	//Maximum ammo that the mag can hold
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Replicated, Category="Gun|Reloading") 
	int MaxAmmo;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Reloading")
	bool ReloadIndividualRounds;

	//Trace
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	UFC_BulletImpactData* BulletImpactData;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	float TraceDistance;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category="Gun|Trace", meta=(ToolTip="This may help performance to disable"))
	bool TraceComplex;
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Trace", meta=(ToolTip="This traces from the camera instead of the barrel"))
	bool TraceFromCamera;
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Trace", meta=(ToolTip="Trace to the camera"))
	bool TraceToCamera;
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Trace", meta=(ToolTip="Trace to the camera, from camera, but do a second trace from the barrel to that point (Overrides other options)"))
	bool ThirdPersonStyleTrace;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	TEnumAsByte<EDrawDebugTrace::Type> TraceDebug;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	TEnumAsByte<ETraceTypeQuery>  TraceChannel;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	bool DebugImpacts;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	bool DebugSurfaceImpacts;
	
	
	
	

	//Disable Features
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Disable Features")
	bool bDoesPlaySound;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Disable Features")
	bool bDoesPlayCameraShake;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Disable Features")
	bool bDoesPlayProceduralAnimation;

	//Shotguns
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Shotgun", Replicated)
	float BaseSpread;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Shotgun", Replicated)
	int Flechettes;

	//AI
	UPROPERTY(SaveGame,  BlueprintReadOnly, EditAnywhere, Category="Gun|AI")
	float AIBaseSpread;
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Gun|AI")
	float AIAdditiveSpread;
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Gun|AI")
	float AISoundHearingRange;
	UPROPERTY(SaveGame,  BlueprintReadWrite, EditAnywhere, Category="Gun|AI")
	float AISoundLoudness;
	UPROPERTY(SaveGame,  BlueprintReadOnly, EditAnywhere, Category="Gun|AI")
	FName AISoundTag;

	//Basic
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Basic", Replicated)
	float RPM;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Basic", Replicated)
	float BaseDamage;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	TArray<FName> HeadshotBones = {"head"};
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category="Gun|Trace")
	float HeadshotDamageMulitplier = 2;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Basic")
	TArray<EFireMode> Firemodes;
	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, Category="Gun|Basic")
	TSubclassOf<UDamageType> DamageType;

	//Recoil
	UPROPERTY(SaveGame, BlueprintReadWrite, EditAnywhere, Category="Gun|Recoil", Replicated)
	FRecoilStruct RecoilStruct;
	UPROPERTY()
	FTransform RecoilTransform;
	
	//Misc
	UPROPERTY(BlueprintReadOnly, Category="Gun|Misc", Replicated)
	APawn* OwningPawn;
	UPROPERTY()
	float TimeSinceLastShot;
	UPROPERTY(SaveGame, BlueprintReadOnly, Category="Gun|Misc", Replicated)
	EFireMode FireMode;
	UPROPERTY()
	FTimerHandle FireTimer;
	UPROPERTY()
	float FireAnimationTimer;
	UPROPERTY()
	int SequentialShots;
	
	
	//Functions
	UFUNCTION(BlueprintCallable, Category="Gun Component", meta=(ToolTip="Begins firing the weapon with selected firemode"))
	void FireStart();
	
	UFUNCTION(BlueprintCallable, Category="Gun Component", meta=(ToolTip="Stops firing the weapon with selected firemode"))
	void FireStop();
	
	UFUNCTION(BlueprintCallable, Category="Gun Component", meta=(ToolTip="Fires weapon once, provided the cooldown has reset"))
	void FireOnce();
	
	UFUNCTION(BlueprintCallable, Category="Gun Component", Server, Reliable, meta=(ToolTip="Set the firemode directly, no matter the firemode array"))
	void SetFiremode(EFireMode NewFireMode);

	UFUNCTION(BlueprintCallable, Category="Gun Component", meta=(ToolTip="Set ammo directly, no matter the max ammo"))
	void SetAmmo(int NewAmmo);
	
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Gun Component", meta=(ToolTip="Inits the weapon"))
	void Init(UFC_Barrel* InBarrel, APawn* InOwningCharacter);

	UFUNCTION(BlueprintPure, Category="Gun Component")
	FTransform GetRecoilTransform();
	
	UFUNCTION()
	TArray<FCustomHitData> LineTrace();
	
	UFUNCTION(BlueprintCallable, Category="Gun Component", meta=(ToolTip="Cycles between valid firemodes"), Server, Reliable)
	void CycleFiremodes();
	
	UFUNCTION()
	void FireShot();
	
	UFUNCTION(Server, Unreliable)
	void AIReportSound();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Gets the delay between shots based on RPM"))
	const float GetFireRate();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Tells you whether the gun is owned by an AI or not"))
	bool isBot();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component")
	bool DidHeadshot(FHitResult HitResult);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Pass an array of hit data in, and it will tell you weather you hit a character"))
	bool DidHitClass(bool& isHeadshot, TSubclassOf<AActor> ActorClass, TArray<FCustomHitData> Hits);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Tells you whether the gun is owned by an AI or not"))
	void GetBarrelInfo(FVector& Location, FVector& Forward);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Tells you whether the gun is owned by an AI or not"))
	float GetSpread();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Gun Component", meta=(ToolTip="Tells you whether the gun is owned by an AI or not"))
	bool IsSuppressed();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetCurrentAmmo();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetMaxAmmo();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerApplyDamageToHitResults(const TArray<FCustomHitData>& HitResults);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetOwningPawn(APawn* NewPawn);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetBarrel(UFC_Barrel* NewBarrel);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetMaxAmmo(int NewMaxAmmo);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetFlechettes(int NewFlechettes);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetSpread(float NewSpread);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetDamage(float NewDamage);
	
	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	int ReloadWithAmmoPool(int AmmoPool);
	

	UFUNCTION(NetMulticast, Unreliable)
	void MultiFire();
	
	UFUNCTION(Client, Reliable)
	void ClientFire();

	UFUNCTION()
	void FireVisuals();

	UFUNCTION()
	void AddInput(float x, float y);

	UFUNCTION(BlueprintCallable)
	void SaveStats();

	UFUNCTION(BlueprintCallable)
	void LoadStats();
	

	UPROPERTY(BlueprintAssignable)
	FGunMulticast OnFireVisuals;
	
	UPROPERTY(BlueprintAssignable)
	FGunMulticast OnClientFire;

	UPROPERTY(BlueprintAssignable)
	FGunMulticast OnProxyFire;

	UPROPERTY(BlueprintAssignable)
	FHitMulticast OnClientHit;

	UPROPERTY(BlueprintAssignable)
	FHitMulticast OnServerHit;
	
	UPROPERTY(BlueprintAssignable)
	FGunMulticast OnReloadComplete;
};
