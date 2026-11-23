// The settings: RecompFrontend's General, Controls, Graphics and Sound tabs with their defaults, in place of the
// menus upstream carried over from Zelda 64: Recompiled. Upstream's General tab kept Rumble Strength, Joystick
// Deadzone and Background Input, and its Sound tab Main Volume, which RecompFrontend's tabs have too. Its own
// graphics options (fixed 720p-4K resolutions, the Stretch aspect ratio, the graphics API and the CRT shaders through
// librashader) need its changes to RT64 and N64ModernRuntime and a library of its own, and are left out: RecompFrontend's
// Graphics tab has Auto and the window's aspect ratio. There are no mods (the game has no decompilation to write them
// against), so no Mods tab. The General tab adds upstream's four cheats, all Off by default, which mt_bonus.cpp applies
// each graphics task (mt::cheat_on()). Upstream's Local Multiplayer setting is RecompFrontend's Assign players here
// (src/main/main.cpp).

#include <cstdint>
#include <filesystem>

#include "recompui/config.h"
#include "util/file.h"
#include "mt_game.h"

namespace {
    enum class Toggle : uint32_t {
        Off,
        On,
    };

    struct CheatOption {
        const char* id;
        const char* title;
        const char* description;
    };

    // In mt::Cheat's order. The ids are upstream's settings' names.
    constexpr CheatOption cheat_options[] = {
        { "cheat_all_courts", "All Courts",
          "Cheat. Makes every court selectable. Courts unlocked this way are saved with your game, so switching this off "
          "again does not lock them." },
        { "cheat_star_characters", "Star Characters",
          "Cheat. Gives every character their Star version. Saved with your game once unlocked." },
        { "cheat_all_tournaments", "All Tournaments Cleared",
          "Cheat. Marks every tournament cup as won, with everything that unlocks. Saved with your game once unlocked." },
        { "cheat_infinite_bounces", "Infinite Bounces",
          "Cheat. The ball may bounce any number of times before you have to return it. Only while this is on." },
    };
}

bool mt::cheat_on(mt::Cheat cheat) {
    auto value = recompui::config::get_general_config().get_option_value(cheat_options[static_cast<uint32_t>(cheat)].id);
    return std::get<uint32_t>(value) == static_cast<uint32_t>(Toggle::On);
}

void mt::init_config() {
    std::filesystem::path app_dir = recompui::file::get_app_folder_path();
    if (!app_dir.empty()) {
        std::filesystem::create_directories(app_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    recomp::config::Config& general = recompui::config::create_general_tab(general_options);
    for (const CheatOption& cheat : cheat_options) {
        general.add_enum_option(cheat.id, cheat.title, cheat.description, {{Toggle::Off, "Off"}, {Toggle::On, "On"}}, Toggle::Off);
    }
    recompui::config::create_controls_tab();
    recompui::config::create_graphics_tab();
    recompui::config::create_sound_tab();
    recompui::config::finalize();
}
