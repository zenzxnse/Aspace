#pragma once
#include <raylib.h>
#include <string>
#include "collisionshapes.hpp"

/*
 Abstract base class (interface) for every entity --------------------------
 [All entities are derived from this class.]
*/
class Entity {
public:
    // ---------- Life-cycle --------------------------------------------------
    virtual ~Entity()            { if (texture.id) UnloadTexture(texture); }
    Entity(const Entity&)        = delete;  
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&)             = default;  // movable
    Entity& operator=(Entity&&)  = default;

    // ---------- Core behaviour (default implementations) -------------------
    virtual void update([[maybe_unused]] float dt, const Camera2D&) {}
    virtual void draw(const Camera2D&) const {}

    // ---------- Gameplay API ------------------------------------------------
    virtual void takeDamage([[maybe_unused]] double amount) {}
    virtual void heal([[maybe_unused]] double amount) {}
    virtual void attack([[maybe_unused]] Entity& target) {}
    virtual void onCollision([[maybe_unused]] Entity& other) {}
    virtual bool isAliveAndCollidable() { return isAlive && isCollidable; } // changed return type to bool
    virtual void recalcOverallAABB(); // update AABB if needed
    inline Rectangle getOverallAABB() const { return overallAABB; } // AABB for this entity

    // ---------- Progression -------------------------------------------------
    virtual void levelUp() {}
    virtual void gainExperience([[maybe_unused]] int amount) {}

    // ---------- State setters ----------------------------------------------
    virtual void setTexture(const std::string& path); // *implemented below*
    virtual void setPosition(Vector2 pos)         { position = pos;  recalcOverallAABB(); }
    virtual void setSize(Vector2 s)               { size = s;        recalcOverallAABB(); }
    virtual void setHealth(double h)              { health = h; }
    virtual void setSpeed(double s)               { speed = s; }
    virtual void setRotation(float r)         { rotation = r; }
    virtual void setScale(float s)               { scale = s; recalcOverallAABB(); }
    // ... (other trivial setters can stay inline)

    // ---------- Query helpers (non-virtual) --------------------------------
    const Vector2&   getPosition()    const { return position; }
    const Rectangle& getCollision()   const { return collisionBox; }
    Vector2&   getMutablePosition() { return position; } // mutable access
    bool             alive()          const { return isAlive; }

    Entity() = default;

    // Keep heavy data protected so derived classes can poke it ---------------
    Vector2   position{0,0};
    Vector2   size{64,64};
    Texture2D texture{};           // RAII handled by ~Entity()
    Vector2   offset{0,0};

    double health        = 100.0;
    double speed         = 100.0;  // units/sec
    double damage        = 10.0;
    double attackSpeed   = 1.0;    // attacks/sec
    double attackRange   = 50.0;
    double attackCD      = 0.0;    // seconds until next attack
    float rotation       = 0.0f;

    bool   isAlive       = true;
    bool   isColliding   = false; 
    bool   isCollidable   = true;  
    int    level         = 1;
    int    xp            = 0;
    float scale          = 1.0f; // scale factor for the entity
    Color  tint          = WHITE;

    Vector2   velocity{0,0};
    Rectangle  overallAABB{}; // AABB for this entity
    Rectangle collisionBox{};
    double    defense         = 0.0;
    double    mana            = 0.0, manaRegen = 0.0;
    double    stamina         = 0.0, stamRegen = 0.0;
    bool      invincible      = false;
    double    invincTimer     = 0.0;
    CollisionShape shape;

    void recalcCollision() {      // keep AABB in sync
        collisionBox = { position.x - size.x*0.5f,
                         position.y - size.y*0.5f,
                         size.x, size.y };
    }

    // --- common helpers available to children ------------------------------
    static Vector2 lerp(Vector2 a, Vector2 b, float t) {
        return Vector2{ a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t };
    }
};
