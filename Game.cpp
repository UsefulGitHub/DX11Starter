#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "Mesh.h"
#include "Transform.h"

// Did you know you can press ctrl twice in VS2022 to reveal inline hints? They are pretty useful.

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	// Initialize all the member variables to appease C++
	ambientLight = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	firstLight = {};
	dir2 = {};
	dir3 = {};
	pl1 = {};
	pl2 = {};
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	//trf = new Transform();
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// ImGui clean up (as requested above <3)
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize ImGui itself and platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();

	// Create our camera
	camera = std::make_shared<Camera>(
		0.0f,
		2.5f,
		-15.0f,
		(float)windowWidth / windowHeight, // Turn one into a float so you aren't doing integer division!
		XM_PIDIV4, // Pi divided by 4, 45 degrees
		2.0f,
		1.0f
	);

	// Loads the shaders, then creates our materials
	LoadShaders();
	LoadTexturesAndCreateMaterials();
	CreateGeometry();
	CreateRenderables();
	SetupTransforms();
	InitLighting();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		vs = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
		ps = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
		fps = std::make_shared<SimplePixelShader>(device, context, FixPath(L"FancyPixelShader.cso").c_str());
	}
}

// --------------------------------------------------------
// Loads textures with the CreateWICTextureFromFile() function
// --------------------------------------------------------
void Game::LoadTexturesAndCreateMaterials()
{
	// Create a sampler state that holds our texture sampling options
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState;
	
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 8;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, sampState.GetAddressOf());

	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dirtSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Ground_FarmlandDry2_3.2x3.2_1K_albedo.tif").c_str(),
		nullptr,
		dirtSRV.GetAddressOf()
	);
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dirtSpecSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Ground_FarmlandDry2_3.2x3.2_1K_roughness.tif").c_str(),
		nullptr,
		dirtSpecSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Ground_ForestMoss02_1x1_512_albedo.tif").c_str(),
		nullptr,
		mossSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossSpecSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Ground_ForestMoss02_1x1_512_roughness.tif").c_str(),
		nullptr,
		mossSpecSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> barkSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Bark_Yucca_0.125x0.125_512_albedo.tif").c_str(),
		nullptr,
		barkSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> barkSpecSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/TexturesCom_Bark_Yucca_0.125x0.125_512_roughness.tif").c_str(),
		nullptr,
		barkSpecSRV.GetAddressOf()
	);

	// Create materials
	mat1 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.9f, 1.0f, 1.0f), 0.06f, vs, ps);
	mat2 = std::make_shared<Material>(DirectX::XMFLOAT4(0.8f, 0.9f, 0.1f, 1.0f), 0.02f, vs, ps);
	mat3 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.82f, 0.9f, 1.0f), 0.05f, vs, ps);

	// Assign textures to materials
	mat1->AddTextureSampler("BasicSampler", sampState);
	mat1->AddTextureSRV("SurfaceTexture", dirtSRV);
	mat1->AddTextureSRV("SpecularTexture", dirtSpecSRV);

	mat2->AddTextureSampler("BasicSampler", sampState);
	mat2->AddTextureSRV("SurfaceTexture", mossSRV);
	mat2->AddTextureSRV("SpecularTexture", mossSpecSRV);

	mat3->AddTextureSampler("BasicSampler", sampState);
	mat3->AddTextureSRV("SurfaceTexture", barkSRV);
	mat3->AddTextureSRV("SpecularTexture", barkSpecSRV);
}

// --------------------------------------------------------
// Loads the meshes from, then constructs them from, files
// - After this step, the geometry can be reused and drawn over and over by renderables without any more mathematics
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// At position 0: the cube
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	// At position 1: the cylinder
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	// At position 2: the helix
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));
	// At position 3: the quad
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));
	// At position 4: the double sided quad
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device, context));
	// At position 5: the sphere
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
	// At position 6: the torus
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));
}

