#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE               g_hInst = NULL;
    HWND                    g_hWnd = NULL;

    Microsoft::WRL::ComPtr<ID3D11Device> g_pd3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pd3dDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> g_pDXGISwapChain;

    Microsoft::WRL::ComPtr < ID3D11Texture2D>        g_pBackBuffer;
    Microsoft::WRL::ComPtr < ID3D11RenderTargetView> g_pRenderTarget;

    D3D11_TEXTURE2D_DESC    g_bbDesc;
    D3D11_VIEWPORT          g_viewport;

    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/

    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window?its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
        {
            HMENU hMenu;
            hMenu = GetMenu(hWnd);
            if (hMenu != NULL)
            {
                DestroyMenu(hMenu);
            }
            DestroyWindow(hWnd);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        // Registers the window class
        WNDCLASSEX wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"WindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

        if (!RegisterClassEx(&wcex))
        {
            DWORD dwError = GetLastError();
            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
                return HRESULT_FROM_WIN32(dwError);
        }

        // Creates a window
        g_hInst = hInstance;

        RECT rc = { 0, 0, 640, 480 };

        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindow(
            L"WindowClass",                                             // Window class
            L"Game Graphics Programming Lab 01: Direct3D 11 Basics",    // Window text
            WS_OVERLAPPEDWINDOW,                                        // Window style
            CW_USEDEFAULT, 

             // Size and position
            CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, 

            NULL,       // Parent window   
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
        );

        if (g_hWnd == NULL)
        {
            DWORD dwError = GetLastError();
            return HRESULT_FROM_WIN32(dwError);
        }
           
        // Shows the window
        ShowWindow(g_hWnd, nCmdShow);

        // Returns a result code of HRESULT type
        return S_OK;
    }


    HRESULT InitDevice()
    {
        // Create Direct3D device and context
        D3D_FEATURE_LEVEL lvl[] = {
            D3D_FEATURE_LEVEL_11_1, 
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1, 
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3, 
            D3D_FEATURE_LEVEL_9_2, 
            D3D_FEATURE_LEVEL_9_1 
        };

        DWORD createDeviceFlags = 0;
        #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
        D3D_FEATURE_LEVEL fl;

        HRESULT hr = D3D11CreateDevice(
            nullptr, 
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr, 
            createDeviceFlags, 
            nullptr,
            0, 
            D3D11_SDK_VERSION, 
            &device, 
            &fl, 
            &context
        );

        if (hr == E_INVALIDARG)
        {
            /*hr = D3D11CreateDevice(
                nullptr, 
                D3D_DRIVER_TYPE_HARDWARE, 
                nullptr,
                createDeviceFlags, 
                &lvl[1], 
                _countof(lvl) - 1,
                D3D11_SDK_VERSION, 
                &device, 
                &fl, 
                &context
            );*/
            DWORD dwError = GetLastError();
            return HRESULT_FROM_WIN32(dwError);
        }

        device.As(&g_pd3dDevice);
        context.As(&g_pd3dDeviceContext);

        /*if (device)device.Reset();
        if (context)context.Reset();*/


        // Create Swap chain
        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
        desc.Windowed = TRUE; // Sets the initial state of full-screen mode.
        desc.BufferCount = 2;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;      //multisampling setting
        desc.SampleDesc.Quality = 0;    //vendor-specific flag
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.OutputWindow = g_hWnd;

        // Create the DXGI device object to use in other factories, such as Direct2D.
        Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
        g_pd3dDevice.As(&dxgiDevice);

        // Create swap chain.
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;

        hr = dxgiDevice->GetAdapter(&adapter);

        if (SUCCEEDED(hr))
        {
            adapter->GetParent(IID_PPV_ARGS(&factory));

            hr = factory->CreateSwapChain(
                g_pd3dDevice.Get(),
                &desc,
                &g_pDXGISwapChain
            );
        }

        /*if (dxgiDevice) dxgiDevice.Reset();
        if (adapter) adapter.Reset();
        if (factory) factory.Reset();*/
        

        // Create Render target view
        hr = g_pDXGISwapChain->GetBuffer(
            0,
            __uuidof(ID3D11Texture2D),
            (void**)&g_pBackBuffer);

        hr = g_pd3dDevice->CreateRenderTargetView(
            g_pBackBuffer.Get(),
            nullptr,
            g_pRenderTarget.GetAddressOf()
        );

        if (FAILED(hr))
        {
            DWORD dwError = GetLastError();
            return HRESULT_FROM_WIN32(dwError);
        }

        g_pBackBuffer->GetDesc(&g_bbDesc);

        // Create Viewport
        ZeroMemory(&g_viewport, sizeof(D3D11_VIEWPORT));
        g_viewport.Height = (float)g_bbDesc.Height;
        g_viewport.Width = (float)g_bbDesc.Width;
        g_viewport.MinDepth = 0;
        g_viewport.MaxDepth = 1;

        g_pd3dDeviceContext->RSSetViewports(
            1,
            &g_viewport
        );

    }

    void CleanupDevice()
    {
        if (g_pd3dDeviceContext) g_pd3dDeviceContext->ClearState();

        if (g_pRenderTarget)g_pRenderTarget.Reset();
        if (g_pBackBuffer) g_pBackBuffer.Reset();
        if (g_pDXGISwapChain)g_pDXGISwapChain.Reset();

        if (g_pd3dDeviceContext)g_pd3dDeviceContext.Reset();
        if (g_pd3dDevice)g_pd3dDevice.Reset();
    }

    void Render()
    {
        // Clear the backbuffer
        float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
        g_pd3dDeviceContext->ClearRenderTargetView(g_pRenderTarget.Get(), ClearColor);

        g_pDXGISwapChain->Present(0, 0);
    }
}