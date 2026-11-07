// Neon Stack - desenha uma peça (hold/preview) com caixas Slate. Sem assets.
// Uso: crie WBP_PieceIcon com pai NSPieceIconWidget (designer vazio) e use-o dentro de um SizeBox no HUD.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simulation/NSTypes.h"
#include "NSPieceIconWidget.generated.h"

UCLASS()
class TETRIS_API UNSPieceIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNSPieceIconWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "NeonStack|UI")
	void SetPiece(ENSPieceType InPiece, bool bInDimmed = false);

	UFUNCTION(BlueprintPure, Category = "NeonStack|UI")
	ENSPieceType GetPiece() const { return Piece; }

	/** Cores na ordem I, O, T, S, Z, J, L (art-bible §4.1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", EditFixedSize)
	TArray<FLinearColor> PieceColors;

	/** Opacidade quando o hold está bloqueado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", meta = (ClampMin = 0, ClampMax = 1))
	float DimmedOpacity = 0.35f;

	/** Vão entre células, em fração da célula. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|UI", meta = (ClampMin = 0, ClampMax = 0.4))
	float CellPadding = 0.08f;

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	UPROPERTY(EditAnywhere, Category = "NeonStack|UI")
	ENSPieceType Piece = ENSPieceType::None;

	UPROPERTY(EditAnywhere, Category = "NeonStack|UI")
	bool bDimmed = false;
};
