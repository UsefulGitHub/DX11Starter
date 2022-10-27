#pragma once

#include "SimpleShader.h"
#include <memory>
#include <unordered_map>

class Material
{
public:
	Material(
		DirectX::XMFLOAT4 colorTint,
		float roughness,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps
	);
	
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVS();
	std::shared_ptr<SimplePixelShader> GetPS();

	DirectX::XMFLOAT4 SetColorTint(DirectX::XMFLOAT4 newColorTint);
	float SetRoughness(float newRoughness);
	std::shared_ptr<SimpleVertexShader> SetVS(std::shared_ptr<SimpleVertexShader> newVS);
	std::shared_ptr<SimplePixelShader> SetPS(std::shared_ptr<SimplePixelShader> newPS);

	void PrepareMaterial();

	void AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddTextureSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState);

private:
	DirectX::XMFLOAT4 colorTint;
	float roughness;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> textureSamplers;
};

