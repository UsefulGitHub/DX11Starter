#include "Game.h"

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
	dir1 = {};
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
		0.0f,
		-5.0f,
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
		skyVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVertexShader.cso").c_str());
		skyPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPixelShader.cso").c_str());
		shadowVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"ShadowVS.cso").c_str());
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

#pragma region Moss (Grasslands)
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/moss_albedo.tif").c_str(),
		nullptr,
		mossAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/moss_normals.tif").c_str(),
		nullptr,
		mossNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/moss_roughness.tif").c_str(),
		nullptr,
		mossRoughnessSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mossMetalnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/moss_metal.tif").c_str(),
		nullptr,
		mossMetalnessSRV.GetAddressOf()
	);
#pragma endregion

#pragma region Desert
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> desertAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/desert_albedo.tif").c_str(),
		nullptr,
		desertAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> desertNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/desert_normals.tif").c_str(),
		nullptr,
		desertNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> desertRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/desert_roughness.tif").c_str(),
		nullptr,
		desertRoughnessSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> desertMetalnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/desert_metal.tif").c_str(),
		nullptr,
		desertMetalnessSRV.GetAddressOf()
	);
#pragma endregion

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> splatMapSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/test_splat.png").c_str(),
		nullptr,
		splatMapSRV.GetAddressOf()
	);

#pragma region Terrain
	// Create a texture procedurally
	int noiseWidth = 1024;
	int noiseHeight = 1024;
	int noiseDimensions = noiseWidth * noiseHeight;
	DirectX::XMFLOAT4* topNoise = new DirectX::XMFLOAT4[noiseDimensions];
	int xIndex = 0;
	float xoff = 0.0f;
	float yoff = 0.0f;
	for (int i = 0; i < noiseDimensions; i++)
	{
		topNoise[i].x = perlin(xoff, yoff) / 0.5f + 0.5f;
		if (xIndex < noiseWidth)
		{
			xoff += 0.05;
			xIndex += 1;
		}
		else
		{
			xIndex = 0;
			xoff = 0.0f;
			yoff += 0.05;
		}
	}

	// Create a simple texture of the specified size
	D3D11_TEXTURE2D_DESC td = {};
	td.ArraySize = 1;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.MipLevels = 1;
	td.Height = noiseHeight;
	td.Width = noiseWidth;
	td.SampleDesc.Count = 1;

	// Initial data for the texture
	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = topNoise;
	data.SysMemPitch = sizeof(float) * 4 * noiseWidth;

	// Actually create it
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	device->CreateTexture2D(&td, &data, texture.GetAddressOf());

	// Create the shader resource view for this texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = td.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noiseSRV;
	device->CreateShaderResourceView(texture.Get(), &srvDesc, noiseSRV.GetAddressOf());

	delete[] topNoise;
#pragma endregion

	// Create materials
	// High roughness is a matte surface, low roughness is shiny
	mat1 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps);
	//mat2 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.7f, 0.7f, 1.0f), vs, ps);

	// Assign textures to materials
	mat1->AddTextureSRV("AlbedoMap1",			mossAlbedoSRV);
	mat1->AddTextureSRV("NormalMap1",			mossNormalSRV);
	mat1->AddTextureSRV("RoughnessMap1",		mossRoughnessSRV);
	mat1->AddTextureSRV("MetalnessMap1",		mossMetalnessSRV);
	mat1->AddTextureSRV("AlbedoMap2",			desertAlbedoSRV);
	mat1->AddTextureSRV("NormalMap2",			desertNormalSRV);
	mat1->AddTextureSRV("RoughnessMap2",		desertRoughnessSRV);
	mat1->AddTextureSRV("MetalnessMap2",		desertMetalnessSRV);
	mat1->AddTextureSRV("SplatMap",				splatMapSRV);
	mat1->AddTextureSRV("TerrainMap",			noiseSRV);
	mat1->AddTextureSampler("BasicSampler",		sampState);
}

// --------------------------------------------------------
// Loads the meshes from, then constructs them from, files
// - After this step, the geometry can be reused and drawn over and over by renderables without any more mathematics
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// We need only one mesh- the sphere
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));

	Microsoft::WRL::ComPtr<ID3D11SamplerState> skySampState;

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 8;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, skySampState.GetAddressOf());

	// Now that the meshes are loaded in we can create the sky-
	sky = std::make_shared<Sky>(
		device.Get(),
		context.Get(),
		meshes[0], // Skybox is a cube
		skyVS,
		skyPS,
		skySampState.Get(),
		FixPath(L"../../Assets/Textures/Sky/corona_lf.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/corona_rt.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/corona_up.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/corona_dn.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/corona_ft.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/corona_bk.png").c_str()
		);
}

// --------------------------------------------------------
// Fills the renderables array
// - Assembles the renderables inside from meshes and materials
// --------------------------------------------------------
void Game::CreateRenderables()
{
	// At position 0: the sphere
	renderables.push_back(std::make_shared<Renderable>(meshes[0], mat1));
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
	transforms[0]->SetPosition(0.0, 0.0, 0.0);
}

// --------------------------------------------------------
// Makes the texture used for terrain mapping
// - Also programmatically draws initial random continents into the texture
// --------------------------------------------------------
void Game::CreateTerrainResources()
{
	//D3D11_TEXTURE2D_DESC desc;
	//desc.Width = 1024;
	//desc.Height = 1024;
	//desc.MipLevels = desc.ArraySize = 1;
	//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//desc.SampleDesc.Count = 1;
	//desc.Usage = D3D11_USAGE_DYNAMIC;
	//desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//desc.MiscFlags = 0;

	//ID3D11Device* pd3dDevice; // Don't forget to initialize this
	//ID3D11Texture2D* pTexture = NULL;
	//pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);
}

