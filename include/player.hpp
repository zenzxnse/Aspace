// Player.hpp -----------------------------------------------------------------

#pragma once
#include "entity.hpp"
#include "spritePart.hpp"
#include <vector>
#include <cmath>

class Player : public Entity
{
public:
    Player(Texture2D& sharedTex, Vector2 pos)
        : extTexture(&sharedTex)          // remember we do NOT own it
    {
        texture = sharedTex;              // shallow copy is fine
        ownsTexture = false;              // flag so ~Entity doesn't unload it
        size      = { (float)texture.width, (float)texture.height };
        position  = pos;
        offset    = { size.x*0.5f, size.y*0.5f };
        recalcCollision();
    }

    // keep the path‑loading ctor for stand‑alone tests
    explicit Player(const std::string& path)
    {
        setTexture(path);
        ownsTexture = true;
        recalcCollision();
    }

    // ---------- parts -------------------------------------------------------
    template<typename... Args>
    void addPart(Texture2D* tex, Vector2 local, int z = 0, Args&&... animCtorArgs)
    {
        parts.emplace_back(SpritePart{ tex, Animation{std::forward<Args>(animCtorArgs)...},
                                       local, z });
    }
    void addPart(Texture2D* tex,
        const Animation& anim,
        Vector2 local,
        int z = 0)
    {
        parts.emplace_back(SpritePart{ tex, anim, local, z });
    }

    // ---------- input / behaviour ------------------------------------------
    void setTarget(Vector2 world) { target = world; }
    void update(float dt, const Camera2D&) override
    {
        /* ----- constant‑speed movement ----------------------------------- */
        Vector2 dir = { target.x - position.x, target.y - position.y };
        float   len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    
        const float SPEED = 300.f;          // pixels per second
    
        if (len > 1e-2f)                    // avoid jitter at the end
        {
            dir.x /= len; dir.y /= len;     // normalise
            float move = std::min(len, SPEED * dt);
            position.x += dir.x * move;
            position.y += dir.y * move;
    
            /* optional: face mouse --------------------------------------- */
            rotation = std::atan2(dir.y, dir.x) * RAD2DEG + 90.0f;
        }
    
        /* ----- update attachments ---------------------------------------- */
        for (auto& p : parts) p.update(dt);
        recalcCollision();
    }

    void draw(const Camera2D&) const override
    {
        static std::vector<const SpritePart*> sorted;
        sorted.clear();
        for (auto& p : parts) sorted.push_back(&p);
        std::sort(sorted.begin(), sorted.end(),
                  [](auto* a, auto* b){ return a->z < b->z; });
    
        for (auto* p : sorted) if (p->z < 0) p->draw(position, rotation);
    
        Rectangle src{0,0,(float)texture.width,(float)texture.height};
        Rectangle dst{position.x, position.y, (float)texture.width, (float)texture.height};
        DrawTexturePro(texture, src, dst, offset, rotation, tint);
    
        for (auto* p : sorted) if (p->z >= 0) p->draw(position, rotation);
    }
    ~Player() override
    {
        if (ownsTexture) { /* base dtor will unload `texture` */ }
        else              { texture.id = 0; }    // prevent double‑free
    }

private:
    std::vector<SpritePart> parts;
    Vector2 target = position;         // where we’re lerping toward
    bool      ownsTexture = false;
    Texture2D* extTexture = nullptr;     // pointer only, not owned
};
