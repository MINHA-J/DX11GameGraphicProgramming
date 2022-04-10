#include "Cube/minCube.h"

void minCube::Update(_In_ FLOAT deltaTime)
{
    cubeTime += deltaTime;
    XMMATRIX mSpin = XMMatrixRotationZ(-cubeTime);
    XMMATRIX mOrbit = XMMatrixRotationY(-cubeTime * 1.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.1f, 0.1f, 0.1f);
    m_world = mScale * mSpin * mTranslate * mOrbit;
}