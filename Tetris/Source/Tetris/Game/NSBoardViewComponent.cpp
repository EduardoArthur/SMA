#include "Game/NSBoardViewComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Data/NSGameConfig.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Simulation/NSGameSimulation.h"
#include "UObject/ConstructorHelpers.h"

UNSBoardViewComponent::UNSBoardViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // liga só enquanto houver pulso

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		BlockMesh = CubeFinder.Object;
	}

	// art-bible §4.1 (I, O, T, S, Z, J, L)
	const TCHAR* Hex[NSPieceTypeCount] = {
		TEXT("2DE2FF"), TEXT("FFC23D"), TEXT("FF3DCB"), TEXT("9CFF4A"),
		TEXT("FF5E5B"), TEXT("8E6BFF"), TEXT("3D7BFF")
	};
	for (const TCHAR* H : Hex)
	{
		PieceColors.Add(FLinearColor::FromSRGBColor(FColor::FromHex(H)));
	}
}

void UNSBoardViewComponent::InitializeView(const UNSGameConfig* Config)
{
	if (Config)
	{
		Width = Config->BoardWidth;
		VisibleHeight = Config->VisibleHeight;
	}
	CellToInstance.Init(INDEX_NONE, Width * VisibleHeight);
	LastBoardRevision = MAX_uint32;

	if (!FixedISM)
	{
		FixedISM = CreateISM(TEXT("FixedBlocks"), BlockMaterial);
		GhostISM = CreateISM(TEXT("GhostBlocks"), GhostMaterial ? GhostMaterial.Get() : BlockMaterial.Get());
		ActiveISM = CreateISM(TEXT("ActiveBlocks"), BlockMaterial);

		// 4 instâncias fixas para a peça ativa e para o ghost (começam invisíveis)
		for (int32 i = 0; i < 4; ++i)
		{
			ActiveISM->AddInstance(CellTransform(FIntPoint(0, 0), false));
			GhostISM->AddInstance(CellTransform(FIntPoint(0, 0), false));
		}
	}
}

UInstancedStaticMeshComponent* UNSBoardViewComponent::CreateISM(FName Name, UMaterialInterface* Material)
{
	UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(GetOwner(), Name);
	ISM->SetupAttachment(this);
	ISM->SetStaticMesh(BlockMesh);
	if (Material)
	{
		ISM->SetMaterial(0, Material);
	}
	ISM->SetNumCustomDataFloats(NumCustomFloats);
	ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ISM->SetCastShadow(false);
	ISM->RegisterComponent();
	return ISM;
}

void UNSBoardViewComponent::Sync(const FNSGameSimulation& Sim)
{
	if (!FixedISM)
	{
		return;
	}

	if (Sim.GetBoardRevision() != LastBoardRevision)
	{
		RebuildFixed(Sim);
		LastBoardRevision = Sim.GetBoardRevision();
	}

	const FNSActivePiece& Active = Sim.GetActive();
	FIntPoint ActiveCells[4] = { FIntPoint::ZeroValue, FIntPoint::ZeroValue, FIntPoint::ZeroValue, FIntPoint::ZeroValue };
	FIntPoint GhostCells[4] = { FIntPoint::ZeroValue, FIntPoint::ZeroValue, FIntPoint::ZeroValue, FIntPoint::ZeroValue };
	const bool bHasPiece = Active.IsValid();
	bool bGhostVisible = false;
	if (bHasPiece)
	{
		Active.GetCells(ActiveCells);
		const FNSActivePiece Ghost = Sim.GetGhost();
		Ghost.GetCells(GhostCells);
		bGhostVisible = Ghost.Origin != Active.Origin; // peça já apoiada: o ghost ficaria sobreposto
	}
	UpdatePieceISM(GhostISM, GhostCells, bGhostVisible, Active.Type, ActiveBrightness, GhostAlpha);
	UpdatePieceISM(ActiveISM, ActiveCells, bHasPiece, Active.Type, ActiveBrightness, 1.f);
}

void UNSBoardViewComponent::RebuildFixed(const FNSGameSimulation& Sim)
{
	const FNSBoardGrid& Board = Sim.GetBoard();

	TArray<FTransform> Transforms;
	InstanceType.Reset();
	CellToInstance.Init(INDEX_NONE, Width * VisibleHeight);

	for (int32 Y = 0; Y < VisibleHeight; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const uint8 Value = Board.Get(X, Y);
			if (Value == 0)
			{
				continue;
			}
			CellToInstance[Y * Width + X] = Transforms.Num();
			Transforms.Add(CellTransform(FIntPoint(X, Y), true));
			InstanceType.Add(static_cast<ENSPieceType>(Value));
		}
	}

	FixedISM->ClearInstances();
	if (Transforms.Num() > 0)
	{
		FixedISM->AddInstances(Transforms, /*bShouldReturnIndices*/ false);
	}
	for (int32 i = 0; i < InstanceType.Num(); ++i)
	{
		const ENSPieceType Type = InstanceType[i];
		SetInstanceData(FixedISM, i, GetPieceColor(Type), static_cast<float>(static_cast<int32>(Type) - 1), FixedBrightness, 0.f, 1.f);
	}
	InstancePulse.Init(0.f, InstanceType.Num());
	SetComponentTickEnabled(false);
	FixedISM->MarkRenderStateDirty();
}

