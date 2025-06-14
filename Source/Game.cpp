// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <vector>
#include "Game.h"
// #include "Components/DrawComponent.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Random.h"
#include "Actors/ParticleSystem.h"
#include <iostream>
#include <fstream>
#include <map>
#include "CSV.h"
#include "Json.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "HUD.h"
#include "UIElements/UIScreen.h"
#include "Actors/DynamicGround.h"
#include "Actors/Fox.h"
#include "Actors/Frog.h"
#include "Actors/Lever.h"
#include "Actors/Trigger.h"

#include "Actors/Fairy.h"
#include "Actors/FlyingShooterEnemy.h"
#include "Actors/Projectile.h"
#include "Components/AABBComponent.h"
#include "Components/DrawComponents/DrawAnimatedComponent.h"


std::vector<int> ParseIntList(const std::string& str) {
    std::vector<int> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(std::stoi(item));
        }
    }

    return result;
}

Game::Game(int windowWidth, int windowHeight, int FPS)
    : mResetLevel(false),
      mWindow(nullptr),
      mRenderer(nullptr),
      mWindowWidth(windowWidth),
      mWindowHeight(windowHeight),
      mLogicalWindowWidth(windowWidth),
      mLogicalWindowHeight(windowHeight),
      mTicksCount(0),
      mIsRunning(true),
      mUpdatingActors(false),
      mFPS(FPS),
      mIsPaused(false),
      mCamera(nullptr),
      mPlayer(nullptr),
      mLevelData(nullptr),
      mLevelDataDynamicGrounds(nullptr),
      mController(nullptr),
      mHitstopActive(false),
      mHitstopDuration(0.15f),
      mHitstopTimer(0.0f),
      mIsSlowMotion(false),
      mIsAccelerated(false),
      mLeftStickYState(StickState::Neutral),
      mBackGroundTexture(nullptr),
      mSky(nullptr),
      mMountains(nullptr),
      mTreesBack(nullptr),
      mTreesFront(nullptr),
      mAudio(nullptr),
      mHUD(nullptr),
      mPauseMenu(nullptr),
      mSceneManagerState(SceneManagerState::None),
      mFadeDuration(0.5f),
      mSceneManagerTimer(0.0f),
      mFadeAlpha(0),
      mGameScene(GameScene::MainMenu),
      mNextScene(GameScene::MainMenu),
      mContinueScene(GameScene::Level4)
{
    // float ratio = mOriginalWindowHeight / static_cast<float>(mLogicalWindowHeight);
    // int tileSize = static_cast<int>(mOriginalTileSize / ratio);
    // mScale = static_cast<float>(tileSize) / mOriginalTileSize;
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("Echoes of Elementum", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               mWindowWidth, mWindowHeight,
                               // SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
                               SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!mWindow) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        SDL_Log("Failed to initialize SDL_mixer");
        return false;
    }

    if (static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight) < mOriginalWindowWidth / mOriginalWindowHeight) {
        mLogicalWindowWidth = static_cast<float>(mWindowWidth);
        mLogicalWindowHeight = static_cast<float>(mWindowWidth) / (mOriginalWindowWidth / mOriginalWindowHeight);
        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);
        float ratio = mOriginalWindowWidth / static_cast<float>(mLogicalWindowWidth);
        int tileSize = static_cast<int>(mOriginalTileSize / ratio);
        mScale = static_cast<float>(tileSize) / mOriginalTileSize;
    }
    else {
        mLogicalWindowWidth = static_cast<float>(mWindowHeight) * (mOriginalWindowWidth / mOriginalWindowHeight);
        mLogicalWindowHeight = static_cast<float>(mWindowHeight);
        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);
        float ratio = mOriginalWindowHeight / static_cast<float>(mLogicalWindowHeight);
        int tileSize = static_cast<int>(mOriginalTileSize / ratio);
        mScale = static_cast<float>(tileSize) / mOriginalTileSize;
    }


    // Esconde o cursor
    SDL_ShowCursor(SDL_DISABLE);

    // Inicializa controle
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            mController = SDL_GameControllerOpen(i);
            if (mController)
                break;
        }
    }

    // --------------
    // TODO - PARTE 3
    // --------------

    // TODO 1. (1 linha): Uma biblioteca Random.h foi incluída nesse projeto para a geração de números aleatórios.
    //  Utilize a função Random::Init para inicializar o gerador de números aleatórios (~1 linha).
    Random::Init();

    mAudio = new AudioSystem(16);

    mTicksCount = SDL_GetTicks();

    // Init all game actors
    // InitializeActors();

    SetGameScene(GameScene::MainMenu);

    // const std::string backgroundAssets = "../Assets/Sprites/Background/";
    // mBackGroundTexture = LoadTexture(backgroundAssets + "fundoCortadoEspichado.png");
    // mBackGroundTexture = LoadTexture(backgroundAssets + "Free-Nature-Backgrounds-Pixel-Art5.png");
    // mBackGroundTexture = LoadTexture(backgroundAssets + "Free-Nature-Backgrounds-Pixel-Art4.png");
    // mSky = LoadTexture(backgroundAssets + "sky_cloud.png");
    // mMountains = LoadTexture(backgroundAssets + "mountain2.png");
    // mTreesBack = LoadTexture(backgroundAssets + "pine1.png");
    // mTreesFront = LoadTexture(backgroundAssets + "pine2.png");

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Verifique se o estado do SceneManager mSceneManagerState é SceneManagerState::None.
    //  Se sim, verifique se a cena passada scene passada como parâmetro é uma das cenas válidas (MainMenu, Level1, Level2).
    //  Se a cena for válida, defina mNextScene como essa nova cena, mSceneManagerState como SceneManagerState::Entering e
    //  mSceneManagerTimer como o tempo de transição passado como parâmetro.
    //  Se a cena for inválida, registre um erro no log e retorne.
    //  Se o estado do SceneManager não for SceneManagerState::None, registre um erro no log e retorne.
    // Verifica se o gerenciador de cenas está pronto para uma nova transição
    if (mSceneManagerState == SceneManagerState::None) {
        // Verifica se a cena é válida
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2 || scene == GameScene::Level3 || scene == GameScene::Level4) {
            mNextScene = scene;
            mSceneManagerState = SceneManagerState::Entering;
            mSceneManagerTimer = transitionTime;
        }
        else {
            SDL_Log("SetGameScene: Cena inválida passada como parâmetro.");
            return;
        }
    }
    else {
        SDL_Log("SetGameScene: Já há uma transição de cena em andamento.");
        return;
    }
}

