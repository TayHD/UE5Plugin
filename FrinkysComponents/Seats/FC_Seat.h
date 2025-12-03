#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/Object.h"
#include "FC_Seat.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSit, APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMove, float, Input);

UCLASS(ClassGroup = (FRINKYCORE), meta = (BlueprintSpawnableComponent))
class FRINKYSCOMPONENTS_API UFC_Seat : public USceneComponent
{
    GENERATED_BODY()

public:
    UFC_Seat();

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void InitializeComponent() override;
    virtual void OnComponentCreated() override;
    virtual void BeginPlay() override;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Debug Preview Components
    UPROPERTY()
    USkeletalMeshComponent* DebugMeshComponent;

    UPROPERTY()
    USkeletalMeshComponent* PreviewEjectMeshComponent;

    // Configuration Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Animation")
    UAnimSequence* Pose;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Preview")
    USkeletalMesh* PreviewMesh;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Preview")
    FRotator PreviewRotation = FRotator(0.f, -90.f, 0.f);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Preview")
    FVector PreviewLocation = FVector(0.f, 0.f, -96.f);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Preview")
    bool bEnablePreviews = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Position")
    FVector ExitPosition;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Position")
    FVector PawnOffset;

    // View Limits
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|View Limits")
    float MaxViewPitch = 90.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|View Limits")
    float MinViewPitch = -90.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|View Limits")
    float MaxViewYaw = 90.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|View Limits")
    float MinViewYaw = -90.f;

    // Rotation Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Rotation")
    bool bInheritOwnerRotation = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Rotation")
    bool bRotateWithController = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Rotation")
    FRotator SeatRotationOffset = FRotator::ZeroRotator;

    // Gameplay Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Gameplay")
    bool bPreventDetachInAir = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Gameplay")
    FGameplayTag Tag;

    // Stored Values
    UPROPERTY()
    float SavedMaxViewPitch = 90.f;

    UPROPERTY()
    float SavedMinViewPitch = -90.f;

    UPROPERTY()
    float SavedMaxViewYaw = 90.f;

    UPROPERTY()
    float SavedMinViewYaw = -90.f;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttachedPawn)
    APawn* AttachedPawn;

    // Functions
    UFUNCTION()
    virtual void OnRep_AttachedPawn();

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Seat")
    virtual void Sit(APawn* Pawn);

    UFUNCTION(NetMulticast, Reliable, Category = "Seat")
    virtual void SitMulti(APawn* Pawn);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Seat")
    virtual void Eject();

    UFUNCTION(NetMulticast, Reliable, Category = "Seat")
    virtual void EjectMulti();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Seat")
    bool IsPawnSeated() const;

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void AddForwardInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void AddRightInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Seat")
    bool CanEject() const;

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void UpdateSeatRotation();

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
    FOnSit OnSit;

    UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
    FOnSit OnEject;

    UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
    FOnMove OnRightInput;

    UPROPERTY(BlueprintAssignable, Category = "Seat|Events")
    FOnMove OnForwardInput;

private:
    void SaveOriginalViewLimits(APlayerController* PlayerController);
    void RestoreOriginalViewLimits(APlayerController* PlayerController);
    void ApplyViewLimits(APlayerController* PlayerController);
    FRotator GetDesiredPawnRotation() const;
};