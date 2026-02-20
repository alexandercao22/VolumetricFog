#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"

class DirectionalLight
{
private:
	struct LightBuffer
	{
		DirectX::XMFLOAT4X4 vpMatrix;
		DirectX::XMFLOAT3 colour = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	}bufferData;

	DepthBufferD3D11 shadowMaps;
	StructuredBufferD3D11 lightBuffer;
	CameraD3D11 shadowCameras;

	void InitializeShadowMaps(ID3D11Device* device);

public:
	DirectionalLight() = default;
	DirectionalLight(ID3D11Device* device, DirectX::XMFLOAT3 colour, DirectX::XMFLOAT3 direction);
	//~DirectionalLight() = default;
	~DirectionalLight();

	void Initialize(ID3D11Device* device, DirectX::XMFLOAT3 colour, DirectX::XMFLOAT3 direction);

	ID3D11DepthStencilView* GetShadowMapDSV() const;
	ID3D11ShaderResourceView* GetShadowMapsSRV() const;
	ID3D11ShaderResourceView* GetLightBufferSRV() const;
	ID3D11Buffer* GetLightCameraConstantBuffer() const;
};