void Game::ResetGameScene(float transitionTime)
{
    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Chame SetGameScene passando o mGameScene atual e o tempo de transição.
    SetGameScene(mGameScene, transitionTime);
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    const std::string backgroundAssets = "../Assets/Sprites/Background/";

    if (mNextScene != GameScene::MainMenu) {
        // Pool de Fireballs
        for (int i = 0; i < 5; i++) {
            new FireBall(this);
        }

        // Pool de Partículas
        for (int i = 0; i < 200; i++) {
            new Particle(this);
        }

        // Pool de Projectiles
        for (int i = 0; i < 10; i++) {
            new Projectile(this);
        }

        // Volta player
        if (mPlayer) {
            mPlayer->GetComponent<DrawAnimatedComponent>()->SetIsVisible(true);
        }

        // Guarda último level que o player estava
        mContinueScene = mNextScene;
        mIsPaused = false;
    }
    else {
        // Se está no menu, pausa draw de player
        if (mPlayer) {
            mPlayer->GetComponent<DrawAnimatedComponent>()->SetIsVisible(false);
        }
    }

    // Reset gameplay state
    mGamePlayState = GamePlayState::Playing;

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu) {
        mBackGroundTexture = LoadTexture(backgroundAssets + "Menu6.png");

        // Initialize main menu actors
        LoadMainMenu();
    }
    else if (mNextScene == GameScene::Level1) {
        mBackGroundTexture = LoadTexture(backgroundAssets + "Free-Nature-Backgrounds-Pixel-Art5.png");

        const std::string levelsAssets = "../Assets/Levels/";

        LoadLevel(levelsAssets + "Forest/Forest.json");

        mCamera = new Camera(this, Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2,
                                           mPlayer->GetPosition().y - mLogicalWindowHeight / 2));

        mHUD = new HUD(this, "../Assets/Fonts/K2D-Bold.ttf");

        // TODO 1. Toque a música de fundo "MusicMain.ogg" em loop e armaze o SoundHandle retornado em mMusicHandle.
        mMusicHandle = mAudio->PlaySound("Greenpath.wav", true);
        mAudio->CacheSound("Hornet.wav");
        mAudio->CacheSound("MantisLords.wav");
    }
    else if (mNextScene == GameScene::Level2) {
        mBackGroundTexture = LoadTexture(backgroundAssets + "Free-Nature-Backgrounds-Pixel-Art4.png");

        const std::string levelsAssets = "../Assets/Levels/";

        LoadLevel(levelsAssets + "Run/Run.json");

        mCamera = new Camera(this, Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2,
                                           mPlayer->GetPosition().y - mLogicalWindowHeight / 2));

        mHUD = new HUD(this, "../Assets/Fonts/K2D-Bold.ttf");

        // TODO 1. Toque a música de fundo "MusicMain.ogg" em loop e armaze o SoundHandle retornado em mMusicHandle.
        mMusicHandle = mAudio->PlaySound("Greenpath.wav", true);
    }
    else if (mNextScene == GameScene::Level3) {
        mBackGroundTexture = LoadTexture(backgroundAssets + "Free-Nature-Backgrounds-Pixel-Art4.png");

        const std::string levelsAssets = "../Assets/Levels/";

        LoadLevel(levelsAssets + "Pain/Pain.json");

        mCamera = new Camera(this, Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2,
                                           mPlayer->GetPosition().y - mLogicalWindowHeight / 2));

        mHUD = new HUD(this, "../Assets/Fonts/K2D-Bold.ttf");

        // TODO 1. Toque a música de fundo "MusicMain.ogg" em loop e armaze o SoundHandle retornado em mMusicHandle.
        mMusicHandle = mAudio->PlaySound("Greenpath.wav", true);
    }
    else if (mNextScene == GameScene::Level4) {
        mBackGroundTexture = LoadTexture(backgroundAssets + "fundoCortadoEspichado.png");

        const std::string levelsAssets = "../Assets/Levels/";

        LoadLevel(levelsAssets + "Musgo/Musgo.json");

        mCamera = new Camera(this, Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2,
                                           mPlayer->GetPosition().y - mLogicalWindowHeight / 2));

        mHUD = new HUD(this, "../Assets/Fonts/K2D-Bold.ttf");

        // TODO 1. Toque a música de fundo "MusicMain.ogg" em loop e armaze o SoundHandle retornado em mMusicHandle.
        mMusicHandle = mAudio->PlaySound("Greenpath.wav", true);
        mAudio->CacheSound("MantisLords.wav");
    }

    // Set new scene
    mGameScene = mNextScene;
}

void Game::LoadMainMenu()
{
    // --------------
    // TODO - PARTE 1
    // --------------
    // Esse método será usado para criar uma tela de UI e adicionar os elementos do menu principal.
    auto mainMenu = new UIScreen(this, "../Assets/Fonts/K2D-Bold.ttf");
    const Vector2 buttonSize = Vector2(mLogicalWindowWidth / 8, 50 * mScale);
    mainMenu->SetSize(Vector2(mLogicalWindowWidth / 3, mLogicalWindowHeight / 3));
    mainMenu->SetPosition(Vector2(mLogicalWindowWidth / 3, 2 * mLogicalWindowHeight / 3));
    Vector2 buttonPos = Vector2((mainMenu->GetSize().x - buttonSize.x) / 2, 0);
    // buttonPos = Vector2::Zero;

    std::string name = "CONTINUAR";
    Vector2 nameDims = Vector2(30 * name.size(), 44) * mScale;
    int buttonPointSize = static_cast<int>(34 * mScale);
    mainMenu->AddButton(name, buttonPos, buttonSize, buttonPointSize,
    [this]() {SetGameScene(mContinueScene, 0.5f);});

    name = "NOVO JOGO";
    nameDims = Vector2(30 * name.size(), 44) * mScale;
    mainMenu->AddButton(name, buttonPos + Vector2(0, 2 * 35) * mScale, buttonSize, buttonPointSize,
    [this]() {
    SetGameScene(GameScene::Level4, 0.5f);
    delete mPlayer;
    mPlayer = nullptr;
});

    name = "OPÇÕES";
    nameDims = Vector2(30 * name.size(), 44) * mScale;
    mainMenu->AddButton(name, buttonPos + Vector2(0, 4 * 35) * mScale, buttonSize, buttonPointSize,
    nullptr);

    name = "SAIR";
    nameDims = Vector2(30 * name.size(), 44) * mScale;
    mainMenu->AddButton(name, buttonPos + Vector2(0, 6 * 35) * mScale, buttonSize, buttonPointSize,
    [this]() {Quit();});
}

