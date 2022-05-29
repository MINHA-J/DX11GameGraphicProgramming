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
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
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
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene
      Summary:  Add scene to renderer
      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene
      Modifies: [m_scenes].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull
      Summary:  Return scene with the given name or null
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene
      Summary:  Set the main scene
      Args:     PCWSTR pszSceneName
                  The name of the scene
      Modifies: [m_pszMainSceneName].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput
      Summary:  Add the pixel shader into the renderer and initialize it
      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data
      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime)
    {
        // The only object to handle input is the camera object
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }



    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update
      Summary:  Update the renderables each frame
      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render
      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);


        // Clear the depth buffer to 1.0 (maximum depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


        // Create camera constant buffer and update
        m_camera.Initialize(m_d3dDevice.Get());

        CBChangeOnCameraMovement cbCamera
        {
            .View = XMMatrixTranspose(m_camera.GetView()),
            .CameraPosition =
            {
                XMVectorGetX(m_camera.GetEye()),
                XMVectorGetY(m_camera.GetEye()),
                XMVectorGetZ(m_camera.GetEye()),
                XMVectorGetW(m_camera.GetEye())
            }
        };
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cbCamera, 0u, 0u);

        auto scene = m_scenes.find(m_pszMainSceneName);

        // update lights constant buffer
        CBLights cbLight = {};
        for (int i = 0u; i < NUM_LIGHTS; ++i)
        {
            cbLight.LightPositions[i] = (scene->second)->GetPointLight(i)->GetPosition();
            cbLight.LightColors[i] = (scene->second)->GetPointLight(i)->GetColor();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cbLight, 0u, 0u);

        // Update variables that change once per frame
        for (auto renderable : (scene->second)->GetRenderables())
        {
            UINT strides[2] = { sizeof(SimpleVertex), sizeof(NormalData) };
            UINT offsets[2] = { 0u, 0u };
            ID3D11Buffer* vertInstBuffers[2] = 
            { 
                renderable.second->GetVertexBuffer().Get(), 
                renderable.second->GetNormalBuffer().Get() 
            };

            // Set the vertex buffer, index buffer, and the input layout
            m_immediateContext->IASetVertexBuffers(0, 2, renderable.second->GetVertexBuffer().GetAddressOf(), strides, offsets);
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            m_immediateContext->IASetInputLayout(renderable.second->GetVertexLayout().Get());

            // Create renderable constant buffer and update
            CBChangesEveryFrame cbFrame =
            {
                .World = XMMatrixTranspose(renderable.second->GetWorldMatrix()),
                .OutputColor = renderable.second->GetOutputColor(),
                .HasNormalMap = renderable.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(renderable.second->GetConstantBuffer().Get(), 0u, nullptr, &cbFrame, 0u, 0u);

            // Set Shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(renderable.second->GetVertexShader().Get(), nullptr, 0u);

            // Set the constant buffer
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
             // m_immediateContext->VSSetConstantBuffers(4u, 1u, renderable.second->GetNormalBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(renderable.second->GetPixelShader().Get(), nullptr, 0u);

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
             // m_immediateContext->PSSetConstantBuffers(4u, 1u, renderable.second->GetNormalBuffer().GetAddressOf());

            if (renderable.second->HasTexture())
            {
                // For each meshes
                for (UINT i = 0u; i < renderable.second->GetNumMeshes(); ++i)
                {
                    UINT index = renderable.second->GetMesh(i).uMaterialIndex;

                    // Get the materials and set them as the shader resources and samplers
                    ID3D11ShaderResourceView* ResourceView[2];
                    ResourceView[0] = renderable.second->GetMaterial(index)->pDiffuse->GetTextureResourceView().Get();
                    ResourceView[1] = renderable.second->GetMaterial(index)->pNormal->GetTextureResourceView().Get();

                    ID3D11SamplerState* samplerStates[2];
                    samplerStates[0] = renderable.second->GetMaterial(index)->pDiffuse->GetSamplerState().Get();
                    samplerStates[1] = renderable.second->GetMaterial(index)->pNormal->GetSamplerState().Get();

                    //m_immediateContext->PSSetShaderResources(1u, 1u, renderable.second->GetMaterial(index)->pSpecularExponent->GetTextureResourceView().GetAddressOf());
                    //m_immediateContext->PSSetSamplers(1u, 1u, renderable.second->GetMaterial(index)->pSpecularExponent->GetSamplerState().GetAddressOf());

                    //m_immediateContext->PSSetShaderResources(1u, 1u, renderable.second->GetMaterial(index)->pNormal->GetTextureResourceView().GetAddressOf());
                    //m_immediateContext->PSSetSamplers(1u, 1u, renderable.second->GetMaterial(index)->pNormal->GetSamplerState().GetAddressOf());
                    m_immediateContext->PSSetShaderResources(0u, 2u, ResourceView);
                    m_immediateContext->PSSetSamplers(0u, 2u, samplerStates);

                    // Draw them by their respective indices, base index, and base vertex
                    m_immediateContext->DrawIndexed(
                        renderable.second->GetMesh(i).uNumIndices,
                        renderable.second->GetMesh(i).uBaseIndex,
                        renderable.second->GetMesh(i).uBaseVertex
                    );
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(
                    renderable.second->GetNumIndices(),
                    0,
                    0
                );
            }
        }

        // After rendering the renderables, render the voxels of the main scene
        for (auto voxel : (scene->second)->GetVoxels())
        {
            // Set the vertex buffer, index buffer, instancing buffer and the input layout
            UINT strides[3] = { sizeof(SimpleVertex), sizeof(NormalData), sizeof(InstanceData) };
            UINT offsets[3] = { 0u, 0u, 0u };
            ID3D11Buffer* vertInstBuffers[3] = 
            { 
                voxel->GetVertexBuffer().Get(), 
                voxel->GetNormalBuffer().Get(), 
                voxel->GetInstanceBuffer().Get() 
            };

            m_immediateContext->IASetVertexBuffers(0, 3, vertInstBuffers, strides, offsets);
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());

            // Create renderable constant buffer and update
            CBChangesEveryFrame cbFrame =
            {
                .World = XMMatrixTranspose(voxel->GetWorldMatrix()),
                .OutputColor = voxel->GetOutputColor(),
                .HasNormalMap = voxel->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cbFrame, 0u, 0u);

            // Set Shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);

            // Set the constant buffer
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            if (voxel->HasTexture())
            {
                for (UINT i = 0u; i < voxel->GetNumMeshes(); ++i)
                {
                    UINT index = voxel->GetMesh(i).uMaterialIndex;

                    // Get the materials and set them as the shader resources and samplers
                    ID3D11ShaderResourceView* ResourceView[2];
                    ResourceView[0] = voxel->GetMaterial(index)->pDiffuse->GetTextureResourceView().Get();
                    ResourceView[1] = voxel->GetMaterial(index)->pNormal->GetTextureResourceView().Get();

                    ID3D11SamplerState* samplerStates[2];
                    samplerStates[0] = voxel->GetMaterial(index)->pDiffuse->GetSamplerState().Get();
                    samplerStates[1] = voxel->GetMaterial(index)->pNormal->GetSamplerState().Get();

                    m_immediateContext->PSSetShaderResources(0u, 2u, ResourceView);
                    m_immediateContext->PSSetSamplers(0u, 2u, samplerStates);

                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(i).uNumIndices,
                        voxel->GetNumInstances(),
                        voxel->GetMesh(i).uBaseIndex,
                        voxel->GetMesh(i).uBaseVertex,
                        0);
                }
            }
            else
            {
                m_immediateContext->DrawIndexedInstanced(
                    voxel->GetNumIndices(),
                    voxel->GetNumInstances(),
                    0,
                    0,
                    0);
            }

        }

        // render the model
        for (auto model : (scene->second)->GetModels())
        {
            // Set vertex buffer
            UINT aStrides[3] = { static_cast<UINT>(sizeof(SimpleVertex)), static_cast <UINT>(sizeof(NormalData)), static_cast<UINT>(sizeof(AnimationData)) };
            UINT aOffsets[3] = { 0u, 0u, 0u };

            ID3D11Buffer* aBuffers[3] =
            {
             model.second->GetVertexBuffer().Get(),
             model.second->GetNormalBuffer().Get(),
             model.second->GetAnimationBuffer().Get()
            };

            // Set the vertex buffer, index buffer, and the input layout
            m_immediateContext->IASetVertexBuffers(0, 3, aBuffers, aStrides, aOffsets);
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            m_immediateContext->IASetInputLayout(model.second->GetVertexLayout().Get());

            // Update and bind the constant buffer
            CBChangesEveryFrame cbFrame =
            {
                .World = XMMatrixTranspose(model.second->GetWorldMatrix()),
                .OutputColor = model.second->GetOutputColor(),
                .HasNormalMap = model.second->HasNormalMap()
            };
            m_immediateContext->UpdateSubresource(model.second->GetConstantBuffer().Get(), 0u, nullptr, &cbFrame, 0u, 0u);

            CBSkinning cbSkin = {}; // Set the bone transformations in skinning constant buffer using
            for (UINT i = 0; i < model.second->GetBoneTransforms().size(); ++i)
            {
                cbSkin.BoneTransforms[i] = XMMatrixTranspose(model.second->GetBoneTransforms()[i]);
            }
            m_immediateContext->UpdateSubresource(model.second->GetSkinningConstantBuffer().Get(), 0u, nullptr, &cbSkin, 0u, 0u);

            // Set Shaders and constant buffers, shader resources, and samplers
            m_immediateContext->VSSetShader(model.second->GetVertexShader().Get(), nullptr, 0u);

            // Set the constant buffer
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(4u, 1u, model.second->GetSkinningConstantBuffer().GetAddressOf());
            
            m_immediateContext->PSSetShader(model.second->GetPixelShader().Get(), nullptr, 0u);

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            // m_immediateContext->PSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf()); 
            m_immediateContext->PSSetConstantBuffers(4u, 1u, model.second->GetSkinningConstantBuffer().GetAddressOf());

            if (model.second->HasTexture())
            {
                // For each meshes
                for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
                {
                    UINT index = model.second->GetMesh(i).uMaterialIndex;

                    // Get the materials and set them as the shader resources and samplers
                    ID3D11ShaderResourceView* ResourceView[2];
                    ResourceView[0] = model.second->GetMaterial(index)->pDiffuse->GetTextureResourceView().Get();
                    ResourceView[1] = model.second->GetMaterial(index)->pNormal->GetTextureResourceView().Get();

                    ID3D11SamplerState* samplerStates[2];
                    samplerStates[0] = model.second->GetMaterial(index)->pDiffuse->GetSamplerState().Get();
                    samplerStates[1] = model.second->GetMaterial(index)->pNormal->GetSamplerState().Get();

                    m_immediateContext->PSSetShaderResources(0u, 2u, ResourceView);
                    m_immediateContext->PSSetSamplers(0u, 2u, samplerStates);

                    // Draw them by their respective indices, base index, and base vertex
                    m_immediateContext->DrawIndexed(
                        model.second->GetMesh(i).uNumIndices,
                        model.second->GetMesh(i).uBaseIndex,
                        model.second->GetMesh(i).uBaseVertex
                    );
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(
                    model.second->GetNumIndices(),
                    0,
                    0
                );
            }
        }

        // Present our back buffer to our front buffer
        m_swapChain->Present(0, 0);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }

}