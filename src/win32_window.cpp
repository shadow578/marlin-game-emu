#include "win32_window.h"
#include <stdio.h>

static win32_window::draw_fn draw_func = nullptr;
static win32_window::key_fn key_func = nullptr;

static int min_millis = 16;
static int last_millis = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_PAINT:
  {
    // only redraw if enough time has passed
    int current_millis = GetTickCount();
    if (current_millis - last_millis < min_millis)
    {
      return 0;
    }

    // calculate and show FPS
    int fps = 1000 / (current_millis - last_millis);
    char fps_str[32];
    sprintf(fps_str, "FPS: %d", fps);
    SetWindowText(hwnd, fps_str);

    last_millis = current_millis;

    // paint the window
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (draw_func != nullptr)
      draw_func(hdc);

    EndPaint(hwnd, &ps);
    return 0;
  }

  case WM_KEYDOWN:
    key_func(wParam);
    break;

  case WM_TIMER:
  {
    // Timer triggered, redraw the window
    InvalidateRect(hwnd, NULL, TRUE);
    return 0;
  }

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

void win32_window::set_draw_fn(draw_fn fn)
{
  draw_func = fn;
}

void win32_window::set_key_fn(key_fn fn)
{
  key_func = fn;
}

void win32_window::set_target_fps(int fps)
{
  min_millis = 1000 / fps;
}

bool win32_window::create_and_run(HINSTANCE hInstance, int nCmdShow)
{
  // Register the window class
  const char CLASS_NAME[] = "Sample Window Class";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);

  // Create the window
  HWND hwnd = CreateWindowEx(
      0,                   // Optional window styles
      CLASS_NAME,          // Window class
      "GameEmu",           // Window text
      WS_OVERLAPPEDWINDOW, // Window style

      // Size and position
      CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,

      NULL,      // Parent window
      NULL,      // Menu
      hInstance, // Instance handle
      NULL       // Additional application data
  );

  if (hwnd == NULL)
  {
    return false;
  }

  ShowWindow(hwnd, nCmdShow);

  // Set up a timer for continuous redraw
  SetTimer(hwnd, 10, 1, NULL);

  // Run the message loop
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return true;
}
