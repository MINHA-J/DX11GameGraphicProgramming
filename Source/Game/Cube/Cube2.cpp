#include "Cube/Cube2.h"

Cube2::Cube2(const std::filesystem::path& textureFilePath)
    : BaseCube(textureFilePath),
    cubeTime(0.0f)
{ }

void Cube2::Update(_In_ FLOAT deltaTime)
{
    cubeTime += deltaTime;
    XMMATRIX mSpin = XMMatrixRotationZ(cubeTime);
    XMMATRIX mOrbit = XMMatrixRotationY(-cubeTime);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
    m_world = mScale * mOrbit * mTranslate * mOrbit;
}