#include "Cube/RotatingCube.h"

RotatingCube::RotatingCube(const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{
}


//HRESULT RotatingCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
//{
//    BasicMeshEntry basicMeshEntry;
//    basicMeshEntry.uNumIndices = NUM_INDICES;
//
//    m_aMeshes.push_back(basicMeshEntry);
//
//    if (HasTexture())
//    {
//        SetMaterialOfMesh(0, 0);
//    }
//
//    return initialize(pDevice, pImmediateContext);
//}


void RotatingCube::Update(_In_ FLOAT deltaTime)
{
    // Rotate cube around the origin
    static FLOAT t = 0.0f;
    t += deltaTime;

    XMMATRIX mSpin = XMMatrixRotationZ(-t);
    XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);
    // XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 30.0f, -50.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 30.0f, -5.0f);
    XMMATRIX mScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);

    m_world = mScale * mSpin * mTranslate * mOrbit;
}


//#include "Cube/Cube2.h"
//
//Cube2::Cube2(const std::filesystem::path& textureFilePath)
//    : BaseCube(textureFilePath),
//    cubeTime(0.0f)
//{ }
//
//void Cube2::Update(_In_ FLOAT deltaTime)
//{
//    cubeTime += deltaTime;
//    XMMATRIX mSpin = XMMatrixRotationZ(cubeTime);
//    XMMATRIX mOrbit = XMMatrixRotationY(-cubeTime);
//    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
//    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
//    m_world = mScale * mOrbit * mTranslate * mOrbit;
//}