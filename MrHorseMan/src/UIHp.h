#pragma once

#include "UIElement.h"
#include "EntityManager.h"
#include "Vector2D.h"

class UIHp : public UIElement
{

public:

	UIHp(int id, SDL_Rect bounds, const char* text);
	virtual ~UIHp();

	// Called each loop iteration
	bool Update(float dt);

	bool CleanUp() override;

	int maxHp;
	int currentHp;
	int previousHp = maxHp;

	SDL_Rect hpFullBarRect;
	SDL_Rect hpRedBarRect;
	SDL_Color redColor = { 255, 0, 0, 255 };
	SDL_Color grayColor = { 100, 100, 100, 255 };

	bool HpUpdate(int currentHp, int maxHp, int previousHp);

private:

	bool canClick = false;
	bool drawBasic = false;
};
