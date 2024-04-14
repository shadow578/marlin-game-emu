#include <iostream>
#include "win32_window.h"

void draw_fn(HDC hdc) {
    std::cout << "Drawing" << std::endl;

    // Draw a red rectangle
    RECT rect = {10, 10, 100, 100};
    HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    win32_window::set_draw_fn(draw_fn);
    if (!win32_window::create_and_run(hInstance, nCmdShow)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    return 0;
}
