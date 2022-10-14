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
		1.0f,
		1.0f
	);

	// Loads the shaders, then creates our materials
	LoadShaders();
	CreateMaterials();
	CreateGeometry();
	CreateRenderables();

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
// Creates the materials we are using
// --------------------------------------------------------
void Game::CreateMaterials()
{
	mat1 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.5f, 1.0f, 1.0f), vs, ps);
	mat2 = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.2f, 0.1f, 1.0f), vs, ps);
	mat3 = std::make_shared<Material>(DirectX::XMFLOAT4(0.3f, 0.2f, 0.9f, 1.0f), vs, fps);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	sphere = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context);
	meshes.push_back(sphere);
}

void Game::CreateRenderables()
{
	ent1 = std::make_shared<Renderable>(meshes[0], mat3);
}

// --------------------------------------------------------
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

	ImGui::Begin("Camera Editor"); // Everything after is part of the window
	ImGui::Text("To be improved... I tried interfacing with the transform but ran out of time.");

	if (ImGui::SliderFloat("Field of View", camera->GetFOV(), XM_PIDIV4, XM_PIDIV2))
	{
		camera->UpdateProjectionMatrix(*camera->GetAspectRatio());
	}

	ImGui::End(); // Ends the current window
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

	//Transform* trf1 = ent1->GetTransform();
	//trf1->SetScale(0.2f, 0.2f, 0.2f);
	//trf1->SetPosition(0.0f, sin(totalTime) * deltaTime, 0.0f);

	//Transform* trf2 = ent2->GetTransform();
	//trf2->SetPosition(sin(totalTime) * 10.0f * deltaTime, -0.5f, 0.0f);
	//trf2->SetScale(0.5f, 0.4f, 1.0f);

	//Transform* trf3 = ent3->GetTransform();
	//trf3->Rotate(0.1f * deltaTime, 0.1f * deltaTime, 0.1f * deltaTime);

	//Transform* trf4 = ent4->GetTransform();
	//trf4->SetScale(sin(totalTime) * 10.0f * deltaTime, sin(totalTime) * 10 * deltaTime, 0.0f);
	//trf4->SetPosition((sin(totalTime) * 10.0f) * deltaTime, -0.2f, 0.1f);

	//Transform* trf5 = ent5->GetTransform();
	//trf5->Rotate(0.0f, 0.0f, 0.2f * deltaTime);
	//trf5->SetScale(0.6f, 0.6f, 1.0f);
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

	ent1->Draw(context, camera);

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