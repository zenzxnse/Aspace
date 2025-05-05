/*────────────────────── playerController.hpp ───────────────────────*/
#pragma once
#include "entity.hpp"
#include <memory>
#include "playercontroller.hpp"

/*
 *  A generic wrapper that owns ANY ship type.
 *  • TShip must derive from Entity (so World can query collision, etc.)
 *  • Input / boost logic is handled here, ship focuses on movement/render
 */

 class CameraTarget : public Entity
 {
     // empty tag‑type
 };

template<typename TShip>
class PlayerController : public CameraTarget
{
public:
    template<typename... Args>
    explicit PlayerController(Args&&... shipCtor)
        : ship(std::make_unique<TShip>(std::forward<Args>(shipCtor)...))
    {}

    TShip&       getShip()       { return *ship; }
    const TShip& getShip() const { return *ship; }

    // ───── forward the setters you actually care about ──────────────
    void setSpeed(double s) override             { ship->setSpeed(s); }
    void setHealth(double h) override            { ship->setHealth(h); }
    void takeDamage(double d) override           { ship->takeDamage(d); }
    void heal(double h) override                 { ship->heal(h); }
    // …and any other virtual setters you want to expose…

    /* forward helpers ------------------------------------------------ */
    void setTarget(Vector2 p)               { ship->setTarget(p); }
    template<typename... Args>
    void addPart(Args&&... a)               { ship->addPart(std::forward<Args>(a)...); }

    /* per-frame ------------------------------------------------------- */
    void update(float dt, const Camera2D& cam) override
    {
        // e.g. drag mouse → ship
        Vector2 wm = GetScreenToWorld2D(GetMousePosition(), cam);
        ship->setTarget(wm);

        ship->update(dt, cam);

        // sync our “camera target” position
        position = ship->getPosition();
        recalcCollision();
    }

    void draw(const Camera2D& cam) const override
    {
        ship->draw(cam);
    }

private:
    std::unique_ptr<TShip> ship;
};
