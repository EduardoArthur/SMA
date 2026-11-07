// Neon Stack - garante uma cena jogável em qualquer mapa (mesmo vazio/Untitled).
// Chamado no StartPlay dos GameModes, ANTES do BeginPlay dos atores.

#pragma once

#include "CoreMinimal.h"

class UWorld;

namespace NSBootstrap
{
	/**
	 * Cria o que estiver faltando:
	 * - bGameplay: ANSBoardActor na origem
	 * - câmera ortográfica (Y=500, Z=1000, largura 4400) ativada para o Player 0
	 * - uma luz direcional, se o mapa não tiver nenhuma luz (para os materiais padrão não ficarem pretos)
	 * - ANSBackgroundActor, ANSAudioDirector e (no jogo) ANSFXDirector
	 */
	void EnsureScene(UWorld* World, bool bGameplay);

	/** Se o jogador não estiver vendo por nenhuma câmera, usa a primeira CameraActor do mapa. Chamar no BeginPlay. */
	void ActivateCamera(UWorld* World);
}
