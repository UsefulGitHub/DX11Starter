#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <string>
#include <fstream>
#include <vector>

class Mesh
{
public:
	Mesh(
		Vertex* vertices,
		int numVertices,
		unsigned int* indices,
		int numIndices,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);

	Mesh(
		const std::wstring& objFile,
		Microsoft::WRL::ComPtr<ID3D11Device> device
	);

	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	int indexCount;
};

