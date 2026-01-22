#pragma once

#include "Input.h"
#include "Render.h"
#include "Module.h"
#include "Vector2D.h"

enum class UIHPElementType
{
	HP
};

enum class UIHPElementState
{
	NORMAL

};

class UIHPElement : public std::enable_shared_from_this<UIHPElement>
{
public:

	UIHPElement() {}

	// Constructor
	UIHPElement(UIHPElementType type, int id) : HPtype(type), HPid(id), HPstate(UIHPElementState::NORMAL) {}

	// Constructor
	UIHPElement(UIHPElementType type, SDL_Rect bounds, const char* text, int number) :
		HPtype(type),
		HPstate(UIHPElementState::NORMAL),
		HPtext(text),
		HPnumber(number)
	{
		HPcolor.r = 255; HPcolor.g = 255; HPcolor.b = 255;
		HPtexture = NULL;
	}


	// Called each loop iteration
	virtual bool Update(float dt)
	{
		return true;
	}

	// 
	void SetTexture(SDL_Texture* tex)
	{
		HPtexture = tex;
		HPsection = { 0, 0, 0, 0 };
	}

	// 
	void SetObserver(Module* module)
	{
		HPobserver = module;
	}

	// 


	virtual bool CleanUp()
	{
		return true;
	}

	virtual bool Destroy()
	{
		return true;
	}

public:

	int HPid;
	UIHPElementType HPtype;
	UIHPElementState HPstate;

	std::string HPtext;       // UIElement text (if required)
	int HPnumber;
	SDL_Rect HPbounds;        // Position and size
	SDL_Rect HPboundsPlus;
	SDL_Color HPcolor;        // Tint color

	SDL_Texture* HPtexture;   // Texture atlas reference
	SDL_Rect HPsection;       // Texture atlas base section

	Module* HPobserver;        // Observer 

	bool HPpendingToDelete = false;
};