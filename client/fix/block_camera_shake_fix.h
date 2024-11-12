#pragma once

#include "../command/command.h"

/// Function for command chimera_block_camera_shake
ChimeraCommandError bcsf_command(size_t argc, const char **argv) noexcept;

/// Function to enable the camera shake fix
void enable_camera_shake_fix() noexcept;