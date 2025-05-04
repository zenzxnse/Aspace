#pragma once
#include <raylib.h>
#include <string>

// Abstract base class (interface) for every entity --------------------------
class Entity {
public:
    // ---------- Life-cycle --------------------------------------------------
    virtual ~Entity()            { if (texture.id) UnloadTexture(texture); }
    Entity(const Entity&)        = delete;  
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&)             = default;  // movable
    Entity& operator=(Entity&&)  = default;

    // ---------- Core behaviour (default implementations) -------------------
    virtual void update(float dt, const Camera2D&) {}
    virtual void draw(const Camera2D&) const {}

    // ---------- Gameplay API ------------------------------------------------
    virtual void takeDamage(double amount) {}
    virtual void heal(double amount) {}
    virtual void attack(Entity& target) {}

    // ---------- Progression -------------------------------------------------
    virtual void levelUp() {}
    virtual void gainExperience(int amount) {}

    // ---------- State setters ----------------------------------------------
    virtual void setTexture(const std::string& path); // *implemented below*
    virtual void setPosition(Vector2 pos)         { position = pos;  recalcCollision(); }
    virtual void setSize(Vector2 s)               { size = s;        recalcCollision(); }
    virtual void setHealth(double h)              { health = h; }
    virtual void setSpeed(double s)               { speed = s; }
    virtual void setRotation(float r)         { rotation = r; }
    // ... (other trivial setters can stay inline)

    // ---------- Query helpers (non-virtual) --------------------------------
    const Vector2&   getPosition()    const { return position; }
    const Rectangle& getCollision()   const { return collisionBox; }
    Vector2&   getMutablePosition() { return position; } // mutable access
    bool             alive()          const { return isAlive; }

protected:
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
    int    level         = 1;
    int    xp            = 0;
    Color  tint          = WHITE;

    Vector2   velocity{0,0};
    Rectangle collisionBox{};
    double    defense         = 0.0;
    double    mana            = 0.0, manaRegen = 0.0;
    double    stamina         = 0.0, stamRegen = 0.0;
    bool      invincible      = false;
    double    invincTimer     = 0.0;

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

// Inline implementation kept in the header for brevity
inline void Entity::setTexture(const std::string& path)
{
    if (texture.id) UnloadTexture(texture);

    Image img = LoadImage(path.c_str());
    ImageResize(&img, (int)size.x, (int)size.y);
    texture   = LoadTextureFromImage(img);
    UnloadImage(img);

    offset = { size.x*0.5f, size.y*0.5f };
}
