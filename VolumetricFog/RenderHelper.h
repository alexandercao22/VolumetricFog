#pragma once

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

void Render(ID3D11DeviceContext* context, D3D11_VIEWPORT& viewport, ShaderD3D11* vertexShader, ShaderD3D11* pixelShader,
	ID3D11InputLayout* inputLayout, ID3D11SamplerState** meshSamplerState, MeshD3D11* meshes, size_t totalMeshes,
	QuadTree<MeshD3D11>* quadTree, DirectX::BoundingFrustum* cameraFrustum);

void RenderMeshes(ID3D11DeviceContext* immediateContext, MeshD3D11* meshes, size_t totalMeshes,
	ID3D11SamplerState** meshSamplerState);

void RenderMeshes(ID3D11DeviceContext* context, MeshD3D11* meshes, size_t totalMeshes, ID3D11SamplerState** meshSamplerState,
	QuadTree<MeshD3D11>* quadTree, DirectX::BoundingFrustum* cameraFrustum);

void RenderReflectiveMesh(ID3D11DeviceContext* context, MeshD3D11* reflectiveMesh, ID3D11SamplerState** meshSamplerState);

void RenderTessellatedMesh(ID3D11DeviceContext* context, MeshD3D11* tessellationMesh, ID3D11SamplerState** meshSamplerState,
	ShaderD3D11* tessellationHS, ShaderD3D11* tessellationDS, ConstantBufferD3D11* tessellationPositions,
	ConstantBufferD3D11* particleConstantBuffer);

void RenderShadowMaps(ID3D11DeviceContext* context, ID3D11InputLayout* inputLayout, SpotLightCollectionD3D11* spotLights,
	ShaderD3D11* shadowVS, D3D11_VIEWPORT* cubeView, MeshD3D11* meshes, size_t totalMeshes, MeshD3D11* reflectiveMesh,
	DirectionalLight* directionLight, MeshD3D11* tessellationMesh);

void RenderReflection(ID3D11DeviceContext* context, ShaderD3D11* reflectiveCubeVS, ID3D11Buffer** cubeCamBuffer,
	D3D11_VIEWPORT* cubeView, DepthBufferD3D11& cubeDepth, ID3D11SamplerState** meshSamplerState, MeshD3D11* meshes, size_t totalMeshes,
	ID3D11ShaderResourceView** cubeSRVg, ID3D11RenderTargetView** cubeRTVg, const UINT NR_OF_GBUFFERS,
	ID3D11UnorderedAccessView** cubeUAVs, ShaderD3D11* reflectiveCubeCS, ShaderD3D11* deferredPS, SpotLightCollectionD3D11* spotLights,
	SamplerD3D11* shadowSampler, DirectionalLight* directionLight);

void DeferredRendering(ID3D11DeviceContext *context, DepthBufferD3D11 *depthStencil, ShaderD3D11 *deferredCS,
	ID3D11ShaderResourceView **DRsrv, ID3D11RenderTargetView **DRrtv, const UINT NR_OF_GBUFFERS, ID3D11UnorderedAccessView *&DRuav,
	D3D11_VIEWPORT &viewport, ShaderD3D11 *vertexShader, ShaderD3D11 *pixelShader, ID3D11InputLayout *inputLayout,
	ID3D11SamplerState **meshSamplerState, MeshD3D11 *meshes, size_t totalMeshes, MeshD3D11 *reflectiveMesh,
	SpotLightCollectionD3D11 *spotLights, SamplerD3D11 *shadowSampler, DirectionalLight *directionLight,
	ID3D11RenderTargetView *rtv, ShaderD3D11 *reflectiveCubePS, ID3D11Buffer *&camPosBuffer, ID3D11ShaderResourceView *&cubeTextureSRV,
	ID3D11SamplerState *cubeSamplerState, StructuredBufferD3D11 *particleBuffer, ShaderD3D11 *particleCS, ShaderD3D11 *particleVS,
	ShaderD3D11 *particleGS, ShaderD3D11 *particlePS, ConstantBufferD3D11 *particleConstantBuffer, ConstantBufferD3D11 *particleDeltaTime,
	ShaderD3D11 *tessellationHS, ShaderD3D11 *tessellationDS, MeshD3D11 *tessellationMesh, ConstantBufferD3D11 *tessellationPositions,
	ID3D11InputLayout *inputLayoutCulling, ShaderD3D11 *cullingVS, ShaderD3D11 *cullingPS, MeshD3D11 *frustumMesh,
	ConstantBufferD3D11 *frustumCbuffer, QuadTree<MeshD3D11> *quadTree, DirectX::BoundingFrustum *cameraFrustum,
	MeshD3D11 *meshBoundingBoxLines, ShaderD3D11 *volFogRayCS, ConstantBufferD3D11 *rayConstBuffer, ConstantBufferD3D11 *rayConstData);

void RenderParticles(ID3D11DeviceContext *context, StructuredBufferD3D11 *particleBuffer, ShaderD3D11 *particleCS,
	ShaderD3D11 *particleVS, ShaderD3D11 *particleGS, ShaderD3D11 *particlePS,
	ConstantBufferD3D11 *particleConstantBuffer, ConstantBufferD3D11 *particleDeltaTime);

void RenderCullingBoxes(ID3D11DeviceContext* context, ID3D11InputLayout* inputLayoutCulling, ShaderD3D11* cullingVS,
	ShaderD3D11* cullingPS, ShaderD3D11* deferredCS, MeshD3D11* frustumMesh, ConstantBufferD3D11* frustumCbuffer,
	MeshD3D11* meshBoundingBoxLines, size_t totalMeshes);
