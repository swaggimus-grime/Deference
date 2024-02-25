#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd)
{
    try {
        Def::App app("Deference", 1280, 720);
        return app.Run();
    }
    catch (const std::exception& e) {
        MessageBox(nullptr, e.what(), "UH OH!", MB_OK | MB_ICONEXCLAMATION);
    }
}