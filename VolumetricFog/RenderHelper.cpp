#include "RenderHelper.h"

void Render(ID3D11DeviceContext* context, D3D11_VIEWPORT& viewport, ShaderD3D11* vertexShader, ShaderD3D11* pixelShader, 
	ID3D11InputLayout* inputLayout, ID3D11SamplerState** meshSamplerState, MeshD3D11* meshes, size_t totalMeshes,
	QuadTree<MeshD3D11>* quadTree, DirectX::BoundingFrustum* cameraFrustum)
{
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	vertexShader->BindShader(context);
	context->RSSetViewports(1, &viewport);
	pixelShader->BindShader(context);

	//RenderMeshes(context, meshes, totalMeshes, aMeshSampler);
	RenderMeshes(context, meshes, totalMeshes, meshSamplerState, quadTree, cameraFrustum);
}

void RenderMeshes(ID3D11DeviceContext* immediateContext, MeshD3D11* meshes, size_t totalMeshes,
	ID3D11SamplerState** meshSamplerState)
{
	for (int i = 0; i < totalMeshes; i++)
	{
		meshes[i].BindMeshBuffers(immediateContext);
		for (int j = 0; j < meshes[i].GetNrOfSubMeshes(); j++)
		{
			ID3D11ShaderResourceView* ambientSRV = meshes[i].GetAmbientSRV(j);
			ID3D11ShaderResourceView* diffuseSRV = meshes[i].GetDiffuseSRV(j);
			ID3D11ShaderResourceView* specularSRV = meshes[i].GetSpecularSRV(j);

			immediateContext->PSSetShaderResources(0, 1, &ambientSRV);
			immediateContext->PSSetShaderResources(1, 1, &diffuseSRV);
			immediateContext->PSSetShaderResources(2, 1, &specularSRV);
			immediateContext->PSSetSamplers(1, 1, meshSamplerState);
			meshes[i].PerformSubMeshDrawCall(immediateContext, j);
		}
	}
}

void RenderMeshes(ID3D11DeviceContext* context, MeshD3D11* meshes, size_t totalMeshes, ID3D11SamplerState** meshSamplerState,
	QuadTree<MeshD3D11>* quadTree, DirectX::BoundingFrustum* cameraFrustum)
{
	std::vector<const MeshD3D11*> toBeRendered;
	toBeRendered = quadTree->CheckTree(*cameraFrustum);

	for (int i = 0; i < toBeRendered.size(); i++)
	{
		toBeRendered[i]->BindMeshBuffers(context);
		for (int j = 0; j < toBeRendered[i]->GetNrOfSubMeshes(); j++)
		{
			ID3D11ShaderResourceView* ambientSRV = toBeRendered[i]->GetAmbientSRV(j);
			ID3D11ShaderResourceView* diffuseSRV = toBeRendered[i]->GetDiffuseSRV(j);
			ID3D11ShaderResourceView* specularSRV = toBeRendered[i]->GetSpecularSRV(j);

			context->PSSetShaderResources(0, 1, &ambientSRV);
			context->PSSetShaderResources(1, 1, &diffuseSRV);
			context->PSSetShaderResources(2, 1, &specularSRV);
			context->PSSetSamplers(1, 1, meshSamplerState);
			toBeRendered[i]->PerformSubMeshDrawCall(context, j);
		}
	}
}

void RenderReflectiveMesh(ID3D11DeviceContext* context, MeshD3D11* reflectiveMesh, ID3D11SamplerState** meshSamplerState)
{
	reflectiveMesh->BindMeshBuffers(context);
	for (int j = 0; j < reflectiveMesh->GetNrOfSubMeshes(); j++)
	{
		ID3D11ShaderResourceView* ambientSRV = reflectiveMesh->GetAmbientSRV(j);
		ID3D11ShaderResourceView* specularSRV = reflectiveMesh->GetSpecularSRV(j);

		context->PSSetShaderResources(1, 1, &ambientSRV);
		context->PSSetShaderResources(2, 1, &specularSRV);
		context->PSSetSamplers(1, 1, meshSamplerState);
		reflectiveMesh->PerformSubMeshDrawCall(context, j);
	}
}

