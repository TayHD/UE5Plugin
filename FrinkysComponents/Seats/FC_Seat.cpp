#include "FC_Seat.h"
#include "Components/CapsuleComponent.h"
#include "FrinkysComponents/General/FC_Functions.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UFC_Seat::UFC_Seat()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UFC_Seat::BeginPlay()
{
    Super::BeginPlay();
    // Additional initialization if needed
}

void UFC_Seat::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (IsValid(AttachedPawn))
    {
        UpdateSeatRotation();
    }
}

void UFC_Seat::UpdateSeatRotation()
{
    if (!IsValid(AttachedPawn))
        return;

    FRotator TargetRotation = GetDesiredPawnRotation();
    
    // Apply smooth rotation interpolation
    FRotator CurrentRotation = AttachedPawn->GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(
        CurrentRotation,
        TargetRotation,
        GetWorld()->GetDeltaSeconds(),
        10.0f  // Adjust this value to control rotation speed
    );

    AttachedPawn->SetActorRotation(NewRotation);
}

FRotator UFC_Seat::GetDesiredPawnRotation() const
{
    FRotator DesiredRotation = GetComponentRotation();

    if (bInheritOwnerRotation)
    {
        if (AActor* Owner = GetOwner())
        {
            DesiredRotation = Owner->GetActorRotation();
        }
    }

    if (bRotateWithController && IsValid(AttachedPawn))
    {
        if (AController* Controller = AttachedPawn->GetController())
        {
            // Only inherit the yaw from the controller
            DesiredRotation.Yaw = Controller->GetControlRotation().Yaw;
        }
    }

    // Apply the seat rotation offset
    return DesiredRotation + SeatRotationOffset;
}

void UFC_Seat::SitMulti_Implementation(APawn* Pawn)
{
    if (!IsValid(Pawn))
        return;

    const ACharacter* Character = Cast<ACharacter>(Pawn);
    if (IsValid(Character))
    {
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    // Attach pawn to seat
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
    Pawn->AttachToComponent(this, AttachRules);
    Pawn->SetActorRelativeLocation(PawnOffset);
    
    // Initialize rotation
    UpdateSeatRotation();
}

void UFC_Seat::EjectMulti_Implementation()
{
    if (!IsValid(AttachedPawn))
        return;

    if (bPreventDetachInAir)
    {
        ACharacter* Character = Cast<ACharacter>(AttachedPawn);
        if (Character && Character->GetCharacterMovement()->IsFalling())
        {
            return;
        }
    }

    // Restore original settings
    AttachedPawn->bUseControllerRotationYaw = true;
    
    // Set exit position and detach
    AttachedPawn->SetActorRelativeLocation(ExitPosition);
    FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
    AttachedPawn->DetachFromActor(DetachRules);

    ACharacter* Character = Cast<ACharacter>(AttachedPawn);
    if (Character)
    {
        Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
        {
            RestoreOriginalViewLimits(PlayerController);
        }
    }

    OnEject.Broadcast(AttachedPawn);
}

void UFC_Seat::SaveOriginalViewLimits(APlayerController* PlayerController)
{
    if (!IsValid(PlayerController) || !PlayerController->PlayerCameraManager)
        return;

    SavedMaxViewPitch = PlayerController->PlayerCameraManager->ViewPitchMax;
    SavedMinViewPitch = PlayerController->PlayerCameraManager->ViewPitchMin;
    SavedMaxViewYaw = PlayerController->PlayerCameraManager->ViewYawMax;
    SavedMinViewYaw = PlayerController->PlayerCameraManager->ViewYawMin;
}

void UFC_Seat::RestoreOriginalViewLimits(APlayerController* PlayerController)
{
    if (!IsValid(PlayerController) || !PlayerController->PlayerCameraManager)
        return;

    PlayerController->PlayerCameraManager->ViewPitchMax = SavedMaxViewPitch;
    PlayerController->PlayerCameraManager->ViewPitchMin = SavedMinViewPitch;
    PlayerController->PlayerCameraManager->ViewYawMax = SavedMaxViewYaw;
    PlayerController->PlayerCameraManager->ViewYawMin = SavedMinViewYaw;
}

void UFC_Seat::ApplyViewLimits(APlayerController* PlayerController)
{
    if (!IsValid(PlayerController) || !PlayerController->PlayerCameraManager)
        return;

    PlayerController->PlayerCameraManager->ViewPitchMax = MaxViewPitch;
    PlayerController->PlayerCameraManager->ViewPitchMin = MinViewPitch;
    PlayerController->PlayerCameraManager->ViewYawMax = MaxViewYaw;
    PlayerController->PlayerCameraManager->ViewYawMin = MinViewYaw;
}

bool UFC_Seat::CanEject() const
{
    if (!IsValid(AttachedPawn))
        return false;

    if (bPreventDetachInAir)
    {
        const ACharacter* Character = Cast<ACharacter>(AttachedPawn);
        if (Character && Character->GetCharacterMovement()->IsFalling())
        {
            return false;
        }
    }

    return true;
}

void UFC_Seat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UFC_Seat, AttachedPawn);
}

