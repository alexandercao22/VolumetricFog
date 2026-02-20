#include "SamplerD3D11.h"

SamplerD3D11::SamplerD3D11(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode, std::optional<std::array<float, 4>> borderColour)
{
	Initialize(device, adressMode, borderColour);
}

SamplerD3D11::~SamplerD3D11()
{
	if (this->sampler != nullptr)
		this->sampler->Release();
}

void SamplerD3D11::Initialize(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode,
	std::optional<std::array<float, 4>> borderColour)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = adressMode;
	samplerDesc.AddressV = adressMode;
	samplerDesc.AddressW = adressMode;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (borderColour.has_value())
	{
		std::array<float, 4> color = borderColour.value();
		samplerDesc.BorderColor[0] = color[0];
		samplerDesc.BorderColor[1] = color[1];
		samplerDesc.BorderColor[2] = color[2];
		samplerDesc.BorderColor[3] = color[3];
	}

	HRESULT hr = device->CreateSamplerState(&samplerDesc, &this->sampler);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create sampler state\n";
	}

	//this->sampler->AddRef();
}

ID3D11SamplerState* SamplerD3D11::GetSamplerState() const
{
	return this->sampler;
}
