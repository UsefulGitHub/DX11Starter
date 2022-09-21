#include "Renderable.h"

Renderable::Renderable()
{
	trf = Transform();
}

Renderable::Renderable(std::shared_ptr<Mesh> meshToUse)
{
	trf = Transform();
	mesh = meshToUse;
}

std::shared_ptr<Mesh> Renderable::GetMesh()
{
    return std::shared_ptr<Mesh>();
}

Transform* Renderable::GetTransform()
{
    return &trf;
}

void Renderable::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer
)
{
	// Initializing the constant buffer for the vertex shader
	VertexShaderExternalData vsData;
	vsData.colorTint = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	vsData.world = trf.GetWorldMatrix();

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	// Map shows you where the ID3D11 resource is, at least for the moment (things move on/in VRAM)
	// - While a resource is mapped, if anything in the pipeline needs to access the resource, the pipeline stops and waits (bad)
	// - Context->Map fills up mappedBuffer with a memory address
	// - mappedBuffer.pData is a pointer to the position on VRAM of the buffer we asked context to map
	// - So we don't really send anything to mappedBuffer, we copy from vsData to vsConstantBuffer through our mediator
	// - The fastest way to send data around in C++ is memcpy
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(vsConstantBuffer.Get(), 0);

	// Binding the constant buffer
	context->VSSetConstantBuffers(
		0,
		1,
		vsConstantBuffer.GetAddressOf()
	);

	// Drawing the meshes!
	mesh->Draw();
}
