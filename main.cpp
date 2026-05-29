#include "include\core\Game.h"
#include "core\LevelManager.h"
#include "component\PhysicsComponent.h"
#include "Utils\AABB.h"

int main() {
    Game game;
    game.Init();
    game.Run();
    game.Cleanup();
    return 0;
}