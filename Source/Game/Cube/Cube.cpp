#include "Cube/Cube.h"


#include "Cube/Cube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Cube

  Summary:  Constructor

  Args:     const std::filesystem::path& textureFilePath
              Path to the texture to use
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
Cube::Cube(_In_ const std::filesystem::path& textureFilePath)
    : BaseCube(textureFilePath)
{
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Cube

  Summary:  Constructor

  Args:     const XMFLOAT4& outputColor
              Default color of the cube
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
Cube::Cube(_In_ const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{
}

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Update

  Summary:  Updates the cube every frame

  Args:     FLOAT deltaTime
              Elapsed time

  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void Cube::Update(_In_ FLOAT deltaTime)
{
    // Does nothing
}


//Cube1::Cube1(const std::filesystem::path& textureFilePath)
//	: BaseCube(textureFilePath)
//{ }
//
//void Cube1::Update(_In_ FLOAT deltaTime)
//{
//	m_world *= XMMatrixRotationY(deltaTime);
//}

// YourCube(0, 0, 0, 0, 1, 0, 1, 1, 1, {1, 1, 1} ) Cube1
// YourCube(0, 0, 0, 0, -0.2f, -1, 0.3f, 0.3f, 0.3f, {1, 1, 1} ) Cube2