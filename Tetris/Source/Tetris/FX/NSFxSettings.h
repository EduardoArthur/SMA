// Neon Stack - tradução da opção "Intensidade de FX" em escalas (art-bible §6).

#pragma once

#include "CoreMinimal.h"
#include "Data/NSSaveGame.h"

namespace NSFx
{
	/** Multiplicador de partículas: Baixa 30% / Média 60% / Alta 100%. */
	inline float ParticleScale(ENSFxIntensity I)
	{
		switch (I)
		{
		case ENSFxIntensity::Low:    return 0.3f;
		case ENSFxIntensity::Medium: return 0.6f;
		default:                     return 1.0f;
		}
	}

	/** Flash de tela: Baixa off / Média 50% / Alta 100%. */
	inline float FlashScale(ENSFxIntensity I)
	{
		switch (I)
		{
		case ENSFxIntensity::Low:    return 0.0f;
		case ENSFxIntensity::Medium: return 0.5f;
		default:                     return 1.0f;
		}
	}

	/** Bloom: Baixa 0,5× / Média 0,8× / Alta 1×. */
	inline float BloomScale(ENSFxIntensity I)
	{
		switch (I)
		{
		case ENSFxIntensity::Low:    return 0.5f;
		case ENSFxIntensity::Medium: return 0.8f;
		default:                     return 1.0f;
		}
	}
}
