// GuestData.h
#pragma once

#include "CoreMinimal.h"
#include "GuestData.generated.h"

UENUM(BlueprintType)
enum class EGuestType : uint8
{
    Normal          UMETA(DisplayName = "Normal Guest"),
    SerialKiller    UMETA(DisplayName = "Serial Killer"),
    Alien           UMETA(DisplayName = "Alien (Disguised)"),
    Unstable        UMETA(DisplayName = "Unstable Traveler"),
    Vagrant         UMETA(DisplayName = "Vagrant (No Payment)")
};

USTRUCT(BlueprintType)
struct FCORECPP_API FGuestData
{
    GENERATED_BODY()
    
    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GuestName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IDNumber; // e.g. "A12345678"
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* IDPhoto; // For UI display
    
    // Payment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreditCardName; // Name on card
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreditCardNumber; // e.g. "4532 1234 5678 9010"
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CardExpiration;
    
    // Stay details
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NightsStaying; // 1-3 nights
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RoomRate; // How much they'll pay per night
    
    // Guest type (hidden from player)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGuestType GuestType;
    
    // Suspicion level (for Papers Please gameplay)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuspicionLevel; // 0-1 (affects AI behavior)
    
    // Validation flags
    UPROPERTY(BlueprintReadOnly)
    bool bIDMatchesCard; // Does name on ID match card?
    
    UPROPERTY(BlueprintReadOnly)
    bool bCardExpired; // Is card past expiration?
    
    UPROPERTY(BlueprintReadOnly)
    bool bOnWatchlist; // Is this person flagged? (future feature)
    
    // Constructor with defaults
    FGuestData()
    {
        GuestName = "John Doe";
        IDNumber = "A00000000";
        CreditCardName = "John Doe";
        CreditCardNumber = "0000 0000 0000 0000";
        CardExpiration = FDateTime::Now() + FTimespan::FromDays(365);
        NightsStaying = 1;
        RoomRate = 75.0f;
        GuestType = EGuestType::Normal;
        SuspicionLevel = 0.0f;
        bIDMatchesCard = true;
        bCardExpired = false;
        bOnWatchlist = false;
        IDPhoto = nullptr;
    }
    
    // Helper: Validate guest data (check for mismatches)
    void ValidateData()
    {
        // Check if names match
        bIDMatchesCard = (GuestName.Equals(CreditCardName, ESearchCase::IgnoreCase));
        
        // Check if card is expired
        bCardExpired = (CardExpiration < FDateTime::Now());
    }
};