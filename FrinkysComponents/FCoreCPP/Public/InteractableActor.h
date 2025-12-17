// InteractableActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FrinkysComponents/General/FC_InteractionPoint.h"
#include "InteractableActor.generated.h"

// Forward declarations
class AGuestPawn;
class AHotelPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStarted, AActor*, Interactor, class AInteractableActor*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionCompleted, AActor*, Interactor, class AInteractableActor*, Interactable);

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Instant         UMETA(DisplayName = "Instant"),        // Happens immediately (light switch, door)
    Duration        UMETA(DisplayName = "Duration"),       // Takes time (smoking, sitting)
    Animated        UMETA(DisplayName = "Animated"),       // Plays animation (sitting, using)
    Transaction     UMETA(DisplayName = "Transaction")     // Costs money/resources (vending, phone)
};

UCLASS()
class FCORECPP_API AInteractableActor : public AActor
{
    GENERATED_BODY()
    
public:
    AInteractableActor();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // ===== COMPONENTS =====
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* InteractableRoot;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFC_InteractionPoint* PlayerInteractionPoint; // For player interactions
    
    // ===== INTERACTION SETTINGS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionName = "Interact";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionType InteractionType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDuration = 0.0f; // How long interaction takes (if Duration type)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionCost = 0.0f; // Money cost (if Transaction type)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bPlayerCanInteract = true; // Can players use this?
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bAICanInteract = true; // Can AI use this?
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanBeUsedByMultiple = false; // Can multiple actors use simultaneously? (bench vs vending)
    
    // ===== STATE (Replicated) =====
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Interaction State")
    bool bIsCurrentlyInUse; // Is someone using this right now?
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Interaction State")
    AActor* CurrentUser; // Who is currently using this
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Interaction State")
    float InteractionStartTime; // When did interaction start (for duration tracking)
    
    // ===== FUNCTIONS =====
    
    // Main interaction function (called by PlayerController on server)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual bool StartInteraction(AActor* Interactor);
    
    // Check if actor can interact with this
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    virtual bool CanInteract(AActor* Interactor) const;
    
    // Complete the interaction (called after duration or immediately)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void CompleteInteraction(AActor* Interactor);
    
    // Cancel/interrupt interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    virtual void CancelInteraction(AActor* Interactor);
    
    // Override these in child classes for specific behavior
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionStarted(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionCompleted(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionCancelled(AActor* Interactor);
    
    // Query functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    bool IsInUse() const { return bIsCurrentlyInUse; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    AActor* GetCurrentUser() const { return CurrentUser; }
    
    // ===== BLUEPRINT EVENTS =====
    
    UPROPERTY(BlueprintAssignable, Category = "Interaction Events")
    FOnInteractionStarted OnInteractionStartedEvent;
    
    UPROPERTY(BlueprintAssignable, Category = "Interaction Events")
    FOnInteractionCompleted OnInteractionCompletedEvent;
    
protected:
    // Player interaction callback (from FC_InteractionPoint)
    UFUNCTION()
    virtual void OnPlayerInteracted(APawn* Interactor);
    
    // Timer for duration-based interactions
    FTimerHandle InteractionTimer;
    
    // Helper to check if actor has enough money (for transactions)
    virtual bool CanAffordInteraction(AActor* Interactor) const;
};