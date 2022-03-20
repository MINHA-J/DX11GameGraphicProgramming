#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/
    HINSTANCE               g_hInst = NULL;
    HWND                    g_hWnd = NULL;

    Microsoft::WRL::ComPtr<ID3D11Device> m_pd3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_pDXGISwapChain;

    Microsoft::WRL::ComPtr < ID3D11Texture2D>        m_pBackBuffer;
    Microsoft::WRL::ComPtr < ID3D11RenderTargetView> m_pRenderTarget;

    D3D11_TEXTURE2D_DESC    m_bbDesc;
    D3D11_VIEWPORT          m_viewport;

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
        wcex.hIcon = LoadIcon(hInstance, NULL);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"WindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);

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

        device.As(&m_pd3dDevice);
        context.As(&m_pd3dDeviceContext);

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
        m_pd3dDevice.As(&dxgiDevice);

        // Create swap chain.
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;

        hr = dxgiDevice->GetAdapter(&adapter);

        if (SUCCEEDED(hr))
        {
            adapter->GetParent(IID_PPV_ARGS(&factory));

            hr = factory->CreateSwapChain(
                m_pd3dDevice.Get(),
                &desc,
                &m_pDXGISwapChain
            );
        }
        
        // Create Render target view
        hr = m_pDXGISwapChain->GetBuffer(
            0,
            __uuidof(ID3D11Texture2D),
            (void**)&m_pBackBuffer);

        hr = m_pd3dDevice->CreateRenderTargetView(
            m_pBackBuffer.Get(),
            nullptr,
            m_pRenderTarget.GetAddressOf()
        );

        if (FAILED(hr))
        {
            DWORD dwError = GetLastError();
            return HRESULT_FROM_WIN32(dwError);
        }

        m_pBackBuffer->GetDesc(&m_bbDesc);

        // Create Viewport
        ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
        m_viewport.Height = (float)m_bbDesc.Height;
        m_viewport.Width = (float)m_bbDesc.Width;
        m_viewport.MinDepth = 0;
        m_viewport.MaxDepth = 1;

        m_pd3dDeviceContext->RSSetViewports(
            1,
            &m_viewport
        );

    }

    void CleanupDevice()
    {
        if (m_pd3dDeviceContext) m_pd3dDeviceContext->ClearState();

        if (m_pRenderTarget != nullptr)m_pRenderTarget.ReleaseAndGetAddressOf();
        if (m_pDXGISwapChain != nullptr)m_pDXGISwapChain.ReleaseAndGetAddressOf();
        if (m_pd3dDeviceContext != nullptr)m_pd3dDeviceContext.ReleaseAndGetAddressOf();
        if (m_pd3dDevice != nullptr)m_pd3dDevice.ReleaseAndGetAddressOf();
    }

    void Render()
    {
        //
        // Clear the backbuffer
        //
        ID3D11RenderTargetView* renderTarget;
        renderTarget = m_pRenderTarget.Get();

        float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
        m_pd3dDeviceContext->ClearRenderTargetView(renderTarget, ClearColor);

        m_pDXGISwapChain->Present(0, 0);
        
    }
}