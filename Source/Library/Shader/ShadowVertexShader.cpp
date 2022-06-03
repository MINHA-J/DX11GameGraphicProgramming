#include "Shader/ShadowVertexShader.h"

namespace library
{
    ShadowVertexShader::ShadowVertexShader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel)
        : VertexShader(pszFileName, pszEntryPoint, pszShaderModel)
    {
    }

    HRESULT ShadowVertexShader::Initialize(_In_ ID3D11Device* pDevice)
    {
        ComPtr<ID3DBlob> vsBlob;
        HRESULT hr = compile(vsBlob.GetAddressOf());
        if (FAILED(hr))
        {
            WCHAR szMessage[256];
            swprintf_s(
                szMessage,
                L"The FX file %s cannot be compiled. Please run this executable from the directory that contains the FX file.",
                m_pszFileName
            );
            MessageBox(
                nullptr,
                szMessage,
                L"Error",
                MB_OK
            );
            return hr;
        }

        hr = pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC aLayouts[] =
        {
            //{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            //{ "INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            //{ "INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            //{ "INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            //{ "INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0u,
                .AlignedByteOffset = 0u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0u,
                .AlignedByteOffset = 12u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "NORMAL",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0u,
                .AlignedByteOffset = 20u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "TANGENT",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 1u,
                .AlignedByteOffset = 0u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "BITANGENT",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 1u,
                .AlignedByteOffset = 12u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "INSTANCE_TRANSFORM",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 2u,
                .AlignedByteOffset = 0u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1u
            },
            {
                .SemanticName = "INSTANCE_TRANSFORM",
                .SemanticIndex = 1u,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 2u,
                .AlignedByteOffset = 16u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1u
            },
            {
                .SemanticName = "INSTANCE_TRANSFORM",
                .SemanticIndex = 2u,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 2u,
                .AlignedByteOffset = 32u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1u
            },
            {
                .SemanticName = "INSTANCE_TRANSFORM",
                .SemanticIndex = 3u,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 2u,
                .AlignedByteOffset = 48u,
                .InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
                .InstanceDataStepRate = 1u
            }
        };
        UINT uNumElements = ARRAYSIZE(aLayouts);

        // Create the input layout
        hr = pDevice->CreateInputLayout(aLayouts, uNumElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_vertexLayout.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        return hr;
    }
}