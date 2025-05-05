#pragma once
#include <entity.hpp>
#include <spritePart.hpp>
#include <raylib.h>
#include <random>
#include <vector>
#include <cmath>
#include <raymath.h>

class BigShip : public Entity
{
public:
    // ------------------------------------------------------------ constants
    static constexpr float WORLD_W   = 10'000.f;
    static constexpr float WORLD_H   = 15'000.f;
    static constexpr float WANDER_RADIUS = 800.f;       // how far next goal may be
    static constexpr float GOAL_EPS   = 12.f;           // distance considered “arrived”
    static constexpr float NEW_GOAL_INTERVAL = 3.0f;    // secs – safety timer

    // ------------------------------------------------------------ CTORS
    // 1) uses an ALREADY‑LOADED texture (you keep ownership)
    BigShip(Texture2D& hull, Vector2 start)
        : _rng(std::random_device{}()), _timeToNewGoal(0.f)
    {
        texture   = hull;           /* we do NOT own it               */
        size      = { (float)texture.width, (float)texture.height };
        position  = start;
        offset    = { size.x/2, size.y/2 };
        speed     = 50.0;
        _pickNewDest();
        recalcCollision();
    }

    // 2) self‑loading ctor — this one **straightens** the sprite once
    explicit BigShip(const std::string& path, Vector2 start = {0,0})
        : _rng(std::random_device{}()), _timeToNewGoal(0.f), ownsTexture(true)
    {
        Image img = LoadImage(path.c_str());
        // sprite points ≈ +90° – rotate it upright
        ImageRotateCW(&img);        // 90° clockwise (one call = 90 deg)
        texture = LoadTextureFromImage(img);
        UnloadImage(img);

        size     = { (float)texture.width, (float)texture.height };
        position = start;
        offset   = { size.x/2, size.y/2 };
        speed    = 50.0;
        _pickNewDest();
        recalcCollision();
    }

    // ------------------------------------------------------------ parts API
    template<typename... Args>
    void addPart(Texture2D* tex, Vector2 local, int z, Args&&... animArgs)
    {
        parts.emplace_back(SpritePart{ tex, Animation{std::forward<Args>(animArgs)...},
                                       local, z });
    }
    void addPart(Texture2D* tex,const Animation& anim,Vector2 local,int z=0)
    { parts.emplace_back(SpritePart{tex,anim,local,z}); }

    // ------------------------------------------------------------ behaviour
    void update(float dt,const Camera2D&) override
    {
        /* ---- choose new goal occasionally ---------------------------- */
        _timeToNewGoal -= dt;
        if (_timeToNewGoal <= 0.f ||
            Vector2Distance(position,_goal) < GOAL_EPS) _pickNewDest();

        /* ---- steer towards goal -------------------------------------- */
        Vector2 d{ _goal.x-position.x, _goal.y-position.y };
        float   len = std::hypot(d.x,d.y);
        if (len > 1e-3f)
        {
            d.x/=len; d.y/=len;
            float step = speed * dt;
            position.x += d.x*step;
            position.y += d.y*step;
            rotation = std::atan2(d.y,d.x)*RAD2DEG + 90.f;   // nose forward
        }

        for (auto& p:parts) p.update(dt);
        recalcCollision();
    }

    void draw(const Camera2D&) const override
    {
        std::vector<const SpritePart*> sorted;
        sorted.reserve(parts.size());
        for (auto& p:parts) sorted.push_back(&p);
        std::sort(sorted.begin(),sorted.end(),
                  [](auto*a,auto*b){return a->z<b->z;});

        Vector2 pivotWorld{position.x+offset.x,position.y+offset.y};
        for(auto* p:sorted) if(p->z<0) p->draw(pivotWorld,rotation);

        Rectangle src{0,0,size.x,size.y};
        Rectangle dst{position.x,position.y,size.x,size.y};
        DrawTexturePro(texture,src,dst,offset,rotation,WHITE);

        for(auto* p:sorted) if(p->z>=0) p->draw(pivotWorld,rotation);
    }

    ~BigShip() override
    {
        if (ownsTexture) UnloadTexture(texture);
        else texture.id = 0;
    }

private:
    // --------------------------------------------------------- random goal
    void _pickNewDest()
    {
        std::uniform_real_distribution<float> dx(-WANDER_RADIUS, WANDER_RADIUS);
        std::uniform_real_distribution<float> dy(-WANDER_RADIUS, WANDER_RADIUS);

        Vector2 candidate{ position.x + dx(_rng), position.y + dy(_rng) };
        // clamp inside world bounds with a small margin
        candidate.x = std::clamp(candidate.x, 64.f, WORLD_W - 64.f);
        candidate.y = std::clamp(candidate.y, 64.f, WORLD_H - 64.f);

        _goal = candidate;
        _timeToNewGoal = NEW_GOAL_INTERVAL;   // safety timer restart
    }

    // ------------------------------------------------------ member data
    std::vector<SpritePart> parts;

    Vector2               _goal{};             // current destination
    float                 _timeToNewGoal;      // secs
    std::mt19937          _rng;

    bool      ownsTexture = false;
    Texture2D* extTexture = nullptr;           // (kept for symmetry)
};
