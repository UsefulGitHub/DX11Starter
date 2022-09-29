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
	void CreateGeometry();

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
	
	// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// SimpleShader constructs (the above shaders can be commented out now, these are just functionalized versions of the above ones)
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimplePixelShader> fps;

	// Other D3D resources for shaders
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

	std::shared_ptr<Mesh> triangle;
	std::shared_ptr<Mesh> square;
	std::shared_ptr<Mesh> nonagon;

	std::shared_ptr<Camera> camera;

	std::shared_ptr<Renderable> ent1;
	std::shared_ptr<Renderable> ent2;
	std::shared_ptr<Renderable> ent3;
	std::shared_ptr<Renderable> ent4;
	std::shared_ptr<Renderable> ent5;

	std::shared_ptr<Material> mat1;
	std::shared_ptr<Material> mat2;
};

