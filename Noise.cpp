#include "Noise.h"

// Function to linearly interpolate between a0 and a1
// Weight w should be in the range [0.0, 1.0]
// Perlin Noise Function 1 of 3 grabbed from https://en.wikipedia.org/wiki/Perlin_noise
float interpolate(float a0, float a1, float w)
{
	return (a1 - a0) * w + a0;
}

// Create pseudorandom direction vector
// Perlin Noise Function 2 of 3 adapted from https://en.wikipedia.org/wiki/Perlin_noise
DirectX::XMFLOAT2 randomGradient(int ix, int iy)
{
	// No precomputed gradients mean this works for any number of grid coordinates
	const unsigned int w = 8 * sizeof(unsigned int);
	const unsigned int s = w / 2; // rotation width
	unsigned int a = ix, b = iy;
	a *= 3284157443; b ^= a << s | a >> w - s;
	b *= 1911520717; a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (PI / ~(~0u >> 1)); // in [0, 2*Pi]
	DirectX::XMFLOAT2 v;
	v.x = cos(random); v.y = sin(random);
	return v;
}

// Computes the dot product of the distance and gradient vectors
// Perlin Noise Function 3 of 3 adapted from https://en.wikipedia.org/wiki/Perlin_noise
float dotGridGradient(int ix, int iy, float x, float y)
{
	// Get gradient from integer coordinates
	DirectX::XMFLOAT2 gradient = randomGradient(ix, iy);

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = x - (float)iy;

	// Compute the dot-product
	return (dx * gradient.x + dy * gradient.y);
}

// Compute Perlin noise at coordinates x,y
// THE Perlin Noise Function adapted from https://en.wikipedia.org/wiki/Perlin_noise
float perlin(float x, float y)
{
	// Determine grid cell coordinates
	int x0 = (int)floor(x);
	int x1 = x0 + 1;
	int y0 = (int)floor(y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial / s-curve here
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;

	n0 = dotGridGradient(x0, y0, x, y);
	n1 = dotGridGradient(x1, y0, x, y);
	ix0 = interpolate(n0, n1, sx);

	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	ix1 = interpolate(n0, n1, sx);

	value = interpolate(ix0, ix1, sy);
	return value;

	// Will return in range -1 to 1
	// To make it in range of 0 to 1, multiply by 1/2 and add 1/2
}

