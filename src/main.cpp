/***********************************************************************************
 *                                      [MAIN]
 * @brief Entry point of the game.
 * @details Most of the .hpp files contain their implementations directly, instead of separate .cpp files.
 * @details This approach is used to simplify development and debugging, avoiding the need to switch between .cpp and .hpp files.
 * @details This game features advanced collision detection using the Separating Axis Theorem (SAT).
 * @details Implemented with Raylib, enabling precise collision detection.
 * @details Several animation utilities are also implemented to simplify sprite animation.
 *
 * MIT License
 *
 * Copyright (c) 2025 Zenzxnse
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ************************************************************************************/

#include <raylib.h>
#include "world.hpp"
#include "basicship.hpp"
#include "playercontroller.hpp"
#include "blb63dreadnaught.hpp"


/* Runs from the last tests performed on collision*/
int main()
{
    InitWindow(2000, 1500, "Hello World!");
    SetTargetFPS(60);

    World world("rsc/Environment/white_local_star_2.png");

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

    Texture2D DarthDreadBigB = util::LoadTextureNN("rsc/DarthBigB.png", 0.5f);

    Texture2D dreadNaught = util::LoadTextureNN("rsc/BLB63dreadnaught.png");

    Texture2D baseEngine = util::LoadTextureNN("rsc/Main Ship/Main Ship - Engines/PNGs/Main Ship - Engines - Base Engine.png", 3);

    Animation flamesIdle = util::MakeStripAnimation(
        "idle", flamesTex, 3, 0.1f);

    Animation flamesPowering = util::MakeStripAnimation(
        "powering", poweringTex, 4, 0.1f);

    Animation baseEngineAnim = util::MakeStripAnimation(
        "baseEngine", baseEngine, 1, 0.1f, Animation::LoopMode::Once);

    auto& player = world.spawn<BasicShip>({ 500, 300 }, hullTex);
    player.addPart(&flamesTex, flamesIdle,
        Vector2{0, 0}, -1);
    player.addPart(&poweringTex, flamesPowering,
            Vector2{0, 0}, -1);

    player.addPart(&baseEngine, baseEngineAnim,
            Vector2{0, 0}, -1);
    
    world.setCameraTarget(&player);

    world.spawn<BLB63DreadNaught>({ 800, 600 }, DarthDreadBigB);
    world.spawn<BLB63DreadNaught>({ 1000, 800 }, dreadNaught);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Vector2 worldMouse =
            util::ScreenToWorld(GetMousePosition(), world.getCamera());
        player.setTarget(worldMouse);

        world.update(dt);

        BeginDrawing();
            world.draw();
        EndDrawing();
    }

    UnloadTexture(hullTex);
    UnloadTexture(flamesTex);
    UnloadTexture(poweringTex);
    UnloadTexture(dreadNaught);
    UnloadTexture(baseEngine);
    CloseWindow();
    return 0;
}
