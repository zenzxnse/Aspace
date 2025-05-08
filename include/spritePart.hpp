#pragma once
#include "animator.hpp"
#include <cmath>


struct SpritePart
{
    Texture2D* tex  {};
    Animation  anim;
    Vector2    local {};     // attachment point in *ship* space
    int        z     = 0;    // draw order (‑ve = behind hull)
    float      relRot= 0.f;  // extra spin for the part itself (optional)
    bool       active = true; // if false, don't draw this part

    void update(float dt) { if(active) anim.Update(dt); }

    void draw(Vector2 worldPos, float shipRotDeg) const
    {

        if (!active) return;

        /* rotate local offset from ship space → world space */
        const float rad = shipRotDeg * DEG2RAD;
        Vector2 off {
            local.x * std::cos(rad) - local.y * std::sin(rad),
            local.x * std::sin(rad) + local.y * std::cos(rad)
        };

        /* compute pivot = centre of current frame */
        const Frame& f = anim.Current();
        Vector2 pivot { f.src.width * 0.5f, f.src.height * 0.5f };

        /* draw: SAME rotation, SAME pivot ---------------------------------- */
        anim.Draw(*tex,
                worldPos, off,                // entityPos + rotated local
                shipRotDeg + relRot,          // total rotation
                1.0f,
                pivot);                       // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }
};