UIScreen* Game::LoadPauseMenu()
{
    // --------------
    // TODO - PARTE 1
    // --------------

    // Esse método será usado para criar uma tela de UI e adicionar os elementos do menu principal.
    auto pauseMenu = new UIScreen(this, "../Assets/Fonts/K2D-Bold.ttf");
    const Vector2 buttonSize = Vector2(mLogicalWindowWidth / 6, 50 * mScale);
    pauseMenu->SetSize(Vector2(mLogicalWindowWidth / 3, mLogicalWindowHeight / 3));
    pauseMenu->SetPosition(Vector2(mLogicalWindowWidth / 3, 5 * mLogicalWindowHeight / 12));
    Vector2 buttonPos = Vector2((pauseMenu->GetSize().x - buttonSize.x) / 2, 0);
    // const Vector2 buttonPos = Vector2::Zero;

    std::string name = "CONTINUAR";
    Vector2 nameDims = Vector2(15 * name.size(), 28) * mScale;
    int buttonPointSize = static_cast<int>(34 * mScale);
    pauseMenu->AddButton(name, buttonPos, buttonSize, buttonPointSize,
    [this]() {TogglePause();});

    name = "OPÇÕES";
    nameDims = Vector2(15 * name.size(), 28) * mScale;
    pauseMenu->AddButton(name, buttonPos + Vector2(0, 2 * 35) * mScale, buttonSize, buttonPointSize,
    nullptr);

    name = "VOLTAR AO MENU";
    nameDims = Vector2(15 * name.size(), 28) * mScale;
    pauseMenu->AddButton(name, buttonPos + Vector2(0, 4 * 35) * mScale, buttonSize, buttonPointSize,
    [this]() {
        SetGameScene(GameScene::MainMenu, 0.5f);
    });

    return pauseMenu;
}

void Game::InitializeActors() {
    // Pool de Fireballs
    for (int i = 0; i < 5; i++) {
        new FireBall(this);
    }

    // Pool de Partículas
    for (int i = 0; i < 200; i++) {
        new Particle(this);
    }

    const std::string levelsAssets = "../Assets/Levels/";

    LoadLevel(levelsAssets + "Forest/Forest.json");
    // LoadLevel(levelsAssets + "Pain/Pain.json");
    // LoadLevel(levelsAssets + "Run/Run.json");
    // LoadLevel(levelsAssets + "Musgo/Musgo.json");

    mCamera = new Camera(this, Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2,
                                       mPlayer->GetPosition().y - mLogicalWindowHeight / 2));
}