void RenderTessellatedMesh(ID3D11DeviceContext* context, MeshD3D11* tessellationMesh, ID3D11SamplerState** meshSamplerState,
	ShaderD3D11* tessellationHS, ShaderD3D11* tessellationDS, ConstantBufferD3D11* tessellationPositions,
	ConstantBufferD3D11* particleConstantBuffer)
{
	tessellationHS->BindShader(context);
	ID3D11Buffer* buffer = tessellationPositions->GetBuffer();
	context->HSSetConstantBuffers(0, 1, &buffer);

	tessellationDS->BindShader(context);
	buffer = particleConstantBuffer->GetBuffer();
	context->DSSetConstantBuffers(0, 1, &buffer);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	tessellationMesh->BindMeshBuffers(context);
	for (int i = 0; i < tessellationMesh->GetNrOfSubMeshes(); i++)
	{
		ID3D11ShaderResourceView* ambientSRV = tessellationMesh[i].GetAmbientSRV(i);
		ID3D11ShaderResourceView* diffuseSRV = tessellationMesh[i].GetDiffuseSRV(i);
		ID3D11ShaderResourceView* specularSRV = tessellationMesh[i].GetSpecularSRV(i);

		context->PSSetShaderResources(0, 1, &ambientSRV);
		context->PSSetShaderResources(1, 1, &diffuseSRV);
		context->PSSetShaderResources(2, 1, &specularSRV);
		context->PSSetSamplers(1, 1, meshSamplerState);
		tessellationMesh->PerformSubMeshDrawCall(context, i);
	}
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void RenderShadowMaps(ID3D11DeviceContext* context, ID3D11InputLayout* inputLayout, SpotLightCollectionD3D11* spotLights, 
	ShaderD3D11* shadowVS, D3D11_VIEWPORT* cubeView, MeshD3D11* meshes, size_t totalMeshes, MeshD3D11* reflectiveMesh, 
	DirectionalLight* directionLight, MeshD3D11* tessellationMesh)
{
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shadowVS->BindShader(context);
	context->PSSetShader(nullptr, nullptr, 0);
	context->RSSetViewports(1, cubeView);

	// Spot lights
	for (int i = 0; i < spotLights->GetNrOfLights(); i++)
	{
		ID3D11DepthStencilView* dsView = spotLights->GetShadowMapDSV(i);
		context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);
		context->OMSetRenderTargets(0, nullptr, dsView);

		ID3D11Buffer* cameraConstantBuffer = spotLights->GetLightCameraConstantBuffer(i);
		context->VSSetConstantBuffers(3, 1, &cameraConstantBuffer);

		for (int j = 0; j < totalMeshes; j++)
		{
			meshes[j].BindMeshBuffers(context);
			for (int k = 0; k < meshes[j].GetNrOfSubMeshes(); k++)
			{
				meshes[j].PerformSubMeshDrawCall(context, k);
			}
		}

		reflectiveMesh->BindMeshBuffers(context);
		for (int j = 0; j < reflectiveMesh->GetNrOfSubMeshes(); j++)
		{
			reflectiveMesh->PerformSubMeshDrawCall(context, j);
		}

		tessellationMesh->BindMeshBuffers(context);
		for (int j = 0; j < tessellationMesh->GetNrOfSubMeshes(); j++)
		{
			tessellationMesh->PerformSubMeshDrawCall(context, j);
		}

		context->OMSetRenderTargets(0, nullptr, nullptr);
	}

	// Directional light
	ID3D11DepthStencilView* dsView = directionLight->GetShadowMapDSV();
	context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);
	context->OMSetRenderTargets(0, nullptr, dsView);

	ID3D11Buffer* cameraConstantBuffer = directionLight->GetLightCameraConstantBuffer();
	context->VSSetConstantBuffers(3, 1, &cameraConstantBuffer);

	for (int i = 0; i < totalMeshes; i++)
	{
		meshes[i].BindMeshBuffers(context);
		for (int j = 0; j < meshes[i].GetNrOfSubMeshes(); j++)
		{
			meshes[i].PerformSubMeshDrawCall(context, j);
		}
	}

	reflectiveMesh->BindMeshBuffers(context);
	for (int j = 0; j < reflectiveMesh->GetNrOfSubMeshes(); j++)
	{
		reflectiveMesh->PerformSubMeshDrawCall(context, j);
	}

	tessellationMesh->BindMeshBuffers(context);
	for (int j = 0; j < tessellationMesh->GetNrOfSubMeshes(); j++)
	{
		tessellationMesh->PerformSubMeshDrawCall(context, j);
	}

	context->OMSetRenderTargets(0, nullptr, nullptr);
}

