#include "MainCamera.h"

MainCamera::MainCamera(ID3D11Device* device, const MatrixInfo& matrixInfo)
{
	Initialize(device, matrixInfo);
}

void MainCamera::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
	DirectX::XMVECTOR dir = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction)), amount);
	DirectX::XMVECTOR thisPos = XMLoadFloat3(&this->position);
	DirectX::XMVECTOR newVector = DirectX::XMVectorAdd(dir, thisPos);

	DirectX::XMStoreFloat3(&this->position, newVector);
}

void MainCamera::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
{
	// Get the rotation matrix
	DirectX::XMVECTOR axisVec = DirectX::XMLoadFloat3(&axis);
	DirectX::XMMATRIX rotate = DirectX::XMMatrixRotationAxis(axisVec, amount);

	// Covert FLOAT3 to VECTOR
	DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&this->forward);
	DirectX::XMVECTOR rightVec = DirectX::XMLoadFloat3(&this->right);
	DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&this->up);

	// Multiply all vector with the rotation matrix
	DirectX::XMVECTOR newForward = DirectX::XMVector3Transform(forwardVec, rotate);
	DirectX::XMVECTOR newRight = DirectX::XMVector3Transform(rightVec, rotate);
	DirectX::XMVECTOR newUp = DirectX::XMVector3Transform(upVec, rotate);

	// Convert VECTOR to FLOAT3
	DirectX::XMStoreFloat3(&this->forward, newForward);
	DirectX::XMStoreFloat3(&this->right, newRight);
	DirectX::XMStoreFloat3(&this->up, newUp);
}

void MainCamera::CreateMatrices()
{
	// Translation & Rotation
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-this->position.x, -this->position.y, -this->position.z);
	translation = DirectX::XMMatrixTranspose(translation);

	DirectX::XMStoreFloat4x4(&this->worldMatrix, translation);

	// View
	DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, 0.0f }; // Camera pos
	DirectX::XMVECTOR focusPos = DirectX::XMLoadFloat3(&this->forward); // Camera direction
	DirectX::XMVECTOR upDir = DirectX::XMLoadFloat3(&this->up); // Up direction of camera
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	// Projection
	float fov = this->cameraInfo.fovAngleY;
	float aspectRatio = this->cameraInfo.aspectRatio;
	float nearZ = this->cameraInfo.nearZ;
	float farZ = this->cameraInfo.farZ;
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

	// Multiply & Store to float4x4
	DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(view, projection); // View before projection

	viewProjMatrix = DirectX::XMMatrixTranspose(viewProjMatrix);
	DirectX::XMStoreFloat4x4(&this->viewProjMatrix, viewProjMatrix);
}

void MainCamera::TransformFrustum()
{
	if (!GetKeyState('V'))
	{
		this->oldPosition = this->position;
		this->frustum.Origin = this->position;
	}
	else
	{
		this->frustum.Origin = this->oldPosition;
	}

	DirectX::XMMATRIX rotateX = DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&this->right), -this->rotation.x);
	DirectX::XMMATRIX rotateY = DirectX::XMMatrixRotationAxis({ 0.0f, 1.0f, 0.0f }, -this->rotation.y);
	DirectX::XMMATRIX rotation = DirectX::XMMatrixMultiply(rotateX, rotateY);
	rotation = DirectX::XMMatrixTranspose(rotation);

	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionNormalize(DirectX::XMLoadFloat4(&this->frustum.Orientation));
	DirectX::XMStoreFloat4(&this->frustum.Orientation, quaternion);

	this->frustum.Transform(this->frustum, rotation);
}

void MainCamera::Initialize(ID3D11Device* device, const MatrixInfo& matrixInfo)
{
	this->cameraInfo = matrixInfo;
	this->position = this->cameraInfo.position;
	this->forward = this->cameraInfo.forward;
	this->right = this->cameraInfo.right;
	this->up = this->cameraInfo.up;

	this->CreateMatrices();

	DirectX::XMFLOAT4X4 matrices[2] = { this->worldMatrix, this->viewProjMatrix };
	this->cameraBuffer.Initialize(device, sizeof(matrices), matrices);

	DirectX::XMMATRIX viewProj = DirectX::XMLoadFloat4x4(&this->viewProjMatrix);
	viewProj = DirectX::XMMatrixTranspose(viewProj);
	DirectX::BoundingFrustum::CreateFromMatrix(this->frustum, viewProj);
}

void MainCamera::MoveForward(float amount)
{
	this->MoveInDirection(amount * movementSpeed, DirectX::XMFLOAT3(this->forward.x, 0.0f, this->forward.z));
}

void MainCamera::MoveRight(float amount)
{
	this->MoveInDirection(amount * movementSpeed, DirectX::XMFLOAT3(this->forward.z, 0.0f, -this->forward.x));
}

void MainCamera::MoveUp(float amount)
{
	this->MoveInDirection(amount * movementSpeed, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
}

void MainCamera::RotateForward(float amount)
{
	amount *= mouseSensitivity;
	this->RotateAroundAxis(amount, this->forward);
	this->rotation.z = amount;
}

void MainCamera::RotateRight(float amount)
{
	amount *= mouseSensitivity;
	this->RotateAroundAxis(amount, this->right);
	this->rotation.x = amount;
}

void MainCamera::RotateUp(float amount)
{
	amount *= mouseSensitivity;
	this->RotateAroundAxis(amount, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	this->rotation.y = amount;
}

const DirectX::XMFLOAT3& MainCamera::GetPosition() const
{
	return this->position;
}

const DirectX::XMFLOAT3& MainCamera::GetForward() const
{
	return this->forward;
}

const DirectX::XMFLOAT3& MainCamera::GetRight() const
{
	return this->right;
}

const DirectX::XMFLOAT3& MainCamera::GetUp() const
{
	return this->up;
}

MatrixInfo MainCamera::GetMatrixInfo()
{
	return this->cameraInfo;
}

DirectX::BoundingFrustum MainCamera::GetFrustum()
{
	return this->frustum;
}

void MainCamera::SetPosition(DirectX::XMFLOAT3 setPosition)
{
	this->position = setPosition;
	this->oldPosition = setPosition;
}

void MainCamera::SetRotation(DirectX::XMFLOAT3 forwardVec, DirectX::XMFLOAT3 rightVec, DirectX::XMFLOAT3 upVec)
{
	this->forward = forwardVec;
	this->right = rightVec;
	this->up = upVec;

	DirectX::XMMATRIX viewProj = DirectX::XMLoadFloat4x4(&this->viewProjMatrix);
	viewProj = DirectX::XMMatrixTranspose(viewProj);
	DirectX::BoundingFrustum::CreateFromMatrix(this->frustum, viewProj);
}

DirectX::XMFLOAT4X4 MainCamera::GetWorldMatrix() const
{
	return this->worldMatrix;
}

DirectX::XMFLOAT4X4 MainCamera::GetViewProjectionMatrix() const
{
	return this->viewProjMatrix;
}

ID3D11Buffer* MainCamera::GetConstantBuffer() const
{
	return this->cameraBuffer.GetBuffer();
}

void MainCamera::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	this->CreateMatrices();

	DirectX::XMFLOAT4X4 matrices[2] = { this->worldMatrix, this->viewProjMatrix };

	HRESULT hr = context->Map(this->cameraBuffer.GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Deny GPU access to mappedResource
	CopyMemory(mappedResource.pData, &matrices, sizeof(matrices));
	context->Unmap(this->cameraBuffer.GetBuffer(), 0); // Reenable GPU access to mappedResource

	this->TransformFrustum();
}