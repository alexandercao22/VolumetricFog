#include "SpotLightCollectionD3D11.h"

void SpotLightCollectionD3D11::InitializeShadowMaps(ID3D11Device* device)
{
	UINT arrSize = this->bufferData.size();

	DepthBufferInfo depthBufferInfo;
	ZeroMemory(&depthBufferInfo, sizeof(DepthBufferInfo));

	// D3D11_TEXTURE2D_DESC
	depthBufferInfo.depthDesc.Width = 1024;
	depthBufferInfo.depthDesc.Height = 1024;
	depthBufferInfo.depthDesc.MipLevels = 1;
	depthBufferInfo.depthDesc.ArraySize = arrSize;
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
	depthBufferInfo.srvDesc.Texture2DArray.ArraySize = arrSize;

	this->shadowMaps.Initialize(device, depthBufferInfo);
}

SpotLightCollectionD3D11::~SpotLightCollectionD3D11()
{
	for (int i = 0; i < this->shadowCameras.size(); i++)
	{
		ID3D11Buffer* releaseCOM = this->shadowCameras[i].GetConstantBuffer();
		releaseCOM->Release();
	}
}

void SpotLightCollectionD3D11::Initialize(ID3D11Device* device, const SpotLightData& lightInfo)
{
	for (int i = 0; i < lightInfo.perLightInfo.size(); i++)
	{
		// Create shadow cameras
		ProjectionInfo projInfo;
		projInfo.fovAngleY = lightInfo.perLightInfo[i].outAngle * 2;
		projInfo.aspectRatio = 1.0f;
		projInfo.nearZ = lightInfo.perLightInfo[i].projectionNearZ;
		projInfo.farZ = lightInfo.perLightInfo[i].projectionFarZ;

		CameraD3D11 shadowCam;
		shadowCam.RotateRight(lightInfo.perLightInfo[i].rotationX);
		shadowCam.RotateUp(lightInfo.perLightInfo[i].rotationY);
		shadowCam.Initialize(device, projInfo, lightInfo.perLightInfo[i].initialPosition);

		LightBuffer lightBuffer;
		lightBuffer.colour = lightInfo.perLightInfo[i].colour;
		DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationRollPitchYaw(lightInfo.perLightInfo[i].rotationX, 
			lightInfo.perLightInfo[i].rotationY, 0);
		DirectX::XMVECTOR directionVec = { 0.0f, 0.0f, 1.0f }; // Default direction
		DirectX::XMStoreFloat3(&lightBuffer.direction, DirectX::XMVector3Transform(directionVec, rotationMat));
		lightBuffer.outAngle = lightInfo.perLightInfo[i].outAngle;
		lightBuffer.inAngle = lightInfo.perLightInfo[i].inAngle;
		lightBuffer.position = lightInfo.perLightInfo[i].initialPosition;
		lightBuffer.range = lightInfo.perLightInfo[i].projectionFarZ;

		DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
		DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&shadowCam.GetForward()); // Camera direction
		DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&shadowCam.GetUp()); // Up direction of camera
		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);
		DirectX::XMMATRIX perspective = DirectX::XMMatrixPerspectiveFovLH(projInfo.fovAngleY, projInfo.aspectRatio, projInfo.nearZ, projInfo.farZ);
		DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, perspective); // View before projection

		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-lightBuffer.position.x, -lightBuffer.position.y, -lightBuffer.position.z);

		viewProjMatrix = DirectX::XMMatrixMultiply(translation, viewProjMatrix);
		viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);

		DirectX::XMStoreFloat4x4(&lightBuffer.vpMatrix, viewProjMatrix);

		this->bufferData.push_back(lightBuffer);

		this->shadowCameras.push_back(std::move(shadowCam));
	}
	// Initialize structured buffer
	this->lightBuffer.Initialize(device, sizeof(LightBuffer), this->bufferData.size(), this->bufferData.data(), false, true, false);

	this->InitializeShadowMaps(device);
}

void SpotLightCollectionD3D11::UpdateLightBuffers(ID3D11DeviceContext* context)
{
	this->lightBuffer.UpdateBuffer(context, this->bufferData.data());
}

UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
	return (UINT)this->bufferData.size();
}

ID3D11DepthStencilView* SpotLightCollectionD3D11::GetShadowMapDSV(UINT lightIndex) const
{
	return this->shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetShadowMapsSRV() const
{
	return this->shadowMaps.GetSRV();
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetLightBufferSRV() const
{
	return this->lightBuffer.GetSRV();
}

ID3D11Buffer* SpotLightCollectionD3D11::GetLightCameraConstantBuffer(UINT lightIndex) const
{
	return this->shadowCameras[lightIndex].GetConstantBuffer();
}
