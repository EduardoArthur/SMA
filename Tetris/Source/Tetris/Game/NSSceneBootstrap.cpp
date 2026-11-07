#include "Game/NSSceneBootstrap.h"

#include "Tetris.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Light.h"
#include "Engine/World.h"
#include "Audio/NSAudioDirector.h"
#include "FX/NSBackgroundActor.h"
#include "FX/NSFXDirector.h"
#include "Game/NSBoardActor.h"

namespace
{
	template <typename T>
	T* FindFirst(UWorld* World)
	{
		for (TActorIterator<T> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	template <typename T>
	T* SpawnIfMissing(UWorld* World, const FVector& Location = FVector::ZeroVector, const FRotator& Rotation = FRotator::ZeroRotator)
	{
		if (T* Existing = FindFirst<T>(World))
		{
			return Existing;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		T* Actor = World->SpawnActor<T>(T::StaticClass(), Location, Rotation, Params);
		UE_LOG(LogNeonStack, Log, TEXT("Bootstrap: criado %s"), *T::StaticClass()->GetName());
		return Actor;
	}
}

void NSBootstrap::EnsureScene(UWorld* World, bool bGameplay)
{
	if (!World)
	{
		return;
	}

	if (bGameplay)
	{
		SpawnIfMissing<ANSBoardActor>(World);
	}

	// Câmera: usa a do mapa se houver; garante que alguma esteja ativa para o Player 0
	ACameraActor* Camera = FindFirst<ACameraActor>(World);
	if (!Camera)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Camera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector(-1500.f, 500.f, 1000.f), FRotator::ZeroRotator, Params);
		if (UCameraComponent* Comp = Camera ? Camera->GetCameraComponent() : nullptr)
		{
			Comp->SetProjectionMode(ECameraProjectionMode::Orthographic);
			Comp->SetOrthoWidth(4400.f);
			Comp->SetAutoCalculateOrthoPlanes(false);
			Comp->SetOrthoNearClipPlane(0.f);
			Comp->SetOrthoFarClipPlane(20000.f);
			Comp->SetConstraintAspectRatio(false);
		}
		UE_LOG(LogNeonStack, Log, TEXT("Bootstrap: criada câmera ortográfica"));
	}
	bool bAnyActive = false;
	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		bAnyActive |= It->GetAutoActivatePlayerIndex() == 0;
	}
	if (Camera && !bAnyActive)
	{
		Camera->SetAutoActivateForPlayer(EAutoReceiveInput::Player0);
	}

	// Luz: só se o mapa não tiver nenhuma (materiais Unlit do script não precisam)
	if (!FindFirst<ALight>(World))
	{
		if (ADirectionalLight* Light = SpawnIfMissing<ADirectionalLight>(World, FVector(-2000.f, 500.f, 1500.f), FRotator::ZeroRotator)) // aponta para +X
		{
			if (USceneComponent* LightRoot = Light->GetRootComponent())
			{
				LightRoot->SetMobility(EComponentMobility::Movable);
			}
		}
	}

	SpawnIfMissing<ANSBackgroundActor>(World);
	SpawnIfMissing<ANSAudioDirector>(World);
	if (bGameplay)
	{
		SpawnIfMissing<ANSFXDirector>(World);
	}
}
