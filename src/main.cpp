#include <iostream>
#include "win32_window.h"
#include "compat.h"
#include "game_renderer.h"
#include "game/game.h"

void draw_fn(HDC hdc) {
    set_hdc(hdc);
    ui.draw();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    snake.enter_game();

    win32_window::set_draw_fn(draw_fn);
    if (!win32_window::create_and_run(hInstance, nCmdShow)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    return 0;
}
