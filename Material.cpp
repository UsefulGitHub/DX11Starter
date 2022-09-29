#include "Material.h"

Material::Material(std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps)
    :
    vs(vs),
    ps(ps)
{
}

std::shared_ptr<SimpleVertexShader> Material::GetVS()
{
    return vs;
}

std::shared_ptr<SimplePixelShader> Material::GetPS()
{
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
