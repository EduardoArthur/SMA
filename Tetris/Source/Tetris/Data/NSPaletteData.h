// Neon Stack - paletas de ambiente (art-bible §4.3). Asset: DA_Palettes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NSPaletteData.generated.h"

USTRUCT(BlueprintType)
struct FNSPalette
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor SkyTop = FLinearColor::Black;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor SkyHorizon = FLinearColor::Black;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor SunTop = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor SunBottom = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor Grid = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack") FLinearColor Accent = FLinearColor::White;
};

UCLASS(BlueprintType)
class TETRIS_API UNSPaletteData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UNSPaletteData();

	/** Sunset, Midnight, Toxic, Inferno (art-bible §4.3). Ciclam após a última. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack")
	TArray<FNSPalette> Palettes;

	const FNSPalette& GetPalette(int32 Index) const;
};
