#include "RenderTargetD3D11.h"

RenderTargetD3D11::~RenderTargetD3D11()
{
	if (this->texture != nullptr)
		this->texture->Release();
	for (int i = 0; i < rtvs.size(); i++)
	{
		if (this->rtvs[i] != nullptr)
			this->rtvs[i]->Release();
	}
	if (this->srv != nullptr)
		this->srv->Release();
}

void RenderTargetD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, bool hasSRV)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 6;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.BindFlags |= hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);
	if (FAILED(hr))
		return;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	ID3D11RenderTargetView* rtv;
	for (int i = 0; i < 6; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateRenderTargetView(this->texture, &rtvDesc, &rtv);
		this->rtvs.push_back(rtv);
	}

	if (hasSRV)
	{
		hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	}
}

void RenderTargetD3D11::Initialize(ID3D11Device* device, RenderTargetInfo info)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = info.width;
	desc.Height = info.height;
	desc.MipLevels = 1;
	desc.ArraySize = info.arraySize;
	desc.Format = info.format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.BindFlags |= info.hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = info.miscFlags;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);
	if (FAILED(hr))
		return;

	ID3D11RenderTargetView* rtv;
	for (int i = 0; i < info.arraySize; i++)
	{
		if (info.rtvDesc != nullptr)
		{
			info.rtvDesc->Texture2DArray.FirstArraySlice = i;
		}
		hr = device->CreateRenderTargetView(this->texture, info.rtvDesc, &rtv);
		this->rtvs.push_back(rtv);
	}

	if (info.hasSRV)
	{
		hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	}
}

ID3D11RenderTargetView* RenderTargetD3D11::GetRTV(size_t rtvIndex) const
{
	return this->rtvs[rtvIndex];
}

ID3D11ShaderResourceView* RenderTargetD3D11::GetSRV() const
{
	return this->srv;
}
