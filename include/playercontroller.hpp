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
    {
        ship = std::make_unique<TShip>(std::forward<Args>(shipCtor)...);
    }

    TShip& getShip()            { return *ship; }
    const TShip& getShip() const{ return *ship; }

    /* forward helpers ------------------------------------------------ */
    void setTarget(Vector2 p)               { ship->setTarget(p); }
    template<typename... Args>
    void addPart  (Args&&...a)              { ship->addPart(std::forward<decltype(a)>(a)...); }

    /* per‑frame ------------------------------------------------------- */
    void update(float dt,const Camera2D& cam) override
    {
        /* target = mouse world space */
        Vector2 wm = GetScreenToWorld2D(GetMousePosition(), cam);
        ship->setTarget(wm);

        ship->update(dt, cam);

        /* keep controller's pos in sync so camera can still follow us */
        position = ship->getPosition();
        recalcCollision();
    }

    void draw(const Camera2D& cam) const override { ship->draw(cam); }

private:
    std::unique_ptr<TShip> ship;
};
