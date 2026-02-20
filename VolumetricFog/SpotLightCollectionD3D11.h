#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"

struct SpotLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowMapInfo;

	struct PerLightInfo
	{
		DirectX::XMFLOAT3 colour;
		float rotationX = 0.0f;
		float rotationY = 0.0f;
		float outAngle = 0.0f;
		float inAngle = 0.0f;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
		DirectX::XMFLOAT3 initialPosition;
	};

	std::vector<PerLightInfo> perLightInfo;
};

class SpotLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		DirectX::XMFLOAT4X4 vpMatrix;
		DirectX::XMFLOAT3 colour = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 direction = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		float outAngle = 0.0f; // Angle from the direction vector to the edge of the light cone
		float inAngle = 0.0f; // Angle from the edge of the light cone to the edge of where the light starts to attenuate
		DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		float range = 0.0f;
	};

	std::vector<LightBuffer> bufferData;

	DepthBufferD3D11 shadowMaps;
	StructuredBufferD3D11 lightBuffer;
	std::vector<CameraD3D11> shadowCameras;

	void InitializeShadowMaps(ID3D11Device* device);

public:
	SpotLightCollectionD3D11() = default;
	//~SpotLightCollectionD3D11() = default;
	~SpotLightCollectionD3D11();
	SpotLightCollectionD3D11(const SpotLightCollectionD3D11& other) = delete;
	SpotLightCollectionD3D11& operator=(const SpotLightCollectionD3D11& other) = delete;
	SpotLightCollectionD3D11(SpotLightCollectionD3D11&& other) = delete;
	SpotLightCollectionD3D11& operator=(DepthBufferD3D11 && other) = delete;

	void Initialize(ID3D11Device* device, const SpotLightData& lightInfo);

	void UpdateLightBuffers(ID3D11DeviceContext* context);

	UINT GetNrOfLights() const;
	ID3D11DepthStencilView* GetShadowMapDSV(UINT lightIndex) const;
	ID3D11ShaderResourceView* GetShadowMapsSRV() const;
	ID3D11ShaderResourceView* GetLightBufferSRV() const;
	ID3D11Buffer* GetLightCameraConstantBuffer(UINT lightIndex) const;
};