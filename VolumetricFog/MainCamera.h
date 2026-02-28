#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <string>

#include "ConstantBufferD3D11.h"

struct MatrixInfo
{
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 forward = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float fovAngleY = 0.0f;
	float aspectRatio = 0.0f;
	float nearZ = 0.0f;
	float farZ = 0.0f;
};

class MainCamera
{
private:
	DirectX::XMFLOAT4X4 worldMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 viewProjMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 right = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }; // Roll / Pitch / Yaw

	MatrixInfo cameraInfo;
	ConstantBufferD3D11 cameraBuffer;

	DirectX::BoundingFrustum frustum;

	const float movementSpeed = 5.0f;
	const float mouseSensitivity = 2.0f;

	void MoveInDirection(float amount, const DirectX::XMFLOAT3& direction);
	void RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis);

	void CreateMatrices();

	void TransformFrustum();
	DirectX::XMFLOAT3 oldPosition = { 0.0f, 0.0f, 0.0f };

public:
	MainCamera() = default;
	MainCamera(ID3D11Device* device, const MatrixInfo& matrixInfo);
	~MainCamera() = default;
	MainCamera(const MainCamera& other) = delete;
	MainCamera& operator=(const MainCamera& other) = delete;
	MainCamera(MainCamera&& other) = default;
	MainCamera& operator=(MainCamera&& other) = default;

	void Initialize(ID3D11Device* device, const MatrixInfo& matrixInfo);

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateForward(float amount);
	void RotateRight(float amount);
	void RotateUp(float amount);

	const DirectX::XMFLOAT3& GetPosition() const;
	const DirectX::XMFLOAT3& GetForward() const;
	const DirectX::XMFLOAT3& GetRight() const;
	const DirectX::XMFLOAT3& GetUp() const;
	MatrixInfo GetMatrixInfo();
	DirectX::BoundingFrustum GetFrustum();

	void SetPosition(DirectX::XMFLOAT3 setPosition);
	void SetRotation(DirectX::XMFLOAT3 forwardVec, DirectX::XMFLOAT3 rightVec, DirectX::XMFLOAT3 upVec);

	DirectX::XMFLOAT4X4 GetWorldMatrix() const;
	DirectX::XMFLOAT4X4 GetViewProjectionMatrix() const;

	ID3D11Buffer* GetConstantBuffer() const;
	void UpdateInternalConstantBuffer(ID3D11DeviceContext* context);
};
