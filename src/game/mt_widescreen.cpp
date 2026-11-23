// Widescreen. Under the Aspect Ratio Expand (the Graphics tab's default) in a window wider than 4:3, the game draws
// its own wider 3D view (the game hack, mt_wide_aspect_bits() below) into its 4:3 frame, which RT64 draws at 4:3 and
// stretches to the window (lib-patches/rt64.patch, rt64_set_game_widescreen()): the 3D keeps its proportions at any
// window shape, the game culls for the view it draws, and menus and the score display are stretched to fit.
//
// MT_WIDE_RT64=1 (a desktop build's environment) widens with RT64's own Expand instead, as the page did before the game
// hack: then the culling is changed. The game skips objects whose bounding sphere lies outside its 4:3 view
// (func_800346A0, the sphere-vs-frustum test on the camera block at 0x80062B88), so with the picture showing more on
// the left and right, those objects popped in and out at the screen edges. Hooks in mt.us.toml scale the test's
// horizontal factor (camera block +0x80) by mt_cull_x_scale(): (4:3) / window aspect then, 1 otherwise. Only the
// visibility test is changed; the camera block itself (also used to place sprites on screen) is left as the game
// wrote it.

#include <algorithm>
#include <atomic>
#include <cstdlib>

#include "ultramodern/config.hpp"
#include "recompui/recompui.h"

namespace {
    std::atomic<float> cull_x_scale{1.0f};
}

extern "C" void rt64_set_game_widescreen(bool enabled);   // lib-patches/rt64.patch

bool mt_use_rt64_expand() {
    static const bool on = std::getenv("MT_WIDE_RT64") != nullptr;
    return on;
}

// Whether the game draws its own wider view: Expand, in a window wider than 4:3, without MT_WIDE_RT64.
static bool mt_game_widescreen() {
    if (mt_use_rt64_expand() || ultramodern::renderer::get_graphics_config().ar_option != ultramodern::renderer::AspectRatio::Expand) {
        return false;
    }
    int width = 0, height = 0;
    recompui::get_window_size(width, height);
    return (width > 0) && (height > 0) && (static_cast<float>(width) / static_cast<float>(height) > 4.0f / 3.0f);
}

// Called once per graphics task (mt_fixup_gfx_task), so the value follows window resizes and
// settings changes with at most a frame of delay.
void mt_widescreen_update() {
    rt64_set_game_widescreen(mt_game_widescreen());
    float scale = 1.0f;
    // Only for RT64's own widening (MT_WIDE_RT64=1); with the game hack the game culls for the wider view itself.
    if (mt_use_rt64_expand() && ultramodern::renderer::get_graphics_config().ar_option == ultramodern::renderer::AspectRatio::Expand) {
        int width = 0, height = 0;
        recompui::get_window_size(width, height);
        if (width > 0 && height > 0) {
            const float aspect = static_cast<float>(width) / static_cast<float>(height);
            scale = std::min(1.0f, (4.0f / 3.0f) / aspect);
        }
    }
    cull_x_scale.store(scale, std::memory_order_relaxed);
}

extern "C" float mt_cull_x_scale() {
    return cull_x_scale.load(std::memory_order_relaxed);
}

// Opening flyover in 4:3: the intro's camera flies up to the airship, through its hull and down
// onto the stadium. Widescreen showed scenery the game never meant to be seen at the sides (the
// airship's underside and red band around the overhead stadium shot), so while the flyover runs
// the presenter shows only the centred 4:3 part (RT64 patch: rt64_set_pillarbox).
// Detection reads the camera block's view matrix (0x80062B88 +0x40) each frame:
//  - start: the airship approach, camera at the airship's position looking slightly upward
//    (nothing in menus or matches looks like this);
//  - continue while the camera stays on the flyover path (approach, head-on banner, the drop
//    through the hull, the overhead shot and the descent to the court, centred on the court);
//  - end as soon as it leaves that path (it turns to follow Mario at court level), when a menu or
//    title frame appears (the intro was skipped), or after 16 s.
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <cstring>

extern "C" void rt64_set_pillarbox(bool enabled);   // lib/rt64 (Mario Tennis patch)

void mt_widescreen_flyover_check(uint8_t* rdram, bool narrow_frame) {
    static bool in_flyover = false;
    static std::chrono::steady_clock::time_point started;
    float m[16];
    for (int i = 0; i < 16; i++) {
        uint32_t w;
        std::memcpy(&w, rdram + 0x62B88 + 0x40 + i * 4, 4);
        std::memcpy(&m[i], &w, 4);
    }
    const float tx = m[12], ty = m[13], tz = m[14];
    const bool approach = (m[5] > 0.95f) && (m[6] <= 0.0f) && (ty > -1700.0f) && (ty < -1400.0f) &&
                          (tx > -900.0f) && (tx < 1.0f) && (tz > -1000.0f) && (tz < 100.0f);
    const bool descent = (m[0] > 0.999f) && (std::fabs(tx) < 1.0f) && (ty <= 0.0f) && (tz < -280.0f) && (tz > -2000.0f);
    const auto now = std::chrono::steady_clock::now();
    if (!in_flyover && approach) {
        started = now;
    }
    in_flyover = in_flyover ? (approach || descent) : approach;
    // Skipping the intro jumps to the title screen and menus, whose camera can still look like the
    // descent; menu frames use the narrow layout (mt_dlfix.cpp), which the flyover never does. The
    // flyover lasts about 14 s, so it is also capped.
    if (narrow_frame || (now - started) > std::chrono::seconds(16)) {
        in_flyover = false;
    }
    const bool expand = ultramodern::renderer::get_graphics_config().ar_option == ultramodern::renderer::AspectRatio::Expand;
    rt64_set_pillarbox(expand && in_flyover);
}

// Expand mode: the game hack (see mt.us.toml). Returns the aspect func_80033C34 should build the 3D
// camera with, as float bits: the game's own value (0x3FAB6DB7, 1.34 for its 4:3 picture) scaled by
// window aspect / (4:3). The picture is then stretched to the window (rt64_set_game_widescreen()), so 3D
// keeps its proportions at any window shape. Otherwise the original value.
extern "C" uint32_t mt_wide_aspect_bits(uint32_t original_bits) {
    if (!mt_game_widescreen()) {
        return original_bits;
    }
    int width = 0, height = 0;
    recompui::get_window_size(width, height);
    const float window_aspect = static_cast<float>(width) / static_cast<float>(height);
    float original;
    std::memcpy(&original, &original_bits, 4);
    const float wide = original * window_aspect / (4.0f / 3.0f);
    uint32_t bits;
    std::memcpy(&bits, &wide, 4);
    return bits;
}
