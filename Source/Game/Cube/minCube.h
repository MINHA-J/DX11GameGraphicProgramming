#include "Common.h"

#include "Renderer/Renderable.h"
#include "Renderer/DataTypes.h"
#include "Cube/BaseCube.h"


class minCube : public BaseCube
{
public:
	minCube() = default;
	~minCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;

private:
	FLOAT cubeTime;
};