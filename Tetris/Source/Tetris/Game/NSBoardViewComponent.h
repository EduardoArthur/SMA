// Neon Stack - desenha a grade com Instanced Static Mesh. Sem lógica de jogo.
// Spec: _gdks-output/04-engine/cpp-specs/02-gameplay.md §5

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Simulation/NSTypes.h"
#include "NSBoardViewComponent.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UNSGameConfig;
class FNSGameSimulation;

/**
 * Custom data por instância (7 floats), lidos pelo material via PerInstanceCustomData:
 * [0..2] RGB · [3] PatternIndex (0..6) · [4] Brightness · [5] Pulse · [6] Alpha
 */
UCLASS(ClassGroup = (NeonStack), meta = (BlueprintSpawnableComponent))
class TETRIS_API UNSBoardViewComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNSBoardViewComponent();

	// --- Aparência (preencher no BP_Board) ---
	/** M1: /Engine/BasicShapes/Cube (100 uu, pivô no centro). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	TObjectPtr<UStaticMesh> BlockMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	TObjectPtr<UMaterialInterface> BlockMaterial;

	/** Se vazio, usa BlockMaterial. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	TObjectPtr<UMaterialInterface> GhostMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View", meta = (ClampMin = 1))
	float CellSize = 100.f;

	/** < 1 deixa um pequeno vão entre os blocos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View", meta = (ClampMin = 0.1, ClampMax = 1))
	float BlockScale = 0.95f;

	/** Cores na ordem I, O, T, S, Z, J, L (art-bible §4.1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View", EditFixedSize)
	TArray<FLinearColor> PieceColors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	float FixedBrightness = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	float ActiveBrightness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View")
	float GhostAlpha = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeonStack|View", meta = (ClampMin = 0.01, Units = "s"))
	float PulseDecayTime = 0.15f;

	/** Cria os ISMs. Chamado pelo ANSBoardActor no BeginPlay. */
	void InitializeView(const UNSGameConfig* Config);

	/** Atualiza as instâncias a partir do estado da simulação. */
	void Sync(const FNSGameSimulation& Sim);

	/** Acende os blocos fixos nessas células (Pulse = Strength) e decai em PulseDecayTime. */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|View")
	void PulseCells(const TArray<FIntPoint>& Cells, float Strength = 1.f);

	/** Pinta linhas inteiras de branco (início da limpeza). */
	UFUNCTION(BlueprintCallable, Category = "NeonStack|View")
	void FlashRows(const TArray<int32>& Rows);

	UFUNCTION(BlueprintPure, Category = "NeonStack|View")
	FVector CellToLocal(FIntPoint Cell) const;

	UFUNCTION(BlueprintPure, Category = "NeonStack|View")
	FLinearColor GetPieceColor(ENSPieceType Type) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UInstancedStaticMeshComponent* CreateISM(FName Name, UMaterialInterface* Material);
	void RebuildFixed(const FNSGameSimulation& Sim);
	void UpdatePieceISM(UInstancedStaticMeshComponent* ISM, const FIntPoint Cells[4], bool bVisible, ENSPieceType Type, float Brightness, float Alpha);
	void SetInstanceData(UInstancedStaticMeshComponent* ISM, int32 Index, const FLinearColor& Color, float Pattern, float Brightness, float Pulse, float Alpha);
	FTransform CellTransform(FIntPoint Cell, bool bVisible) const;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> FixedISM;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> ActiveISM;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> GhostISM;

	int32 Width = 10;
	int32 VisibleHeight = 20;
	uint32 LastBoardRevision = MAX_uint32;
	TArray<int32> CellToInstance;   // [Y*Width+X] -> índice no FixedISM, -1 se vazio
	TArray<float> InstancePulse;    // pulso por instância do FixedISM
	TArray<ENSPieceType> InstanceType;

	static constexpr int32 NumCustomFloats = 7;
};
