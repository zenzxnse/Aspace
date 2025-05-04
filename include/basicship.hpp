/*────────────────────────── basicShip.hpp ──────────────────────────*/
#pragma once
#include "entity.hpp"
#include "spritePart.hpp"
#include <vector>
#include <cmath>

/*  A direct rename of your old Player class.
 *  Nothing in its behaviour changed — idle movement, parts, drawing. */
class BasicShip : public Entity
{
public:
    /*  ▸ share‑texture ctor (recommended) */
    BasicShip(Texture2D& sharedTex, Vector2 pos)
        : extTexture(&sharedTex)
    {
        texture = sharedTex;            // shallow copy
        size    = { (float)texture.width, (float)texture.height };
        position= pos;
        offset  = { size.x*0.5f, size.y*0.5f };
        recalcCollision();
    }

    /*  ▸ self‑loading ctor (optional) */
    explicit BasicShip(const std::string& path, Vector2 pos = {0,0})
    {
        setTexture(path);
        position = pos;
        ownsTexture = true;
        recalcCollision();
    }

    /* -------- sprites / parts ------------------------------------ */
    template<typename... Args>
    void addPart(Texture2D* tex, Vector2 local, int z, Args&&...animArgs)
    {
        parts.emplace_back(SpritePart{ tex, Animation{std::forward<Args>(animArgs)...},
                                       local, z });
    }
    void addPart(Texture2D* tex, const Animation& anim,
                 Vector2 local, int z = 0)
    {
        parts.emplace_back(SpritePart{ tex, anim, local, z });
    }

    /* -------- simple movement API -------------------------------- */
    void setTarget(Vector2 world) { target = world; }

    /* -------- core update / draw --------------------------------- */
    void update(float dt, const Camera2D&) override
    {

        bool boosting = IsMouseButtonDown(MOUSE_BUTTON_LEFT);


        Vector2 d = { target.x-position.x, target.y-position.y };
        float   L = sqrtf(d.x*d.x + d.y*d.y);
        if (L > 1e-2f)
        {
            d.x/=L; d.y/=L;
            float mv = std::min(L, (float)speed*(boosting ? 2.f : 1.f) * dt);
            position.x += d.x*mv;
            position.y += d.y*mv;
            rotation = atan2f(d.y,d.x) * RAD2DEG + 90.f;
        }

        parts[0].active = !boosting; 
        parts[1].active = boosting;

        for (auto& p: parts) p.update(dt);
        recalcCollision();
    }

    void draw(const Camera2D&) const override
    {
        std::vector<const SpritePart*> sorted(parts.size());
        std::transform(parts.begin(),parts.end(),sorted.begin(),
                       [](const SpritePart& p){ return &p; });
        std::sort(sorted.begin(),sorted.end(),
                  [](auto*a,auto*b){ return a->z < b->z; });

        Vector2 pivotWorld = { position.x + offset.x, position.y + offset.y };
        for (auto* p:sorted) if (p->z<0) p->draw(pivotWorld,rotation);

        Rectangle src{0,0,size.x,size.y};
        Rectangle dst{position.x,position.y,size.x,size.y};
        DrawTexturePro(texture,src,dst,offset,rotation,WHITE);

        for (auto* p:sorted) if (p->z>=0) p->draw(pivotWorld,rotation);
    }

    ~BasicShip() override
    {
        if (!ownsTexture) texture.id = 0;   // avoid double‑unload
    }

private:
    /* state */
    std::vector<SpritePart> parts;
    Vector2 target = position;
    bool    ownsTexture = false;
    Texture2D* extTexture = nullptr;
};
