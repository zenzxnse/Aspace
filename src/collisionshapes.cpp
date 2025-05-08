#include "collisionshapes.hpp"
#include <raymath.h>
#include <algorithm>
#include <limits>
#include <sstream>
#include <cmath> //

// --- ConvexPolygon Implementation ---

ConvexPolygon::ConvexPolygon(const std::vector<Vector2>& vertices) : localVertices(vertices) {
    worldVertices.resize(localVertices.size());
    if (!localVertices.empty()) {
        localCenter = {0,0};
        for(const auto& v : localVertices) {
            localCenter = Vector2Add(localCenter, v);
        }
        localCenter = Vector2Scale(localCenter, 1.0f / localVertices.size());
    } else {
        localCenter = {0,0};
    }
    worldCenter = localCenter; // Initially same, will be transformed
}

void ConvexPolygon::transform(Vector2 entityPosition, float entityRotationDegrees, float entityScale) {
    if (localVertices.empty()) return;

    float rotationRadians = entityRotationDegrees * DEG2RAD;
    float cosTheta = cosf(rotationRadians);
    float sinTheta = sinf(rotationRadians);

    for (size_t i = 0; i < localVertices.size(); ++i) {
        Vector2 v = localVertices[i];

        // Scale (around local origin {0,0} which is the pivot)
        v.x *= entityScale;
        v.y *= entityScale;

        // Rotate (around local origin {0,0} which is the pivot)
        float rotatedX = v.x * cosTheta - v.y * sinTheta;
        float rotatedY = v.x * sinTheta + v.y * cosTheta;
        v = {rotatedX, rotatedY};

        // Translate to world position
        worldVertices[i] = Vector2Add(entityPosition, v);
    }

    // Transform the local center to world space
    Vector2 scaledCenter = Vector2Scale(localCenter, entityScale);
    float rotatedCenterX = scaledCenter.x * cosTheta - scaledCenter.y * sinTheta;
    float rotatedCenterY = scaledCenter.x * sinTheta + scaledCenter.y * cosTheta;
    worldCenter = Vector2Add(entityPosition, {rotatedCenterX, rotatedCenterY});
}

void ConvexPolygon::drawLines(Color color) const {
    if (worldVertices.size() >= 2) {
        for (size_t i = 0; i < worldVertices.size(); ++i) {
            DrawLineV(worldVertices[i], worldVertices[(i + 1) % worldVertices.size()], color);
        }
    }
    // Optional but it is possible to draw vertex points
    // for(const auto& v : worldVertices) {
    //     DrawCircleV(v, 2, Fade(color, 0.7f));
    // }
    // DrawWorldCenter for debugging
    // DrawCircleV(worldCenter, 3, BLUE);
}

Rectangle ConvexPolygon::getAABB() const {
    if (worldVertices.empty()) return {0,0,0,0};
    float minX = worldVertices[0].x;
    float maxX = minX;
    float minY = worldVertices[0].y;
    float maxY = minY;
    for (size_t i = 1; i < worldVertices.size(); ++i) {
        const auto &v = worldVertices[i];
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y);
        maxY = std::max(maxY, v.y);
    }
    return { minX, minY, maxX - minX, maxY - minY };
}

// --- CollisionShape Implementation ---

void CollisionShape::addPolygon(const std::vector<Vector2>& adjustedLocalVertices) {
    polygons.emplace_back(adjustedLocalVertices);
}

void CollisionShape::updateWorldVertices(Vector2 entityPosition, float entityRotationDegrees, float entityScale) {
    for (auto& poly : polygons) {
        poly.transform(entityPosition, entityRotationDegrees, entityScale);
    }
}

void CollisionShape::drawLines(Color color) const {
    for (const auto& poly : polygons) {
        poly.drawLines(color);
    }
}

