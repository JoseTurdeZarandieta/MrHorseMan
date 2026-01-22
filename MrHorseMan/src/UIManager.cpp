#include "UIManager.h"
#include "UIButton.h"
#include "UIHp.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"

UIManager::UIManager() :Module()
{
	name = "UIManager";
}

UIManager::~UIManager() {}

bool UIManager::Start()
{
	return true;
}

std::shared_ptr<UIElement> UIManager::CreateUIElement(UIElementType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	std::shared_ptr<UIElement> uiElement = std::make_shared<UIElement>();

	// L16: TODO 1: Implement CreateUIElement function that instantiates a new UIElement according to the UIElementType and add it to the list of UIElements
	//Call the constructor according to the UIElementType
	switch (type)
	{
	case UIElementType::BUTTON:
		uiElement = std::make_shared<UIButton>(id, bounds, text);
		break;
	/*case UIElementType::HP:
		uiElement = std::make_shared<UIHp>(id, bounds, text);
		break;*/
	}

	//Set the observer
	uiElement->observer = observer;

	// Created GuiControls are add it to the list of controls
	UIElementsList.push_back(uiElement);

	return uiElement;
}

std::shared_ptr<UIHPElement> UIManager::CreateUIHPElement(UIHPElementType type, int id, const char* text, int number, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	std::shared_ptr<UIHPElement> uiHPElement = std::make_shared<UIHPElement>();


	switch (type)
	{

	case UIHPElementType::HP:
		uiHPElement = std::make_shared<UIHp>(id, bounds, text, number);
		break;
	}

	//Set the observer
	uiHPElement->HPobserver = observer;

	// Created GuiControls are add it to the list of controls
	UIHPElementsList.push_back(uiHPElement);

	return uiHPElement;
}

bool UIManager::Update(float dt)
{	
	//List to store entities pending deletion
	std::list<std::shared_ptr<UIElement>> pendingDelete;

	for (const auto& uiElement : UIElementsList)
	{
		//If the entity is marked for deletion, add it to the pendingDelete list
		if (uiElement->pendingToDelete)
		{
			pendingDelete.push_back(uiElement);
		}
		else {
			uiElement->Update(dt);
		}
	}

	//Now iterates over the pendingDelete list and destroys the uiElement
	for (const auto uiElement : pendingDelete)
	{
		uiElement->CleanUp();
		UIElementsList.remove(uiElement);
	}

	/*---- ----- ---- ---- UI HP ---- ---- ---- ---- ----*/
	std::list<std::shared_ptr<UIHPElement>> HPpendingDelete;

	for (const auto& uiHPElement : UIHPElementsList)
	{
		//If the entity is marked for deletion, add it to the pendingDelete list
		if (uiHPElement->HPpendingToDelete)
		{
			HPpendingDelete.push_back(uiHPElement);
		}
		else {
			uiHPElement->Update(dt);
		}
	}

	//Now iterates over the pendingDelete list and destroys the uiElement
	for (const auto uiHPElement : HPpendingDelete)
	{
		uiHPElement->CleanUp();
		UIHPElementsList.remove(uiHPElement);
	}



	return true;
}

bool UIManager::CleanUp()
{
	for (const auto& uiElement : UIElementsList)
	{
		uiElement->CleanUp();
	}

	for (const auto& uiHPElement : UIHPElementsList)
	{
		uiHPElement->CleanUp();
	}

	return true;
}


