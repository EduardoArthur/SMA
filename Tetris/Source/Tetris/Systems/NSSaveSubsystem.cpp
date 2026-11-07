#include "Systems/NSSaveSubsystem.h"

#include "Tetris.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace NSSave
{
	static const TCHAR* SlotName = TEXT("NeonStack_Save");
	constexpr int32 UserIndex = 0;
}

void UNSSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(NSSave::SlotName, NSSave::UserIndex))
	{
		Data = Cast<UNSSaveGame>(UGameplayStatics::LoadGameFromSlot(NSSave::SlotName, NSSave::UserIndex));
	}
	if (!Data)
	{
		Data = Cast<UNSSaveGame>(UGameplayStatics::CreateSaveGameObject(UNSSaveGame::StaticClass()));
	}
	UE_LOG(LogNeonStack, Log, TEXT("Save carregado. Recorde: %lld"), Data ? Data->HighScore : 0LL);
}

UNSSaveSubsystem* UNSSaveSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UNSSaveSubsystem>() : nullptr;
}

FNSSettings UNSSaveSubsystem::GetSettings() const
{
	return Data ? Data->Settings : FNSSettings();
}

void UNSSaveSubsystem::SetSettings(const FNSSettings& NewSettings)
{
	if (!Data)
	{
		return;
	}
	Data->Settings = NewSettings;
	Save();
	OnSettingsChanged.Broadcast(Data->Settings);
}

int64 UNSSaveSubsystem::GetHighScore() const
{
	return Data ? Data->HighScore : 0;
}

int32 UNSSaveSubsystem::GetHighScoreLines() const
{
	return Data ? Data->HighScoreLines : 0;
}

int32 UNSSaveSubsystem::GetHighScoreLevel() const
{
	return Data ? Data->HighScoreLevel : 0;
}

bool UNSSaveSubsystem::SubmitResult(int64 Score, int32 Lines, int32 Level)
{
	if (!Data || Score <= Data->HighScore)
	{
		return false;
	}
	Data->HighScore = Score;
	Data->HighScoreLines = Lines;
	Data->HighScoreLevel = Level;
	Data->HighScoreDate = FDateTime::Now();
	Save();
	return true;
}

void UNSSaveSubsystem::Save()
{
	if (Data)
	{
		UGameplayStatics::SaveGameToSlot(Data, NSSave::SlotName, NSSave::UserIndex);
	}
}
