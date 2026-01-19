
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
	if (Engine::GetInstance().scene->GetCurrentScene() == SceneID::MAIN_MENU) {
		state = UIElementState::DISABLED;

	}
	else if (Engine::GetInstance().scene->GetCurrentScene() == SceneID::LEVEL1 || Engine::GetInstance().scene->GetCurrentScene() == SceneID::LEVEL2) {
		state = UIElementState::NORMAL;
	}
	if (state == UIElementState::DISABLED) {
		return false;
	}
	if (state == UIElementState::NORMAL) {
		int currentHp = Engine::GetInstance().scene->player->health;
		const char* currentHpStr = std::to_string(currentHp).c_str();
		Engine::GetInstance().render->DrawText( currentHpStr, bounds.x, bounds.y, bounds.w, bounds.h, { 255,255,255,255 });
	}

	return false;
}

bool UIHp::CleanUp()
{
	pendingToDelete = true;
	return true;
}
