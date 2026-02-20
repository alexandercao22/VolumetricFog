#include "DepthBufferD3D11.h"

DepthBufferD3D11::DepthBufferD3D11(ID3D11Device* device, UINT width, UINT height, bool hasSRV)
{
	Initialize(device, width, height, hasSRV);
}

DepthBufferD3D11::DepthBufferD3D11(ID3D11Device* device, DepthBufferInfo depthBufferInfo)
{
	Initialize(device, depthBufferInfo);
}

DepthBufferD3D11::~DepthBufferD3D11()
{
	if (this->texture != nullptr)
		this->texture->Release();
	for (int i = 0; i < this->depthStencilViews.size(); i++)
	{
		if (this->depthStencilViews[i] != nullptr)
			this->depthStencilViews[i]->Release();
	}
	if (this->srv != nullptr)
		this->srv->Release();
}

void DepthBufferD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, bool hasSRV, UINT arraySize)
{
	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = arraySize;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	
	HRESULT hr = device->CreateTexture2D(&depthDesc, nullptr, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create depth buffer texture\n";
		return;
	}

	for (int i = 0; i < arraySize; i++)
	{
		ID3D11DepthStencilView* depthView;
		hr = device->CreateDepthStencilView(this->texture, nullptr, &depthView);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth stencil\n";
			return;
		}
		this->depthStencilViews.push_back(depthView);
	}

	if (hasSRV)
	{
		hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth srv\n";
			return;
		}
	}
}

void DepthBufferD3D11::Initialize(ID3D11Device* device, DepthBufferInfo depthBufferInfo)
{
	HRESULT hr = device->CreateTexture2D(&depthBufferInfo.depthDesc, nullptr, &this->texture);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create depth buffer texture\n";
		return;
	}

	for (int i = 0; i < depthBufferInfo.depthDesc.ArraySize; i++)
	{
		depthBufferInfo.dsvDesc.Texture2DArray.FirstArraySlice = i;
		ID3D11DepthStencilView* depthView;
		hr = device->CreateDepthStencilView(this->texture, &depthBufferInfo.dsvDesc, &depthView);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth stencil\n";
			return;
		}
		this->depthStencilViews.push_back(depthView);
	}

	if (depthBufferInfo.HasSRV)
	{
		hr = device->CreateShaderResourceView(this->texture, &depthBufferInfo.srvDesc, &this->srv);
		if (FAILED(hr))
		{
			std::cerr << "Failed to create depth srv1\n";
			return;
		}
	}
}

ID3D11DepthStencilView* DepthBufferD3D11::GetDSV(UINT arrayIndex) const
{
	return this->depthStencilViews[arrayIndex];
}

ID3D11ShaderResourceView* DepthBufferD3D11::GetSRV() const
{
	return this->srv;
}
