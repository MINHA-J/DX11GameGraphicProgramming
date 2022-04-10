#pragma once

#include "Common.h"

#include "Renderer/Renderable.h"
#include "Renderer/DataTypes.h"
#include "Cube/BaseCube.h"


class Cube1 : public BaseCube
{
public:
	Cube1() = default;
	~Cube1() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};