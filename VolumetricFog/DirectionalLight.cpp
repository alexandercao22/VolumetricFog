#include "DirectionalLight.h"

void DirectionalLight::InitializeShadowMaps(ID3D11Device* device)
{
	DepthBufferInfo depthBufferInfo;
	ZeroMemory(&depthBufferInfo, sizeof(DepthBufferInfo));

	// D3D11_TEXTURE2D_DESC
	depthBufferInfo.depthDesc.Width = 1024;
	depthBufferInfo.depthDesc.Height = 1024;
	depthBufferInfo.depthDesc.MipLevels = 1;
	depthBufferInfo.depthDesc.ArraySize = 1;
	depthBufferInfo.depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthBufferInfo.depthDesc.SampleDesc.Count = 1;
	depthBufferInfo.depthDesc.SampleDesc.Quality = 0;
	depthBufferInfo.depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferInfo.depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferInfo.depthDesc.CPUAccessFlags = 0;
	depthBufferInfo.depthDesc.MiscFlags = 0;

	// D3D11_DEPTH_STENCIL_VIEW_DESC
	depthBufferInfo.dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferInfo.dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthBufferInfo.dsvDesc.Texture2DArray.MipSlice = 0;
	depthBufferInfo.dsvDesc.Texture2DArray.FirstArraySlice = 0;
	depthBufferInfo.dsvDesc.Texture2DArray.ArraySize = 1;
	// Set depthBufferInfo.dsvDesc.Texture2DArray.FirstArraySlice in for loop

	depthBufferInfo.HasSRV = true;

	// D3D11_SHADER_RESOURCE_VIEW_DESC
	depthBufferInfo.srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthBufferInfo.srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	depthBufferInfo.srvDesc.Texture2DArray.MostDetailedMip = 0;
	depthBufferInfo.srvDesc.Texture2DArray.MipLevels = 1;
	depthBufferInfo.srvDesc.Texture2DArray.FirstArraySlice = 0;
	depthBufferInfo.srvDesc.Texture2DArray.ArraySize = 1;

	this->shadowMaps.Initialize(device, depthBufferInfo);
}

DirectionalLight::DirectionalLight(ID3D11Device* device, DirectX::XMFLOAT3 colour, DirectX::XMFLOAT3 direction)
{
	Initialize(device, colour, direction);
}

DirectionalLight::~DirectionalLight()
{
	ID3D11Buffer* releaseCOM = this->shadowCameras.GetConstantBuffer();
	releaseCOM->Release();
}

void DirectionalLight::Initialize(ID3D11Device* device, DirectX::XMFLOAT3 colour, DirectX::XMFLOAT3 direction)
{
	this->bufferData.colour = colour;
	DirectX::XMVECTOR normalizedDir = DirectX::XMLoadFloat3(&direction);
	normalizedDir = DirectX::XMVector3Normalize(normalizedDir);
	DirectX::XMStoreFloat3(&this->bufferData.direction, normalizedDir);

	OrthographicInfo orthInfo;
	orthInfo.width = 60.0f;
	orthInfo.height = 60.0f;
	orthInfo.nearZ = 0.1f;
	orthInfo.farZ = 100.0f;

	DirectX::XMFLOAT3 position = { 
		-50.0f * this->bufferData.direction.x,
		-50.0f * this->bufferData.direction.y,
		-50.0f * this->bufferData.direction.z
	};

	DirectX::XMVECTOR camForward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&this->shadowCameras.GetForward()));

	// Rotate around x-axis
	DirectX::XMVECTOR directionX = { 0.0f, this->bufferData.direction.y, this->bufferData.direction.z };
	directionX = DirectX::XMVector3Normalize(directionX);
	DirectX::XMVECTOR angleVecX = DirectX::XMVector3Dot(directionX, camForward);
	float angleX = DirectX::XMScalarACos(DirectX::XMVectorGetX(angleVecX));
	this->shadowCameras.RotateRight(angleX);

	// Rotate around y-axis
	DirectX::XMVECTOR directionY = { this->bufferData.direction.x, 0.0f, this->bufferData.direction.z };
	directionY = DirectX::XMVector3Normalize(directionY);
	DirectX::XMVECTOR angleVecY = DirectX::XMVector3Dot(directionY, camForward);
	float angleY = DirectX::XMScalarACos(DirectX::XMVectorGetX(angleVecY));
	this->shadowCameras.RotateUp(angleY);

	this->shadowCameras.InitializeOrthographic(device, orthInfo, position);

	DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
	DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&this->shadowCameras.GetForward()); // Camera direction
	DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&this->shadowCameras.GetUp()); // Up direction of camera
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	DirectX::XMMATRIX orthographic = DirectX::XMMatrixOrthographicLH(orthInfo.width, orthInfo.height, orthInfo.nearZ, orthInfo.farZ);

	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, orthographic); // View before projection
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-position.x, -position.y, -position.z);
	viewProjMatrix = DirectX::XMMatrixMultiply(translation, viewProjMatrix);
	viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 viewProjFloat;
	DirectX::XMStoreFloat4x4(&this->bufferData.vpMatrix, viewProjMatrix);

	this->lightBuffer.Initialize(device, sizeof(LightBuffer), 1, &this->bufferData, false, true, false);

	this->InitializeShadowMaps(device);
}

ID3D11DepthStencilView* DirectionalLight::GetShadowMapDSV() const
{
	return this->shadowMaps.GetDSV(0);
}

ID3D11ShaderResourceView* DirectionalLight::GetShadowMapsSRV() const
{
	return this->shadowMaps.GetSRV();
}

ID3D11ShaderResourceView* DirectionalLight::GetLightBufferSRV() const
{
	return this->lightBuffer.GetSRV();
}

ID3D11Buffer* DirectionalLight::GetLightCameraConstantBuffer() const
{
	return this->shadowCameras.GetConstantBuffer();
}