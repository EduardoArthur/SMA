#include "UI/NSPieceIconWidget.h"

#include "Rendering/DrawElements.h"
#include "Simulation/NSPieceTables.h"
#include "Styling/CoreStyle.h"

UNSPieceIconWidget::UNSPieceIconWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const TCHAR* Hex[NSPieceTypeCount] = {
		TEXT("2DE2FF"), TEXT("FFC23D"), TEXT("FF3DCB"), TEXT("9CFF4A"),
		TEXT("FF5E5B"), TEXT("8E6BFF"), TEXT("3D7BFF")
	};
	for (const TCHAR* H : Hex)
	{
		PieceColors.Add(FLinearColor::FromSRGBColor(FColor::FromHex(H)));
	}
}

void UNSPieceIconWidget::SetPiece(ENSPieceType InPiece, bool bInDimmed)
{
	Piece = InPiece;
	bDimmed = bInDimmed;
	Invalidate(EInvalidateWidgetReason::Paint);
}

int32 UNSPieceIconWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	if (Piece == ENSPieceType::None)
	{
		return MaxLayer;
	}

	// Limites da peça no estado 0
	const TConstArrayView<FIntPoint> Cells = FNSPieceTables::GetCells(Piece, 0);
	int32 MinX = MAX_int32, MaxX = MIN_int32, MinY = MAX_int32, MaxY = MIN_int32;
	for (const FIntPoint& C : Cells)
	{
		MinX = FMath::Min(MinX, C.X); MaxX = FMath::Max(MaxX, C.X);
		MinY = FMath::Min(MinY, C.Y); MaxY = FMath::Max(MaxY, C.Y);
	}
	const int32 W = MaxX - MinX + 1;
	const int32 H = MaxY - MinY + 1;

	// Escala fixa de 4×2 células para todas as peças terem o mesmo tamanho de bloco
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const float CellSize = FMath::Min(Size.X / 4.f, Size.Y / 2.f);
	const FVector2D Origin((Size.X - W * CellSize) * 0.5f, (Size.Y - H * CellSize) * 0.5f);
	const float Pad = CellSize * CellPadding;

	const int32 ColorIndex = static_cast<int32>(Piece) - 1;
	FLinearColor Color = PieceColors.IsValidIndex(ColorIndex) ? PieceColors[ColorIndex] : FLinearColor::White;
	if (bDimmed)
	{
		Color = FLinearColor(0.5f, 0.5f, 0.5f, DimmedOpacity);
	}
	Color *= InWidgetStyle.GetColorAndOpacityTint();

	const FSlateBrush* Brush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const int32 DrawLayer = MaxLayer + 1;
	for (const FIntPoint& C : Cells)
	{
		const float X = static_cast<float>(C.X - MinX);
		const float Y = static_cast<float>(MaxY - C.Y); // Y da grade é para cima; na tela é para baixo
		const FVector2D Pos = Origin + FVector2D(X * CellSize + Pad, Y * CellSize + Pad);
		const FVector2D BoxSize(CellSize - 2.f * Pad, CellSize - 2.f * Pad);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			DrawLayer,
			AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(Pos)),
			Brush,
			ESlateDrawEffect::None,
			Color);
	}
	return DrawLayer;
}