void Game::LoadObjects(const std::string &fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Erro ao abrir o arquivo");
        return;
    }
    nlohmann::json mapData;
    file >> mapData;
    Ground *ground;
    for (const auto &layer: mapData["layers"])
    {
        if (layer["name"] == "Grounds")
        {
            for (const auto &obj: layer["objects"])
            {
                std::string name = obj["name"];
                float x = static_cast<float>(obj["x"]) * mScale;
                float y = static_cast<float>(obj["y"]) * mScale;
                float width = static_cast<float>(obj["width"]) * mScale;
                float height = static_cast<float>(obj["height"]) * mScale;
                int id = obj["id"];
                bool isSpike = false;
                bool isMoving = false;
                float respawnPositionX = 0.0f;
                float respawnPositionY = 0.0f;
                float movingDuration = 0.0f;
                float speedX = 0.0f;
                float speedY = 0.0f;
                float growSpeedX = 0.0f;
                float growSpeedY = 0.0f;
                int growthDirection = 0;
                float minHeight = 0.0f;
                float minWidth = 0.0f;
                bool isOscillating = false;

                if (obj.contains("properties")) {
                    for (const auto &prop: obj["properties"]) {
                        std::string propName = prop["name"];
                        if (propName == "Spike") {
                            isSpike = prop["value"];
                        }
                        else if (propName == "Moving") {
                            isMoving = prop["value"];
                        }
                        else if (propName == "RespawnPositionX") {
                            respawnPositionX = static_cast<float>(prop["value"]) * mScale;
                        }
                        else if (propName == "RespawnPositionY") {
                            respawnPositionY = static_cast<float>(prop["value"]) * mScale;
                        }
                        else if (propName == "MovingDuration") {
                            movingDuration = prop["value"];
                        }
                        else if (propName == "SpeedX") {
                            speedX = prop["value"];
                        }
                        else if (propName == "SpeedY") {
                            speedY = prop["value"];
                        }
                        else if (propName == "GrowSpeedX") {
                            growSpeedX = static_cast<float>(prop["value"]);
                        }
                        else if (propName == "GrowSpeedY") {
                            growSpeedY = static_cast<float>(prop["value"]);
                        }
                        else if (propName == "GrowthDirection") {
                            growthDirection = static_cast<int>(prop["value"]);
                        }
                        else if (propName == "MinHeight") {
                            minHeight = static_cast<float>(prop["value"]) * mScale;
                        }
                        else if (propName == "MinWidth") {
                            minWidth = static_cast<float>(prop["value"]) * mScale;
                        }
                        else if (propName == "Oscillate") {
                            isOscillating = static_cast<float>(prop["value"]);
                        }
                    }
                }

                if (name == "DynamicGround") {
                    auto* dynamicGround = new DynamicGround(this, minWidth, minHeight, isSpike, isMoving, movingDuration, Vector2(speedX, speedY));
                    dynamicGround->SetId(id);
                    dynamicGround->SetRespawPosition(Vector2(respawnPositionX, respawnPositionY));
                    dynamicGround->SetIsOscillating(isOscillating);
                    dynamicGround->SetMaxWidth(width);
                    dynamicGround->SetMaxHeight(height);
                    dynamicGround->SetMinWidth(minWidth);
                    dynamicGround->SetMinHeight(minHeight);
                    dynamicGround->SetGrowSpeed(Vector2(growSpeedX, growSpeedY));
                    switch (growthDirection) {
                        case 0:
                            dynamicGround->SetGrowDirection(GrowthDirection::Up);
                            dynamicGround->SetPosition(Vector2(x + width / 2, y + height - minHeight / 2));
                        break;
                        case 1:
                            dynamicGround->SetGrowDirection(GrowthDirection::Down);
                            dynamicGround->SetPosition(Vector2(x + width / 2, y + minHeight / 2));
                        break;
                        case 2:
                            dynamicGround->SetGrowDirection(GrowthDirection::Left);
                            dynamicGround->SetPosition(Vector2(x + width - minWidth / 2, y + height / 2));
                        break;
                        case 3:
                            dynamicGround->SetGrowDirection(GrowthDirection::Right);
                            dynamicGround->SetPosition(Vector2(x + minWidth / 2, y + height / 2));
                        break;
                    }
                    dynamicGround->SetStartingPosition(Vector2(x + width / 2, y + height / 2));
                    dynamicGround->SetSprites();
                }
                else {
                    ground = new Ground(this, width, height, isSpike, isMoving, movingDuration, Vector2(speedX, speedY));
                    ground->SetId(id);
                    ground->SetPosition(Vector2(x + width / 2, y + height / 2));
                    ground->SetRespawPosition(Vector2(respawnPositionX, respawnPositionY));
                    ground->SetStartingPosition(Vector2(x + width / 2, y + height / 2));
                    ground->SetSprites();
                }
            }
        }
        if (layer["name"] == "Triggers") {
            for (const auto &obj: layer["objects"]) {
                float x = static_cast<float>(obj["x"]) * mScale;
                float y = static_cast<float>(obj["y"]) * mScale;
                float width = static_cast<float>(obj["width"]) * mScale;
                float height = static_cast<float>(obj["height"]) * mScale;
                std::string target;
                std::string event;
                std::string grounds;
                std::string enemies;
                std::vector<int> groundsIds;
                std::vector<int> enemiesIds;
                float fixedCameraPositionX = 0;
                float fixedCameraPositionY = 0;
                std::string scene;
                if (obj.contains("properties")) {
                    for (const auto &prop: obj["properties"]) {
                        std::string propName = prop["name"];
                        if (propName == "Target") {
                            target = prop["value"];
                        }
                        else if (propName == "Event") {
                            event = prop["value"];
                        }
                        else if (propName == "Grounds") {
                            grounds = prop["value"];
                        }
                        else if (propName == "Enemies") {
                            enemies = prop["value"];
                        }
                        else if (propName == "FixedCameraPositionX") {
                            fixedCameraPositionX = prop["value"];
                        }
                        else if (propName == "FixedCameraPositionY") {
                            fixedCameraPositionY = prop["value"];
                        }
                        else if (propName == "Scene") {
                            scene = prop["value"];
                        }
                    }
                }
                groundsIds = ParseIntList(grounds);
                enemiesIds = ParseIntList(enemies);

                auto* trigger = new Trigger(this, width, height);
                trigger->SetPosition(Vector2(x + width / 2, y + height / 2));
                trigger->SetTarget(target);
                trigger->SetEvent(event);
                trigger->SetGroundsIds(groundsIds);
                trigger->SetEnemiesIds(enemiesIds);
                trigger->SetFixedCameraPosition(Vector2(fixedCameraPositionX, fixedCameraPositionY));
                trigger->SetScene(scene);
            }
        }
        if (layer["name"] == "Levers") {
            for (const auto &obj: layer["objects"]) {
                float x = static_cast<float>(obj["x"]) * mScale;
                float y = static_cast<float>(obj["y"]) * mScale;
                float width = static_cast<float>(obj["width"]) * mScale;
                float height = static_cast<float>(obj["height"]) * mScale;
                std::string target;
                std::string event;
                std::string grounds;
                std::string enemies;
                std::vector<int> groundsIds;
                std::vector<int> enemiesIds;
                float fixedCameraPositionX = 0;
                float fixedCameraPositionY = 0;
                if (obj.contains("properties")) {
                    for (const auto &prop: obj["properties"]) {
                        std::string propName = prop["name"];
                        if (propName == "Target") {
                            target = prop["value"];
                        }
                        else if (propName == "Event") {
                            event = prop["value"];
                        }
                        else if (propName == "Grounds") {
                            grounds = prop["value"];
                        }
                        else if (propName == "Enemies") {
                            enemies = prop["value"];
                        }
                        else if (propName == "FixedCameraPositionX") {
                            fixedCameraPositionX = prop["value"];
                        }
                        else if (propName == "FixedCameraPositionY") {
                            fixedCameraPositionY = prop["value"];
                        }
                    }
                }
                if ((target == "DynamicGround" || target == "Ground") && !grounds.empty()) {
                    groundsIds = ParseIntList(grounds);
                }
                if (target == "Enemy") {
                    enemiesIds = ParseIntList(enemies);
                }
                auto* lever = new Lever(this);
                lever->SetPosition(Vector2(x + width / 2, y + height / 2));
                lever->SetTarget(target);
                lever->SetEvent(event);
                lever->SetGroundsIds(groundsIds);
                lever->SetEnemiesIds(enemiesIds);
                lever->SetFixedCameraPosition(Vector2(fixedCameraPositionX, fixedCameraPositionY));
            }
        }
        if (layer["name"] == "Enemies")
            for (const auto &obj: layer["objects"])
            {
                std::string name = obj["name"];
                int id = obj["id"];
                float x = static_cast<float>(obj["x"]) * mScale;
                float y = static_cast<float>(obj["y"]) * mScale;
                float MinPosX = 0;
                float MaxPosX = 0;
                float MinPosY = 0;
                float MaxPosY = 0;
                std::string grounds;
                std::vector<int> ids;
                if (name == "Enemy Simple")
                {
                    auto *enemySimple = new EnemySimple(this, 60, 60, 200, 50);
                    enemySimple->SetPosition(Vector2(x, y));
                    enemySimple->SetId(id);
                }
                else if (name == "Flying Enemy")
                {
                    auto *flyingEnemySimple = new FlyingEnemySimple(this, 50, 80, 250, 70);
                    flyingEnemySimple->SetPosition(Vector2(x, y));
                    flyingEnemySimple->SetId(id);
                }
                else if (name == "FlyingShooterEnemy")
                {
                    auto *flyingShooterEnemy = new FlyingShooterEnemy(this, 50, 80, 250, 50);
                    flyingShooterEnemy->SetPosition(Vector2(x, y));
                    flyingShooterEnemy->SetId(id);
                }
                else if (name == "Fox")
                {
                    if (obj.contains("properties")) {
                        for (const auto &prop: obj["properties"]) {
                            std::string propName = prop["name"];
                            if (propName == "UnlockGrounds") {
                                grounds = prop["value"];
                            }
                        }
                    }
                    ids = ParseIntList(grounds);
                    auto *fox = new Fox(this, 100, 170, 300, 200);
                    fox->SetPosition(Vector2(x, y));
                    fox->SetId(id);
                    fox->SetUnlockGroundsIds(ids);
                }
                else if (name == "Frog")
                {
                    if (obj.contains("properties")) {
                        for (const auto &prop: obj["properties"]) {
                            std::string propName = prop["name"];
                            if (propName == "MinPosX") {
                                MinPosX = static_cast<float>(prop["value"]) * mScale;
                            }
                            else if (propName == "MaxPosX") {
                                MaxPosX = static_cast<float>(prop["value"]) * mScale;
                            }
                            else if (propName == "MinPosY") {
                                MinPosY =static_cast<float>(prop["value"]) * mScale;
                            }
                            else if (propName == "MaxPosY") {
                                MaxPosY = static_cast<float>(prop["value"]) * mScale;
                            }
                            else if (propName == "UnlockGrounds") {
                                grounds = prop["value"];
                            }
                        }
                    }
                    ids = ParseIntList(grounds);
                    auto *frog = new Frog(this, 165, 137, 300, 200);
                    frog->SetPosition(Vector2(x, y));
                    frog->SetId(id);
                    frog->SetArenaMinPos(Vector2(MinPosX, MinPosY));
                    frog->SetArenaMaxPos(Vector2(MaxPosX, MaxPosY));
                    frog->SetUnlockGroundsIds(ids);
                }
            }
        if (layer["name"] == "Player")
            for (const auto &obj: layer["objects"])
            {
                float x = static_cast<float>(obj["x"]) * mScale;
                float y = static_cast<float>(obj["y"]) * mScale;
                if (mPlayer) {
                    mPlayer->SetSword();
                    // Faz isso para o player ser sempre o último a ser atualizado a cada frame
                    RemoveActor(mPlayer);
                    AddActor(mPlayer);
                }
                else {
                    mPlayer = new Player(this, 50, 85);
                }
                mPlayer->SetState(ActorState::Active);
                mPlayer->SetPosition(Vector2(x, y));
                mPlayer->SetStartingPosition(Vector2(x, y));
                mPlayer->GetComponent<AABBComponent>()->SetActive(true);
            }
    }
}

