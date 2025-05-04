#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <functional>
#include <limits>
#include <algorithm>
#include <cmath>

/* ───────────────────────────── Frame ───────────────────────────── */

struct Frame
{
    Rectangle src            = {0, 0, 0, 0};   // region in the spritesheet
    float     duration       = 0.0f;           // seconds
    Vector2   offset         = {0, 0};         // local visual adjustment

    Vector2   GetCenter() const;              // center of the frame in local space
};

Vector2 Frame::GetCenter() const
{
    return { src.x + src.width / 2.0f, src.y + src.height / 2.0f };
}

/* ───────────────────────── Animation ───────────────────────────── */

class Animation
{
public:
    enum class LoopMode { Loop, Once, PingPong };

    explicit Animation(std::string name,
                       LoopMode mode          = LoopMode::Loop,
                       float playbackSpeed    = 1.0f)              // 1.0 = normal speed
        : m_name(std::move(name)),
          m_mode(mode),
          m_playbackSpeed(playbackSpeed)
    {}

    /* -- authoring ---------------------------------------------------- */
    Animation& AddFrame(Rectangle src, float seconds, Vector2 offset = {0, 0})
    {
        m_frames.push_back({src, seconds, offset});
        return *this;
    }

    std::string& Name() { return m_name; }

    /* -- runtime ------------------------------------------------------ */
    void Update(float dt)
    {
        if (m_frames.empty() || m_finished) return;

        if (m_frames.size() == 1) return;

        // honour playback speed (can be <0 for reverse)
        dt *= m_playbackSpeed;
        m_elapsed += dt;

        // allow tiny or even 0‑length frames safely
        const float frameDur = std::max(m_frames[m_idx].duration,
                                        std::numeric_limits<float>::epsilon());

        // advance while we still have enough accumulated time
        while (std::abs(m_elapsed) >= frameDur && !m_finished)
        {
            m_elapsed = std::fmod(m_elapsed, frameDur);

            // direction step (+1 or -1 in ping‑pong)
            int step = (m_reverse ? -1 : 1);
            m_idx += step;

            if (m_idx >= m_frames.size() || m_idx < 0)
                HandleLoopBoundary();
        }
    }

    /* Draw variant 1:
       - position is where *this part* lives in world/parent space
       - optional pivot lets you spin around whatever point you like   */
    void Draw(Texture2D tex,
              Vector2   position,
              float     rotation           = 0.0f,
              float     scale              = 1.0f,
              Vector2   pivot              = {0, 0},
              Color     tint               = WHITE) const
    {
        if (m_frames.empty()) return;
        const Frame& f = m_frames[m_idx];

        Rectangle dest = { position.x + f.offset.x,
                           position.y + f.offset.y,
                           f.src.width  * scale,
                           f.src.height * scale };

        DrawTexturePro(tex, f.src, dest, pivot, rotation, tint);
    }

    /* Draw variant 2:
       - *entityPos*   = absolute position of the owning Entity
       - *partOffset*  = local offset of this attachment on that Entity */
    void Draw(Texture2D tex,
              Vector2   entityPos,
              Vector2   partOffset,
              float     rotation           = 0.0f,
              float     scale              = 1.0f,
              Vector2   pivot              = {0, 0},
              Color     tint               = WHITE) const
    {
        Draw(tex, {entityPos.x + partOffset.x,
                   entityPos.y + partOffset.y},
             rotation, scale, pivot, tint);
    }

    /* ------------- misc getters/setters ----------------------------- */
    const std::string& Name()        const { return m_name; }
    const Frame&       Current()     const { return m_frames[m_idx]; }
    bool               Finished()    const { return m_finished; }

    void  Reset(bool forceToStart = true)
    {
        m_idx      = (m_playbackSpeed >= 0 || m_mode == LoopMode::PingPong) ? 0 : static_cast<int>(m_frames.size() - 1);
        m_elapsed  = 0.0f;
        m_reverse  = false;
        m_finished = false;
        if (!forceToStart && m_idx >= m_frames.size()) m_idx = 0;
    }
    void  SetLoopMode(LoopMode mode)        { m_mode = mode; }
    void  SetPlaybackSpeed(float speed)     { m_playbackSpeed = speed; }
    float PlaybackSpeed()            const  { return m_playbackSpeed; }

    void setFramesOffsetToCenter()
    {
        for (auto& f : m_frames)
        {
            f.offset.x = -f.src.width  / 2.0f;
            f.offset.y = -f.src.height / 2.0f;
        }
    }

    void setFramesOffsetToTopLeft()
    {
        for (auto& f : m_frames)
        {
            f.offset.x = f.src.x;
            f.offset.y = f.src.y;
        }
    }

private:
    /* -- helper: what to do when we run past an end ------------------ */
    void HandleLoopBoundary()
    {
        switch (m_mode)
        {
        case LoopMode::Loop:
            m_idx = (m_idx < 0) ? static_cast<int>(m_frames.size() - 1) : 0;
            break;

        case LoopMode::Once:
            m_idx      = (m_idx < 0) ? 0 : static_cast<int>(m_frames.size() - 1);
            m_finished = true;
            break;

        case LoopMode::PingPong:
            m_reverse  = !m_reverse;
            m_idx      = std::clamp(m_idx, 0, static_cast<int>(m_frames.size() - 1));
            break;
        }
    }

    /* -- data -------------------------------------------------------- */
    std::string        m_name;
    std::vector<Frame> m_frames;

    /* playback state */
    int    m_idx           = 0;
    float  m_elapsed       = 0.0f;
    bool   m_reverse       = false;
    bool   m_finished      = false;
    LoopMode m_mode        = LoopMode::Loop;
    float     m_playbackSpeed = 1.0f;  // negative = play backwards
};