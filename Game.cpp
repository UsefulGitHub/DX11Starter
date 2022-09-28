#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "Mesh.h"
#include "BufferStructs.h"
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

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
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

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);

		// Get size as the next multiple of 16 (instead of hardcoding a size here!)
		unsigned int size = sizeof(VertexShaderExternalData);
		size = (size + 15) / 16 * 16; // This will work even if your struct size changes
		// Basically first it gets the amount of 16s needed to be greater than or equal to the actual size -
		// For 0 to 15 it will be 1, for 16 to 31 it will be 2, 32 to 47 it will be 3 etc.
		// Then it multiplies that "number of 16s that are needed" number by 16 so it is a multiple of 16

		// Describe the constant buffer
		D3D11_BUFFER_DESC cbDesc	= {}; // Sets struct to all zeroes, clean slate (if we don't do this, it could be full of random numbers!)
		cbDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth			= size; // Must be a multiple of 16
		// NOTE: This buffer will be read by the gpu and written to by the cpu, probably fairly often (at least once per frame)
		cbDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage				= D3D11_USAGE_DYNAMIC;

		// Create the constant buffer
		device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());
	}
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Creating the triangle
	{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	triangle = std::make_shared<Mesh>(vertices, 3, indices, 3, device, context);
	}

	// Creating the square
	{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 magenta = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex squareVertices[] =
	{
		{ XMFLOAT3(+0.1f, +0.1f, +0.0f), red },
		{ XMFLOAT3(+0.1f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(+0.7f, +0.1f, +0.0f), green },
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), magenta }
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int squareIndices[] = { 0, 3, 2, 0, 1, 3 };

	square = std::make_shared<Mesh>(squareVertices, 4, squareIndices, 6, device, context);
	}

	// Creating the nonagon
	{
		// Create some temporary variables to represent colors
		// - Not necessary, just makes things more readable
		XMFLOAT4 yellow = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 teal = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		XMFLOAT4 magenta = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

		// Set up the vertices of the triangle we would like to draw
		// - We're going to copy this array, exactly as it exists in CPU memory
		//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
		// - Note: Since we don't have a camera or really any concept of
		//    a "3d world" yet, we're simply describing positions within the
		//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
		// - This means (0,0) is at the very center of the screen.
		// - These are known as "Normalized Device Coordinates" or "Homogeneous 
		//    Screen Coords", which are ways to describe a position without
		//    knowing the exact size (in pixels) of the image/window/etc.  
		// - Long story short: Resizing the window also resizes the triangle,
		//    since we're describing the triangle in terms of the window itself
		Vertex nonagonVertices[] =
		{
			{ XMFLOAT3(-0.4f, -0.3f, +0.0f), yellow },
			{ XMFLOAT3(-0.3f, +0.0f, +0.0f), teal },
			{ XMFLOAT3(-0.4f, +0.3f, +0.0f), magenta },
			{ XMFLOAT3(-0.5f, +0.5f, +0.0f), yellow },
			{ XMFLOAT3(-0.7f, +0.5f, +0.0f), teal },
			{ XMFLOAT3(-0.8f, +0.3f, +0.0f), magenta },
			{ XMFLOAT3(-0.9f, +0.0f, +0.0f), yellow },
			{ XMFLOAT3(-0.8f, -0.3f, +0.0f), teal },
			{ XMFLOAT3(-0.6f, -0.5f, +0.0f), magenta },
			{ XMFLOAT3(-0.6f, +0.0f, +0.0f), magenta }
		};

		// Set up indices, which tell us which vertices to use and in which order
		// - This is redundant for just 3 vertices, but will be more useful later
		// - Indices are technically not required if the vertices are in the buffer 
		//    in the correct order and each one will be used exactly once
		// - But just to see how it's done...
		unsigned int nonagonIndices[] = {9,1,0,9,2,1,9,3,2,9,4,3,9,5,4,9,6,5,9,7,6,9,8,7,9,0,8};

		nonagon = std::make_shared<Mesh>(nonagonVertices, 10, nonagonIndices, 27, device, context);

		// Set up the renderable game objects!
		ent1 = std::make_shared<Renderable>(triangle);
		ent2 = std::make_shared<Renderable>(nonagon);
		ent3 = std::make_shared<Renderable>(nonagon);
		ent4 = std::make_shared<Renderable>(nonagon);
		ent5 = std::make_shared<Renderable>(square);
	}
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
	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
	{
		//delete(trf);
		//trf = nullptr;
		Quit();
	}

	Transform* trf1 = ent1->GetTransform();
	trf1->SetScale(0.2f, 0.2f, 0.2f);
	trf1->SetPosition(0.0f, sin(totalTime) * deltaTime, 0.0f);

	Transform* trf2 = ent2->GetTransform();
	trf2->SetPosition(sin(totalTime) * 10.0f * deltaTime, -0.5f, 0.0f);
	trf2->SetScale(0.5f, 0.4f, 1.0f);

	Transform* trf3 = ent3->GetTransform();
	trf3->Rotate(0.1f * deltaTime, 0.1f * deltaTime, 0.1f * deltaTime);

	Transform* trf4 = ent4->GetTransform();
	trf4->SetScale(sin(totalTime) * 10.0f * deltaTime, sin(totalTime) * 10 * deltaTime, 0.0f);
	trf4->SetPosition((sin(totalTime) * 10.0f) * deltaTime, -0.2f, 0.1f);

	Transform* trf5 = ent5->GetTransform();
	trf5->Rotate(0.0f, 0.0f, 0.2f * deltaTime);
	trf5->SetScale(0.6f, 0.6f, 1.0f);
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

	ent1->Draw(context, vsConstantBuffer, camera);
	ent2->Draw(context, vsConstantBuffer, camera);
	ent3->Draw(context, vsConstantBuffer, camera);
	ent4->Draw(context, vsConstantBuffer, camera);
	ent5->Draw(context, vsConstantBuffer, camera);

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