void UFC_Seat::OnRep_AttachedPawn()
{
    if (AttachedPawn)
    {
        UFC_Seat* OldSeat = UFC_Functions::GetActorSeat(AttachedPawn);
        if (IsValid(OldSeat))
        {
            OldSeat->Eject();
        }

        SitMulti(AttachedPawn);
        AttachedPawn->bUseControllerRotationYaw = false;

        ACharacter* Character = Cast<ACharacter>(AttachedPawn);
        if (IsValid(Character) && IsValid(Character->Controller))
        {
            Character->GetCharacterMovement()->DisableMovement();
            Character->Controller->SetControlRotation(GetComponentRotation());

            APlayerController* PlayerController = Cast<APlayerController>(Character->Controller);
            if (IsValid(PlayerController))
            {
                SaveOriginalViewLimits(PlayerController);
                ApplyViewLimits(PlayerController);
            }
        }

        OnSit.Broadcast(AttachedPawn);
    }
}

void UFC_Seat::InitializeComponent()
{
    Super::InitializeComponent();
}

void UFC_Seat::OnComponentCreated()
{
    Super::OnComponentCreated();

#if WITH_EDITOR
    if (bEnablePreviews)
    {
        DebugMeshComponent = NewObject<USkeletalMeshComponent>(this, TEXT("DebugMesh"));
        DebugMeshComponent->RegisterComponent();
        DebugMeshComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
        DebugMeshComponent->SetSkeletalMesh(PreviewMesh);
        DebugMeshComponent->SetRelativeRotation(PreviewRotation);
        DebugMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DebugMeshComponent->SetRelativeLocation(PawnOffset + PreviewLocation);
        DebugMeshComponent->SetHiddenInGame(true);

        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Engine/EngineMaterials/BlinkingCaret.BlinkingCaret'"));
        if (Material)
        {
            for (int32 i = 0; i < DebugMeshComponent->GetNumMaterials(); i++)
            {
                DebugMeshComponent->SetMaterial(i, Material);
            }
        }

        PreviewEjectMeshComponent = NewObject<USkeletalMeshComponent>(this, TEXT("EjectMesh"));
        PreviewEjectMeshComponent->RegisterComponent();
        PreviewEjectMeshComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
        PreviewEjectMeshComponent->SetSkeletalMesh(PreviewMesh);
        PreviewEjectMeshComponent->SetRelativeRotation(PreviewRotation);
        PreviewEjectMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PreviewEjectMeshComponent->SetRelativeLocation(ExitPosition + FVector(0.f, 0.f, -96.f));
        PreviewEjectMeshComponent->SetHiddenInGame(true);

        if (Material)
        {
            for (int32 i = 0; i < PreviewEjectMeshComponent->GetNumMaterials(); i++)
            {
                PreviewEjectMeshComponent->SetMaterial(i, Material);
            }
        }

        if (Pose)
        {
            DebugMeshComponent->PlayAnimation(Pose, true);
        }
    }
#endif
}

void UFC_Seat::Sit_Implementation(APawn* Pawn)
{
    if (AttachedPawn != Pawn)
    {
        AttachedPawn = Pawn;
        OnRep_AttachedPawn();
    }
}

void UFC_Seat::Eject_Implementation()
{
    if (!CanEject())
        return;

    EjectMulti();
    AttachedPawn = nullptr;
    OnRep_AttachedPawn();
}

bool UFC_Seat::IsPawnSeated() const
{
    return IsValid(AttachedPawn);
}

void UFC_Seat::AddForwardInput(float Value)
{
    OnForwardInput.Broadcast(Value);
}

void UFC_Seat::AddRightInput(float Value)
{
    OnRightInput.Broadcast(Value);
}