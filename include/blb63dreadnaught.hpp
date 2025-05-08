#pragma once
#include <entity.hpp>
#include <spritePart.hpp>
#include <raylib.h>
#include <random>
#include <vector>
#include <cmath>
#include <raymath.h>

class BLB63DreadNaught : public Entity
{
public:
    // ------------------------------------------------------------ constants
    static constexpr float WORLD_W   = 8192.0f * 2.f;
    static constexpr float WORLD_H   = 4096.0f * 2.f;
    static constexpr float WANDER_RADIUS = 2000.0f;       // how far next goal may be
    static constexpr float GOAL_EPS   = 12.f;           // distance considered “arrived”
    static constexpr float NEW_GOAL_INTERVAL = 3.0f;    // secs – safety timer

    // ------------------------------------------------------------ CTORS
    // uses an ALREADY‑LOADED texture (keep ownership)
    BLB63DreadNaught(Texture2D& hull, Vector2 start)
        : _rng(std::random_device{}()), _timeToNewGoal(0.f)
    {
        texture   = hull;           /* we do NOT own it               */
        size      = { (float)texture.width, (float)texture.height };
        position  = start;
        offset    = { size.x/2, size.y/2 };
        speed     = 50.0;
        _pickNewDest();
        recalcCollision();

        shape.addPolygon({
            {-76.0f,  -345.0f},
            {-84.0f,  -135.0f},
            {-34.0f,  -107.0f},
            {  2.0f,  -208.0f},
            { 34.0f,  -111.0f},
            {104.0f,  -152.0f},
            { 75.0f,  -344.0f},
            {168.0f,  -166.0f},
            {182.0f,   -30.0f},
            {153.0f,    64.0f},
            {211.0f,   117.0f},
            {153.0f,   105.0f},
            {224.0f,   174.0f},
            {185.0f,   162.0f},
            {193.0f,   223.0f},
            {162.0f,   205.0f},
            {148.0f,   284.0f},
            { 19.0f,   268.0f},
            { 22.0f,   189.0f},
            {  2.0f,   194.0f},
            {-17.0f,   193.0f},
            {-24.0f,   263.0f},
            {-141.0f,  287.0f},
            {-155.0f,  191.0f},
            {-187.0f,  224.0f},
            {-183.0f,  168.0f},
            {-223.0f,  174.0f},
            {-153.0f,  107.0f},
            {-212.0f,  117.0f},
            {-149.0f,   68.0f},
            {-185.0f, -105.0f},
            {-76.0f,  -346.0f}
        });

    }

    explicit BLB63DreadNaught(const std::string& path, Vector2 start = {0,0})
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

        shape.addPolygon({
            {-76.0f,  -345.0f},
            {-84.0f,  -135.0f},
            {-34.0f,  -107.0f},
            {  2.0f,  -208.0f},
            { 34.0f,  -111.0f},
            {104.0f,  -152.0f},
            { 75.0f,  -344.0f},
            {168.0f,  -166.0f},
            {182.0f,   -30.0f},
            {153.0f,    64.0f},
            {211.0f,   117.0f},
            {153.0f,   105.0f},
            {224.0f,   174.0f},
            {185.0f,   162.0f},
            {193.0f,   223.0f},
            {162.0f,   205.0f},
            {148.0f,   284.0f},
            { 19.0f,   268.0f},
            { 22.0f,   189.0f},
            {  2.0f,   194.0f},
            {-17.0f,   193.0f},
            {-24.0f,   263.0f},
            {-141.0f,  287.0f},
            {-155.0f,  191.0f},
            {-187.0f,  224.0f},
            {-183.0f,  168.0f},
            {-223.0f,  174.0f},
            {-153.0f,  107.0f},
            {-212.0f,  117.0f},
            {-149.0f,   68.0f},
            {-185.0f, -105.0f},
            {-76.0f,  -346.0f}
        });

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
            // Calculate target rotation angle
            float targetRotation = std::atan2(d.y,d.x)*RAD2DEG + 90.f;
            
            // Normalize angles to 0-360 range
            while (rotation < 0) rotation += 360.f;
            while (rotation >= 360.f) rotation -= 360.f;
            while (targetRotation < 0) targetRotation += 360.f;
            while (targetRotation >= 360.f) targetRotation -= 360.f;
            
            // Find shortest rotation path
            float angleDiff = targetRotation - rotation;
            if (angleDiff > 180.f) angleDiff -= 360.f;
            if (angleDiff < -180.f) angleDiff += 360.f;
            
            // Rotate smoothly
            float rotationSpeed = 120.0f; // degrees per second
            float maxRotation = rotationSpeed * dt;
            float actualRotation = std::clamp(angleDiff, -maxRotation, maxRotation);
            rotation += actualRotation;
            
            // Move in the direction we're facing
            float moveAngle = (rotation - 90.f) * DEG2RAD;
            float step = speed * dt;
            position.x += std::cos(moveAngle) * step;
            position.y += std::sin(moveAngle) * step;
        }

        for (auto& p:parts) p.update(dt);
        shape.updateWorldVertices(position,rotation);
        recalcOverallAABB();
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
        DrawRectangleLinesEx(getOverallAABB(), 2.0f, GREEN); // Draw the AABB
        shape.drawLines(RED);
        DrawTexturePro(texture,src,dst,offset,rotation,WHITE);

        for(auto* p:sorted) if(p->z>=0) p->draw(pivotWorld,rotation);
    }

    ~BLB63DreadNaught() override
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