void Game::CreateShadowMapResources()
{
	// Create shadow requirements ------------------------------------------
	shadowMapResolution = 1024;
	shadowProjectionSize = 10.0f;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution;
	shadowDesc.Height = shadowMapResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible positive value storable in the depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create the "camera" matrices for the shadow map rendering

	// View
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 20, -20, 0),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowViewMatrix, shView);

	// Projection - we want ORTHOGRAPHIC for directional light shadows
	// NOTE: This particular projection is set up to be SMALLER than
	// the overall "scene", to show what happens when objects go
	// outside the shadow area.  In a game, you'd never want the
	// user to see this edge, but I'm specifically making the projection
	// small in this demo to show you that it CAN happen.
	//
	// Ideally, the first two parameters below would be adjusted to
	// fit the scene (or however much of the scene the user can see
	// at a time).  More advanced techniques, like cascaded shadow maps,
	// would use multiple (usually 4) shadow maps with increasingly larger
	// projections to ensure large open world games have shadows "everywhere"
	XMMATRIX shProj = XMMatrixOrthographicLH(shadowProjectionSize, shadowProjectionSize, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, shProj);

}

// --------------------------------------------------------
// Makes the light objects
// - Also gives the space to adjust colors/transforms before the game starts
// --------------------------------------------------------
void Game::InitLighting()
{
	// Directional Light 1
	dir1 = {};
	dir1.Type = LIGHT_TYPE_DIRECTIONAL;
	dir1.Direction = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	dir1.Color = DirectX::XMFLOAT3(1.0f, 0.8f, 0.85f);
	dir1.Intensity = 1.0f;

	// Directional Light 2
	dir2= {};
	dir2.Type = LIGHT_TYPE_DIRECTIONAL;
	dir2.Direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	dir2.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	dir2.Intensity = 1.0f;

	// Directional Light 3
	dir3= {};
	dir3.Type = LIGHT_TYPE_DIRECTIONAL;
	dir3.Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	dir3.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	dir3.Intensity = 1.0f;

	// Point Light 1
	pl1 = {};
	pl1.Type = LIGHT_TYPE_POINT;
	pl1.Position = DirectX::XMFLOAT3(0.1, -1, 0.2);
	pl1.Range = 18.0f;
	pl1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	pl1.Intensity = 1.0f;

	// Point Light 2
	pl2 = {};
	pl2.Type = LIGHT_TYPE_POINT;
	pl2.Position = DirectX::XMFLOAT3(0.9, -1.6, 4.0);
	pl2.Range = 45.0f;
	pl2.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	pl2.Intensity = 1.0f;
}

// --------------------------------------------------------
// Renders the shadow map from the light's point of view
// --------------------------------------------------------
void Game::RenderShadowMap()
{
	// Initial pipeline setup - No RTV necessary - Clear shadow map
	context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());

	// Set the shadow-specific vertex shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);
	context->PSSetShader(0, 0, 0); // No pixel shader

	// Need to create a viewport that matches the shadow map resolution
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Loop and draw all entities
	for (auto& e : renderables)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();

		// Draw the mesh
		e->GetMesh()->Draw();
	}

	// Put everything back
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
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

	//ImGui::Begin("Stats"); // Everything after is part of the window
	//ImGui::Text("ms/frame: %.3f - FPS: %.1f", 1000.0f / framerate, framerate);
	//ImGui::Text("display size X: %.0f", frameIO.DisplaySize.x);
	//ImGui::Text("display size Y: %.0f", frameIO.DisplaySize.y);
	//ImGui::End(); // Ends the current window

	ImGui::Begin("Editor");
	ImGui::Text("Point Light Positions");
	ImGui::SliderFloat3("Point Light Position 1", &pl1.Position.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Point Light Position 2", &pl2.Position.x, -10.0f, 10.0f);
	ImGui::Text("Point Light Colors");
	ImGui::SliderFloat3("Point Light Color 1", &pl1.Color.x, 0.0f, 1.0f);
	ImGui::SliderFloat3("Point Light Color 2", &pl2.Color.x, 0.0f, 1.0f);
	ImGui::Text("Point Light Ranges");
	ImGui::SliderFloat("Point Light 1", &pl1.Range, 0.0f, 100.0f);
	ImGui::SliderFloat("Point Light 2", &pl2.Range, 0.0f, 100.0f);
	ImGui::Text("Directional Light Colors");
	ImGui::SliderFloat3("Directional Light Color 1", &dir1.Color.x, 0.0f, 1.0f);
	ImGui::SliderFloat3("Directional Light Color 2", &dir2.Color.x, 0.0f, 1.0f);
	ImGui::SliderFloat3("Directional Light Color 3", &dir3.Color.x, 0.0f, 1.0f);
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

	for (int i = 0; i < renderables.size(); i++)
	{
		renderables[i]->GetTransform()->Rotate(0.0f, deltaTime * 0.1f, 0.0f);
	}

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
		// They also get the directional light
		renderables[i]->GetMaterial()->GetPS()->SetData(
			"directionalLight1",
			&dir1,
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

	sky->Draw(context, camera);

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