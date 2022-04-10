/*+===================================================================
  File:      MAIN.CPP

  Summary:   This application demonstrates creating a Direct3D 11 device

  Origin:    http://msdn.microsoft.com/en-us/library/windows/apps/ff729718.aspx

  Originally created by Microsoft Corporation under MIT License
  � 2022 Kyung Hee University
===================================================================+*/

#include "Common.h"

#include <memory>


#include "Game/Game.h"
#include "Cube/Cube1.h"
#include "Cube/Cube2.h"
#include "Cube/minCube.h"

/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Function: wWinMain

  Summary:  Entry point to the program. Initializes everything and
            goes into a message processing loop. Idle time is used to
            render the scene.

  Args:     HINSTANCE hInstance
              Handle to an instance.
            HINSTANCE hPrevInstance
              Has no meaning.
            LPWSTR lpCmdLine
              Contains the command-line arguments as a Unicode
              string
            INT nCmdShow
              Flag that says whether the main application window
              will be minimized, maximized, or shown normally

  Returns:  INT
              Status code.
-----------------------------------------------------------------F-F*/
INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"Game Graphics Programming Lab 04: 3D Spaces and Transformations");

    std::shared_ptr<library::VertexShader> vertexShader = std::make_shared<library::VertexShader>(L"Shaders/Shaders.fxh", "VS", "vs_5_0");
    if (FAILED(game->GetRenderer()->AddVertexShader(L"MainShader", vertexShader)))
    {
        return 0;
    }

    std::shared_ptr<library::PixelShader> pixelShader = std::make_shared<library::PixelShader>(L"Shaders/Shaders.fxh", "PS", "ps_5_0");
    if (FAILED(game->GetRenderer()->AddPixelShader(L"MainShader", pixelShader)))
    {
        return 0;
    }

    // 첫번째 Cube
    std::shared_ptr<Cube1> BigCube = std::make_shared<Cube1>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"Cube1", BigCube)))
    {
        return 0;
    }

    HRESULT hr = S_OK;
    hr = game->GetRenderer()->SetVertexShaderOfRenderable(L"Cube1", L"MainShader");
    if (FAILED(hr))
        return hr;
    hr = game->GetRenderer()->SetPixelShaderOfRenderable(L"Cube1", L"MainShader");
    if (FAILED(hr))
        return hr;

    // 두번째 Cube
    std::shared_ptr<Cube2> SmallCube = std::make_shared<Cube2>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"Cube2", SmallCube)))
    {
        return 0;
    }
    hr = game->GetRenderer()->SetVertexShaderOfRenderable(L"Cube2", L"MainShader");
    if (FAILED(hr))
        return hr;
    hr = game->GetRenderer()->SetPixelShaderOfRenderable(L"Cube2", L"MainShader");
    if (FAILED(hr))
        return hr;

    // my Cube
    std::shared_ptr<minCube> my = std::make_shared<minCube>();
    if (FAILED(game->GetRenderer()->AddRenderable(L"Cube3", my)))
    {
        return 0;
    }
    hr = game->GetRenderer()->SetVertexShaderOfRenderable(L"Cube3", L"MainShader");
    if (FAILED(hr))
        return hr;
    hr = game->GetRenderer()->SetPixelShaderOfRenderable(L"Cube3", L"MainShader");
    if (FAILED(hr))
        return hr;


    if (FAILED(game->Initialize(hInstance, nCmdShow)))
    {
        return 0;
    }

    return game->Run();
}