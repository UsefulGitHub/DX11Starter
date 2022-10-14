#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <vector>
#include "Mesh.h"
#include "Transform.h"
#include "Renderable.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
// Not including the ImGui headers here because they are in DXCore.h,
// which this includes and inherits from <3.

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateMaterials();
	void CreateGeometry();
	void CreateRenderables();
	void SetupTransforms();

	// ImGui helper methods
	ImGuiIO PrepImGui(float deltaTime);
	void UpdateImGui(ImGuiIO frameIO);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// SimpleShader constructs
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimplePixelShader> fps;

	// Camera (The)
	std::shared_ptr<Camera> camera;

	// Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Materials
	std::vector<std::shared_ptr<Material>> materials;
	std::shared_ptr<Material> mat1;
	std::shared_ptr<Material> mat2;
	std::shared_ptr<Material> mat3;

	// Renderables
	std::vector<std::shared_ptr<Renderable>> renderables;

	// Transforms
	// - This is uniquely a vector of plain pointers, because renderables return pointers to their transforms
	std::vector<Transform*> transforms;
};

