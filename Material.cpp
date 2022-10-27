#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    float roughness,
    std::shared_ptr<SimpleVertexShader> vs,
    std::shared_ptr<SimplePixelShader> ps)
    :
    colorTint(colorTint),
    roughness(roughness),
    vs(vs),
    ps(ps)
{
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
    return colorTint;
}

float Material::GetRoughness()
{
    return roughness;
}

std::shared_ptr<SimpleVertexShader> Material::GetVS()
{
    return vs;
}

std::shared_ptr<SimplePixelShader> Material::GetPS()
{
    return ps;
}

DirectX::XMFLOAT4 Material::SetColorTint(DirectX::XMFLOAT4 newColorTint)
{
    colorTint = newColorTint;
    return colorTint;
}

float Material::SetRoughness(float newRoughness)
{
    roughness = newRoughness;
    return roughness;
}

std::shared_ptr<SimpleVertexShader> Material::SetVS(std::shared_ptr<SimpleVertexShader> newVS)
{
    vs = newVS;
    return vs;
}

std::shared_ptr<SimplePixelShader> Material::SetPS(std::shared_ptr<SimplePixelShader> newPS)
{
    ps = newPS;
    return ps;
}

/// <summary>
/// Send all of the data we need for this material down to the GPU
/// </summary>
void Material::PrepareMaterial()
{
    vs->SetShader();
    ps->SetShader();
}

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ shaderName, srv });
}

void Material::AddTextureSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState)
{
    textureSamplers.insert({ shaderName, sampState });
}
