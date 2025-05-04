/*────────────────── main.cpp (using PlayerController) ─────────────────*/
#include <raylib.h>
#include "utilities.hpp"           // LoadTextureNN, MakeStripAnimation …
#include "world.hpp"
#include "basicship.hpp"           // renamed “old Player” → BasicShip
#include "playercontroller.hpp"    // template wrapper

int main()
{
    InitWindow(2000, 1200, "Hello World!");
    SetTargetFPS(60);

    /* 1.  World ------------------------------------------------------ */
    World world;

    /* 2.  Shared textures ------------------------------------------- */
    Texture2D hullTex   = util::LoadTextureNN(
        "rsc/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Full health.png",
        3);
    Texture2D flamesTex = util::LoadTextureNN(
        "rsc/Main Ship/Main Ship - Engine Effects/PNGs/"
        "Main Ship - Engines - Base Engine - Idle.png",
        3);
    Texture2D poweringTex = util::LoadTextureNN(
        "rsc/Main Ship/Main Ship - Engine Effects/PNGs/"
        "Main Ship - Engines - Base Engine - Powering.png",
        3);

    Texture2D baseEngine = util::LoadTextureNN("rsc/Main Ship/Main Ship - Engines/PNGs/Main Ship - Engines - Base Engine.png", 3);

    /* 3.  Make a ship animation bank -------------------------------- */
    Animation flamesIdle = util::MakeStripAnimation(
        "idle", flamesTex, 3, 0.1f);

    Animation flamesPowering = util::MakeStripAnimation(
        "powering", poweringTex, 4, 0.1f);

    Animation baseEngineAnim = util::MakeStripAnimation(
        "baseEngine", baseEngine, 1, 0.1f, Animation::LoopMode::Once);


    /* 4.  Spawn the player controller + ship ------------------------ */
    using MyPlayer = PlayerController<BasicShip>;

    //  - first arg: spawn position (World::spawn appends it)
    //  - following args are forwarded to BasicShip ctor
    auto& player = world.spawn<MyPlayer>(
        Vector2{500, 300},          // ← world position
        hullTex                    // BasicShip(Texture2D&,Vector2)
        );

    // bolt thruster onto the internal ship
    player.getShip().addPart(&flamesTex, flamesIdle,
                             Vector2{0, 0}, -1);
    player.getShip().addPart(&poweringTex, flamesPowering,
                             Vector2{0, 0}, -1);

    player.getShip().addPart(&baseEngine, baseEngineAnim,
                             Vector2{0, 0}, -1);

    /* 5.  Main loop -------------------------------------------------- */
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        /* 5a.  Give controller the current mouse target */
        Vector2 worldMouse =
            util::ScreenToWorld(GetMousePosition(), world.getCamera());
        player.setTarget(worldMouse);

        /* 5b.  Update everything */
        world.update(dt);

        /* 5c.  Draw frame */
        BeginDrawing();
            world.draw();
        EndDrawing();
    }

    /* 6.  Cleanup ---------------------------------------------------- */
    UnloadTexture(hullTex);
    UnloadTexture(flamesTex);
    CloseWindow();
    return 0;
}