// --- ShapeParser Implementation ---
namespace ShapeParser {

// Basic helper to parse "(x, y)" into Vector2. Robust error handling needed.
Vector2 ParseSingleVertex(const std::string& vertexToken) {
    Vector2 vertex = {0, 0};
    size_t openParen = vertexToken.find('(');
    size_t comma = vertexToken.find(',');
    size_t closeParen = vertexToken.find(')');

    if (openParen != std::string::npos && comma != std::string::npos && closeParen != std::string::npos) {
        try {
            std::string xStr = vertexToken.substr(openParen + 1, comma - (openParen + 1));
            std::string yStr = vertexToken.substr(comma + 1, closeParen - (comma + 1));
            vertex.x = std::stof(xStr);
            vertex.y = std::stof(yStr);
        } catch (const std::exception& e) {
            TraceLog(LOG_WARNING, "PARSER: Failed to parse vertex token: '%s'. Error: %s", vertexToken.c_str(), e.what());
        }
    } else {
         TraceLog(LOG_WARNING, "PARSER: Malformed vertex token: '%s'", vertexToken.c_str());
    }
    return vertex;
}

std::vector<Vector2> ParseVerticesFromString(const std::string& verticesLine) {
    std::vector<Vector2> vertices;
    std::stringstream lineStream(verticesLine);
    std::string token;

    // Tokenize by ' , ' (comma surrounded by optional spaces)
    // More robustly, find first '(', then find matching ')', parse content.
    size_t currentPos = 0;
    while(currentPos < verticesLine.length()){
        size_t openParenPos = verticesLine.find('(', currentPos);
        if(openParenPos == std::string::npos) break;

        size_t closeParenPos = verticesLine.find(')', openParenPos);
        if(closeParenPos == std::string::npos) break; // Malformed

        std::string vertexToken = verticesLine.substr(openParenPos, closeParenPos - openParenPos + 1);
        vertices.push_back(ParseSingleVertex(vertexToken));
        currentPos = closeParenPos + 1;
    }
    return vertices;
}


CollisionShape LoadFromPhysicsEditor(const std::string& fileContent, const std::string& bodyName, const Vector2& imageSize) {
    CollisionShape shape;
    shape.name = bodyName;
    std::stringstream ss(fileContent);
    std::string line;

    // --- Conceptual Parsing ---
    // 1. Find the section for `bodyName`.
    // 2. Extract `AnchorPointAbs: { ax, ay }` or `AnchorPointRel: { rax, ray }`.
    //    If Rel, AnchorPointAbs = {rax * imageSize.x, ray * imageSize.y}.
    //    PhysicsEditor seems to provide AnchorPointAbs directly.
    //    Example: AnchorPointAbs: { 246.344,247.968 } for "pngwing"
    Vector2 anchorPointAbs = {0,0}; // Default, should be parsed

    bool inTargetBodySection = false;
    bool lookingForPolygons = false;

    // Extremely simplified example:
    if (bodyName == "pngwing") { // Hardcoded for example
        anchorPointAbs = {246.344f, 247.968f}; // From your example data
    }
    // This anchorPointAbs is the pivot in image coordinates (top-left origin).

    while (std::getline(ss, line)) {
        // Trim whitespace from line
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.find("Name:        " + bodyName) != std::string::npos) {
            inTargetBodySection = true;
        }

        if (!inTargetBodySection) continue;

        // Find anchor point (if not already hardcoded/passed)
        // Example: if(line.find("AnchorPointAbs:") != std::string::npos) { anchorPointAbs = ParseAnchor(line); }

        if (line.find("Hull polygon:") != std::string::npos || line.find("Convex sub polygons:") != std::string::npos) {
            lookingForPolygons = true;
            continue; // Next line has the vertices
        }

        if (lookingForPolygons && !line.empty() && line.front() == '(') {
            std::vector<Vector2> rawVertices = ParseVerticesFromString(line);
            if (!rawVertices.empty()) {
                std::vector<Vector2> adjustedVertices;
                adjustedVertices.reserve(rawVertices.size());
                for (const auto& rv : rawVertices) {
                    // Adjust vertices to be relative to the anchor point
                    adjustedVertices.push_back(Vector2Subtract(rv, anchorPointAbs));
                }
                shape.addPolygon(adjustedVertices);
            }
            // If only taking first polygon list, might set lookingForPolygons = false;
            // Or continue if multiple "Convex sub polygons" lines follow.
            // A real parser would handle multiple sub-polygon lines under "Convex sub polygons:".
            // For "Hull polygon", it's usually one complex polygon that might need decomposition
            // if it's concave and you're using it directly with SAT. PhysicsEditor provides convex sub-polygons.
        }

        if (inTargetBodySection && line.empty() && lookingForPolygons) { // End of polygon section for this body
            // break; // Or reset flags if parsing multiple bodies
        }
    }
    if (shape.polygons.empty()) {
        TraceLog(LOG_WARNING, "PARSER: No polygons loaded for body '%s'. Check parser logic and data.", bodyName.c_str());
    }
    return shape;
}


} // namespace ShapeParser

