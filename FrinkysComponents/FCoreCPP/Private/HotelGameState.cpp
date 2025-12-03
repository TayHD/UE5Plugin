// HotelGameState.cpp
#include "HotelGameState.h"
#include "Net/UnrealNetwork.h"

AHotelGameState::AHotelGameState()
{
    // Start with some seed money
    CurrentRunCash = 500.0f;
    CurrentRunProfit = 0.0f;
    TotalBankBalance = 0.0f;
    
    // Enable replication
    bReplicates = true;
}

void AHotelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Replicate economy values to all clients
    DOREPLIFETIME(AHotelGameState, CurrentRunCash);
    DOREPLIFETIME(AHotelGameState, CurrentRunProfit);
    DOREPLIFETIME(AHotelGameState, TotalBankBalance);
    DOREPLIFETIME(AHotelGameState, Upgrades);
}

void AHotelGameState::Server_AddIncome_Implementation(float Amount, EIncomeSource Source)
{
    // Only server can modify economy
    if (!HasAuthority())
    {
        return;
    }
    
    // Add to cash
    CurrentRunCash += Amount;
    
    // Update profit
    UpdateProfit(Amount, true);
    
    // Trigger replication
    OnRep_CurrentCash();
    
    // Get source name for logging
    FString SourceName;
    switch (Source)
    {
        case EIncomeSource::RoomRental:     SourceName = "Room Rental"; break;
        case EIncomeSource::VendingMachine: SourceName = "Vending Machine"; break;
        case EIncomeSource::PhoneCalls:     SourceName = "Phone Calls"; break;
        case EIncomeSource::Tips:           SourceName = "Tips"; break;
    }
    
    // Broadcast transaction event
    OnTransactionMade.Broadcast(Amount, true, SourceName);
    
    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Income: +$%.2f from %s (New Total: $%.2f)"), 
           Amount, *SourceName, CurrentRunCash);
}

void AHotelGameState::Server_AddExpense_Implementation(float Amount, EExpenseSource Source)
{
    // Only server can modify economy
    if (!HasAuthority())
    {
        return;
    }
    
    // Subtract from cash
    CurrentRunCash -= Amount;
    
    // Update profit
    UpdateProfit(Amount, false);
    
    // Trigger replication
    OnRep_CurrentCash();
    
    // Get source name for logging
    FString SourceName;
    switch (Source)
    {
        case EExpenseSource::Refund:    SourceName = "Refund"; break;
        case EExpenseSource::Supplies:  SourceName = "Supplies"; break;
        case EExpenseSource::Repairs:   SourceName = "Repairs"; break;
        case EExpenseSource::Utilities: SourceName = "Utilities"; break;
        case EExpenseSource::Fines:     SourceName = "Fines"; break;
    }
    
    // Broadcast transaction event
    OnTransactionMade.Broadcast(Amount, false, SourceName);
    
    // Log for debugging
    UE_LOG(LogTemp, Warning, TEXT("Expense: -$%.2f from %s (New Total: $%.2f)"), 
           Amount, *SourceName, CurrentRunCash);
}

void AHotelGameState::Server_BankProfit_Implementation()
{
    // Only server can bank profit
    if (!HasAuthority())
    {
        return;
    }
    
    // Only bank positive profit
    if (CurrentRunProfit > 0.0f)
    {
        TotalBankBalance += CurrentRunProfit;
        
        UE_LOG(LogTemp, Log, TEXT("Banked Profit: $%.2f (Total Bank: $%.2f)"), 
               CurrentRunProfit, TotalBankBalance);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No profit to bank (Profit: $%.2f)"), 
               CurrentRunProfit);
    }
}

void AHotelGameState::Server_ResetRunEconomy_Implementation()
{
    // Only server can reset
    if (!HasAuthority())
    {
        return;
    }
    
    // Reset current run values
    CurrentRunCash = 500.0f; // Starting money
    CurrentRunProfit = 0.0f;
    
    // Trigger replication
    OnRep_CurrentCash();
    
    UE_LOG(LogTemp, Log, TEXT("Economy reset for new run (Starting Cash: $%.2f)"), 
           CurrentRunCash);
}

void AHotelGameState::OnRep_CurrentCash()
{
    // This fires on clients when CurrentRunCash changes
    // Broadcast to UI so it can update
    float Delta = 0.0f; // Could track last value to calculate delta
    OnCashChanged.Broadcast(CurrentRunCash, Delta);
}

void AHotelGameState::UpdateProfit(float Delta, bool bIsIncome)
{
    if (bIsIncome)
    {
        CurrentRunProfit += Delta;
    }
    else
    {
        CurrentRunProfit -= Delta;
    }
}