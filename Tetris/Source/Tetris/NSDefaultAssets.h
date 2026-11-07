// Neon Stack - caminhos padrão dos assets criados pelo script de setup (Scripts/setup_neon_stack.py).
// Quando um campo de asset fica vazio no editor, o código carrega o asset padrão por caminho.
// Assim o jogo funciona com as classes C++ colocadas direto no mapa, sem precisar criar Blueprints.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

namespace NSAssets
{
	// Dados
	inline const TCHAR* GameConfig   = TEXT("/Game/NeonStack/Data/DA_GameConfig.DA_GameConfig");
	inline const TCHAR* Palettes     = TEXT("/Game/NeonStack/Data/DA_Palettes.DA_Palettes");

	// Materiais
	inline const TCHAR* Collection   = TEXT("/Game/NeonStack/Art/Materials/MPC_NeonStack.MPC_NeonStack");
	inline const TCHAR* BlockMat     = TEXT("/Game/NeonStack/Art/Materials/M_Block.M_Block");
	inline const TCHAR* GhostMat     = TEXT("/Game/NeonStack/Art/Materials/M_Ghost.M_Ghost");

	// Mapas
	inline const TCHAR* GameMap      = TEXT("/Game/NeonStack/Maps/L_Game.L_Game");
	inline const TCHAR* MenuMap      = TEXT("/Game/NeonStack/Maps/L_MainMenu.L_MainMenu");

	// Input
	inline const TCHAR* InputDir     = TEXT("/Game/NeonStack/Input/");

	/** Carrega um asset padrão (síncrono). Retorna nullptr se não existir. */
	template <typename T>
	T* Load(const TCHAR* Path)
	{
		return Cast<T>(FSoftObjectPath(Path).TryLoad());
	}

	/** Carrega um asset da pasta de input pelo nome (ex.: "IA_MoveLeft"). */
	template <typename T>
	T* LoadInput(const FString& Name)
	{
		const FString Path = FString(InputDir) + Name + TEXT(".") + Name;
		return Cast<T>(FSoftObjectPath(Path).TryLoad());
	}

	/** Se Ptr estiver vazio, preenche com o asset padrão. */
	template <typename T, typename PtrType>
	void FillIfEmpty(PtrType& Ptr, const TCHAR* Path)
	{
		if (!Ptr)
		{
			Ptr = Load<T>(Path);
		}
	}
}