void UNSBoardViewComponent::UpdatePieceISM(UInstancedStaticMeshComponent* ISM, const FIntPoint Cells[4], bool bVisible, ENSPieceType Type, float Brightness, float Alpha)
{
	const FLinearColor Color = bVisible ? GetPieceColor(Type) : FLinearColor::Black;
	const float Pattern = bVisible ? static_cast<float>(static_cast<int32>(Type) - 1) : 0.f;

	for (int32 i = 0; i < 4; ++i)
	{
		const FIntPoint Cell = bVisible ? Cells[i] : FIntPoint(0, 0);
		const bool bCellVisible = bVisible && Cell.Y < VisibleHeight;
		ISM->UpdateInstanceTransform(i, CellTransform(Cell, bCellVisible), /*bWorldSpace*/ false, /*bMarkRenderStateDirty*/ false, /*bTeleport*/ true);
		SetInstanceData(ISM, i, Color, Pattern, Brightness, 0.f, Alpha);
	}
	ISM->MarkRenderStateDirty();
}

void UNSBoardViewComponent::SetInstanceData(UInstancedStaticMeshComponent* ISM, int32 Index, const FLinearColor& Color, float Pattern, float Brightness, float Pulse, float Alpha)
{
	const float Data[NumCustomFloats] = { Color.R, Color.G, Color.B, Pattern, Brightness, Pulse, Alpha };
	ISM->SetCustomData(Index, TArrayView<const float>(Data, NumCustomFloats), /*bMarkRenderStateDirty*/ false);
}

FTransform UNSBoardViewComponent::CellTransform(FIntPoint Cell, bool bVisible) const
{
	const float Scale = bVisible ? (CellSize / 100.f) * BlockScale : 0.f;
	return FTransform(FQuat::Identity, CellToLocal(Cell), FVector(Scale));
}

FVector UNSBoardViewComponent::CellToLocal(FIntPoint Cell) const
{
	// levels §2.2: Y = horizontal, Z = vertical, X = profundidade
	return FVector(0.f, Cell.X * CellSize + CellSize * 0.5f, Cell.Y * CellSize + CellSize * 0.5f);
}

FLinearColor UNSBoardViewComponent::GetPieceColor(ENSPieceType Type) const
{
	const int32 Index = static_cast<int32>(Type) - 1;
	return PieceColors.IsValidIndex(Index) ? PieceColors[Index] : FLinearColor::White;
}

void UNSBoardViewComponent::PulseCells(const TArray<FIntPoint>& Cells, float Strength)
{
	if (!FixedISM)
	{
		return;
	}
	bool bAny = false;
	for (const FIntPoint& C : Cells)
	{
		if (C.X < 0 || C.X >= Width || C.Y < 0 || C.Y >= VisibleHeight)
		{
			continue;
		}
		const int32 Inst = CellToInstance[C.Y * Width + C.X];
		if (InstancePulse.IsValidIndex(Inst))
		{
			InstancePulse[Inst] = Strength;
			FixedISM->SetCustomDataValue(Inst, 5, Strength, false);
			bAny = true;
		}
	}
	if (bAny)
	{
		FixedISM->MarkRenderStateDirty();
		SetComponentTickEnabled(true);
	}
}

void UNSBoardViewComponent::FlashRows(const TArray<int32>& Rows)
{
	if (!FixedISM)
	{
		return;
	}
	for (const int32 Y : Rows)
	{
		if (Y < 0 || Y >= VisibleHeight)
		{
			continue;
		}
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Inst = CellToInstance[Y * Width + X];
			if (Inst != INDEX_NONE)
			{
				SetInstanceData(FixedISM, Inst, FLinearColor::White, 0.f, 2.f, 1.f, 1.f);
			}
		}
	}
	FixedISM->MarkRenderStateDirty(); // o próximo RebuildFixed (fim da limpeza) restaura tudo
}

void UNSBoardViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bStillPulsing = false;
	const float Decay = DeltaTime / FMath::Max(PulseDecayTime, KINDA_SMALL_NUMBER);
	for (int32 i = 0; i < InstancePulse.Num(); ++i)
	{
		if (InstancePulse[i] <= 0.f)
		{
			continue;
		}
		InstancePulse[i] = FMath::Max(0.f, InstancePulse[i] - Decay);
		FixedISM->SetCustomDataValue(i, 5, InstancePulse[i], false);
		bStillPulsing |= InstancePulse[i] > 0.f;
	}
	FixedISM->MarkRenderStateDirty();

	if (!bStillPulsing)
	{
		SetComponentTickEnabled(false);
	}
}
