#include "Cube/Cube2.h"

void Cube2::Update(_In_ FLOAT deltaTime)
{
    cubeTime += deltaTime;
    XMMATRIX mSpin = XMMatrixRotationZ(-cubeTime);
    XMMATRIX mOrbit = XMMatrixRotationY(-cubeTime * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
    m_world = mScale * mSpin * mTranslate * mOrbit;
}