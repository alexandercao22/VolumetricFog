#pragma once

#include <d3d11_4.h>
#include <vector>

struct RenderTargetInfo
{
	UINT width;
	UINT height;
	UINT arraySize;
	DXGI_FORMAT format;
	UINT miscFlags;
	bool hasSRV;
	D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc;
};

class RenderTargetD3D11
{
private:
	ID3D11Texture2D* texture = nullptr;
	std::vector<ID3D11RenderTargetView*> rtvs;
	ID3D11ShaderResourceView* srv = nullptr;

public:
	RenderTargetD3D11() = default;
	~RenderTargetD3D11();
	RenderTargetD3D11(const RenderTargetD3D11& other) = delete;
	RenderTargetD3D11& operator=(const RenderTargetD3D11& other) = delete;
	RenderTargetD3D11(RenderTargetD3D11&& other) = delete;
	RenderTargetD3D11& operator=(RenderTargetD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, UINT width, UINT height,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool hasSRV = false);
	void Initialize(ID3D11Device* device, RenderTargetInfo info);

	ID3D11RenderTargetView* GetRTV(size_t rtvIndex) const;
	ID3D11ShaderResourceView* GetSRV() const;
};