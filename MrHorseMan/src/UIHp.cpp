
#include "UIHp.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"

UIHp::UIHp(int id, SDL_Rect bounds, const char* text, int number) : UIHPElement(UIHPElementType::HP, id) {
	this->HPbounds = bounds;
	this->HPtext = text;
	//this->HPnumber = number;
	
	canClick = false;
	drawBasic = false;
}

UIHp::~UIHp()
{

}

bool UIHp::Update(float dt)
{
	Engine::GetInstance().render->DrawRectangle(HPbounds, 0, 0, 20, 255, true, false);
	Engine::GetInstance().render->DrawText(HPtext.c_str(), HPbounds.x, HPbounds.y, HPbounds.w, HPbounds.h, { 255,255,255,255 });

	//char str[128];
	//sprintf_s(str, "%8d", HPnumber);
	//SDL_Surface* sur = TTF_RenderText_Solid(Engine::GetInstance().render->font, str, 128, grayColor);
	//Engine::GetInstance().render->DrawText(str, HPboundsPlus.x, HPboundsPlus.y, HPboundsPlus.w, HPboundsPlus.h, {255,255,255,255});



	return true;
}

bool UIHp::CleanUp()
{
	HPpendingToDelete = true;
	return true;
}
