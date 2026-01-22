#pragma once

#include "Module.h"
#include "UIElement.h"
#include "UIHPElement.h"

#include <list>

class UIManager : public Module
{
public:

	// Constructor
	UIManager();

	// Destructor
	virtual ~UIManager();

	// Called before the first frame
	 bool Start();

	 // Called each loop iteration
	 bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	std::shared_ptr<UIElement> CreateUIElement(UIElementType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds = { 0,0,0,0 });

	std::shared_ptr<UIHPElement> CreateUIHPElement(UIHPElementType type, int id, const char* text, int number, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds = { 0,0,0,0 });

public:

	std::list<std::shared_ptr<UIElement>> UIElementsList;
	std::list<std::shared_ptr<UIHPElement>> UIHPElementsList;

	SDL_Texture* texture;

};
