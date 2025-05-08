#pragma once
// ────────────────────────────────────────────────────────────────
// ▸ utilities.hpp — common helpers for loading textures, slicing
//   sprite‑strips, and building animations in one line.
//   Everything sits inside namespace `util` to avoid collisions.
//   Remember: caller owns resulting Texture2D → call UnloadTexture.
// ────────────────────────────────────────────────────────────────

#include <raylib.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <animator.hpp>

namespace util {

// ----------------------------------------------------------------
// Load a texture, optionally  ⨉scale  with nearest‑neighbour, and
//   apply point filtering so pixel art stays crisp.
// ----------------------------------------------------------------
inline Texture2D LoadTextureNN(const std::string& path,
                              int   scale        = 1,
                              bool  pointFilter  = true)
{
    Image img = LoadImage(path.c_str());
    if (scale > 1)
        ImageResizeNN(&img, img.width * scale, img.height * scale);

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (pointFilter)
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    return tex;                          // caller: UnloadTexture(tex)
}


inline Texture2D LoadTextureNNBySize(const std::string& path,
                              int   targetWidth,
                              int   targetHeight,
                              bool  pointFilter  = true)
{
    Image img = LoadImage(path.c_str());
    ImageResizeNN(&img, targetWidth, targetHeight);

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (pointFilter)
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    return tex;                          
}

inline Texture2D LoadTextureNNRotate270(const std::string& path,
                              int   scale        = 1,
                              bool  pointFilter  = true)
{
    Image img = LoadImage(path.c_str());
    if (scale != 0)
        ImageResizeNN(&img, img.width * scale, img.height * scale);

    ImageRotateCCW(&img);                
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (pointFilter)
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    return tex;                          // caller: UnloadTexture(tex)
}

inline Texture2D LoadTextureNNRotate90(const std::string& path,
                              int   scale        = 1,
                              bool  pointFilter  = true)
{
    Image img = LoadImage(path.c_str());
    if (scale != 0)
        ImageResizeNN(&img, img.width * scale, img.height * scale);

    ImageRotateCW(&img);                
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (pointFilter)
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    return tex;                          // caller: UnloadTexture(tex)
}

// ----------------------------------------------------------------
// Slice a horizontal strip spritesheet into equal‑width Rectangles.
// ----------------------------------------------------------------
inline std::vector<Rectangle> SliceStrip(const Texture2D& tex, int frames)
{
    const float w = static_cast<float>(tex.width) / frames;
    const float h = static_cast<float>(tex.height);
    std::vector<Rectangle> rects(frames);
    for (int i = 0; i < frames; ++i)
        rects[i] = { w * i, 0, w, h };
    return rects;
}

// ----------------------------------------------------------------
// Create an Animation object from a simple strip in one call.
// Example:
//   auto flamesIdle = util::MakeStripAnimation("idle", flamesTex, 3, 0.1f);
// ----------------------------------------------------------------
inline Animation MakeStripAnimation(const std::string&  name,
                                    const Texture2D&   tex,
                                    int                frames,
                                    float              frameDuration,
                                    Animation::LoopMode mode          = Animation::LoopMode::Loop,
                                    float              playbackSpeed = 1.0f)
{
    Animation anim(name, mode, playbackSpeed);
    for (auto& rect : SliceStrip(tex, frames))
        anim.AddFrame(rect, frameDuration);
    anim.setFramesOffsetToCenter();      // if center origin could wrap this in a if else later
    return anim;                         // RVO → cheap copy
}

// ----------------------------------------------------------------
// Helpers for basic math / interpolation — handy for smoothing
// movement & camera pans.
// ----------------------------------------------------------------
inline float  lerpf(float a, float b, float t)                  { return a + (b - a) * t; }
inline Vector2 lerpVec2(Vector2 a, Vector2 b, float t)          { return { lerpf(a.x,b.x,t), lerpf(a.y,b.y,t) }; }

template<typename T>
inline T clamp(const T& v, const T& lo, const T& hi)            { return (v < lo) ? lo : (v > hi) ? hi : v; }

// Screen → world convenience (without introducing a World class yet)
inline Vector2 ScreenToWorld(Vector2 screen, const Camera2D& c) { return GetScreenToWorld2D(screen, c); }

} // namespace util
