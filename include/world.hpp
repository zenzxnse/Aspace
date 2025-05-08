/* ───────────────────────────  World.hpp  ──────────────────────────── */
#pragma once
#include <raylib.h>
#include <raymath.h>
#include <memory>
#include <vector>
#include "utilities.hpp"
#include <algorithm>        
#include <type_traits>
#include <cassert>
#include "collisionshapes.hpp"

#include <entity.hpp>
#include <playercontroller.hpp>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   UniformGrid  – simple fixed-size spatial index
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class UniformGrid {
public:
    UniformGrid(int worldW, int worldH, int cellSz)
        : cs(cellSz),
          cols((worldW + cellSz - 1) / cellSz),
          rows((worldH + cellSz - 1) / cellSz),
          buckets(cols * rows) {}

    void insert(Entity* e, const Rectangle& box)  { visitCells(box, [&](int i){ buckets[i].push_back(e); }); }
    void remove(Entity* e, const Rectangle& box)  { visitCells(box, [&](int i){ auto& v=buckets[i]; v.erase(std::remove(v.begin(),v.end(),e),v.end()); }); }

    template<typename Fn>
    void query(const Rectangle& area, Fn&& fn) const { visitCells(area, [&](int i){ for (auto* e : buckets[i]) fn(*e); }); }

private:
    int cs, cols, rows;
    std::vector<std::vector<Entity*>> buckets;

    template<typename Fn>
    void visitCells(const Rectangle& r, Fn&& fn) const {
        int minX = std::clamp(int(r.x              /cs), 0, cols-1);
        int minY = std::clamp(int(r.y              /cs), 0, rows-1);
        int maxX = std::clamp(int((r.x+r.width)  /cs), 0, cols-1);
        int maxY = std::clamp(int((r.y+r.height) /cs), 0, rows-1);
        for (int y=minY; y<=maxY; ++y)
            for (int x=minX; x<=maxX; ++x) fn(y*cols + x);
    }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   World – owns entities, camera, spatial grid
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class World {
public:
    /* public constants -------------------------------------------------- */
    static constexpr int WORLD_W   = 8192 * 2;
    static constexpr int WORLD_H   = 4096 * 2;
    static constexpr int CELL_SIZE = 512;

    /* ctor -------------------------------------------------------------- */

    void setCameraTarget(Entity* e) {
        cameraFollow = e;
    }

    World(const char* bgTexPath = nullptr)
    : grid(WORLD_W, WORLD_H, CELL_SIZE)
    {
        camera.offset   = { GetScreenWidth()*0.5f, GetScreenHeight()*0.5f };
        camera.rotation = 0.0f;
        camera.zoom     = 1.0f;
        camera.target   = { WORLD_W*0.5f, WORLD_H*0.5f };
        backgroundTex = util::LoadTextureNN(bgTexPath ? bgTexPath : "../rsc/Environment/white_local_star_2.png", 2);
    }

    /* generic spawner --------------------------------------------------- */
    template<typename T, typename... Args>
    T& spawn(Vector2 pos, Args&&... ctorArgs)
    {
        static_assert(std::is_base_of_v<Entity, T>, "spawn<T>: T must derive from Entity");

        auto ptr = std::make_unique<T>(std::forward<Args>(ctorArgs)..., pos);
        Entity& ref = *ptr;
        entities.emplace_back(std::move(ptr));

        grid.insert(&ref, ref.getCollision());

        // if constexpr (std::is_base_of_v<CameraTarget, T>) cameraFollow = static_cast<CameraTarget*>(&ref);
        return static_cast<T&>(ref);
    }
    void keepInside(Rectangle& box, Vector2& pos)
    {
        float halfW = box.width  * 0.5f;
        float halfH = box.height * 0.5f;

        pos.x = std::clamp(pos.x, halfW, WORLD_W - halfW);
        pos.y = std::clamp(pos.y, halfH, WORLD_H - halfH);

        box.x = pos.x - halfW;
        box.y = pos.y - halfH;
    }

    /* per-frame --------------------------------------------------------- */
    void update(float dt)
    {
        // First pass: let each entity run its own logic & keep it in its cell
        for (auto& ePtr : entities)
        {
            Entity& E = *ePtr;
            if (!E.isAliveAndCollidable()) continue;
    
            // Remember old cell & bbox
            Cell      oldCell = bucketOf(E.getPosition());
            Rectangle oldBox  = E.getOverallAABB();
    
            // Actually update the entity (movement, AI, shape.updateWorldVertices, etc.)
            E.update(dt, camera);
    
            // Keep it inside the world bounds (if you like)
            keepInside(oldBox, E.getMutablePosition());
    
            // If it moved to a new cell, update the grid
            Cell newCell = bucketOf(E.getPosition());
            if (oldCell != newCell) {
                grid.remove(&E, oldBox);
                grid.insert(&E, E.getOverallAABB());
            }

            zoomControl(); // Handle zoom control here
        }
    
        // Second pass: broad‐phase via grid + narrow‐phase SAT collisions
        for (auto& ePtr : entities)
        {
            Entity& A = *ePtr;
            if (!A.isAliveAndCollidable()) continue;

            // grab A’s rough AABB and collect all possible B’s
            Rectangle aabbA = A.getOverallAABB();
            std::vector<Entity*> candidates;
            grid.query(aabbA, [&](Entity& e){ candidates.push_back(&e); });

            // Now do your narrow‐phase on that snapshot
            for (Entity* B : candidates)
            {
                if (B == &A) continue;
                if (!B->isAliveAndCollidable()) continue;
                // only do each pair once
                if (&A > B) continue;

                Vector2 mtv;
                if (CollisionSystem::CheckShapesCollide(A.shape, B->shape, mtv))
                {
                    // resolve collision by moving both out by half the MTV
                    Vector2 half = Vector2Scale(mtv, 0.5f);
                    A.setPosition(Vector2Subtract(A.getPosition(), half));
                    B->setPosition(Vector2Add   (B->getPosition(), half));

                    // now update the grid entries *after* you’re done querying
                    grid.remove(&A, aabbA);
                    grid.insert(&A, A.getOverallAABB());
                }
            }
        }
    
        if (cameraFollow) camera.target = cameraFollow->getPosition();
        clampCamera();
    }

    void draw()
    {
        BeginMode2D(camera);
            drawBackground();

            Rectangle view = expandedView(64);   // small margin
            grid.query(view, [&](Entity& e){ e.draw(camera); });
        EndMode2D();
    }

    void zoomControl()
    {
        float wheel = GetMouseWheelMove();
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {
            if (wheel > 0) targetZoom = std::min(targetZoom + 0.2f, 3.0f);
            else if (wheel < 0) targetZoom = std::max(targetZoom - 0.2f, 0.5f);
        }
    }

    /* teleport-safe ----------------------------------------------------- */
    void teleport(Entity& e, Vector2 newPos)
    {
        grid.remove(&e, e.getCollision());
        e.setPosition(newPos);
        grid.insert(&e, e.getCollision());
    }

    /* expose camera (read-only) ---------------------------------------- */
    const Camera2D& getCamera() const { return camera; }

private:
    /* helper struct for quick bucket compare --------------------------- */
    struct Cell { int x, y; bool operator!=(const Cell& o) const { return x!=o.x || y!=o.y; } };

    Cell bucketOf(Vector2 p) const { return { int(p.x) / CELL_SIZE, int(p.y) / CELL_SIZE }; }

    Rectangle expandedView(float margin) const
    {
        return { camera.target.x - GetScreenWidth()*0.5f - margin,
                 camera.target.y - GetScreenHeight()*0.5f - margin,
                 (float)GetScreenWidth()  + margin*2,
                 (float)GetScreenHeight() + margin*2 };
    }

    void clampCamera()
    {
        float halfW = GetScreenWidth()*0.5f;
        float halfH = GetScreenHeight()*0.5f;
        camera.target.x = std::clamp(camera.target.x, halfW, WORLD_W - halfW);
        camera.target.y = std::clamp(camera.target.y, halfH, WORLD_H - halfH);
    }

    /* background -------------------------------------------------------- */
    void drawBackground() const
    {
        Rectangle src { 0, 0, (float)backgroundTex.width,  (float)backgroundTex.height };
        Rectangle dst { 0, 0, (float)backgroundTex.width,  (float)backgroundTex.height };
        DrawTexturePro(backgroundTex, src, dst, {0,0}, 0.0f, WHITE);
    }

    /* data -------------------------------------------------------------- */
    UniformGrid                                       grid;
    std::vector<std::unique_ptr<Entity>>              entities;
    Camera2D                                          camera;
    Entity* cameraFollow = nullptr;
    Texture2D                                         backgroundTex{};  
    float   targetZoom     = 1.0f;      // where we want to go
    float   zoomSmoothSpeed = 8.0f;     // the larger, the snappier
};
/* ───────────────────────────────────────────────────────────────────── */