void Game::LoadLevel(const std::string &fileName) {
    // Abre arquivo json
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Erro ao abrir o arquivo");
        return;
    }
    nlohmann::json mapData;
    file >> mapData;

    // Lê altura, largura e tileSize
    int height = int(mapData["height"]);
    int width = int(mapData["width"]);
    int tileSize = int(mapData["tilewidth"]) * mScale;
    mLevelHeight = height;
    mLevelWidth = width;
    mTileSize = tileSize;

    // Lê matrizes de tiles
    for (const auto& layer : mapData["layers"]) {
        if (layer["name"] == "Camada de Blocos 1") {
            std::vector<int> data = layer["data"];
            int** matrix = new int*[height];
            for (int i = 0; i < height; ++i) {
                matrix[i] = new int[width];
                for (int j = 0; j < width; ++j) {
                    matrix[i][j] = data[i * width + j];
                }
            }
            mLevelData = matrix;
        } else if (layer["name"] == "DynamicGrounds") {
            std::vector<int> data = layer["data"];
            int** matrix = new int*[height];
            for (int i = 0; i < height; ++i) {
                matrix[i] = new int[width];
                for (int j = 0; j < width; ++j) {
                    matrix[i][j] = data[i * width + j];
                }
            }
            mLevelDataDynamicGrounds = matrix;
        }
    }

    // Load tilesheet texture
    size_t pos = fileName.rfind(".json");
    std::string tileSheetTexturePath = fileName.substr(0, pos) + ".png";
    mTileSheet = LoadTexture(tileSheetTexturePath);
    // mTileSheet = LoadTexture("../Assets/Levels/Forest/Forest.png");
    // mTileSheet = LoadTexture("../Assets/Levels/Musgo/Musgo.png");

    // Load tilesheet data
    std::string tileSheetDataPath = fileName.substr(0, pos) + "TileSet.json";
    std::ifstream tileSheetFile(tileSheetDataPath);
    // std::ifstream tileSheetFile("../Assets/Levels/Forest/ForestTileSet.json");
    // std::ifstream tileSheetFile("../Assets/Levels/Musgo/MusgoTileSet.json");

    nlohmann::json tileSheetData = nlohmann::json::parse(tileSheetFile);

    for (const auto &tile: tileSheetData["sprites"]) {
        std::string tileFileName = tile["fileName"];
        int x = tile["x"].get<int>();
        int y = tile["y"].get<int>();
        int w = tile["width"].get<int>();
        int h = tile["height"].get<int>();

        size_t dotPos = tileFileName.find('.');
        std::string numberStr = tileFileName.substr(0, dotPos);
        int index = std::stoi(numberStr); // converte para inteiro

        mSTileSheetData[index] = SDL_Rect{x, y, w, h};
    }

    // Cria objetos
    LoadObjects(fileName);
}


