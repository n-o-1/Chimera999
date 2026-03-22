#include "server_ip.h"

#include <cstdint>
#include <cwchar>
#include "../messaging/messaging.h"
#include "../client_signature.h"
#include "../hooks/frame.h"

extern char *console_text;
static char new_text[1024];
static bool first_use = true;

static bool f1_ip_address_override_only_address = true;
static std::string f1_ip_address_string = "*.*.*.*:*";
void set_server_ip_string(std::string string, bool only_address)
{
    f1_ip_address_string = string;
    f1_ip_address_override_only_address = only_address;
}

static void read_buffer() noexcept {
    strcpy(new_text, console_text);
    const char *connect = "connect";
    for(size_t c=0;c<4;c++)
    {
        char *beginning = new_text + c;
        if(strncmp(beginning, connect, strlen(connect)) == 0)
        {
            for(size_t i=strlen(connect); i<sizeof(new_text) - 1 - c;i++)
            {
                if(beginning[i] == 0) break;
                if(beginning[i] != ' ') beginning[i] = '*';
            }
            return;
        }
    }
}

// int swprintf(wchar_t* buffer, size_t bufsz, const wchar_t* format, ...);
// Arguments passed in usual cdecl convention, except bufsz which is passed through EDX
// prefix is a language-dependent string (en: L"Server IP Address - ")
// Arguments are reordered from traditional snprintf to take advantage of fastcall convention
//
// Note: Halo's wide sprintf here has "%s" mean a wide null-terminated string, which is different from the standard lib
static_assert(sizeof(wchar_t) == 2); // It would be really awkward if this wasn't the case in a Windows environment.
static __attribute__((fastcall)) int swprintf_ip_address_long(
    void* /*ECX*/,
    size_t bufsz,
    wchar_t* buffer,
    const wchar_t* /*format*/, // "%s%s:%u"
    const wchar_t* prefix,
    const wchar_t* address,
    std::uint16_t port);
static __attribute__((fastcall)) int swprintf_ip_address_short(
    void*,
    size_t bufsz,
    wchar_t* buffer,
    const wchar_t* /*format*/, // "%s%s"
    const wchar_t* prefix,
    const wchar_t* address);

ChimeraCommandError block_server_ip_command(size_t argc, const char **argv) noexcept {
    // set_server_ip_string("Welcome to Team Fortress 2.", false);

    static auto active = false;
    if(argc == 1)
    {
        bool new_value = bool_value(argv[0]);
        if(new_value != active)
        {
            auto &join_server_ip_text_sig = get_signature("join_server_ip_text_sig");
            auto &swprintf_server_ip_long_sig = get_signature("swprintf_server_ip_long_sig");
            auto &swprintf_server_ip_short_sig = get_signature("swprintf_server_ip_short_sig");
            auto &create_server_ip_text_sig = get_signature("create_server_ip_text_sig");
            auto &console_buffer_text_show_sig = get_signature("console_buffer_text_show_sig");
            if(new_value)
            {
                const unsigned char mod[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
                write_code_c(join_server_ip_text_sig.address() + 5, mod);

                const short mod_create_server_ip[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0x90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x66, 0xB9, 0x00, 0x00, 0x90, 0x90, 0x90 };
                write_code_s(create_server_ip_text_sig.address(), mod_create_server_ip);

                // Supply nops=3 since the original function is caller cleans up the stack, but our replacements are caller cleans up the stack
                replace_call_destination(swprintf_server_ip_long_sig.address() + 11, reinterpret_cast<void*>(&swprintf_ip_address_long), 3);
                replace_call_destination(swprintf_server_ip_short_sig.address() + 11, reinterpret_cast<void*>(&swprintf_ip_address_short), 3);

                write_code_any_value(console_buffer_text_show_sig.address(), static_cast<unsigned char>(0xB8));
                write_code_any_value(console_buffer_text_show_sig.address() + 1, new_text);
                write_code_any_value(console_buffer_text_show_sig.address() + 5, static_cast<unsigned char>(0x90));
                add_preframe_event(read_buffer);

                memset(new_text, 0, sizeof(new_text));
            }
            else
            {
                join_server_ip_text_sig.undo();
                swprintf_server_ip_long_sig.undo();
                swprintf_server_ip_long_sig.undo();
                create_server_ip_text_sig.undo();
                console_buffer_text_show_sig.undo();
                remove_preframe_event(read_buffer);
            }
            active = new_value;
        }
    }
    console_out(active ? "true" : "false");
    return CHIMERA_COMMAND_ERROR_SUCCESS;
}

ChimeraCommandError block_server_ip_command2() noexcept {
    static auto active = true;

            auto &join_server_ip_text_sig = get_signature("join_server_ip_text_sig");
            auto &swprintf_server_ip_long_sig = get_signature("swprintf_server_ip_long_sig");
            auto &swprintf_server_ip_short_sig = get_signature("swprintf_server_ip_short_sig");
            auto &create_server_ip_text_sig = get_signature("create_server_ip_text_sig");
            auto &console_buffer_text_show_sig = get_signature("console_buffer_text_show_sig");
            if(first_use)
            {
                const unsigned char mod[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
                write_code_c(join_server_ip_text_sig.address() + 5, mod);

                const short mod_create_server_ip[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0x90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x66, 0xB9, 0x00, 0x00, 0x90, 0x90, 0x90 };
                write_code_s(create_server_ip_text_sig.address(), mod_create_server_ip);

                replace_call_destination(swprintf_server_ip_long_sig.address() + 11, reinterpret_cast<void*>(&swprintf_ip_address_long), 3);
                replace_call_destination(swprintf_server_ip_short_sig.address() + 11, reinterpret_cast<void*>(&swprintf_ip_address_short), 3);

                write_code_any_value(console_buffer_text_show_sig.address(), static_cast<unsigned char>(0xB8));
                write_code_any_value(console_buffer_text_show_sig.address() + 1, new_text);
                write_code_any_value(console_buffer_text_show_sig.address() + 5, static_cast<unsigned char>(0x90));
                add_preframe_event(read_buffer);

                memset(new_text, 0, sizeof(new_text));
                first_use = false;
            }
    //console_out(active ? "true" : "false");
    return CHIMERA_COMMAND_ERROR_SUCCESS;
}

int swprintf_ip_address_long(
    void* /*ECX*/,
    size_t bufsz,
    wchar_t* buffer,
    const wchar_t* /*format*/, // L"%s%s:%u"
    const wchar_t* prefix,
    const wchar_t* address,
    std::uint16_t port)
{
    if (f1_ip_address_override_only_address)
    {
        return std::swprintf(buffer, bufsz, L"%ls%s", prefix, f1_ip_address_string.c_str());
    }
    else
    {
        return std::swprintf(buffer, bufsz, L"%s", f1_ip_address_string.c_str());
    }
}

int swprintf_ip_address_short(
    void*,
    size_t bufsz,
    wchar_t* buffer,
    const wchar_t* /*format*/, // L"%s%s"
    const wchar_t* prefix,
    const wchar_t* address)
{
    if (f1_ip_address_override_only_address)
    {
        return std::swprintf(buffer, bufsz, L"%ls%s", prefix, f1_ip_address_string.c_str());
    }
    else
    {
        return std::swprintf(buffer, bufsz, L"%s", f1_ip_address_string.c_str());
    }
}