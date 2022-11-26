#pragma once

#include <DirectXMath.h>

constexpr auto PI = 3.14159265359f;

float interpolate(float a0, float a1, float w);
DirectX::XMFLOAT2 randomGradient(int ix, int iy);
float dotGridGradient(int ix, int iy, float x, float y);
float perlin(float x, float y);