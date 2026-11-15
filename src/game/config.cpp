// The settings: RecompFrontend's General, Controls, Graphics and Sound tabs with their defaults, in place of the
// menus upstream carried over from Zelda 64: Recompiled. Upstream's General tab kept Rumble Strength, Joystick
// Deadzone and Background Input, and its Sound tab Main Volume, which RecompFrontend's tabs have too. Its own
// graphics options (fixed 720p-4K resolutions, the Stretch aspect ratio, the graphics API and the CRT shaders through
// librashader) need its changes to RT64 and N64ModernRuntime and a library of its own, and are left out: RecompFrontend's
// Graphics tab has Auto and the window's aspect ratio. There are no mods (the game has no decompilation to write them
// against), so no Mods tab.

#include <filesystem>

#include "recompui/config.h"
#include "util/file.h"
#include "mt_game.h"

void mt::init_config() {
    std::filesystem::path app_dir = recompui::file::get_app_folder_path();
    if (!app_dir.empty()) {
        std::filesystem::create_directories(app_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    recompui::config::create_general_tab(general_options);
    recompui::config::create_controls_tab();
    recompui::config::create_graphics_tab();
    recompui::config::create_sound_tab();
    recompui::config::finalize();
}
