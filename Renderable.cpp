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
	std::shared_ptr<Camera> camera,
	float totalTime
)
{
	// Do Simple Shader's stuff here
	std::shared_ptr<SimpleVertexShader> vs = material->GetVS();
	std::shared_ptr<SimplePixelShader> ps = material->GetPS();

	// Setting all the values in the vertex shader to updated current values, BEFORE telling the material to prepare
	vs->SetMatrix4x4("world", trf.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	
	// Setting all the values in the pixel shader too
	ps->SetFloat4("colorTint", material->GetColorTint()); // Every pixel shader has a tint
	// Extra values!! if a variable doesn't exist in our material's pixel shader, SimpleShader simply skips it
	ps->SetFloat("totalTime", totalTime); // Only some pixel shaders have time

	// Now send it over, then tell it go!
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	// Prep the material so its shaders are ready
	material->PrepareMaterial();

	// Drawing the meshes!
	mesh->Draw();
}
