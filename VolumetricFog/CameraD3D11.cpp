#include "CameraD3D11.h"

CameraD3D11::CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo,
	const DirectX::XMFLOAT3& initialPosition)
{
	this->Initialize(device, projectionInfo, initialPosition);
}

void CameraD3D11::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
	DirectX::XMVECTOR dir = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction)), amount);
	DirectX::XMVECTOR thisPos = XMLoadFloat3(&this->position);
	DirectX::XMVECTOR newVector = DirectX::XMVectorAdd(dir, thisPos);

	DirectX::XMStoreFloat3(&this->position, newVector);
}

void CameraD3D11::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
{
	DirectX::XMVECTOR axisVec = DirectX::XMLoadFloat3(&axis);
 	DirectX::XMMATRIX rotate = DirectX::XMMatrixRotationAxis(axisVec, amount);

	DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&this->forward);
	DirectX::XMVECTOR rightVec = DirectX::XMLoadFloat3(&this->right);
	DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&this->up);

	DirectX::XMVECTOR newForward = DirectX::XMVector3Transform(forwardVec, rotate);
	DirectX::XMVECTOR newRight = DirectX::XMVector3Transform(rightVec, rotate);
	DirectX::XMVECTOR newUp = DirectX::XMVector3Transform(upVec, rotate);

	DirectX::XMStoreFloat3(&this->forward, newForward);
	DirectX::XMStoreFloat3(&this->right, newRight);
	DirectX::XMStoreFloat3(&this->up, newUp);
}

void CameraD3D11::Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo,
	const DirectX::XMFLOAT3& initialPosition)
{
	this->projInfo = projectionInfo;
	this->position = initialPosition;

	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-this->position.x, -this->position.y, -this->position.z);

	translation = DirectX::XMMatrixTranspose(translation);
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, translation);

	DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
	DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&this->forward); // Camera direction
	DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&this->up); // Up direction of camera
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	float fov = this->projInfo.fovAngleY;
	float aspectRatio = this->projInfo.aspectRatio;
	float nearZ = this->projInfo.nearZ;
	float farZ = this->projInfo.farZ;
	DirectX::XMMATRIX perspective = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, perspective); // View before projection
	viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 viewProjFloat;
	DirectX::XMStoreFloat4x4(&viewProjFloat, viewProjMatrix);

	DirectX::XMFLOAT4X4 matrices[2] = { worldMatrix, viewProjFloat };

	this->cameraBuffer.Initialize(device, sizeof(matrices), &matrices);
}

void CameraD3D11::InitializeOrthographic(ID3D11Device* device, const OrthographicInfo& orthographicInfo,
	const DirectX::XMFLOAT3& initialPosition)
{
	this->orthInfo = orthographicInfo;
	this->position = initialPosition;

	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-this->position.x, -this->position.y, -this->position.z);

	translation = DirectX::XMMatrixTranspose(translation);
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, translation);

	DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
	DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&this->forward); // Camera direction
	DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&this->up); // Up direction of camera
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	float width = this->orthInfo.width;
	float height = this->orthInfo.height;
	float nearZ = this->orthInfo.nearZ;
	float farZ = this->orthInfo.farZ;
	DirectX::XMMATRIX orthographic = DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ);

	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, orthographic); // View before projection
	viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 viewProjFloat;
	DirectX::XMStoreFloat4x4(&viewProjFloat, viewProjMatrix);

	DirectX::XMFLOAT4X4 matrices[2] = { worldMatrix, viewProjFloat };

	this->cameraBuffer.Initialize(device, sizeof(matrices), &matrices);
}

void CameraD3D11::MoveForward(float amount)
{
	this->MoveInDirection(amount, this->forward);
}

void CameraD3D11::MoveRight(float amount)
{
	this->MoveInDirection(amount, this->right);
}

void CameraD3D11::MoveUp(float amount)
{
	this->MoveInDirection(amount, this->up);
}

void CameraD3D11::RotateForward(float amount)
{
	this->RotateAroundAxis(amount, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
}

void CameraD3D11::RotateRight(float amount)
{
	this->RotateAroundAxis(amount, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f));
}

void CameraD3D11::RotateUp(float amount)
{
	this->RotateAroundAxis(amount, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
}

const DirectX::XMFLOAT3& CameraD3D11::GetPosition() const
{
	return this->position;
}

const DirectX::XMFLOAT3& CameraD3D11::GetForward() const
{
	return this->forward;
}

const DirectX::XMFLOAT3& CameraD3D11::GetRight() const
{
	return this->right;
}

const DirectX::XMFLOAT3& CameraD3D11::GetUp() const
{
	return this->up;
}

void CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	DirectX::XMFLOAT4X4 viewProj = this->GetViewProjectionMatrix();

	HRESULT hr = context->Map(this->cameraBuffer.GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Deny GPU access to mappedResource
	CopyMemory(mappedResource.pData, &viewProj, sizeof(viewProj));
	context->Unmap(this->cameraBuffer.GetBuffer(), 0); // Reenable GPU access to mappedResource
}

ID3D11Buffer* CameraD3D11::GetConstantBuffer() const
{
	return this->cameraBuffer.GetBuffer();
}

DirectX::XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
	DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
	DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&this->forward); // Camera direction
	DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&this->up); // Up direction of camera
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	float fov = this->projInfo.fovAngleY;
	float aspectRatio = this->projInfo.aspectRatio;
	float nearZ = this->projInfo.nearZ;
	float farZ = this->projInfo.farZ;
	DirectX::XMMATRIX perspective = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, perspective); // View before projection
	viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 viewProjFloat;
	DirectX::XMStoreFloat4x4(&viewProjFloat, viewProjMatrix);
	return viewProjFloat;
}
