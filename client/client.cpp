#include <string>
#include <iostream>
#include <fstream>

#include "client.h"
#include "../version.h"

#include "client_signature.h"
#include "keystone.h"
#include "path.h"
#include "settings.h"
#include "command/console.h"
#include "command/command.h"

//#include "contributors/contributors.h"

#include "debug/budget.h"
#include "debug/devmode.h"
#include "debug/eep.h"
#include "debug/wireframe.h"

#include "enhancements/auto_center.h"
#include "enhancements/disable_buffering.h"
#include "enhancements/gamepad_vertical_scale.h"
#include "enhancements/firing_particle.h"
#include "enhancements/multitexture_overlay.h"
#include "enhancements/block_mouse_acceleration.h"
#include "enhancements/mouse_sensitivity.h"
#include "enhancements/server_messages.h"
#include "enhancements/show_spawn.h"
#include "enhancements/skip_loading.h"
#include "enhancements/throttle_fps.h"
#include "enhancements/uncap_cinematic.h"
#include "enhancements/zoom_blur.h"

#include "fix/camo_fix.h"
#include "fix/descope_fix.h"
#include "fix/fov_fix.h"
#include "fix/magnetism_fix.h"
#include "fix/scope_fix.h"
#include "fix/sniper_hud.h"
#include "fix/widescreen_fix.h"
#include "fix/block_camera_shake_fix.h"

#include "hac2/hac2_help.h"

#include "halo_data/chat.h"
#include "halo_data/keyboard.h"
#include "halo_data/map.h"
#include "halo_data/resolution.h"

#include "hooks/camera.h"
#include "hooks/frame.h"
#include "hooks/rcon_message.h"
#include "hooks/tick.h"
#include "hooks/map_load.h"

#include "interpolation/interpolation.h"

#include "lua/lua.h"

#include "messaging/messaging.h"

#include "startup/fast_startup.h"

#include "visuals/anisotropic_filtering.h"
#include "visuals/letterbox.h"
#include "visuals/server_ip.h"
#include "visuals/vertical_field_of_view.h"

#include "xbox/hud_kill_feed.h"
#include "xbox/safe_zone.h"
#include "xbox/simple_score_screen.h"
#include "xbox/split_screen_hud.h"

#include "visuals/gametype_indicator.h"

extern std::vector<ChimeraSignature> *signatures;
extern std::vector<std::string> *missing_signatures;
std::vector<ChimeraCommand> *commands;

LARGE_INTEGER performance_frequency;

static void init() {
    extern bool already_set;
    auto &enabled = **reinterpret_cast<char **>(get_signature("enable_console_sig").address() + 1);
    already_set = enabled != 0;
    if(!already_set)
        enabled = 1;
    remove_tick_event(init);

    settings_read_only(1);
    char z[512] = {};
    sprintf(z,"%s\\chimera", halo_path());
    CreateDirectory(z, nullptr);
    read_init_file("chimerainit.txt", "chimerainit.txt");
    sprintf(z,"%s\\chimera\\chimerainit.txt", halo_path());
    auto *f = fopen(z, "r");
    if(f)
    {
        fclose(f);
        read_init_file(z, "[-path]/chimerainit.txt");
    }
    else
    {
        std::ofstream init(z);
        init << "###" << std::endl;
        init << "### chimerainit.txt" << std::endl;
        init << "###" << std::endl;
        init << "### This file can be used for configuring Chimera commands that are not saved" << std::endl;
        init << "### automatically such as chimera_set_resolution." << std::endl;
        init << "###" << std::endl;
        init << std::endl;
    }
    settings_read_only(0);

    settings_do_not_save(1);
    sprintf(z,"%s\\chimera\\chimerasave.txt", halo_path());
    read_init_file(z, "[-path]/chimerasave.txt");
    settings_do_not_save(0);

    save_all_changes();
    setup_lua();
}

