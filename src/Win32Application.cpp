module;
#include "Tools.h"

module dxvk.app.win32;

import dxvk.app;

namespace dxvk {

    HWND Win32Application::hwnd = nullptr;
    std::unique_ptr<Application> Win32Application::app{};

    int Win32Application::run(UINT width, UINT height, std::wstring name, const HINSTANCE hInstance, const int nCmdShow) {
        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"DXSampleClass";
        RegisterClassEx(&windowClass);

        RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        hwnd = CreateWindow(
            windowClass.lpszClassName,
            name.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,        // We have no parent window.
            nullptr,        // We aren't using menus.
            hInstance,
            nullptr);

        // app = std::make_unique<dxvk::VKApplication>(width, height, name);
        // app =  std::make_unique<dxvk::DXApplication>(width, height, name);
        app =  std::make_unique<dxvk::Application>(width, height, name);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        app->onInit();

        ShowWindow(hwnd, nCmdShow);

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        app->onDestroy();

        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }

    // Main message handler for the sample.
    LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        auto& pSample = getApp();

        switch (message)
        {
        case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
            return 0;

        case WM_KEYDOWN:
            if (pSample)
            {
                pSample->onKeyDown(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_KEYUP:
            if (pSample)
            {
                pSample->onKeyUp(static_cast<UINT8>(wParam));
            }
            return 0;

        case WM_PAINT:
            if (pSample)
            {
                pSample->onUpdate();
                pSample->onRender();
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        // Handle any messages the switch statement didn't.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}