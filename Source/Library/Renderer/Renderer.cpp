#include "Renderer/Renderer.h"

namespace library
{
    

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_vertexShader,
                  m_pixelShader, m_vertexLayout, m_vertexBuffer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_vertexShader(nullptr)
        , m_pixelShader(nullptr)
        , m_vertexLayout(nullptr)
        , m_vertexBuffer(nullptr)
    { }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::Initialize

     Summary:  Creates Direct3D device and swap chain

     Args:     HWND hWnd
                 Handle to the window

     Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_vertexShader,
                 m_vertexLayout, m_pixelShader, m_vertexBuffer].

     Returns:  HRESULT
                 Status code
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);

        UINT createDeviceFlags = 0;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };

        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2>           dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);


        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_immediateContext->RSSetViewports(1, &vp);

        // Compile the vertex shader
        ComPtr<ID3DBlob> pVSBlob = nullptr;
        hr = compileShaderFromFile(L"../Library/Shaders/Lab03.fxh", "vs_main", "vs_5_0", pVSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(nullptr,
                L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
            return hr;
        }

        // Create the vertex shader
        hr = m_d3dDevice->CreateVertexShader(
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                0,
                D3D11_INPUT_PER_VERTEX_DATA,
                0 },
        };
        UINT uNumElements = ARRAYSIZE(layout);

        // Create the input layout
        hr = m_d3dDevice->CreateInputLayout(
            layout,
            uNumElements,
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            &m_vertexLayout);
        if (FAILED(hr))
            return hr;

        // Set the input layout
        m_immediateContext->IASetInputLayout(m_vertexLayout.Get());

        // Compile the pixel shader
        ComPtr<ID3DBlob> pPSBlob = nullptr;
        hr = compileShaderFromFile(L"../Library/Shaders/Lab03.fxh", "ps_main", "ps_5_0", pPSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(nullptr,
                L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
            return hr;
        }

        // Create the pixel shader
        hr = m_d3dDevice->CreatePixelShader(
            pPSBlob->GetBufferPointer(),
            pPSBlob->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create vertex buffer
        SimpleVertex  aVertices[] =
        {
            XMFLOAT3(0.0f, 0.5f, 0.5f),
            XMFLOAT3(0.5f, -0.5f, 0.5f),
            XMFLOAT3(-0.5f, -0.5f, 0.5f),
        };

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(SimpleVertex) * 3;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = aVertices;
        hr = m_d3dDevice->CreateBuffer(&bufferDesc, &InitData, m_vertexBuffer.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Set vertex buffer
        UINT uStride = sizeof(SimpleVertex);
        UINT uOffset = 0;
        m_immediateContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

        /*
        // Create index buffer
        ComPtr<ID3D11Buffer> pIndexBuffer;
        WORD indices[] =
        {
            0, 1, 2,
        };

        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(WORD) * 3;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        InitData.pSysMem = indices;
        hr = m_d3dDevice->CreateBuffer(&bufferDesc, &InitData, &pIndexBuffer);
        if (FAILED(hr))
            return hr;

        // Set index buffer
        m_immediateContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
        */

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
     Method:   Renderer::Render

     Summary:  Render the frame
   M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Renders a triangle
        m_immediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0u);
        m_immediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);

        // Calling Draw tells Direct3D to start sending commands to the graphics device.
        m_immediateContext->Draw(3, 0);

        // Present our back buffer to our front buffer
        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::compileShaderFromFile

      Summary:  Helper for compiling shaders with D3DCompile

      Args:     PCWSTR pszFileName
                  A pointer to a constant null-terminated string that
                  contains the name of the file that contains the
                  shader code
                PCSTR pszEntryPoint
                  A pointer to a constant null-terminated string that
                  contains the name of the shader entry point function
                  where shader execution begins
                PCSTR pszShaderModel
                  A pointer to a constant null-terminated string that
                  specifies the shader target or set of shader
                  features to compile against
                ID3DBlob** ppBlobOut
                  A pointer to a variable that receives a pointer to
                  the ID3DBlob interface that you can use to access
                  the compiled code

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::compileShaderFromFile(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR szShaderModel, _Outptr_ ID3DBlob** ppBlobOut)
    {
        HRESULT hr = S_OK;

        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob* errorBlob = nullptr;
        hr = D3DCompileFromFile(
            pszFileName,
            nullptr,
            nullptr,
            pszEntryPoint,
            szShaderModel,
            dwShaderFlags,
            0u,
            ppBlobOut,
            &errorBlob);

        if (FAILED(hr))
        {

            if (errorBlob)
            {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return hr;
        }

        if (errorBlob) errorBlob->Release();

        return S_OK;
    }

}