void initialize_client() noexcept {
    commands = new std::vector<ChimeraCommand>;
    signatures = new std::vector<ChimeraSignature>;
    missing_signatures = new std::vector<std::string>;
    if(!find_required_signatures())
    {
        for(size_t i=0;i<(*missing_signatures).size();i++)
        {
            char message[256] = {};
            sprintf(message, "Could not find %s signature. Make sure you're using Halo Custom Edition version 1.10.", (*missing_signatures)[i].data());
            MessageBox(NULL, message, "Chimera cannot load", MB_OK);
        }
        return;
    }
    initialize_map_load();
    initialize_console();
    initialize_rcon_message();
    add_tick_event(init);
    add_tick_event(camo_fix);
    add_map_load_event(tps_check_svr);
    dart_fix();
    enable_descope_fix();

    QueryPerformanceFrequency(&performance_frequency);

    //add_tick_event(set_contributors);

    if(find_magnetism_signatures())
    {
        fix_magnetism();
    }

    (*commands).emplace_back("chimera", chimera_command, nullptr,
        "This command is the commands directory for Chimera.\n\n"
        "Syntax:\n"
        "  - chimera - Display version and a list of command categories.\n"
        "  - chimera <category> - Display a list of commands in category.\n"
        "  - chimera <command> - Display help for a command."
    , 0, 1, true);

    (*commands).emplace_back("chimera_chat", chat_command, nullptr,
        "Send a chat message.\n\n"
        "Syntax:\n"
        "  - chimera_chat all <message> - Send a message to all players.\n"
        "  - chimera_chat team <message> - Send a message to your team.\n"
        "  - chimera_chat vehicle <message> - Send a message to your vehicle."
    , 1, 1000, true, false);

    (*commands).emplace_back("chimera_rcon_recovery", eep_command, nullptr,
        "Obtain the rcon password of the currently connected server. This may take a while\n"
        "depending on the complexity of the password. It only works with SAPP servers, though.\n\n"
        "Disclaimer: DO NOT USE THIS ON SERVERS YOU DO NOT OWN! HACKING IS A FELONY!\n\n"
        "Syntax:\n"
        "  - chimera_rcon_recover [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_reload_lua", reload_lua_command, "lua",
        "Reload all Lua scripts.\n\n"
        "Syntax:\n"
        "  - chimera_reload_lua"
    , 0, 0, true);

    (*commands).emplace_back("chimera_verbose_init", verbose_init_command, nullptr,
        "Get or set whether chimerainit.txt or chimeraname.txt commands should output messages.\n\n"
        "Syntax:\n"
        "  - chimera_verbose_init"
    , 0, 1, true);

    // Debug

    (*commands).emplace_back("chimera_budget", budget_command, "debug",
        "Get or set whether or show or hide various budgets.\n"
        "Options:\n"
        "  0: Off\n"
        "  1: On (Modded budgets if a mod is installed such as HAC2)\n"
        "  2: On (Stock budgets)\n\n"
        "Syntax:\n"
        "  - chimera_budget [0-2]"
    , 0, 1, find_debug_signatures(), false);

    (*commands).emplace_back("chimera_player_info", player_info_command, "debug",
        "Show player information.\n\n"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_devmode", devmode_command, "debug",
        "Get or set whether or not to enable Halo's developer commands.\n\n"
        "\n"
        "Syntax:\n"
        "  - chimera_devmode [true/false]"
    , 0, 1, find_devmode_sig(), true);

    (*commands).emplace_back("chimera_tps", tps_command, "debug",
        "Get or set tick rate. This value cannot be set below 0.01.\n\n"
        "Syntax:\n"
        "  - chimera_tps [ticks per second]"
    , 0, 1, find_debug_signatures(), false);

    (*commands).emplace_back("chimera_wireframe", wireframe_command, "debug",
        "Get or set whether to enable or disable wireframe mode. This will not work while in a server.\n\n"
        "Syntax:\n"
        "  - chimera_wireframe [true/false]"
    , 0, 1, find_debug_signatures(), false);

    // HAC2
    (*commands).emplace_back("chimera_hac2", hac2_command, "hac2",
         "Display all available HAC2 commands.\n\n"
         "Syntax:\n"
         "  - chimera_hac2"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_bookmarking",hac2_bookmarking_command, "hac2",
         "Display all available HAC2 bookmarking binds.\n"
         "Options:\n"
         "  0: Add current server to your bookmarkers / favorites list.\n"
         "  1: View your bookmarkers / favorites list.\n"
         "  2: Join bookmarked server.\n"
         "  3: Delete bookmarked server.\n"
         "  4: Selects bookmarks in the range 11-20 (example, ctrl + alt + 4 will join bookmark 14).\n"
         "  5: Reconnect to last server.\n\n"
         "Syntax:\n"
        "  - F4/F5/ALT + Bookmark #/DEL + Bookmark #/CTRL + ALT/DEL + Bookmark #/ CTRL + L"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_binds", hac2_binds_command, "hac2",
         "Display other available HAC2 binds.\n"
         "Options:\n"
         "  0: Change your field of view using your mouse up and down.\n"
         "  Press F6 again to lock new FOV.\n"
         "  1: Resets your FOV to its default (70)\n"
         "  2: Set HUD color. This will cycle through all of the HUDs.\n"
         "  This is controlled with your mouse.\n"
         "  3: Resets HUD colors.\n"
         "  4: Enable/disable Sightjacker.\n"
         "  You can switch between player via -/+, or your mouse scroll wheel.\n\n"
         "Syntax:\n"
         "  - F6/LSHIFT + F6/F9/LSHIFT + F9/F7"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_copy", hac2_copy_command, "hac2",
         "Copies the information of a bookmarked server.\n"
         "If no number is provided it copies information of the current server.\n\n"
         "Syntax:\n"
         "  - copy [bookmark #]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_optic", hac2_optic_commands, "hac2",
         "Load and unload an optic medal pack.\n"
         "Options:\n"
         "  0: Display all available medal packs.\n"
         "  1: Load an optic medal pack.\n"
         "  2: Stop the optic medal system. You do not need to use unload to load another pack.\n\n"
         "Syntax:\n"
         "  - optic/optic_load [pack_name]/optic_unload"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_timer", hac2_timer_commands, "hac2",
         "Enables or disables power-up timers (On supported maps.)\n"
         "Options:\n"
         "  0: Enables power-up timers (on supported maps.)\n"
         "  1: Disables the power-up timers.\n"
         "  2: Resets the power-up timers.\n"
         "  3: Starts the power-up timers (use when map loads.)\n"
         "  4: Stops the power-up timers.\n"
         "  5: Broadcasts the power-up timers to the team chat.\n\n"
         "Syntax:\n"
         "  - timer_enable/timer_disable/timer_reset/timer_start/timer_stop/timer_broadcast"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_chat", hac2_chat_commands, "hac2",
         "Hac2 chat options.\n"
         "Options:\n"
         "  0: Redirect kill feed to the upper left part of the hud.\n"
         "  1: Turn HAC2's custom chat on/off. The default is off.\n"
         "  2: Turn HAC2's classic chat on/off. The default is off.\n"
         "  (Requires custom_chat to be enabled.)\n"
         "  3: Edit the number of lines displayed in custom and classic chat modes.\n"
         "  4: Edit the font size of HAC2's custom and classic chat.\n"
         "  5: Edit the font of HAC2's classic chat.\n\n"
         "Syntax:\n"
         "  - redirect_kill_feed [0/1]/custom_chat [0/1]/classic_chat [0/1]\n"
         "  - line_limit [lines]/font_size [size]/font [font]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_fov", hac2_fov_command, "hac2",
         "Can be used as an alternative to F6 for setting fov.\n\n"
         "Syntax:\n"
         "  - fov [degrees]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_widescreen", hac2_widescreen_command, "hac2",
         "Turn HAC2's HUD scaling fix on/off. The default is on\n\n"
         "Syntax:\n"
         "  - hac_widescreen [0/1]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_scope_blur", hac2_scope_blur_command, "hac2",
         "Turn zoom scope blur mask on/off. The default is on\n\n"
         "Syntax:\n"
         "  - scope_blur [0/1]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_queue", hac2_queue_command, "hac2",
         "Allows you to remain in the current game until a free slot is available\n"
         "in the bookmarked server you attempted to join\n\n"
         "Syntax:\n"
         "  - play_during_queue [0/1]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_devcam", hac2_devcam_command, "hac2",
         "Toggles developer camera.\n\n"
         "Syntax:\n"
         "  - devcam [0/1]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_draw", hac2_draw_command, "hac2",
         "Distance is the number of world units to draw.\n"
         "The default is 1024\n"
         "HAC uses 4096 for maps that benefit from increased draw distances.\n\n"
         "Syntax:\n"
         "  - draw [distance]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_recalculate", hac2_recalculate_command, "hac2",
         "Recalculates the checksum for the given map.\n"
         "This is aimed at map makers that don't want to use the cached checksum\n"
         "during development given that the checksum will change with each compile.\n"
         "Use quotation marks for map names with spaces.\n\n"
         "Syntax:\n"
         "  - recalculate [map_name]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_objects", hac2_objects_command, "hac2",
         "Sets the number of visible objects\n\n"
         "Syntax:\n"
         "  - visible_objects [count]"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_cache", hac2_cache_command, "hac2",
         "Clear the entire checksum cache. There's no real reason to use this but it's there anyway.\n\n"
         "Syntax:\n"
         "  - hac_flush_cache"
    , 0, 0, true, false);

    (*commands).emplace_back("chimera_hac2_meme", hac2_meme_command, "hac2",
         "lul.\n\n"
         "Syntax:\n"
         "  - hac_debug_bypass"
    , 0, 0, true, false);


    // Enhancements

    (*commands).emplace_back("chimera_auto_center", auto_center_command, "enhancements",
        "Get or set how auto centering of vehicle cameras should behave.\n"
        "Options:\n"
        "  0: Broken stock behavior\n"
        "  1: Fixed behavior\n"
        "  2: Disable automatic centering\n\n"
        "Syntax:\n"
        "  - chimera_auto_center [0-2]"
    , 0, 1, find_auto_center_signature(), true);

    (*commands).emplace_back("chimera_block_letterbox", block_letterbox_command, "enhancements",
        "Get or set whether or not to block the letterbox effect in cinematics.\n\n"
        "Syntax:\n"
        "  - chimera_block_letterbox [true/false]"
    , 0, 1, find_widescreen_fix_signatures() && find_widescreen_scope_signature(), true);

    (*commands).emplace_back("chimera_block_mo", block_mo_command, "enhancements",
        "Get or set whether or not to disable multitexture overlays. This feature is intended to fix the\n"
        "buggy HUD on the stock sniper rifle, but multitexture overlays may be used correctly on\n"
        "some maps.\n\n"
        "Syntax:\n"
        "  - chimera_block_mo [true/false]"
    , 0, 1, find_multitexture_overlay_signature(), true);

    (*commands).emplace_back("chimera_block_mouse_acceleration", block_mouse_acceleration_command, "enhancements",
        "Get or set whether or not to block mouse acceleration. Note that some mice may still\n"
        "exhibit some mouse acceleration.\n\n"
        "Syntax:\n"
        "  - chimera_block_mouse_acceleration [true/false]"
    , 0, 1, find_block_mouse_acceleration_sigs(), true);

    (*commands).emplace_back("chimera_block_server_messages", block_server_messages_command, "enhancements",
        "Get or set whether or not to block inbound server messages.\n\n"
        "Syntax:\n"
        "  - chimera_block_server_messages [true/false]"
    , 0, 1, find_server_message_sig(), true);

    (*commands).emplace_back("chimera_block_zoom_blur", block_zoom_blur_command, "enhancements",
        "Get or set whether or not to disable the zoom blur.\n\n"
        "Syntax:\n"
        "  - chimera_block_zoom_blur [true/false]"
    , 0, 1, find_zoom_blur_signatures(), true);

    (*commands).emplace_back("chimera_disable_buffering", disable_buffering_command, "enhancements",
        "Get or set whether or not to disable buffering. This may improve input latency.\n\n"
        "Syntax:\n"
        "  - chimera_disable_buffering [true/false]"
    , 0, 1, find_disable_buffering_sig(), true);

    (*commands).emplace_back("chimera_enable_console", enable_console_command, "enhancements",
        "Get or set whether or not to automatically enable the console.\n"
        "Unlike most other features, this feature is enabled by default.\n\n"
        "Syntax:\n"
        "  - chimera_enable_console [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_gamepad_vertical_scale", gamepad_vertical_scale_command, "enhancements",
        "Get or set whether or not to scale gamepad vertical sensitivity.\n\n"
        "Syntax:\n"
        "  - chimera_gamepad_vertical_scale [value]"
    , 0, 1, find_gamepad_vertical_scale_signatures(), true);

    (*commands).emplace_back("chimera_mouse_sensitivity", mouse_sensitivity_command, "enhancements",
        "Set the horizontal and vertical mouse sensitivities.\n\n"
        "Values less than 1 do not work properly if mouse acceleration is enabled.\n\n"
        "Syntax:\n"
        "  - chimera_mouse_sensitivity [<vertical> <horizontal>]"
        "  - chimera_mouse_sensitivity <false>"
    , 0, 2, find_mouse_sensitivity_sigs(), true);

    (*commands).emplace_back("chimera_show_spawns", show_spawns_command, "enhancements",
        "Get or set whether or not to show spawns.\n\n"
        "Syntax:\n"
        "  - chimera_show_spawns [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_skip_loading", skip_loading_command, "enhancements",
        "Get or set whether or not to skip the multiplayer loading screen.\n\n"
        "Syntax:\n"
        "  - chimera_skip_loading [true/false]"
    , 0, 1, find_loading_screen_signatures(), true);

    (*commands).emplace_back("chimera_uncap_cinematic", uncap_cinematic_command, "enhancements",
        "Get or set whether or not to remove the 30 FPS framerate cap in cinematics. This may result\n"
        "in objects jittering during cutscenes if chimera_interpolate is not enabled.\n\n"
        "Syntax:\n"
        "  - chimera_uncap_cinematic [true/false]"
    , 0, 1, find_uncap_cinematic_signatures(), true);

    // Fixes

    (*commands).emplace_back("chimera_aim_assist", aim_assist_command, "fixes",
        "Get or set whether or not fix aim assist for gamepads. This feature is on by default.\n\n"
        "Syntax:\n"
        "  - chimera_aim_assist [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_fov_fix", fov_fix_command, "fixes",
        "Get or set whether or not to fix the FOV. This will make FOV mods more accurate.\n\n"
        "Syntax:\n"
        "  - chimera_fov_fix [true/false]"
    , 0, 1, find_fov_fix_sigs() && find_simple_score_screen_sigs(), true);

    (*commands).emplace_back("chimera_sniper_hud_fix", sniper_hud_fix_command, "fixes",
        "Get or set whether or not to fix the sniper HUD. This may not work on protected maps.\n\n"
        "Syntax:\n"
        "  - chimera_sniper_hud_fix [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_widescreen_fix", widescreen_fix_command, "fixes",
        "Get or set whether or not to (mostly) fix the HUD.\n\n"
        "Note that this will break the HUD if you are using any other widescreen fix.\n\n"
        "Settings:\n"
        "  0: Off\n"
        "  1: On\n"
        "  2: On (center HUD)\n\n"
        "Syntax:\n"
        "  - chimera_widescreen_fix [0-2]"
    , 0, 1, find_widescreen_fix_signatures() && find_widescreen_scope_signature(), true);

    (*commands).emplace_back("chimera_widescreen_scope_fix", widescreen_scope_fix_command, "fixes",
        "Enhance an existing widescreen fix by also fixing the scope mask if it's not fixed.\n\n"
        "Syntax:\n"
        "  - chimera_widescreen_scope_fix [true/false]"
    , 0, 1, find_widescreen_scope_signature(), true);

    (*commands).emplace_back("chimera_bcsf", bcsf_command, "fixes",
        "Get or set whether or not the first person camera shake should be blocked.\n\n"
        "Syntax:\n"
        "  - chimera_bcsf [true/false]"
    , 0, 1, find_block_camera_shake_signature(), true);

    // Interpolation

    (*commands).emplace_back("chimera_interpolate", interpolate_command, "interpolation",
        "Get or set the interpolation level. Interpolation smoothes out object movement between\n"
        "ticks, providing a substantial visual improvement. Higher levels incur greater CPU usage and\n"
        "may impact framerate on slower CPUs.\n\n"
        "Syntax:\n"
        "  - chimera_interpolate [off/low/medium/high/ultra]"
    , 0, 1, find_interpolation_signatures(), true);

    (*commands).emplace_back("chimera_interpolate_predict", interpolate_predict_command, "interpolation",
        "Get or set whether the next tick should be predicted when interpolating. This will prevent\n"
        "objects from appearing as if they are one tick behind, but sudden object movement may\n"
        "cause jitteriness.\n\n"
        "Settings:\n"
        "  0: Off\n"
        "  1: On\n"
        "  2: On (Do not intepolate the first person camera.)\n"
        "  3: On (Only interpolate the first person camera while in a vehicle.)\n"
        "Syntax:\n"
        "  - chimera_interpolate_predict [0-3]"
    , 0, 1, find_interpolation_signatures(), true);

    // Visuals

    (*commands).emplace_back("chimera_af", af_command, "visuals",
        "Get or set whether or not to enable anisotropic filtering.\n\n"
        "Syntax:\n"
        "  - chimera_af [true/false]"
    , 0, 1, find_anisotropic_filtering_signature(), true);

    (*commands).emplace_back("chimera_block_firing_particles", block_firing_particles_command, "visuals",
        "Get or set whether or not to block firing particles.\n\n"
        "Syntax:\n"
        "  - chimera_block_firing_particles [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_block_gametype_indicator", block_gametype_indicator_command, "visuals",
        "Get or set whether or not to turn off the gametype indicator.\n\n"
        "\n"
        "Syntax:\n"
        "  - chimera_block_gametype_indicator [true/false]"
    , 0, 1, find_gametype_indicator_sig(), true);

    (*commands).emplace_back("chimera_block_server_ip", block_server_ip_command, "visuals",
        "Get or set whether or not to hide the server IP. This may be useful for streamers.\n\n"
        "Syntax:\n"
        "  - chimera_block_server_ip [true/false]"
    , 0, 1, find_hide_server_ip_sigs(), true);

    (*commands).emplace_back("chimera_block_vsync", block_vsync_command, "visuals",
        "Get or set whether or not to turn vSync off startup.\n\n"
        "\n"
        "Syntax:\n"
        "  - chimera_block_vsync [true/false]"
    , 0, 1, find_set_resolution_signatures(), true);

    (*commands).emplace_back("chimera_set_resolution", set_resolution_command, "visuals",
        "Change Halo's resolution. Width and height can be either resolution in pixels or an aspect\n"
        "ratio.\n\n"
        "Syntax:\n"
        "  - chimera_set_resolution <width> <height> [refresh rate] [vsync] [windowed]"
    , 2, 5, find_set_resolution_signatures(), true);

    (*commands).emplace_back("chimera_throttle_fps", throttle_fps_command, "visuals",
        "Throttle Halo's framerate.\n\n"
        "Syntax:\n"
        "  - chimera_throttle_fps [max FPS]"
    , 0, 1, find_uncap_cinematic_signatures(), true);

    (*commands).emplace_back("chimera_vfov", vfov_command, "visuals",
        "Get or change your FOV by attempting to lock to a specific vertical FOV. This will\n"
        "distort your FOV if HAC2, Open Sauce, etc. are modifying your horizontal FOV. 1\n"
        "defaults to 55.41 degrees, or Halo’s standard FOV.\n\n"
        "Syntax:\n"
        "  - chimera_vfov [VFOV]"
    , 0, 1, find_interpolation_signatures(), true);

    // Xbox

    (*commands).emplace_back("chimera_hud_kill_feed", hud_kill_feed_command, "xbox",
        "Get or set whether or not to emit kills and deaths messages as HUD text.\n\n"
        "Syntax:\n"
        "  - chimera_hud_kill_feed [true/false]"
    , 0, 1, find_hud_kill_feed_sig(), true);

    (*commands).emplace_back("chimera_safe_zones", safe_zones_command, "xbox",
        "Get or set whether or not to emulate Xbox safe zones.\n\n"
        "Syntax:\n"
        "  - chimera_safe_zones [true/false]"
    , 0, 1, true, true);

    (*commands).emplace_back("chimera_simple_score_screen", simple_score_screen_command, "xbox",
        "Get or set whether or not to use a simplified in-game score screen.\n\n"
        "Syntax:\n"
        "  - chimera_simple_score_screen [true/false]"
    , 0, 1, find_simple_score_screen_sigs(), true);

    (*commands).emplace_back("chimera_split_screen_hud", split_screen_hud_command, "xbox",
        "Get or set whether or not to use Halo's split screen HUD.\n\n"
        "Syntax:\n"
        "  - chimera_simple_score_screen [true/false]"
    , 0, 1, find_split_screen_hud_sigs(), true);

    // Startup

    (*commands).emplace_back("chimera_cache", cache_command, "startup",
        "Get or set whether or not to use a cache for fast startup.\n\n"
        "Syntax:\n"
        "  - chimera_cache [true/false]"
    , 0, 1, find_fast_startup_sigs(), true);

    (*commands).emplace_back("chimera_cache_clear", cache_clear_command, "startup",
        "Clear the cache.\n\n"
        "Syntax:\n"
        "  - chimera_cache_clear"
    , 0, 0, find_fast_startup_sigs(), false);

    (*commands).emplace_back("chimera_modded_stock_maps", modded_stock_maps_command, "startup",
        "Get or set whether or not stock maps will use hardcoded CRC32s. This may be required for\n"
        "some maps to work.\n\n"
        "Syntax:\n"
        "  - chimera_modded_stock_maps [true/false]"
    , 0, 1, find_fast_startup_sigs(), true);

    if(find_fast_startup_sigs()) setup_fast_startup();

    if(custom_keystone_in_use())
    {
      if(find_pc_map_compat_sigs()) setup_pc_map_compatibility();
      if(find_keystone_sigs()) setup_keystone_override();
    }
    if(find_console_fade_fix_sig()) setup_console_text_fix();


    add_frame_event(check_keys);
}

void uninitialize_client() noexcept {
    destroy_lua();
    for(size_t i=0;i<signatures->size();i++)
    {
        (*signatures)[i].undo();
    }
    delete signatures;
    signatures = nullptr;
    delete missing_signatures;
    missing_signatures = nullptr;
    delete commands;
    commands = nullptr;
}
