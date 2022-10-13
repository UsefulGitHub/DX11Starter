#include "Renderable.h"

Renderable::Renderable()
{
	trf = Transform();
}

Renderable::Renderable(std::shared_ptr<Mesh> meshToUse, std::shared_ptr<Material> material)
	:
	mesh(meshToUse),
	material(material)
{
	trf = Transform();
}

std::shared_ptr<Mesh> Renderable::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Renderable::GetMaterial()
{
	return material;
}

Transform* Renderable::GetTransform()
{
    return &trf;
}

void Renderable::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera
)
{
	// Do Simple Shader's stuff here
	std::shared_ptr<SimpleVertexShader> vs = material->GetVS();

	// Setting all the values in the vertex shader to updated current values, BEFORE telling the material to prepare
	vs->SetFloat4("colorTint", material->GetColorTint());
	vs->SetMatrix4x4("world", trf.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	// Now send it over, then tell it go!
	vs->CopyAllBufferData();

	// Prep the material so its shaders are ready
	material->PrepareMaterial();

	// Drawing the meshes!
	mesh->Draw();
}
