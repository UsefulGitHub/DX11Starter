#pragma once

#include "SimpleShader.h"
#include <memory>

class Material
{
public:
	Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps);
	
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVS();
	std::shared_ptr<SimplePixelShader> GetPS();

	DirectX::XMFLOAT4 SetColorTint(DirectX::XMFLOAT4 newColorTint);
	std::shared_ptr<SimpleVertexShader> SetVS(std::shared_ptr<SimpleVertexShader> newVS);
	std::shared_ptr<SimplePixelShader> SetPS(std::shared_ptr<SimplePixelShader> newPS);

	void PrepareMaterial();

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
};

