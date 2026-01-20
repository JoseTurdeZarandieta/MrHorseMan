
#include "UIHp.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Scene.h"
#include "Player.h"
#include "EntityManager.h"

UIHp::UIHp(int id, SDL_Rect bounds, const char* text) : UIElement(UIElementType::HP, id) {
	this->bounds = bounds;
	this->text = text;

	canClick = false;
	drawBasic = false;
}

UIHp::~UIHp()
{

}

bool UIHp::Update(float dt)
{
	Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 20, 255, true, false);
	Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h, { 255,255,255,255 });

	return true;
}

bool UIHp::CleanUp()
{
	pendingToDelete = true;
	return true;
}
