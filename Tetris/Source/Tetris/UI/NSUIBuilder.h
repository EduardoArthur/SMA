// Neon Stack - helpers para montar layouts de UMG em C++ (usados quando não há WBP).
// Se o widget for um Widget Blueprint (WBP_*), o layout do designer é usado e nada disto roda.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace NSUI
{
	// Cores (art-bible §4, §8)
	inline FLinearColor Accent()  { return FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF3DCB"))); }
	inline FLinearColor Cyan()    { return FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2DE2FF"))); }
	inline FLinearColor Amber()   { return FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFC23D"))); }
	inline FLinearColor Panel()   { return FLinearColor(0.02f, 0.015f, 0.04f, 0.85f); }

	/** true se o widget é a classe C++ pura (sem Widget Blueprint). */
	inline bool IsNative(const UUserWidget* Widget)
	{
		return Widget && !Cast<UWidgetBlueprintGeneratedClass>(Widget->GetClass());
	}

	/** Garante uma WidgetTree antes do UUserWidget::Initialize (para os bindings existirem no NativeOnInitialized). */
	inline bool PrepareNativeTree(UUserWidget* Widget)
	{
		if (!IsNative(Widget) || Widget->WidgetTree)
		{
			return false;
		}
		Widget->WidgetTree = NewObject<UWidgetTree>(Widget, TEXT("WidgetTree"), RF_Transient);
		return true;
	}

	inline UTextBlock* Text(UWidgetTree* Tree, const FString& Str, int32 Size, const FLinearColor& Color = FLinearColor::White, bool bCenter = false)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		T->SetText(FText::FromString(Str));
		T->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", Size));
		T->SetColorAndOpacity(FSlateColor(Color));
		if (bCenter)
		{
			T->SetJustification(ETextJustify::Center);
		}
		return T;
	}

	inline UButton* Button(UWidgetTree* Tree, const FString& Label, int32 Size = 26)
	{
		UButton* B = Tree->ConstructWidget<UButton>(UButton::StaticClass());
		B->SetBackgroundColor(FLinearColor(0.15f, 0.05f, 0.25f, 1.f));
		B->AddChild(Text(Tree, Label, Size, FLinearColor::White, true));
		return B;
	}

	inline UImage* Rect(UWidgetTree* Tree, const FLinearColor& Color)
	{
		UImage* I = Tree->ConstructWidget<UImage>(UImage::StaticClass());
		I->SetColorAndOpacity(Color);
		return I;
	}

	inline USizeBox* Sized(UWidgetTree* Tree, UWidget* Child, float W, float H)
	{
		USizeBox* S = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		S->SetWidthOverride(W);
		S->SetHeightOverride(H);
		S->AddChild(Child);
		return S;
	}

	inline UVerticalBox* VBox(UWidgetTree* Tree)
	{
		return Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	}

	inline UVerticalBoxSlot* AddV(UVerticalBox* Box, UWidget* Child, float Bottom = 8.f, EHorizontalAlignment HAlign = HAlign_Fill)
	{
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
		Slot->SetPadding(FMargin(0.f, 0.f, 0.f, Bottom));
		Slot->SetHorizontalAlignment(HAlign);
		return Slot;
	}

	inline UHorizontalBox* Row(UWidgetTree* Tree, UWidget* Label, UWidget* Control, UWidget* Value = nullptr, float LabelWidth = 240.f)
	{
		UHorizontalBox* H = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UHorizontalBoxSlot* L = H->AddChildToHorizontalBox(Sized(Tree, Label, LabelWidth, 40.f));
		L->SetVerticalAlignment(VAlign_Center);
		UHorizontalBoxSlot* C = H->AddChildToHorizontalBox(Sized(Tree, Control, 320.f, 40.f));
		C->SetVerticalAlignment(VAlign_Center);
		if (Value)
		{
			UHorizontalBoxSlot* V = H->AddChildToHorizontalBox(Value);
			V->SetPadding(FMargin(16.f, 0.f, 0.f, 0.f));
			V->SetVerticalAlignment(VAlign_Center);
		}
		return H;
	}

	/** Canvas raiz da árvore. */
	inline UCanvasPanel* RootCanvas(UWidgetTree* Tree)
	{
		UCanvasPanel* Canvas = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		Tree->RootWidget = Canvas;
		return Canvas;
	}

	/** Filho ancorado num ponto da tela (0..1), com alinhamento e deslocamento em pixels. */
	inline UCanvasPanelSlot* Place(UCanvasPanel* Canvas, UWidget* Child, FVector2D Anchor, FVector2D Alignment, FVector2D Offset = FVector2D::ZeroVector)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		Slot->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(Offset);
		Slot->SetAutoSize(true);
		return Slot;
	}

	/** Filho cobrindo a tela inteira. */
	inline UCanvasPanelSlot* Fill(UCanvasPanel* Canvas, UWidget* Child)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		Slot->SetOffsets(FMargin(0.f));
		return Slot;
	}
}