// --- CollisionSystem Implementation ---
namespace CollisionSystem {

void ProjectPolygon(const Vector2& axis, const std::vector<Vector2>& worldVertices, float& min, float& max) {
    if (worldVertices.empty()) {
        min = 0; max = 0;
        return;
    }
    min = Vector2DotProduct(worldVertices[0], axis);
    max = min;
    for (size_t i = 1; i < worldVertices.size(); ++i) {
        float p = Vector2DotProduct(worldVertices[i], axis);
        if (p < min) min = p;
        else if (p > max) max = p;
    }
}

float GetOverlap(float minA, float maxA, float minB, float maxB) {
    return std::max(0.0f, std::min(maxA, maxB) - std::max(minA, minB));
}

std::vector<Vector2> GetUniqueAxes(const std::vector<Vector2>& worldVertices) {
    std::vector<Vector2> axes;
    if (worldVertices.size() < 2) return axes;

    for (size_t i = 0; i < worldVertices.size(); ++i) {
        Vector2 p1 = worldVertices[i];
        Vector2 p2 = worldVertices[(i + 1) % worldVertices.size()];
        Vector2 edge = Vector2Subtract(p2, p1);
        Vector2 normal = Vector2Normalize({-edge.y, edge.x}); // Perpendicular and normalized

        // Deduplication check
        bool foundParallel = false;
        for (const auto& existingAxis : axes) {
            if (fabsf(Vector2DotProduct(normal, existingAxis)) > 0.999f) { // Almost parallel (or anti-parallel)
                foundParallel = true;
                break;
            }
        }
        if (!foundParallel) {
            axes.push_back(normal);
        }
    }
    return axes;
}

bool CheckSATCollision(const ConvexPolygon& polyA, const ConvexPolygon& polyB, Vector2& mtv) {
    if (polyA.worldVertices.empty() || polyB.worldVertices.empty()) return false;

    float overlap = std::numeric_limits<float>::infinity();
    Vector2 smallestAxis = {0, 0};

    std::vector<Vector2> axesA = GetUniqueAxes(polyA.worldVertices);
    std::vector<Vector2> axesB = GetUniqueAxes(polyB.worldVertices);

    auto testAxes = [&](const std::vector<Vector2>& currentAxes) {
        for (const auto& axis : currentAxes) {
            float minA, maxA, minB, maxB;
            ProjectPolygon(axis, polyA.worldVertices, minA, maxA);
            ProjectPolygon(axis, polyB.worldVertices, minB, maxB);

            if (maxA < minB - 1e-3f || maxB < minA - 1e-3f) { // Add tolerance for floating point
                return false; // Found a separating axis
            } else {
                float o = GetOverlap(minA, maxA, minB, maxB);
                if (o < overlap) {
                    overlap = o;
                    smallestAxis = axis;
                }
            }
        }
        return true; // No separating axis found in this set
    };

    if (!testAxes(axesA)) return false;
    if (!testAxes(axesB)) return false;

    // If we got here, there's a collision.
    // Be sure MTV direction pushes polyA away from polyB.
    // The direction vector from A to B.
    Vector2 direction = Vector2Subtract(polyB.worldCenter, polyA.worldCenter);
    if (Vector2DotProduct(direction, smallestAxis) < 0.0f) {
        smallestAxis = Vector2Negate(smallestAxis); // Reverse direction
    }
    mtv = Vector2Scale(smallestAxis, overlap);
    return true;
}

bool CheckShapesCollide(const CollisionShape& shapeA, const CollisionShape& shapeB, Vector2& mtv) {
    for (const auto& polyA : shapeA.polygons) {
        for (const auto& polyB : shapeB.polygons) {
            if (CheckSATCollision(polyA, polyB, mtv)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace CollisionSystem