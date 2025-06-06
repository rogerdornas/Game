cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "game-v0")
project(${PROJECT_NAME})

find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(${PROJECT_NAME}
        Source/Main.cpp
        Source/Math.cpp
        Source/Random.cpp
        Source/Actors/Actor.cpp
        Source/Components/Component.cpp
        Source/Game.cpp
        Source/Components/RigidBodyComponent.cpp
        Source/Components/RigidBodyComponent.h
        Source/Components/CircleColliderComponent.cpp
        Source/Components/CircleColliderComponent.h
        Source/Actors/Particle.cpp
        Source/Actors/Particle.h
        Source/Actors/ParticleSystem.cpp
        Source/Actors/ParticleSystem.h
        Source/Actors/Player.cpp
        Source/Actors/Player.h
        Source/Components/AABBComponent.cpp
        Source/Components/AABBComponent.h
        Source/Actors/Ground.cpp
        Source/Actors/Ground.h
        Source/Camera.cpp
        Source/Camera.h
        Source/Components/DashComponent.cpp
        Source/Components/DashComponent.h
        Source/Actors/Sword.cpp
        Source/Actors/Sword.h
        Source/Actors/FireBall.cpp
        Source/Actors/FireBall.h
        Source/Actors/EnemySimple.cpp
        Source/Actors/EnemySimple.h
        Source/Actors/Enemy.cpp
        Source/Actors/Enemy.h
        Source/Actors/FlyingEnemySimple.cpp
        Source/Actors/FlyingEnemySimple.h
        Source/Json.h
        Source/CSV.h
        Source/Components/DrawComponents/DrawAnimatedComponent.cpp
        Source/Components/DrawComponents/DrawAnimatedComponent.h
        Source/Components/DrawComponents/DrawComponent.cpp
        Source/Components/DrawComponents/DrawComponent.h
        Source/Components/DrawComponents/DrawPolygonComponent.cpp
        Source/Components/DrawComponents/DrawPolygonComponent.h
        Source/Components/DrawComponents/DrawSpriteComponent.cpp
        Source/Components/DrawComponents/DrawSpriteComponent.h
        Source/Components/DrawComponents/DrawGroundSpritesComponent.cpp
        Source/Components/DrawComponents/DrawGroundSpritesComponent.h
        Source/Actors/Fox.cpp
        Source/Actors/Fox.h
        Source/Actors/Trigger.cpp
        Source/Actors/Trigger.h
        Source/Actors/DynamicGround.cpp
        Source/Actors/DynamicGround.h
        Source/Components/DrawComponents/DrawDynamicGroundSpritesComponent.cpp
        Source/Components/DrawComponents/DrawDynamicGroundSpritesComponent.h
        Source/Actors/Lever.cpp
        Source/Actors/Lever.h
        Source/Components/DrawComponents/DrawParticleComponent.cpp
        Source/Components/DrawComponents/DrawParticleComponent.h
        Source/Actors/Effect.cpp
        Source/Actors/Effect.h
        Source/Actors/Frog.cpp
        Source/Actors/Frog.h
        Source/Actors/FrogTongue.cpp
        Source/Actors/FrogTongue.h
)


#target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

