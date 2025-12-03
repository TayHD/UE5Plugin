// HotelGameState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HotelGameState.generated.h"

// Forward declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCashChanged, float, NewAmount, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTransactionMade, float, Amount, bool, bIsIncome, FString, Source);

UENUM(BlueprintType)
enum class EIncomeSource : uint8
{
    RoomRental      UMETA(DisplayName = "Room Rental"),
    VendingMachine  UMETA(DisplayName = "Vending Machine"),
    PhoneCalls      UMETA(DisplayName = "Phone Calls"),
    Tips            UMETA(DisplayName = "Tips")
};

UENUM(BlueprintType)
enum class EExpenseSource : uint8
{
    Refund          UMETA(DisplayName = "Guest Refund"),
    Supplies        UMETA(DisplayName = "Supplies"),
    Repairs         UMETA(DisplayName = "Repairs"),
    Utilities       UMETA(DisplayName = "Utilities"),
    Fines           UMETA(DisplayName = "Fines")
};

USTRUCT(BlueprintType)
struct FHotelUpgrades
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 RoomTier = 1;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    int32 NumRooms = 10;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasBackupGenerator = false;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasSecurityCameras = false;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasMotionSensors = false;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasPepperSpray = false;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasStunBaton = false;
    
    UPROPERTY(SaveGame, BlueprintReadWrite)
    bool bHasPanicButton = false;
};

UCLASS()
class FCORECPP_API AHotelGameState : public AGameStateBase
{
    GENERATED_BODY()
    
public:
    AHotelGameState();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // ===== ECONOMY =====
    
    // Current run cash (can go negative)
    UPROPERTY(ReplicatedUsing=OnRep_CurrentCash, BlueprintReadOnly, Category = "Economy")
    float CurrentRunCash;
    
    // Profit this run (income - expenses)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Economy")
    float CurrentRunProfit;
    
    // Total saved money (persists across runs)
    UPROPERTY(SaveGame, Replicated, BlueprintReadOnly, Category = "Economy")
    float TotalBankBalance;
    
    // Unlocked upgrades (persists across runs)
    UPROPERTY(SaveGame, Replicated, BlueprintReadOnly, Category = "Upgrades")
    FHotelUpgrades Upgrades;
    
    // ===== FUNCTIONS =====
    
    // Add income (room rental, vending sales, etc.)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Economy")
    void Server_AddIncome(float Amount, EIncomeSource Source);
    
    // Add expense (refunds, supplies, repairs, etc.)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Economy")
    void Server_AddExpense(float Amount, EExpenseSource Source);
    
    // Bank the current profit (end of run, saves to TotalBankBalance)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Economy")
    void Server_BankProfit();
    
    // Reset current run values (called at start of new run)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Economy")
    void Server_ResetRunEconomy();
    
    // ===== REPLICATION CALLBACKS =====
    
    UFUNCTION()
    void OnRep_CurrentCash();
    
    // ===== BLUEPRINT EVENTS =====
    
    // Fires when cash changes (for UI updates)
    UPROPERTY(BlueprintAssignable, Category = "Economy")
    FOnCashChanged OnCashChanged;
    
    // Fires when transaction happens (for notifications)
    UPROPERTY(BlueprintAssignable, Category = "Economy")
    FOnTransactionMade OnTransactionMade;
    
private:
    // Helper to update profit calculation
    void UpdateProfit(float Delta, bool bIsIncome);
};