void RenderReflection(ID3D11DeviceContext* context, ShaderD3D11* reflectiveCubeVS, ID3D11Buffer** cubeCamBuffer,
	D3D11_VIEWPORT* cubeView, DepthBufferD3D11& cubeDepth, ID3D11SamplerState** meshSamplerState, MeshD3D11* meshes, size_t totalMeshes,
	ID3D11ShaderResourceView** cubeSRVg, ID3D11RenderTargetView** cubeRTVg, const UINT NR_OF_GBUFFERS,
	ID3D11UnorderedAccessView** cubeUAVs, ShaderD3D11* reflectiveCubeCS, ShaderD3D11* deferredPS, SpotLightCollectionD3D11* spotLights,
	SamplerD3D11* shadowSampler, DirectionalLight* directionLight)
{
	reflectiveCubeVS->BindShader(context);
	deferredPS->BindShader(context);
	reflectiveCubeCS->BindShader(context);
	context->RSSetViewports(1, cubeView);

	// Set shadow components
	ID3D11ShaderResourceView* shadowSRV = spotLights->GetShadowMapsSRV();
	context->CSSetShaderResources(1, 1, &shadowSRV);
	ID3D11ShaderResourceView* dirShadowSRV = directionLight->GetShadowMapsSRV();
	context->CSSetShaderResources(8, 1, &dirShadowSRV);
	ID3D11SamplerState* shadowMapSampler = shadowSampler->GetSamplerState();
	context->CSSetSamplers(0, 1, &shadowMapSampler);

	ID3D11ShaderResourceView* nullSRV[3] = { nullptr };
	context->PSSetShaderResources(0, 3, nullSRV);

	// For every side of the reflective cube
	for (int i = 0; i < 6; i++)
	{
		// Create RTV arrays and SRV arrays
		ID3D11RenderTargetView** rtvArr = new ID3D11RenderTargetView * [NR_OF_GBUFFERS];
		ID3D11ShaderResourceView** srvArr = new ID3D11ShaderResourceView * [NR_OF_GBUFFERS];
		ID3D11ShaderResourceView** nullSrv = new ID3D11ShaderResourceView * [NR_OF_GBUFFERS];
		ID3D11RenderTargetView** nullRtv = new ID3D11RenderTargetView * [NR_OF_GBUFFERS];

		float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int i = 0; i < NR_OF_GBUFFERS; i++)
		{
			context->ClearRenderTargetView(cubeRTVg[i], clearColour);

			rtvArr[i] = cubeRTVg[i];
			srvArr[i] = cubeSRVg[i];
			nullSrv[i] = nullptr;
			nullRtv[i] = nullptr;
		}
		context->ClearDepthStencilView(cubeDepth.GetDSV(0), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
		context->VSSetConstantBuffers(3, 1, &cubeCamBuffer[i]);

		// Bind RTV's
		context->OMSetRenderTargets(NR_OF_GBUFFERS, rtvArr, cubeDepth.GetDSV(0));

		// Render meshes
		RenderMeshes(context, meshes, totalMeshes, meshSamplerState);

		// Unbind RTV's
		context->OMSetRenderTargets(NR_OF_GBUFFERS, nullRtv, nullptr);

		// Bind the compute shader, SRV's and UAV for the CS.
		context->CSSetShaderResources(2, NR_OF_GBUFFERS, srvArr);
		context->CSSetUnorderedAccessViews(0, 1, &cubeUAVs[i], nullptr);
		context->Dispatch(128, 128, 1); // X = Y = 1024/8 = 128

		// Unbind the G-buffer SRV's and UAV
		ID3D11UnorderedAccessView* nullUav = nullptr;
		context->CSSetShaderResources(2, NR_OF_GBUFFERS, nullSrv);
		context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

		delete[] srvArr;
		delete[] rtvArr;
		delete[] nullSrv;
		delete[] nullRtv;
	}
}

