// Neon Stack - dados persistidos (recorde + opções).
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §2

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NSSaveGame.generated.h"

UENUM(BlueprintType)
enum class ENSFxIntensity : uint8
{
	Low,
	Medium,
	High
};

USTRUCT(BlueprintType)
struct FNSSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack", meta = (ClampMin = 0, ClampMax = 100))
	int32 MusicVolume = 70;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack", meta = (ClampMin = 0, ClampMax = 100))
	int32 SfxVolume = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack")
	ENSFxIntensity FxIntensity = ENSFxIntensity::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack")
	bool bScreenShake = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack", meta = (ClampMin = 50, ClampMax = 300))
	int32 DasMs = 170;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeonStack", meta = (ClampMin = 0, ClampMax = 100))
	int32 ArrMs = 50;
};

UCLASS()
class TETRIS_API UNSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	int64 HighScore = 0;

	UPROPERTY()
	int32 HighScoreLines = 0;

	UPROPERTY()
	int32 HighScoreLevel = 0;

	UPROPERTY()
	FDateTime HighScoreDate;

	UPROPERTY()
	FNSSettings Settings;
};