void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    float oldScale = mScale;
                    mWindowWidth = event.window.data1;
                    mWindowHeight = event.window.data2;
                    if (static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight) < mOriginalWindowWidth / mOriginalWindowHeight) {
                        // Comenta essa parte para tirar o zoom do mapa
                        mLogicalWindowWidth = static_cast<float>(mWindowWidth);
                        mLogicalWindowHeight = static_cast<float>(mWindowWidth) / (mOriginalWindowWidth / mOriginalWindowHeight);
                        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);

                        float ratio = mOriginalWindowWidth / static_cast<float>(mLogicalWindowWidth);
                        int tileSize = static_cast<int>(mOriginalTileSize / ratio);
                        mScale = static_cast<float>(tileSize) / mOriginalTileSize;
                    }
                    else {
                        // Comenta essa parte para tirar o zoom do mapa
                        mLogicalWindowWidth = static_cast<float>(mWindowHeight) * (mOriginalWindowWidth / mOriginalWindowHeight);
                        mLogicalWindowHeight = static_cast<float>(mWindowHeight);
                        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);

                        float ratio = mOriginalWindowHeight / static_cast<float>(mLogicalWindowHeight);
                        int tileSize = static_cast<int>(mOriginalTileSize / ratio);
                        mScale = static_cast<float>(tileSize) / mOriginalTileSize;
                    }
                    // const float ratio = mOriginalWindowHeight / static_cast<float>(mLogicalWindowHeight);
                    // const int tileSize = static_cast<int>(32 / ratio);
                    // mScale = static_cast<float>(tileSize) / 32.0f;
                    ChangeResolution(oldScale);
                }
                break;

            case SDL_KEYDOWN:
                // Handle key press for UI screens
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym, SDL_CONTROLLER_BUTTON_INVALID, 0);
                }

                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    TogglePause();
                }

                if (event.key.keysym.sym == SDLK_8)
                    Quit();

                if (event.key.keysym.sym == SDLK_5) {
                    mIsSlowMotion = !mIsSlowMotion;
                    mIsAccelerated = false;
                }

                if (event.key.keysym.sym == SDLK_6) {
                    mIsAccelerated = !mIsAccelerated;
                    mIsSlowMotion = false;
                }

                break;

            case SDL_CONTROLLERBUTTONDOWN:
                // Handle key press for UI screens
                if (!mUIStack.empty()) {
                    mUIStack.back()->HandleKeyPress(SDLK_UNKNOWN, event.cbutton.button, 0);
                }

                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    TogglePause();
                }

                if (mIsPaused) {
                    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                        if (!mUIStack.empty()) {
                            mUIStack.back()->Close();
                            if (mUIStack.size() == 2) {
                                TogglePause();
                            }
                        }
                    }
                }
                break;

            case SDL_CONTROLLERAXISMOTION:
                if (!mUIStack.empty()) {
                    if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                        int value = event.caxis.value;

                        if (value < -DEAD_ZONE) {
                            if (mLeftStickYState != StickState::Up) {
                                mLeftStickYState = StickState::Up;
                                if (!mUIStack.empty()) {
                                    mUIStack.back()->HandleKeyPress(SDLK_UNKNOWN, SDL_CONTROLLER_BUTTON_INVALID, value);
                                }
                            }
                        }
                        else if (value > DEAD_ZONE) {
                            if (mLeftStickYState != StickState::Down) {
                                mLeftStickYState = StickState::Down;
                                if (!mUIStack.empty()) {
                                    mUIStack.back()->HandleKeyPress(SDLK_UNKNOWN, SDL_CONTROLLER_BUTTON_INVALID, value);
                                }
                            }
                        }
                        else {
                            // Voltou à zona morta
                            mLeftStickYState = StickState::Neutral;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    if (!mIsPaused)
    {
        if (mHitstopActive) {}
        else
            for (auto actor: mActors)
                actor->ProcessInput(state, *mController);
    }
}

void Game::TogglePause() {
    if (mGameScene != GameScene::MainMenu) {
        mIsPaused = !mIsPaused;
        if (mIsPaused) {
            if (mMusicHandle.IsValid()) {
                mAudio->PauseSound(mMusicHandle);
            }
            if (mBossMusic.IsValid()) {
                mAudio->PauseSound(mBossMusic);
            }
            mPauseMenu = LoadPauseMenu();
        }
        else {
            if (mMusicHandle.IsValid()) {
                mAudio->ResumeSound(mMusicHandle);
            }
            if (mBossMusic.IsValid()) {
                mAudio->ResumeSound(mBossMusic);
            }
            mPauseMenu->Close();
            delete mPauseMenu;
            mPauseMenu = nullptr;
        }
        mPlayer->SetCanJump(false);
        mPlayer->SetPrevFireBallPressed(true);
    }
}


void Game::UpdateGame()
{
    // while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 1000.0 / mFPS));

    const auto frameDuration = static_cast<Uint32>(1000.0f / mFPS);
    Uint32 now = SDL_GetTicks();
    if (now < mTicksCount + frameDuration)
        SDL_Delay((mTicksCount + frameDuration) - now);

    float deltaTime = static_cast<float>(SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }

    // Para alterar velocidade do jogo (testes)
    // deltaTime *= 0.5;
    // deltaTime *= 2.5;

    mTicksCount = SDL_GetTicks();

    if (mIsSlowMotion) {
        deltaTime *= 0.5;
    }
    if (mIsAccelerated) {
        deltaTime *= 1.5;
    }

    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255); // Usado para deixar as bordas em preto
    SDL_RenderClear(mRenderer);

    // Update all actors and pending actors
    if (!mIsPaused)
    {
        if (mHitstopActive)
        {
            if (mHitstopTimer < mHitstopDuration) {
                mHitstopTimer += deltaTime;
            }
            else {
                mHitstopActive = false;
            }
        }
        else {
            UpdateActors(deltaTime);
        }
    }

    mAudio->Update(deltaTime);

    // Reinsert UI screens
    for (auto ui : mUIStack) {
        if (ui->GetState() == UIScreen::UIState::Active) {
            ui->Update(deltaTime);
        }
    }

    // Delete any UIElements that are closed
    auto iter = mUIStack.begin();
    while (iter != mUIStack.end()) {
        if ((*iter)->GetState() == UIScreen::UIState::Closing) {
            delete *iter;
            iter = mUIStack.erase(iter);
        } else {
            ++iter;
        }
    }

    if (mResetLevel) {
        // ResetLevel();
        ResetGameScene(3.5f);
        mPlayer->ResetHealthPoints();
        mPlayer->ResetMana();
        mPlayer->ResetHealCount();
        mResetLevel = false;
    }

    UpdateCamera(deltaTime);

    UpdateSceneManager(deltaTime);

}

void Game::UpdateSceneManager(float deltaTime)
{
    // --------------
    // TODO - PARTE 2
    // --------------

    // TODO 1.: Verifique se o estado do SceneManager é SceneManagerState::Entering. Se sim, decremente o mSceneManagerTimer
    //  usando o deltaTime. Em seguida, verifique se o mSceneManagerTimer é menor ou igual a 0. Se for, reinicie o
    //  mSceneManagerTimer para TRANSITION_TIME e mude o estado do SceneManager para SceneManagerState::Active.
    if (mSceneManagerState == SceneManagerState::Entering) {
        mSceneManagerTimer -= deltaTime;

        // Cálculo proporcional da opacidade
        if (mSceneManagerTimer <= mFadeDuration) {
            float progress = 1.0f - (mSceneManagerTimer / mFadeDuration);
            mFadeAlpha = static_cast<Uint8>(progress * 255.0f);
        }

        if (mSceneManagerTimer <= 0.0f) {
            // mSceneManagerTimer = TRANSITION_TIME;  // Reinicia timer para próxima fase
            mSceneManagerTimer = mTransitionTime;  // Reinicia timer para próxima fase
            mSceneManagerState = SceneManagerState::Active;
            mFadeAlpha = 0;
        }
    }

    // TODO 2.: Verifique se o estado do SceneManager é SceneManagerState::Active. Se sim, decremente o mSceneManagerTimer
    //  usando o deltaTime. Em seguida, verifique se o mSceneManagerTimer é menor ou igual a 0. Se for, chame ChangeScene()
    //  e mude o estado do SceneManager para SceneManagerState::None.
    if (mSceneManagerState == SceneManagerState::Active) {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0.0f) {
            ChangeScene();  // Realiza a troca de cena
            mSceneManagerTimer = mFadeDuration;
            mSceneManagerState = SceneManagerState::Exiting;
        }
    }

    if (mSceneManagerState == SceneManagerState::Exiting) {
        mSceneManagerTimer -= deltaTime;

        // Cálculo proporcional da opacidade
        float progress = mSceneManagerTimer / mFadeDuration;
        mFadeAlpha = static_cast<Uint8>(progress * 255.0f);

        if (mSceneManagerTimer <= 0.0f) {
            // mSceneManagerTimer = TRANSITION_TIME;  // Reinicia timer para próxima fase
            mSceneManagerTimer = mTransitionTime;  // Reinicia timer para próxima fase
            mSceneManagerState = SceneManagerState::None;
            mFadeAlpha = 0;
        }
    }
}

void Game::UpdateActors(float deltaTime)
{
    mUpdatingActors = true;
    for (auto actor: mActors)
        actor->Update(deltaTime);

    mUpdatingActors = false;

    for (auto pending: mPendingActors)
        mActors.emplace_back(pending);

    mPendingActors.clear();

    std::vector<Actor *> deadActors;
    for (auto actor: mActors)
        if (actor->GetState() == ActorState::Destroy)
            deadActors.emplace_back(actor);

    for (auto actor: deadActors)
        delete actor;
}

void Game::UpdateCamera(float deltaTime) {
    if (!mCamera) {
        return;
    }
    mCamera->Update(deltaTime);
}

