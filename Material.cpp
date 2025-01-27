#include "Material.h"

Material::Material(
    DirectX::XMFLOAT4 colorTint,
    std::shared_ptr<SimpleVertexShader> vs,
    std::shared_ptr<SimplePixelShader> ps)
    :
    colorTint(colorTint),
    vs(vs),
    ps(ps)
{
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
    return colorTint;
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
    // Assigning SRVs and sampler state to the pixel shader in a loop, because there can be more than one.
    for (auto& t : textureSRVs)
    {
        ps->SetShaderResourceView(t.first.c_str(), t.second);
    }
    for (auto& s : textureSamplers)
    {
        ps->SetSamplerState(s.first.c_str(), s.second);
    }
}

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ shaderName, srv });
}

void Material::AddTextureSampler(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState)
{
    textureSamplers.insert({ shaderName, sampState });
}
