#pragma once

#include "Common.h"

#include "Renderer/Renderable.h"
#include "Renderer/DataTypes.h"
#include "Cube/BaseCube.h"


class Cube2 : public BaseCube
{
public:
	Cube2(const std::filesystem::path& textureFilePath);
	~Cube2() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;

private:
	FLOAT cubeTime = 0;
};