// --------------------------------------------------------
// Fills the renderables array
// - Assembles the renderables inside from meshes and materials
// --------------------------------------------------------
void Game::CreateRenderables()
{
	// At position 0: the cube
	renderables.push_back(std::make_shared<Renderable>(meshes[0], mat1));
	// At position 1: the cylinder
	renderables.push_back(std::make_shared<Renderable>(meshes[1], mat2));
	// At position 2: the helix
	renderables.push_back(std::make_shared<Renderable>(meshes[2], mat3));
	// At position 3: the quad
	renderables.push_back(std::make_shared<Renderable>(meshes[3], mat1));
	// At position 4: the double sided quad
	renderables.push_back(std::make_shared<Renderable>(meshes[4], mat2));
	// At position 5: the sphere
	renderables.push_back(std::make_shared<Renderable>(meshes[5], mat3));
	// At position 6: the torus
	renderables.push_back(std::make_shared<Renderable>(meshes[6], mat1));
}

// --------------------------------------------------------
// Fills the transforms array
// - Also gives the space to adjust transforms before the game starts
// --------------------------------------------------------
void Game::SetupTransforms()
{
	for (int i = 0; i < renderables.size(); i++)
	{
		// Remember that transforms (the array) is all pointers, no real transforms
		transforms.push_back(renderables[i]->GetTransform());
	}

	// Now we can adjust them before the game begins if needed
	transforms[0]->SetPosition(-9.0, 0.0, 0.0);
	transforms[1]->SetPosition(-6.0, 0.0, 0.0);
	transforms[2]->SetPosition(-3.0, 0.0, 0.0);
	transforms[3]->SetPosition(0.0, 0.0, 0.0);
	transforms[4]->SetPosition(3.0, 0.0, 0.0);
	transforms[5]->SetPosition(6.0, 0.0, 0.0);
	transforms[6]->SetPosition(9.0, 0.0, 0.0);
}

// --------------------------------------------------------
// Makes the light objects and sets ambient light values
// - Also gives the space to adjust colors/transforms before the game starts
// --------------------------------------------------------
void Game::InitLighting()
{
	ambientLight = DirectX::XMFLOAT3(0.05f, 0.15f, 0.24f);
	
	// Directional Light 1
	firstLight = {};
	firstLight.Type = LIGHT_TYPE_DIRECTIONAL;
	firstLight.Direction = DirectX::XMFLOAT3(1, 0.5, 0.2);
	firstLight.Color = DirectX::XMFLOAT3(0.53, 0.05, 0.35);
	firstLight.Intensity = 1.0f;

	// Directional Light 2
	dir2= {};
	dir2.Type = LIGHT_TYPE_DIRECTIONAL;
	dir2.Direction = DirectX::XMFLOAT3(0.2, 0.2, 0.2);
	dir2.Color = DirectX::XMFLOAT3(0.9, 0.22, 0.8);
	dir2.Intensity = 1.0f;

	// Directional Light 3
	dir3= {};
	dir3.Type = LIGHT_TYPE_DIRECTIONAL;
	dir3.Direction = DirectX::XMFLOAT3(0.1, -1, 0.2);
	dir3.Color = DirectX::XMFLOAT3(0.753, 0.02, 0.135);
	dir3.Intensity = 1.0f;

	// Point Light 1
	pl1 = {};
	pl1.Type = LIGHT_TYPE_POINT;
	pl1.Position = DirectX::XMFLOAT3(0.1, -1, 0.2);
	pl1.Range = 4.6f;
	pl1.Color = DirectX::XMFLOAT3(0.9, 0.85, 0.935);
	pl1.Intensity = 1.0f;

	// Point Light 2
	pl2 = {};
	pl2.Type = LIGHT_TYPE_POINT;
	pl2.Position = DirectX::XMFLOAT3(0.9, -1.6, 4.0);
	pl2.Range = 15.0f;
	pl2.Color = DirectX::XMFLOAT3(0.4, 0.85, 0.85);
	pl2.Intensity = 1.0f;
}

