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

	vs->SetFloat4("colorTint", DirectX::XMFLOAT4(1, 1, 1, 1));

	vs->SetMatrix4x4("world", trf.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->CopyAllBufferData();

#pragma region oldInitialization

	//// Initializing the constant buffer for the vertex shader
	//VertexShaderExternalData vsData;
	//vsData.colorTint = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	//vsData.world = trf.GetWorldMatrix();
	//vsData.view = camera->GetView();
	//vsData.projection = camera->GetProjection();

	//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	//// Map shows you where the ID3D11 resource is, at least for the moment (things move on/in VRAM)
	//// - While a resource is mapped, if anything in the pipeline needs to access the resource, the pipeline stops and waits (bad)
	//// - Context->Map fills up mappedBuffer with a memory address
	//// - mappedBuffer.pData is a pointer to the position on VRAM of the buffer we asked context to map
	//// - So we don't really send anything to mappedBuffer, we copy from vsData to vsConstantBuffer through our mediator
	//// - The fastest way to send data around in C++ is memcpy
	//context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	//memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	//context->Unmap(vsConstantBuffer.Get(), 0);

	//// Binding the constant buffer
	//context->VSSetConstantBuffers(
	//	0,
	//	1,
	//	vsConstantBuffer.GetAddressOf()
	//);

#pragma endregion

	// Prep the material so its shaders are ready
	material->PrepareMaterial();

	// Drawing the meshes!
	mesh->Draw();
}
