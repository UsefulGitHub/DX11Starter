#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT2 UV;			// The UV of the vertex
	DirectX::XMFLOAT3 Normal;		// The normal of the vertex
	DirectX::XMFLOAT3 Tangent;		// The tangent along the surface, oriented to the u of the uv
};