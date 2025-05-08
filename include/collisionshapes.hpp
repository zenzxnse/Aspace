/***************************************************************** 
 * Collision Shapes and SAT-based Collision Detection System
 * 
 * This file is part of a collision detection system built for Raylib.
 * It provides structures and functions for defining collision shapes,
 * transforming them, and performing collision detection using the 
 * Separating Axis Theorem (SAT).
 * 
 * Author: Zenzxnse
 * Date: 2025-04-26
 * 
 * License: MIT
 * 
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************/



#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <functional> 

// Forward declaration
class Entity;

// Represents a single convex polygon
struct ConvexPolygon {
    // Vertices are stored local to the entity's chosen pivot point.
    // The parser is responsible for ensuring this (e.g., by adjusting
    // for PhysicsEditor's AnchorPoint).
    std::vector<Vector2> localVertices;
    std::vector<Vector2> worldVertices; // Transformed vertices in world space
    Vector2 localCenter;                // Geometric center of localVertices
    Vector2 worldCenter;                // Transformed geometric center

    ConvexPolygon() = default;
    explicit ConvexPolygon(const std::vector<Vector2>& vertices);

    // Transforms localVertices to worldVertices based on entity's state.
    // entityPosition is the world position of the pivot.
    // entityRotationDegrees is the rotation around the pivot.
    // entityScale is the scale applied around the pivot.
    void transform(Vector2 entityPosition, float entityRotationDegrees, float entityScale);
    Rectangle getAABB() const;
    void drawLines(Color color) const; // Draw world vertices as lines
};

// Represents the complete collision shape for an entity
class CollisionShape {
public:
    std::string name; // Optional: for debugging or identification
    std::vector<ConvexPolygon> polygons;
    // Note: The 'anchorPoint' or 'pivot' is implicitly handled by ensuring
    // ConvexPolygon::localVertices are pre-adjusted to be relative to the entity's desired pivot.
    // The entity's `position` member then becomes the world location of this pivot.

    CollisionShape() = default;

    // Adds a pre-adjusted convex polygon (vertices are local to the entity's pivot)
    void addPolygon(const std::vector<Vector2>& adjustedLocalVertices);

    // Updates the world-space representation of all constituent polygons
    // Call this only when the entity's transform (position, rotation, scale) changes.
    void updateWorldVertices(Vector2 entityPosition, float entityRotationDegrees, float entityScale = 1.0f);

    void drawLines(Color color) const;
};

// --- [DEPRECATED] It's not recommended to use this parser as it only works till txt file format for PhysicsEditor or other formats ---
namespace ShapeParser {
    // [DEPRECATED] Not recommended for use in new code.
    // Parses a string of vertices like " (x1,y1) , (x2,y2) , ..."
    // Assumes vertices are for a single polygon.
    std::vector<Vector2> ParseVerticesFromString(const std::string& verticesStr);

    // [DEPRECATED] Not recommended for use in new code.
    // Example: Loads shapes from a PhysicsEditor plain text export.
    // This is a conceptual function. A robust implementation would handle the full file format.
    // It should parse the necessary body, extract its anchor point, and adjust all
    // polygon vertices to be relative to that anchor point before creating ConvexPolygon objects.
    // Returns a CollisionShape with localVertices already adjusted to be relative to the anchor.
    CollisionShape LoadFromPhysicsEditor(const std::string& fileContent,
                                         const std::string& bodyName,
                                         const Vector2& imageSize); // Image size needed if anchor is relative
} // namespace ShapeParser

// --- Collision Detection System (using SAT) ---
namespace CollisionSystem {
   
    /**
     * @brief Projects the vertices of a polygon onto a given axis.
     * 
     * This function is a helper for the Separating Axis Theorem (SAT). It calculates
     * the minimum and maximum extents of the polygon's vertices when projected onto
     * the specified axis.
     * 
     * @param axis The axis onto which the vertices are projected.
     * @param worldVertices The vertices of the polygon in world coordinates.
     * @param min Output parameter to store the minimum projection value.
     * @param max Output parameter to store the maximum projection value.
     */
    void ProjectPolygon(const Vector2& axis, const std::vector<Vector2>& worldVertices, float& min, float& max);

    /**
     * @brief Calculates the overlap between two 1D projections.
     * 
     * This function is a helper for the Separating Axis Theorem (SAT). It determines
     * the amount of overlap between two intervals (projections) along a single axis.
     * 
     * @param minA The minimum value of the first projection.
     * @param maxA The maximum value of the first projection.
     * @param minB The minimum value of the second projection.
     * @param maxB The maximum value of the second projection.
     * @return The overlap amount. A positive value indicates overlap, while a negative
     *         value indicates no overlap.
     */
    float GetOverlap(float minA, float maxA, float minB, float maxB);

    /**
     * @brief Extracts unique axes (normals) from the vertices of a convex polygon.
     * 
     * This function computes the unique axes (normals) from the edges of a convex polygon.
     * These axes are used in the Separating Axis Theorem (SAT) to test for potential
     * separating axes between two polygons.
     * 
     * @param worldVertices The vertices of the convex polygon in world coordinates.
     * @return A vector containing the unique axes (normals) of the polygon.
     */
    std::vector<Vector2> GetUniqueAxes(const std::vector<Vector2>& worldVertices);

    /**
     * @brief Performs a SAT-based collision check between two convex polygons.
     * 
     * This function checks for collision between two convex polygons using the
     * Separating Axis Theorem (SAT). If a collision is detected, it calculates
     * the Minimum Translation Vector (MTV) required to resolve the collision.
     * 
     * @param polyA The first convex polygon.
     * @param polyB The second convex polygon.
     * @param mtv Output parameter to store the Minimum Translation Vector (MTV) if a collision occurs.
     * @return True if a collision is detected, false otherwise.
     */
    bool CheckSATCollision(const ConvexPolygon& polyA, const ConvexPolygon& polyB, Vector2& mtv);

    /**
     * @brief Checks for collision between two collision shapes.
     * 
     * This function iterates through the constituent convex polygons of two collision
     * shapes and performs SAT-based collision checks. If a collision is detected, it
     * calculates the Minimum Translation Vector (MTV) required to resolve the collision.
     * 
     * @param shapeA The first collision shape.
     * @param shapeB The second collision shape.
     * @param mtv Output parameter to store the Minimum Translation Vector (MTV) if a collision occurs.
     * @return True if a collision is detected, false otherwise.
     */

    /**
     * @brief  */ 
    /**
     * @brief Checks if two collision shapes collide and calculates the minimum translation vector (MTV) if they do.
     * 
     * @param shapeA The first collision shape to check.
     * @param shapeB The second collision shape to check.
     * @param mtv A reference to a Vector2 that will store the minimum translation vector (MTV) if a collision is detected.
     *            The MTV represents the smallest vector needed to separate the two shapes.
     * @return true if the shapes collide, false otherwise.
     */
    bool CheckShapesCollide(const CollisionShape& shapeA, const CollisionShape& shapeB, Vector2& mtv);

} // namespace CollisionSystem