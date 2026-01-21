#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "UIManager.h"
#include "UIHp.h"

Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
    LoadScene(currentScene);
	bool ret = true;

	return ret;
}

bool Scene::Start()
{
    return true;
}

bool Scene::PreUpdate()
{
	return true;
}

bool Scene::Update(float dt)
{

    switch (currentScene)
    {
    case SceneID::INTRO_SCREEN:
        break;
    case SceneID::MAIN_MENU:
        UpdateMainMenu(dt);
        break;
    case SceneID::LEVEL1:
        UpdateLevel1(dt);
        break;
    case SceneID::LEVEL2:
        UpdateLevel2(dt);
        break;
    }

    auto input = Engine::GetInstance().input.get();

    // -------- SAVE GAME (F5) ----------
    if (input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
    {
        LOG("Saving game...");
        SaveGame();
    }

    // -------- LOAD GAME (F6) ----------
    if (input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
    {
        LOG("Loading game...");
        LoadGame();
    }
    if (currentScene != SceneID::MAIN_MENU)
        uiHpBox();

    return true;
}

void Scene::uiHpBox() {
        int currentHP;
        SDL_Rect HPBounds = { 50, 50, 120,20 };
        const char* playerHealth = std::to_string(Engine::GetInstance().scene->player->GetHealth()).c_str();;

        if (Engine::GetInstance().scene->player->maxHealth = 100)               currentHP = Engine::GetInstance().scene->player->maxHealth;

        if (currentHP != Engine::GetInstance().scene->player->GetHealth())
        {
            playerHealth = std::to_string(Engine::GetInstance().scene->player->GetHealth()).c_str();
            std::dynamic_pointer_cast<UIHp>(Engine::GetInstance().uiManager->CreateUIElement(UIElementType::HP, 10, playerHealth, HPBounds, this));
            LOG("Player HP: %d", currentHP);
            currentHP = Engine::GetInstance().scene->player->GetHealth();
        }

}


bool Scene::SaveGame()
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("game_state");

    // Player
    pugi::xml_node playerNode = root.append_child("player");
    Vector2D pos = player->GetPosition();
    playerNode.append_attribute("x") = pos.getX();
    playerNode.append_attribute("y") = pos.getY();

    // Enemies
    pugi::xml_node enemiesNode = root.append_child("enemies");

    int id = 0;
    for (auto& e : Engine::GetInstance().entityManager->entities)
    {
        if (e->type != EntityType::ENEMY) continue;

        Enemy* enemy = dynamic_cast<Enemy*>(e.get());
        if (!enemy) continue;

        pugi::xml_node enemyNode = enemiesNode.append_child("enemy");
        enemyNode.append_attribute("id") = id++;
        enemyNode.append_attribute("x") = enemy->position.getX();
        enemyNode.append_attribute("y") = enemy->position.getY();
        enemyNode.append_attribute("dead") = enemy->pendingToDelete;
    }

    doc.save_file("savegame.xml");
    LOG("Game saved!");
    return true;
}

bool Scene::LoadGame()
{
    pugi::xml_document doc;
    if (!doc.load_file("savegame.xml"))
    {
        LOG("Cannot load savegame.xml");
        return false;
    }

    pugi::xml_node root = doc.child("game_state");
    if (!root) return false;

    // Player
    pugi::xml_node playerNode = root.child("player");
    if (playerNode)
    {
        float x = playerNode.attribute("x").as_float();
        float y = playerNode.attribute("y").as_float();
        player->SetPosition({ x, y });
    }

    // Enemies
    pugi::xml_node enemiesNode = root.child("enemies");
    if (enemiesNode)
    {
        auto entityIt = Engine::GetInstance().entityManager->entities.begin();
        for (pugi::xml_node e = enemiesNode.child("enemy"); e && entityIt != Engine::GetInstance().entityManager->entities.end(); e = e.next_sibling("enemy"))
        {
            // Skip until we find the next enemy in the list
            while (entityIt != Engine::GetInstance().entityManager->entities.end() && (*entityIt)->type != EntityType::ENEMY)
            {
                ++entityIt;
            }
            if (entityIt == Engine::GetInstance().entityManager->entities.end()) break;

            Enemy* en = dynamic_cast<Enemy*>(entityIt->get());
            if (!en) { ++entityIt; continue; }

            en->position.setX(e.attribute("x").as_float());
            en->position.setY(e.attribute("y").as_float());
            en->pendingToDelete = e.attribute("dead").as_bool();

            ++entityIt; // move to the next entity
        }
    }

    LOG("Game loaded!");
    return true;
}


bool Scene::PostUpdate()
{
	bool ret = true;

    switch (currentScene)
    {
    case SceneID::INTRO_SCREEN:
        break;
    case SceneID::MAIN_MENU:
        break;
    case SceneID::LEVEL1:
        PostUpdateLevel1();
        break;
    case SceneID::LEVEL2:
        break;
    default:
        break;
    }

	if(Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

bool Scene::OnUIMouseClickEvent(UIElement* uiElement)
{
    switch (currentScene)
    {
    case SceneID::INTRO_SCREEN:
        break;
    case SceneID::MAIN_MENU:
        HandleMainMenuUIEvents(uiElement);
        break;
    case SceneID::LEVEL1:
        break;
    case SceneID::LEVEL2:
        break;
    default:
        break;
    }

    return true;
}

bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

Vector2D Scene::GetPlayerPosition()
{
    if (player) return player->GetPosition();
    else Vector2D(0, 0);
}

// *********************************************
// Scene change functions
// *********************************************

void Scene::LoadScene(SceneID newScene)
{
    auto& engine = Engine::GetInstance();

    switch (newScene)
    {
    case SceneID::MAIN_MENU:
        LoadMainMenu();
        break;

    case SceneID::LEVEL1:
        LoadLevel1();
		LOG("Level 1 loaded");
        break;

    case SceneID::LEVEL2:
        LoadLevel2();
        break;
    }
}

void Scene::LoadSpawning()
{
    auto spawnObjects = Engine::GetInstance().map->GetObjects("Spawns");

    for (const auto& object : spawnObjects) {

        std::string type = object.type;
        if (type.empty()) {
            auto it = object.properties.find("type");
            if (it == object.properties.end()) it = object.properties.find("Type");
            if (it != object.properties.end()) type = it->second;
        }
        if (type.empty()) type = object.name;

        for (auto& c : type) c = (char)std::tolower((unsigned char)c); //omg la deep web

        if (type == "player") {
            player = std::dynamic_pointer_cast<Player>(Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER));
            if (!player) { LOG("ERROR: Player creation failed"); continue; }
            player->position = { object.x + 16.0f, object.y + 16.0f };
            player->spawnPos = player->position;
        }
        else if (type == "enemy") {
            auto enemy = std::dynamic_pointer_cast<Enemy>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY));
            enemy->position = { object.x + 16.0f, object.y + 16.0f };
            enemy->spawnPos = enemy->position;
        }
        else if (type == "item") {
            auto item = std::dynamic_pointer_cast<Item>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM));
            item->position = { object.x, object.y };
        }
        else {
            LOG("Spawn skipped: unknown type='%s' (name='%s') at (%.1f,%.1f)", type.c_str(), object.name.c_str(), object.x, object.y);
        }
    }

}

