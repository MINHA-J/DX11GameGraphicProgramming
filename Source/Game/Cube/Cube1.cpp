#include "Cube/Cube1.h"

Cube1::Cube1(const std::filesystem::path& textureFilePath)
	: BaseCube(textureFilePath)
{ }

void Cube1::Update(_In_ FLOAT deltaTime)
{
	m_world *= XMMatrixRotationY(deltaTime);
}

// YourCube(0, 0, 0, 0, 1, 0, 1, 1, 1, {1, 1, 1} ) Cube1
// YourCube(0, 0, 0, 0, -0.2f, -1, 0.3f, 0.3f, 0.3f, {1, 1, 1} ) Cube2