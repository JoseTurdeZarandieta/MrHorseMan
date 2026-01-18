#include "UIHp.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"
#include "Scene.h"
#include "Player.h"

UIHp::UIHp(int id, SDL_Rect bounds, const char* text) : UIElement(UIElementType::HP, id)
{
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
		int currentHp = Engine::GetInstance().player->GetHealth();
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

//bool UIHp::HpUpdate(int currentHp, int maxHp, int previousHp)
//{
//	this->currentHp = Engine::GetInstance().player->GetHealth();
//	maxHp = Engine::GetInstance().player->maxHealth;
//	previousHp = this->previousHp;
//
//	if (maxHp <= 0 || currentHp == previousHp) return false;
//	if (currentHp != previousHp) {
//		//update hp bar
//		previousHp = currentHp;
//		this->previousHp = previousHp;
//		this->currentHp = currentHp;
//		return true;
//	}
//	return false;
//}
