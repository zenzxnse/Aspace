/* ───────────────────────────  World.hpp  ──────────────────────────── */
#pragma once
#include <raylib.h>
#include <memory>
#include <vector>
#include <algorithm>        
#include <type_traits>

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
    static constexpr int WORLD_W   = 10'000;
    static constexpr int WORLD_H   = 15'000;
    static constexpr int CELL_SIZE = 512;

    /* ctor -------------------------------------------------------------- */
    World()
    : grid(WORLD_W, WORLD_H, CELL_SIZE)
    {
        camera.offset   = { GetScreenWidth()*0.5f, GetScreenHeight()*0.5f };
        camera.rotation = 0.0f;
        camera.zoom     = 1.0f;
        camera.target   = { WORLD_W*0.5f, WORLD_H*0.5f };
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

        if constexpr (std::is_base_of_v<CameraTarget, T>) cameraFollow = static_cast<CameraTarget*>(&ref);
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
        for (auto& e : entities)
            {
                Cell      oldCell = bucketOf(e->getPosition());
                Rectangle oldBox  = e->getCollision();

                e->update(dt, camera);                      // entity logic

                // NEW: keep the rectangle inside world bounds
                keepInside(oldBox, e->getMutablePosition());
                //   ^ you can expose a non-const accessor or add a wrapper in Entity:
                //     Vector2& getMutablePosition() { return position; }

                Cell newCell = bucketOf(e->getPosition());
                if (oldCell != newCell) {
                    grid.remove(e.get(), oldBox);
                    grid.insert(e.get(), e->getCollision());
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
        ClearBackground(BLACK);
        for (int x=0; x<=WORLD_W; x+=32) DrawLine(x, 0, x, WORLD_H, Fade(LIGHTGRAY,0.3f));
        for (int y=0; y<=WORLD_H; y+=32) DrawLine(0, y, WORLD_W, y, Fade(LIGHTGRAY,0.3f));
    }

    /* data -------------------------------------------------------------- */
    UniformGrid                                       grid;
    std::vector<std::unique_ptr<Entity>>              entities;
    Camera2D                                          camera;
    CameraTarget* cameraFollow = nullptr;
};
/* ───────────────────────────────────────────────────────────────────── */
