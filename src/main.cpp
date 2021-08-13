#pragma once

// Console window is displayed in debug mode.
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include "AdhocSimulation.h" // This includes Win32App.h


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    int width = 800;
    int height = 800;

    Win32App winApp(width, height);
    winApp.initialize(hInstance);

    AdhocSimulation* adhocsim = new AdhocSimulation();
    adhocsim->iSetObjectCountXYZ(20, 20, 1);
    adhocsim->iSetObjectScale(0.05f);

    DX12App* dxapp = new DX12App();
    dxapp->setSimulation(adhocsim, 0.05);

    winApp.initDirectX(dxapp);

    return winApp.run();
}