// -dir3-------------------------------------------------------
// Do this first thing in Update()!
//  - Feeds fresh input data to ImGui
//  - Determines whether to capture input
//  - Resets the gui frame
//  - Needs deltaTime to function
// --------------------------------------------------------
ImGuiIO Game::PrepImGui(float deltaTime)
{
	// Get a reference to our custom input manager
	Input& input = Input::GetInstance();
	
	// Reset input manager's gui state so we don’t
	// taint our own input (you’ll uncomment later)
	input.SetKeyboardCapture(false);
	input.SetMouseCapture(false);
	
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	io.KeyCtrl = input.KeyDown(VK_CONTROL);
	io.KeyShift = input.KeyDown(VK_SHIFT);
	io.KeyAlt = input.KeyDown(VK_MENU);
	io.MousePos.x = (float)input.GetMouseX();
	io.MousePos.y = (float)input.GetMouseY();
	io.MouseDown[0] = input.MouseLeftDown();
	io.MouseDown[1] = input.MouseRightDown();
	io.MouseDown[2] = input.MouseMiddleDown();
	io.MouseWheel = input.GetMouseWheel();
	input.GetKeyArray(io.KeysDown, 256);

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture (you’ll uncomment later)
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	//ImGui::ShowDemoWindow();

	// Return the io for our Update method to use
	return io;
}

// --------------------------------------------------------
// Handles all our ImGui window definition.
//  - Takes pointers to values to read/write to them
// --------------------------------------------------------
void Game::UpdateImGui(ImGuiIO frameIO)
{
	float framerate = frameIO.Framerate;

	ImGui::Begin("Stats"); // Everything after is part of the window
	ImGui::Text("ms/frame: %.3f - FPS: %.1f", 1000.0f / framerate, framerate);
	ImGui::Text("display size X: %.0f", frameIO.DisplaySize.x);
	ImGui::Text("display size Y: %.0f", frameIO.DisplaySize.y);
	ImGui::End(); // Ends the current window

	//ImGui::Begin("Camera Editor"); // Everything after is part of the window
	//ImGui::Text("To be improved... I tried interfacing with the transform but ran out of time.");
	//if (ImGui::SliderFloat("Field of View", camera->GetFOV(), XM_PIDIV4, XM_PIDIV2))
	//{
	//	camera->UpdateProjectionMatrix(*camera->GetAspectRatio());
	//}
	//ImGui::End(); // Ends the current window

	ImGui::Begin("Light Editor");
	ImGui::Text("Point Light Positions");
	ImGui::SliderFloat3("Point Light 1", &pl1.Position.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Point Light 2", &pl2.Position.x, -10.0f, 10.0f);
	ImGui::Text("Point Light Colors"); // These just control the position right now- why is that?
	ImGui::SliderFloat3("Point Light 1", &pl1.Color.x, 0.0f, 1.0f);
	ImGui::SliderFloat3("Point Light 2", &pl2.Color.x, 0.0f, 1.0f);
	ImGui::Text("Point Light Ranges");
	ImGui::SliderFloat("Point Light 1", &pl1.Range, 0.0f, 20.0f);
	ImGui::SliderFloat("Point Light 2", &pl2.Range, 0.0f, 20.0f);
	ImGui::End();
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// This next line following must be done at the very top of Update(),
	// so that the UI has fresh input data and it knows
	// a new frame has started!
	ImGuiIO frameIO = PrepImGui(deltaTime);

	// Actually put the gui on screen
	UpdateImGui(frameIO);

	// Update the camera :)
	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
	{
		//delete(trf);
		//trf = nullptr;
		Quit();
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	for (int i = 0; i < renderables.size(); i++)
	{
		// All the renderables get the ambient light cast onto their pixel shader
		renderables[i]->GetMaterial()->GetPS()->SetFloat3("ambientLight", ambientLight);
		// They also get the directional light
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"directionalLight1",
			&firstLight,
			sizeof(Light)
		);
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"directionalLight2",
			&dir2,
			sizeof(Light)
		);
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"directionalLight3",
			&dir3,
			sizeof(Light)
		);
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"pointLight1",
			&pl1,
			sizeof(Light)
		);
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"pointLight2",
			&pl2,
			sizeof(Light)
		);
		renderables[i]->Draw(context, camera, totalTime);
	}

	// The GUI should be the LAST thing drawn before ending the frame!
	// Draw ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		swapChain->Present(vsync ? 1 : 0, 0);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}