void DeferredRendering(ID3D11DeviceContext* context, DepthBufferD3D11* depthStencil, ShaderD3D11* deferredCS,
	ID3D11ShaderResourceView** DRsrv, ID3D11RenderTargetView** DRrtv, const UINT NR_OF_GBUFFERS, ID3D11UnorderedAccessView*& DRuav,
	D3D11_VIEWPORT& viewport, ShaderD3D11* vertexShader, ShaderD3D11* pixelShader, ID3D11InputLayout* inputLayout,
	ID3D11SamplerState** meshSamplerState, MeshD3D11* meshes, size_t totalMeshes, MeshD3D11* reflectiveMesh,
	SpotLightCollectionD3D11* spotLights, SamplerD3D11* shadowSampler, DirectionalLight* directionLight,
	ID3D11RenderTargetView* rtv, ShaderD3D11* reflectiveCubePS, ID3D11Buffer*& camPosBuffer, ID3D11ShaderResourceView*& cubeTextureSRV,
	ID3D11SamplerState* cubeSamplerState, StructuredBufferD3D11* particleBuffer, ShaderD3D11* particleCS, ShaderD3D11* particleVS,
	ShaderD3D11* particleGS, ShaderD3D11* particlePS, ConstantBufferD3D11* particleConstantBuffer,
	ShaderD3D11* tessellationHS, ShaderD3D11* tessellationDS, MeshD3D11* tessellationMesh, ConstantBufferD3D11* tessellationPositions,
	ID3D11InputLayout* inputLayoutCulling, ShaderD3D11* cullingVS, ShaderD3D11* cullingPS, MeshD3D11* frustumMesh,
	ConstantBufferD3D11* frustumCbuffer, QuadTree<MeshD3D11>* quadTree, DirectX::BoundingFrustum* cameraFrustum,
	MeshD3D11* meshBoundingBoxLines, ShaderD3D11 *volFogRayCS, ConstantBufferD3D11 *rayConstBuffer, ConstantBufferD3D11 *rayConstData)
{
	context->RSSetViewports(1, &viewport);
	vertexShader->BindShader(context);
	context->OMSetRenderTargets(1, &rtv, depthStencil->GetDSV(0));
	reflectiveCubePS->BindShader(context);
	context->PSSetConstantBuffers(2, 1, &camPosBuffer);
	context->PSSetShaderResources(0, 1, &cubeTextureSRV);
	context->PSSetSamplers(0, 1, &cubeSamplerState);

	// Set shadow components
	ID3D11ShaderResourceView* spotShadowSRV = spotLights->GetShadowMapsSRV();
	context->CSSetShaderResources(1, 1, &spotShadowSRV);
	ID3D11ShaderResourceView* dirShadowSRV = directionLight->GetShadowMapsSRV();
	context->CSSetShaderResources(8, 1, &dirShadowSRV);
	ID3D11SamplerState* shadowMapSampler = shadowSampler->GetSamplerState();
	context->CSSetSamplers(0, 1, &shadowMapSampler);

	// Create RTV arrays and SRV arrays
	ID3D11RenderTargetView** rtvArr = new ID3D11RenderTargetView*[NR_OF_GBUFFERS];
	ID3D11ShaderResourceView** srvArr = new ID3D11ShaderResourceView*[NR_OF_GBUFFERS];
	ID3D11ShaderResourceView** nullSrv = new ID3D11ShaderResourceView*[NR_OF_GBUFFERS];
	ID3D11RenderTargetView** nullRtv = new ID3D11RenderTargetView*[NR_OF_GBUFFERS];
	float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < NR_OF_GBUFFERS; i++)
	{
		context->ClearRenderTargetView(DRrtv[i], clearColour);

		rtvArr[i] = DRrtv[i];
		srvArr[i] = DRsrv[i];
		nullSrv[i] = nullptr;
		nullRtv[i] = nullptr;
	}
	context->ClearDepthStencilView(depthStencil->GetDSV(0), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	// Bind RTV's
	context->OMSetRenderTargets(NR_OF_GBUFFERS, rtvArr, depthStencil->GetDSV(0));

	// Render meshes
	RenderReflectiveMesh(context, reflectiveMesh, meshSamplerState);
	RenderParticles(context, particleBuffer, particleCS, particleVS, particleGS, particlePS, particleConstantBuffer);
	if (GetKeyState('B'))
	{
		RenderCullingBoxes(context, inputLayoutCulling, cullingVS, cullingPS, deferredCS, frustumMesh, frustumCbuffer,
			meshBoundingBoxLines, totalMeshes);
	}
	Render(context, viewport, vertexShader, pixelShader, inputLayout, meshSamplerState, meshes, totalMeshes, quadTree, cameraFrustum);
	RenderTessellatedMesh(context, tessellationMesh, meshSamplerState, tessellationHS, tessellationDS, tessellationPositions,
		particleConstantBuffer);

	deferredCS->BindShader(context);

	// Unbind RTV's
	context->OMSetRenderTargets(NR_OF_GBUFFERS, nullRtv, nullptr);

	// Bind the compute shader, SRV's and UAV for the CS.
	context->CSSetShaderResources(2, NR_OF_GBUFFERS, srvArr);
	context->CSSetUnorderedAccessViews(0, 1, &DRuav, nullptr);
	context->Dispatch(240, 135, 1); // X = 1920 / 8 = 240, Y = 1080 / 8 = 135

	// Ray-marching volumetric fog
	ID3D11ShaderResourceView *rayDepthSRV = depthStencil->GetSRV();
	ID3D11Buffer *rayBuffer = rayConstBuffer->GetBuffer();
	ID3D11Buffer *rayData = rayConstData->GetBuffer();
	context->CSSetShaderResources(2, 1, &rayDepthSRV);
	volFogRayCS->BindShader(context);
	context->CSSetConstantBuffers(8, 1, &rayBuffer);
	context->CSSetConstantBuffers(9, 1, &rayData);
	context->Dispatch(240, 135, 1); // X = 1920 / 8 = 240, Y = 1080 / 8 = 135

	// Unbind the G-buffer SRV's and UAV
	ID3D11UnorderedAccessView* nullUav = nullptr;
	context->CSSetShaderResources(2, NR_OF_GBUFFERS, nullSrv);
	context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

	context->CSSetShaderResources(1, 1, &nullSrv[0]);
	context->CSSetShaderResources(8, 1, &nullSrv[0]);
	
	delete[] srvArr;
	delete[] rtvArr;
	delete[] nullSrv;
	delete[] nullRtv;
}