void Scene::ChangeScene(SceneID newScene)
{
    UnloadCurrentScene();
    currentScene = newScene;
    LoadScene(currentScene);
}

void Scene::UnloadCurrentScene() {

    switch (currentScene)
    {
    case SceneID::MAIN_MENU:
        UnloadMainMenu();
        break;

    case SceneID::LEVEL1:
        UnloadLevel1();
        break;

    case SceneID::LEVEL2:
        UnloadLevel2();
        break;
    }

}

// *********************************************
// MAIN MENU functions
// *********************************************

void Scene::LoadMainMenu() {

    Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/retro-gaming-short-248416.wav");

    // Instantiate a UIButton in theScene
    
    Engine::GetInstance().map->Load("Assets/Maps/", "MainMenu.tmx");

    SDL_Rect btPos = { 520, 350, 120,20 };
    std::dynamic_pointer_cast<UIButton>(Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 1, "Start Game", btPos, this));
    LOG("Main Menu CREATED");
}

void Scene::UnloadMainMenu() {
    // Clean up UI elements related to the main menu
    //Engine::GetInstance().uiManager->CleanUp();

        // Clean up UI elements related to the Level1
    auto& uiManager = Engine::GetInstance().uiManager;
    uiManager->CleanUp();

    // Reset player reference (sets the shared_ptr to nullptr)
    //player.reset();

    // Clean up map and entities
    Engine::GetInstance().map->CleanUp();
    Engine::GetInstance().entityManager->CleanUp();

}

void Scene::UpdateMainMenu(float dt) {}

void Scene::HandleMainMenuUIEvents(UIElement* uiElement)
{
    switch (uiElement->id)
    {
    case 1: 
        LOG("Main Menu: Start GAme clicked!");
        ChangeScene(SceneID::LEVEL1);
        break;
    default:
        break;
    }
}

// *********************************************
// Level 1 functions
// *********************************************

void Scene::LoadLevel1() {

    Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/level-iv-339695.wav");
    
    //Call the function to load the map. 
    Engine::GetInstance().map->Load("Assets/Maps/", "MapTemplate.tmx");
    LoadSpawning();
}

void Scene::UpdateLevel1(float dt) {
  
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {
        ChangeScene(SceneID::LEVEL2);
    }

}

void Scene::UnloadLevel1() {

    // Clean up UI elements related to the Level1
    auto& uiManager = Engine::GetInstance().uiManager;
    uiManager->CleanUp();

    // Reset player reference (sets the shared_ptr to nullptr)
    player.reset();

    // Clean up map and entities
    Engine::GetInstance().map->CleanUp();
    Engine::GetInstance().entityManager->CleanUp();

}

void  Scene::PostUpdateLevel1() {

    //L15 TODO 3: Call the function to load entities from the map
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
        Engine::GetInstance().map->LoadEntities(player);
    }

    //L15 TODO 4: Call the function to save entities from the map
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
        Engine::GetInstance().map->SaveEntities(player);
    }
}

// *********************************************
// Level 2 functions
// *********************************************

void Scene::LoadLevel2() {

    Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/that-8-bit-music-322062.wav");

    //Call the function to load the map. 
	
    Engine::GetInstance().map->Load("Assets/Maps/", "SecondMap.tmx");
    LoadSpawning();
}

void Scene::UpdateLevel2(float dt) {
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {
        ChangeScene(SceneID::LEVEL1);
    }
}

void Scene::UnloadLevel2() {

    // Clean up UI elements related to the Level2
    auto& uiManager = Engine::GetInstance().uiManager;
    uiManager->CleanUp();

    // Reset player reference (sets the shared_ptr to nullptr)
    player.reset();

    // Clean up map and entities
    Engine::GetInstance().map->CleanUp();
    Engine::GetInstance().entityManager->CleanUp();

}

SceneID Scene::GetCurrentScene() const
{
    return currentScene;
}