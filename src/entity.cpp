#include "entity.hpp"


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

void Entity::recalcOverallAABB()
{
    // update the SAT shape with the current position, rotation, and scale:
    // (scale is optional, but we use it for the sake of completeness)
    shape.updateWorldVertices(position, rotation, scale);

    // If we have no polygons, fall back to the visual rectangle:
    if (shape.polygons.empty())
    {
        float w2 = size.x * 0.5f * scale;
        float h2 = size.y * 0.5f * scale;
        overallAABB = { position.x - w2, position.y - h2, w2*2, h2*2 };
        return;
    }

    // Otherwise scan every vertex in every convex polygon:
    bool first = true;
    float minX=0, minY=0, maxX=0, maxY=0;
    for (auto& poly : shape.polygons)
    {
        for (auto& v : poly.worldVertices)
        {
            if (first)
            {
                minX = maxX = v.x;
                minY = maxY = v.y;
                first = false;
            }
            else
            {
                minX = std::min(minX, v.x);
                minY = std::min(minY, v.y);
                maxX = std::max(maxX, v.x);
                maxY = std::max(maxY, v.y);
            }
        }
    }
    overallAABB = { minX, minY, maxX - minX, maxY - minY };
}

