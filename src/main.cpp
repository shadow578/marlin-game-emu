#include <iostream>
#include "win32_window.h"
#include "compat.h"
#include "game_renderer.h"
#include "game/game.h"

void goto_menu_screen()
{
    menu.enter_game();
}

void draw_fn(HDC hdc)
{
    set_hdc(hdc);
    ui.draw();
}

void key_fn(WPARAM key)
{
    switch (key)
    {
    case VK_LEFT:
        ui.encoderPosition -= 1;
        break;
    case VK_RIGHT:
        ui.encoderPosition += 1;
        break;
    case VK_UP:
    case VK_SPACE:
        ui.did_click = true;
        break;
    case VK_ESCAPE:
        win32_window::set_paused(!win32_window::is_paused());
        break;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    goto_menu_screen();

    win32_window::set_target_fps(30);
    win32_window::set_draw_fn(draw_fn);
    win32_window::set_key_fn(key_fn);
    if (!win32_window::create_and_run(hInstance, nCmdShow))
    {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    return 0;
}