void Game::AddGround(class Ground *g) { mGrounds.emplace_back(g); }

void Game::RemoveGround(class Ground *g)
{
    auto iter = std::find(mGrounds.begin(), mGrounds.end(), g);
    if (iter != mGrounds.end())
        mGrounds.erase(iter);
}

Ground *Game::GetGroundById(int id) {
    for (Ground* g : mGrounds) {
        if (g->GetId() == id) {
            return g;
        }
    }
    return nullptr;
}

void Game::AddFireBall(class FireBall *f) { mFireBalls.emplace_back(f); }

void Game::RemoveFireball(class FireBall *f)
{
    auto iter = std::find(mFireBalls.begin(), mFireBalls.end(), f);
    if (iter != mFireBalls.end())
        mFireBalls.erase(iter);
}

void Game::AddParticle(class Particle *p) { mParticles.emplace_back(p); }

void Game::RemoveParticle(class Particle *p)
{
    auto iter = std::find(mParticles.begin(), mParticles.end(), p);
    if (iter != mParticles.end())
        mParticles.erase(iter);
}

void Game::AddProjectile(class Projectile *p) { mProjectiles.emplace_back(p); }

void Game::RemoveProjectile(class Projectile *p)
{
    auto iter = std::find(mProjectiles.begin(), mProjectiles.end(), p);
    if (iter != mProjectiles.end())
        mProjectiles.erase(iter);
}

void Game::AddEnemy(class Enemy *e) { mEnemies.emplace_back(e); }

void Game::RemoveEnemy(class Enemy *e)
{
    auto iter = std::find(mEnemies.begin(), mEnemies.end(), e);
    if (iter != mEnemies.end())
        mEnemies.erase(iter);
}

Enemy* Game::GetEnemyById(int id) {
    for (Enemy* e : mEnemies) {
        if (e->GetId() == id) {
            return e;
        }
    }
    return nullptr;
}

void Game::AddActor(Actor *actor)
{
    if (mUpdatingActors)
        mPendingActors.emplace_back(actor);

    else
        mActors.emplace_back(actor);
}

void Game::RemoveActor(Actor *actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::sort(mDrawables.begin(), mDrawables.end(), [](const DrawComponent *a, const DrawComponent *b)
    {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    mDrawables.erase(iter);
}

void Game::StarBossMusic(SoundHandle music) {
    mBossMusic = music;
    mAudio->PauseSound(mMusicHandle);
}

void Game::StopBossMusic() {
    mAudio->StopSound(mBossMusic);
    mBossMusic.Reset();
    mAudio->ResumeSound(mMusicHandle);
}

void Game::GenerateOutput()
{
    // Set draw color to green
    // SDL_SetRenderDrawColor(mRenderer, 0, 88, 105, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    if (mCamera) {
        DrawParallaxBackground(); // desenha o fundo com repetição horizontal
        // Ordem de desenho: mais distantes primeiro
        // DrawParallaxLayer(mSky,        0.1f, 0, mWindowHeight / 2);  // camada mais distante
        // DrawParallaxLayer(mMountains,  0.3f, mWindowHeight / 4, mWindowHeight / 3);  // montanhas ao fundo
        // DrawParallaxLayer(mTreesBack,  0.5f, mWindowHeight / 3, mWindowHeight / 2);  // árvores distantes
        // DrawParallaxLayer(mTreesFront, 0.7f, mWindowHeight / 2, mWindowHeight / 2);  // árvores próximas
    }
    else {
        SDL_Rect dest = {
            0,
            0,
            static_cast<int>(mLogicalWindowWidth),
            static_cast<int>(mLogicalWindowHeight)
        };
        SDL_RenderCopy(mRenderer, mBackGroundTexture, nullptr, &dest);
    }

    for (auto drawable: mDrawables)
        drawable->Draw(mRenderer);

    // Draw all UI screens
    for (auto ui :mUIStack)
    {
        ui->Draw(mRenderer);
    }

    if (mSceneManagerState == SceneManagerState::Entering || mSceneManagerState == SceneManagerState::Exiting) {
        SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
        // Define a cor preta (RGBA)
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, mFadeAlpha);

        // Cria o retângulo cobrindo toda a tela
        SDL_Rect fullScreenRect = { 0, 0, mWindowWidth, mWindowHeight };

        // Desenha o retângulo preenchido
        SDL_RenderFillRect(mRenderer, &fullScreenRect);
    }

    if (mSceneManagerState == SceneManagerState::Active) {
        // Define a cor preta (RGBA)
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

        // Cria o retângulo cobrindo toda a tela
        SDL_Rect fullScreenRect = { 0, 0, mWindowWidth, mWindowHeight };

        // Desenha o retângulo preenchido
        SDL_RenderFillRect(mRenderer, &fullScreenRect);
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

SDL_Texture *Game::LoadTexture(const std::string &texturePath)
{
    // TODO 4.1 (~4 linhas): Utilize a função `IMG_Load` para carregar a imagem passada como parâmetro
    //  `texturePath`. Esse função retorna um ponteiro para `SDL_Surface*`. Retorne `nullptr` se a
    //  imagem não foi carregada com sucesso.
    SDL_Surface *surface = IMG_Load(texturePath.c_str());
    if (!surface)
    {
        SDL_Log("Falha ao carregar imagem %s: %s", texturePath.c_str(), IMG_GetError());
        return nullptr;
    }

    // TODO 4.2 (~6 linhas): Utilize a função `SDL_CreateTextureFromSurface` para criar uma textura a partir
    //  da imagem carregada anteriormente. Essa função retorna um ponteiro para `SDL_Texture*`. Logo após criar
    //  a textura, utilize a função `SDL_FreeSurface` para liberar a imagem carregada. Se a textura foi carregada
    //  com sucesso, retorne o ponteiro para a textura. Caso contrário, retorne `nullptr`.
    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface); // Libera a superfície, já não é mais necessária

    if (!texture)
    {
        SDL_Log("Falha ao criar textura a partir de %s: %s", texturePath.c_str(), SDL_GetError());
        return nullptr;
    }
    return texture;
}

UIFont* Game::LoadFont(const std::string& fileName)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Verifique se o arquivo de fonte já está carregado no mapa mFonts.
    //  Se sim, retorne o ponteiro para a fonte carregada.
    //  Se não, crie um novo objeto UIFont, carregue a fonte do arquivo usando o método Load,
    //  e se o carregamento for bem-sucedido, adicione a fonte ao mapa mFonts.
    //  Se o carregamento falhar, descarregue a fonte com Unload e delete o objeto UIFont, retornando nullptr.
    auto iter = mFonts.find(fileName);
    if (iter != mFonts.end()) {
        return iter->second;
    }

    // Fonte ainda não carregada, cria nova instância
    UIFont* font = new UIFont(mRenderer);
    if (font->Load(fileName)) {
        mFonts.emplace(fileName, font);
        return font;
    }
    else {
        // Falha no carregamento — limpa e retorna nullptr
        font->Unload();
        delete font;
        SDL_Log("Falha ao carregar fonte: %s", fileName.c_str());
        return nullptr;
    }
}

