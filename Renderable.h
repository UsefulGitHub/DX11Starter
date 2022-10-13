#pragma once
// Don't include this if it's already been included somewhere else!

#include "Transform.h"
#include "Mesh.h"
#include <memory>
#include <wrl/client.h>
#include "Camera.h"
#include "Material.h"

class Renderable
{
public:
	Renderable();
	Renderable(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	// There isn't really much for a destructor to do here.
	// In general, a class shouldn't delete an object it didn't create.
	// And in our case especially,
	// where multiple renderable entities will use the same mesh,
	// definitely don't delete the mesh.

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	Transform* GetTransform();

	// Draw
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera
	);

private:
	Transform trf;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

