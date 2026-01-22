#pragma once

#include "Entity.h"
#include <SDL3/SDL.h>

struct SDL_Texture;

class ChangeLevel : public Entity
{
public:

	ChangeLevel();
	virtual ~ChangeLevel();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	bool Destroy();

	void ResetToSpawn();

	bool isPicked = false;
	Vector2D spawnPos;
	int texW, texH;

private:

	SDL_Texture* texture;
	const char* texturePath;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
