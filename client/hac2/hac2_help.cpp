#include "hac2_help.h"
#include "../messaging/messaging.h"

ChimeraCommandError hac2_command(size_t argc, const char **argv) noexcept {
  console_out("Available HAC2 Commands:",ColorARGB(1, 1, 0.84, 0));
  console_out("--------------------------------------------");
  console_out("chimera_hac2_bookmarking - Bookmark server binds",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_binds - Other HAC2 key bindings",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_copy - Server information copying",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_optic - Optic medal pack system",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_timer - Power-up timer commands",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_chat - Chat customization options",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_fov - Field of view adjustment",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_widescreen - HUD scaling settings",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_scope_blur - Scope blur settings",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_queue - Server queue management",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_devcam - Developer camera controls",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_draw - Draw distance settings",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_recalculate - Map checksum command",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_objects - Object visibility settings",ColorARGB(1, 1, 0.84, 0));
  console_out("chimera_hac2_cache - Cache command",ColorARGB(1, 1, 0.84, 0));
  console_out("\nFor detailed information about any command, type the command name");
  console_out("Example: chimera_hac2_optic");

  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_bookmarking_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Bookmarking Bindings\n",ColorARGB(1, 1, 0.84, 0));
  console_out("Options:\n");
  console_out("0: F4 - Add current server to your bookmarks / favorites list.\n");
  console_out("1: F5 - View your bookmarks / favorites list.\n");
  console_out("2: ALT + Bookmark # - Join bookmarked server.\n");
  console_out("3: DEL + Bookmark # - Delete bookmarked server.\n");
  console_out("4: CTRL + ALT + Bookmark # - Selects bookmarks in the range 11-20");
  console_out("(Example: ctrl + alt + 4 will join bookmark 14.)\n");
  console_out("(Example: ctrl + del + 4 will delete bookmark 14.)\n");
  console_out("5: CTRL + L - Reconnect to last server.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_binds_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Key Bindings\n",ColorARGB(1, 1, 0.84, 0));
  console_out("Options:\n");
  console_out("0: F6 - Change your FOV by moving your mouse up and down.\n");
  console_out("(Press F6 again to lock new FOV.)\n");
  console_out("1: LSHIFT + F6 - Resets your FOV to its default (70)\n");
  console_out("2: F9 - Set HUD color. Press F9 to cycle through all of the HUDs.");
  console_out("(Color changing is controlled with your mouse.)\n");
  console_out("3: LSHIFT + F9 - Resets HUD colors.\n");
  console_out("4: F7 - Enable/disable Sightjacker.\n");
  console_out("(You can switch between player via -/+, or your mouse scroll wheel.)\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_copy_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Copy Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("copy [bookmark #] - Copies the information of a bookmarked server.\n");
  console_out("(If no number is provided it copies information of the current server.)\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_optic_commands(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Optic Medal Pack Commands",ColorARGB(1, 1, 0.84, 0));
  console_out("Options:\n");
  console_out("0: optic - Display all available medal packs.\n");
  console_out("1: optic load [pack_name] - Load an optic medal pack.\n");
  console_out("2: optic unload - Stop the optic medal system.\n");
  console_out("(You do not need to use unload to load another pack.)\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_timer_commands(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Power-Up Timer Commands",ColorARGB(1, 1, 0.84, 0));
  console_out("Options:\n");
  console_out("0: timer_enable - Enables power-up timers (on supported maps.)\n");
  console_out("1: timer_disable - Disables the power-up timers.\n");
  console_out("2: timer_reset - Resets the power-up timers.\n");
  console_out("3: timer_start - Starts the power-up timers (use when map loads.)\n");
  console_out("4: timer_stop - Stops the power-up timers.\n");
  console_out("5: timer_broadcast - Broadcasts the power-up timers to the team chat.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_chat_commands(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Chat Commands\n",ColorARGB(1, 1, 0.84, 0));
  console_out("Options:\n");
  console_out("0: redirect_kill_feed [0/1] - Redirect kill feed to the upper left part of the hud.\n");
  console_out("1: custom_chat [0/1] - Turn HAC2's custom chat on/off. The default is off.\n");
  console_out("2: classic_chat [0/1] - Turn HAC2's classic chat on/off. The default is off.\n");
  console_out("(Requires custom_chat to be enabled.)\n");
  console_out("3: line_limit [lines] - Edit the number of lines displayed in HAC2 chat modes.\n");
  console_out("4: font_size [size] - Edit the font size of HAC2's custom and classic chat.\n");
  console_out("5: font [font] - Edit the font of HAC2's classic chat.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_fov_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 FOV Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("fov [degrees] - Can be used as an alternative to F6 for setting fov.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_widescreen_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Widescreen Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("hac_widescreen [0/1] - Turn HAC2's HUD scaling fix on/off. The default is on.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_scope_blur_command(size_t argc, const char **argv) noexcept {
    console_out("HAC2 Scope Blur Command\n",ColorARGB(1, 1, 0.84, 0));
    console_out("scope_blur [0/1] - Turn zoom scope blur mask on/off.\n");
    return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_queue_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Server Queue Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("play_during_queue [0/1] - Allows you to remain in the current game\n");
  console_out("until a free slot is available in the bookmarked server you attempted to join.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_devcam_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Devcam Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("devcam [0/1] - Toggles developer camera.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_draw_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Draw Distance Commands\n",ColorARGB(1, 1, 0.84, 0));
  console_out("draw [distance] - Distance is the number of world units to draw. The default is 1024.\n");
  console_out("HAC uses 4096 for maps that benefit from increased draw distances.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_recalculate_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Recalculate Checksum Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("recalculate [map_name] - Recalculates the checksum for the given map.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_objects_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Object Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("visible_objects [count] - Sets the number of visible objects.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_cache_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Cache Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("hac_flush_cache - Clears the entire checksum cache.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError hac2_meme_command(size_t argc, const char **argv) noexcept {
  console_out("HAC2 Meme Command\n",ColorARGB(1, 1, 0.84, 0));
  console_out("hac_debug_bypass - lul.\n");
  return CHIMERA_COMMAND_ERROR_SUCCESS;
}