void RenderParticles(ID3D11DeviceContext* context, StructuredBufferD3D11* particleBuffer, ShaderD3D11* particleCS,
	ShaderD3D11* particleVS, ShaderD3D11* particleGS, ShaderD3D11* particlePS, ConstantBufferD3D11* particleConstantBuffer)
{
	particleCS->BindShader(context);
	ID3D11UnorderedAccessView* uav = particleBuffer->GetUAV();
	context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	context->Dispatch(std::ceil(particleBuffer->GetNrOfElements() / 32.0f), 1, 1);

	ID3D11UnorderedAccessView* nullUav = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	particleVS->BindShader(context);
	ID3D11ShaderResourceView* srv = particleBuffer->GetSRV();
	context->VSSetShaderResources(0, 1, &srv);

	particleGS->BindShader(context);
	ID3D11Buffer* buffer = particleConstantBuffer->GetBuffer();
	context->GSSetConstantBuffers(0, 1, &buffer);

	particlePS->BindShader(context);

	context->Draw(particleBuffer->GetNrOfElements(), 0);

	context->CSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	ID3D11ShaderResourceView* nullSrv = nullptr;
	context->VSSetShaderResources(0, 1, &nullSrv);
}

void RenderCullingBoxes(ID3D11DeviceContext* context, ID3D11InputLayout* inputLayoutCulling, ShaderD3D11* cullingVS,
	ShaderD3D11* cullingPS, ShaderD3D11* deferredCS, MeshD3D11* frustumMesh, ConstantBufferD3D11* frustumCbuffer,
	MeshD3D11* meshBoundingBoxLines, size_t totalMeshes)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetInputLayout(inputLayoutCulling);
	cullingVS->BindShader(context);
	cullingPS->BindShader(context);
	deferredCS->BindShader(context);

	ID3D11Buffer* buffer = frustumCbuffer->GetBuffer();
	context->VSSetConstantBuffers(9, 1, &buffer);

	frustumMesh->BindMeshBuffers(context);
	frustumMesh->PerformSubMeshDrawCall(context, 0);

	for (int i = 0; i < totalMeshes + 2; i++)
	{
		meshBoundingBoxLines[i].BindMeshBuffers(context);
		meshBoundingBoxLines[i].PerformSubMeshDrawCall(context, 0);
	}
}
