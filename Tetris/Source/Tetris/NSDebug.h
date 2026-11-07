// Neon Stack - mensagens de diagnóstico na tela (só em builds de desenvolvimento) + log.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Tetris.h"

namespace NSDebug
{
	inline void Screen(const FString& Message, bool bError = false, float Seconds = 15.f)
	{
		if (bError)
		{
			UE_LOG(LogNeonStack, Error, TEXT("%s"), *Message);
		}
		else
		{
			UE_LOG(LogNeonStack, Log, TEXT("%s"), *Message);
		}
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, Seconds, bError ? FColor::Red : FColor::Cyan, TEXT("[NeonStack] ") + Message);
		}
#endif
	}
}
