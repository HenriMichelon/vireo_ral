#include "Tools.h"

import dxvk.app;
import dxvk.app.win32;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow) {
    return dxvk::Win32Application::run(800, 600, L"DXVK Hello Triangle", hInstance, nCmdShow);
}
