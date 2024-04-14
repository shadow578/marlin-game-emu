#include <iostream>
#include "win32_window.h"
#include "compat.h"
#include "game_renderer.h"
#include "game/game.h"

void draw_fn(HDC hdc)
{
    int frame_start_ms = GetTickCount();

    set_hdc(hdc);
    ui.draw();

    int frame_end_ms = GetTickCount();
    int frame_time_ms = frame_end_ms - frame_start_ms;
    std::cout << "Frame time: " << frame_time_ms << "ms" << std::endl;
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
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    snake.enter_game();

    win32_window::set_target_fps(15);
    win32_window::set_draw_fn(draw_fn);
    win32_window::set_key_fn(key_fn);
    if (!win32_window::create_and_run(hInstance, nCmdShow))
    {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    return 0;
}