void Game::UnloadScene()
{
    if (mPlayer) {
        mPlayer->SetState(ActorState::Paused);
    }

    for (auto it = mActors.begin(); it != mActors.end(); ) {
        Actor* actor = *it;
        if (actor != mPlayer) {
            it = mActors.erase(it);
            delete actor;
        } else {
            ++it;
        }
    }

    // Delete UI screens
    for (auto ui : mUIStack) {
        delete ui;
    }
    mUIStack.clear();

    // Delete level data
    if (mLevelData != nullptr)
    {
        for (int i = 0; i < mLevelHeight; ++i)
        {
            if (mLevelData[i] != nullptr)
                delete[] mLevelData[i];
        }
    }
    delete[] mLevelData;
    mLevelData = nullptr;

    // Delete level data Dynamic Grounds
    if (mLevelDataDynamicGrounds != nullptr)
    {
        for (int i = 0; i < mLevelHeight; ++i)
        {
            if (mLevelDataDynamicGrounds[i] != nullptr)
                delete[] mLevelDataDynamicGrounds[i];
        }
    }
    delete[] mLevelDataDynamicGrounds;
    mLevelDataDynamicGrounds = nullptr;

    SDL_DestroyTexture(mTileSheet);
    mTileSheet = nullptr;

    if (mBackGroundTexture) {
        SDL_DestroyTexture(mBackGroundTexture);
        mBackGroundTexture = nullptr;
    }

    delete mCamera;
    mCamera = nullptr;
}

void Game::Shutdown()
{
    delete mPlayer;
    mPlayer = nullptr;
    UnloadScene();
    // // Delete actors
    // while (!mActors.empty())
    //     delete mActors.back();
    //
    // // Delete level data
    // if (mLevelData != nullptr)
    // {
    //     for (int i = 0; i < mLevelHeight; ++i)
    //     {
    //         if (mLevelData[i] != nullptr)
    //             delete[] mLevelData[i];
    //     }
    // }
    // delete[] mLevelData;
    //
    // // Delete level data Dynamic Grounds
    // if (mLevelDataDynamicGrounds != nullptr)
    // {
    //     for (int i = 0; i < mLevelHeight; ++i)
    //     {
    //         if (mLevelDataDynamicGrounds[i] != nullptr)
    //             delete[] mLevelDataDynamicGrounds[i];
    //     }
    // }
    // delete[] mLevelDataDynamicGrounds;
    //
    // SDL_DestroyTexture(mTileSheet);
    // mTileSheet = nullptr;
    //
    // delete mCamera;

    for (auto font : mFonts) {
        font.second->Unload();
        delete font.second;
    }

    if (mController)
        SDL_GameControllerClose(mController);

    mFonts.clear();

    delete mAudio;
    mAudio = nullptr;

    Mix_CloseAudio();

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::DrawParallaxBackground()
{
    float parallaxFactor = 0.55f; // fundo se move mais devagar que a câmera

    int bgWidth, bgHeight;
    SDL_QueryTexture(mBackGroundTexture, nullptr, nullptr, &bgWidth, &bgHeight);
    bgWidth *= mScale;
    bgHeight *= mScale;
    // Calcula o offset horizontal com base na câmera
    int offsetX = static_cast<int>(mCamera->GetPosCamera().x * parallaxFactor) % bgWidth;
    if (offsetX < 0) offsetX += bgWidth;

    // Desenha blocos horizontais suficientes para cobrir a largura da janela
    for (int x = -offsetX; x < mLogicalWindowWidth; x += bgWidth)
    {
        SDL_Rect dest = {
            x,
            0,
            bgWidth,
            static_cast<int>(mLogicalWindowHeight)
        };

        SDL_RenderCopy(mRenderer, mBackGroundTexture, nullptr, &dest);
    }
}

void Game::DrawParallaxLayer(SDL_Texture *texture, float parallaxFactor, int y, int h)
{
    int texW, texH;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

    int offsetX = static_cast<int>(mCamera->GetPosCamera().x * parallaxFactor) % texW;
    if (offsetX < 0) offsetX += texW;

    for (int x = -offsetX; x < mLogicalWindowWidth; x += texW)
    {
        SDL_Rect dest = {
            x,
            y,
            texW,
            h
        };

        SDL_RenderCopy(mRenderer, texture, nullptr, &dest);
    }
}

void Game::ResetLevel()
{
    // Delete actors
    while (!mActors.empty())
        delete mActors.back();

    // Delete level data
    if (mLevelData != nullptr)
    {
        for (int i = 0; i < mLevelHeight; ++i)
        {
            if (mLevelData[i] != nullptr)
                delete[] mLevelData[i];
        }
    }
    delete[] mLevelData;

    // Delete level data Dynamic Grounds
    if (mLevelDataDynamicGrounds != nullptr)
    {
        for (int i = 0; i < mLevelHeight; ++i)
        {
            if (mLevelDataDynamicGrounds[i] != nullptr)
                delete[] mLevelDataDynamicGrounds[i];
        }
    }
    delete[] mLevelDataDynamicGrounds;

    SDL_DestroyTexture(mTileSheet);
    mTileSheet = nullptr;

    delete mCamera;

    InitializeActors();

    mResetLevel = false;
}

void Game::ChangeResolution(float oldScale)
{
    mTileSize = mOriginalTileSize * mScale;
    for (auto actor : mActors) {
        actor->ChangeResolution(oldScale, mScale);
    }

    if (mCamera) {
        mCamera->ChangeResolution(oldScale, mScale);
        mCamera->SetPosition(Vector2(mPlayer->GetPosition().x - mLogicalWindowWidth / 2, mPlayer->GetPosition().y - mLogicalWindowHeight / 2));
    }

    for (auto UIScreen : mUIStack) {
        UIScreen->ChangeResolution(oldScale, mScale);
    }

    if (static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight) < mOriginalWindowWidth / mOriginalWindowHeight) {
        mLogicalWindowWidth = static_cast<float>(mWindowWidth);
        mLogicalWindowHeight = static_cast<float>(mWindowWidth) / (mOriginalWindowWidth / mOriginalWindowHeight);
        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);
    }
    else {
        mLogicalWindowWidth = static_cast<float>(mWindowHeight) * (mOriginalWindowWidth / mOriginalWindowHeight);
        mLogicalWindowHeight = static_cast<float>(mWindowHeight);
        SDL_RenderSetLogicalSize(mRenderer, mLogicalWindowWidth, mLogicalWindowHeight);
    }
}
