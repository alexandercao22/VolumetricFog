#pragma once

#include <random>

#include "CameraD3D11.h"
#include "ConstantBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "InputLayoutD3D11.h"
#include "MeshD3D11.h"
#include "RenderTargetD3D11.h"
#include "SamplerD3D11.h"
#include "ShaderD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "StructuredBufferD3D11.h"
#include "SubMeshD3D11.h"
#include "VertexBufferD3D11.h"
#include "MainCamera.h"
#include "OBJ_Loader.h"
#include "DirectionalLight.h"
#include "QuadTree.h"

#define PI DirectX::XM_PI		// 180deg
#define PID2 DirectX::XM_PIDIV2 // 90deg
#define PID4 DirectX::XM_PIDIV4 // 45deg

struct ObjInfo
{
	std::string address;
	DirectX::XMMATRIX translation;
	DirectX::XMMATRIX rotation;
	bool translateFirst;
	float scale;
	ObjInfo(std::string address, DirectX::XMVECTOR translation = { 0.0f, 0.0f, 0.0f },
		DirectX::XMVECTOR rotation = { 0.0f, 0.0f, 0.0f }, bool translateFirst = true, float scale = 1.0f)
	{
		this->address = address;
		DirectX::XMVECTOR a = { DirectX::XMVectorGetX(translation), DirectX::XMVectorGetY(translation), DirectX::XMVectorGetZ(translation) };
		this->translation = DirectX::XMMatrixTranslationFromVector(a);
		this->rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(rotation);
		this->translateFirst = translateFirst;
		this->scale = scale;
	}
};

struct CbufferCS
{
	DirectX::XMFLOAT4 camPos;
	UINT totalSpotLights;
	float fullLight;
	float shadows;
	float padding;
};

struct ParticleConstantBufferData
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 vpMatrix;
	DirectX::XMFLOAT3 position;
	float particleSize;
};

struct Positions
{
	DirectX::XMFLOAT4 camPosition;
	DirectX::XMFLOAT4 objPosition;
};

struct MeshBoundPositions
{
	DirectX::XMFLOAT3 min = { (float)INFINITE, (float)INFINITE, (float)INFINITE };
	DirectX::XMFLOAT3 max = { -(float)INFINITE, -(float)INFINITE, -(float)INFINITE };
};

struct BoundingBoxVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 colour;
};

struct RayData
{
	DirectX::XMINT2 resolution;
	float time;
	float deltaTime;
};

void CreateMainCamera(ID3D11DeviceContext* context, ID3D11Device*& device, MainCamera* mainCamera, HWND* window);

void CreateInputLayout(ID3D11Device*& device, InputLayoutD3D11& inputLayout,
	InputLayoutStruct* semanticNames, size_t totalLayouts, ShaderD3D11* deferredVS);

void MainCameraMovement(ID3D11DeviceContext* context, MainCamera* mainCamera, float deltaTime, HWND* window);

void DynamicCubeMapping(ID3D11DeviceContext* context, ID3D11Device*& device, CameraD3D11* cubeCams, ID3D11Buffer** cubeCamBuffer,
	D3D11_VIEWPORT& cubeView, float cubeTextureSize, RenderTargetD3D11* cubeMappingRTV, ID3D11ShaderResourceView*& cubeMappingSRV);

void ObjToMeshes(ID3D11Device*& device, MeshD3D11* mesh, ObjInfo* objInfo, size_t totalMeshes,
	std::vector<MeshBoundPositions>& meshBoundPositions);

void DeferredRenderingSetup(ID3D11DeviceContext* context, ID3D11Device*& device, UINT width, UINT height, const UINT NR_OF_GBUFFERS,
	ID3D11Texture2D** DRtexture, ID3D11ShaderResourceView** DRsrv, ID3D11RenderTargetView** DRrtv,
	ID3D11UnorderedAccessView*& DRuav, IDXGISwapChain*& swapChain);

void SetupLights(ID3D11DeviceContext* context, ID3D11Device*& device, SpotLightCollectionD3D11* spotLights,
	MainCamera* mainCamera, ConstantBufferD3D11* cBufferCS, UINT* totalSpotLights, DirectionalLight* directionLight);

void UpdatePerFrame(ID3D11DeviceContext* context, ID3D11Device*& device, UINT totalSpotLights, MainCamera* mainCamera,
	ConstantBufferD3D11* cBufferCS, ID3D11Buffer*& camPosBuffer, ConstantBufferD3D11* camPosConstBuffer,
	ConstantBufferD3D11* particleConstantBuffer, float particleSize, ConstantBufferD3D11* tessellationPositions,
	DirectX::XMFLOAT4 moveObj, MeshD3D11* frustumMesh, ConstantBufferD3D11* frustumCbuffer, DirectX::BoundingFrustum* cameraFrustum,
	ConstantBufferD3D11 *rayConstData, float time, float deltaTime);

void SetupReflection(ID3D11Device*& device, const UINT NR_OF_GBUFFERS, ID3D11Texture2D** cubeTextureG,
	ID3D11ShaderResourceView** cubeSRVg, ID3D11RenderTargetView** cubeRTVg, ID3D11UnorderedAccessView** cubeUAVs,
	ID3D11Texture2D*& cubeTextures, ID3D11ShaderResourceView*& cubeTextureSRV, UINT size);

void SetupParticles(ID3D11Device*& device, StructuredBufferD3D11* particleBuffer, MainCamera* mainCamera,
	ConstantBufferD3D11* particleConstantBuffer, int nrOfParticles, float particleSize);

void SetupTessellation(ID3D11Device*& device, ConstantBufferD3D11* tessellationPositions, MainCamera* mainCamera,
	DirectX::XMFLOAT4 moveObj);

void SetupCulling(ID3D11Device*& device, MainCamera* mainCamera, QuadTree<MeshD3D11>* quadTree, MeshD3D11* meshes, size_t totalMeshes,
	std::vector<MeshBoundPositions>& meshBoundPositions, MeshD3D11* frustumMesh, DirectX::BoundingFrustum* cameraFrustum,
	ConstantBufferD3D11* frustumCbuffer, MeshD3D11* meshBoundingBoxLines);

void SetupRayMarchingVolFog(ID3D11Device *&device, ConstantBufferD3D11 *rayConstBuffer, MainCamera *mainCamera, 
	ConstantBufferD3D11 *rayConstData, UINT width, UINT height);
