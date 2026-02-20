#pragma once

#include <d3d11_4.h>
#include <iostream>
#include <vector>

struct DepthBufferInfo
{
	D3D11_TEXTURE2D_DESC depthDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	bool HasSRV;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
};

class DepthBufferD3D11
{
private:
	ID3D11Texture2D* texture = nullptr;
	std::vector<ID3D11DepthStencilView*> depthStencilViews;
	ID3D11ShaderResourceView* srv = nullptr;

public:
	DepthBufferD3D11() = default;
	DepthBufferD3D11(ID3D11Device* device, UINT width, UINT height, bool hasSRV = false);
	DepthBufferD3D11(ID3D11Device* device, DepthBufferInfo depthBufferInfo);
	~DepthBufferD3D11();
	DepthBufferD3D11(const DepthBufferD3D11& other) = delete;
	DepthBufferD3D11& operator=(const DepthBufferD3D11& other) = delete;
	DepthBufferD3D11(DepthBufferD3D11&& other) = delete;
	DepthBufferD3D11& operator=(DepthBufferD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, UINT width, UINT height,
		bool hasSRV = false, UINT arraySize = 1);
	void Initialize(ID3D11Device* device, DepthBufferInfo depthBufferInfo);

	ID3D11DepthStencilView* GetDSV(UINT arrayIndex) const;
	ID3D11ShaderResourceView* GetSRV() const;
};