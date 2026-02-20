#include "ShaderD3D11.h"

#include <fstream>
#include <string>
#include <iostream>
#include <d3d10misc.h>

ShaderD3D11::~ShaderD3D11()
{
	if (this->shader.vertex != nullptr && this->type == ShaderType::VERTEX_SHADER)
	{
		this->shader.vertex->Release();
	}
	else if (this->shader.hull != nullptr && this->type == ShaderType::HULL_SHADER)
	{
		this->shader.hull->Release();
	}
	else if (this->shader.domain != nullptr && this->type == ShaderType::DOMAIN_SHADER)
	{
		this->shader.domain->Release();
	}
	else if (this->shader.geometry != nullptr && this->type == ShaderType::GEOMETRY_SHADER)
	{
		this->shader.geometry->Release();
	}
	else if (this->shader.pixel != nullptr && this->type == ShaderType::PIXEL_SHADER)
	{
		this->shader.pixel->Release();
	}
	else if (this->shader.compute != nullptr && this->type == ShaderType::COMPUTE_SHADER)
	{
		this->shader.compute->Release();
	}

	if (this->shaderBlob != nullptr)
	{
		this->shaderBlob->Release();
	}
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize)
{
	Initialize(device, shaderType, dataPtr, dataSize);
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, LPCWSTR wStr)
{
	Initialize(device, shaderType, wStr);
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize)
{
	this->type = shaderType;
	HRESULT hr = S_OK;
	switch (shaderType) 
	{
		case ShaderType::VERTEX_SHADER:
			hr = device->CreateVertexShader(dataPtr, dataSize, nullptr, &this->shader.vertex);
			break;
		case ShaderType::HULL_SHADER:
			hr = device->CreateHullShader(dataPtr, dataSize, nullptr, &this->shader.hull);
			break;
		case ShaderType::DOMAIN_SHADER:
			hr = device->CreateDomainShader(dataPtr, dataSize, nullptr, &this->shader.domain);
			break;
		case ShaderType::GEOMETRY_SHADER:
			hr = device->CreateGeometryShader(dataPtr, dataSize, nullptr, &this->shader.geometry);
			break;
		case ShaderType::PIXEL_SHADER:
			hr = device->CreatePixelShader(dataPtr, dataSize, nullptr, &this->shader.pixel);
			break;
		case ShaderType::COMPUTE_SHADER:
			hr = device->CreateComputeShader(dataPtr, dataSize, nullptr, &this->shader.compute);
			break;
	}
	if (FAILED(hr))
	{
		std::cerr << "Failed to create shader\n";
		return;
	}
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, LPCWSTR wStr)
{
	this->type = shaderType;
	HRESULT hr = D3DReadFileToBlob(wStr, &this->shaderBlob);
	if (FAILED(hr))
	{
		std::cerr << "Failed to read file to blob\n";
		return;
	}
	Initialize(device, this->type, this->shaderBlob->GetBufferPointer(), this->shaderBlob->GetBufferSize());
}

const void* ShaderD3D11::GetShaderByteData() const
{
	return this->shaderBlob->GetBufferPointer();
}

size_t ShaderD3D11::GetShaderByteSize() const
{
	return this->shaderBlob->GetBufferSize();
}

void ShaderD3D11::BindShader(ID3D11DeviceContext* context) const
{
	switch (this->type)
	{
	case ShaderType::VERTEX_SHADER:
		context->VSSetShader(this->shader.vertex, nullptr, 0);
		break;
	case ShaderType::HULL_SHADER:
		context->HSSetShader(this->shader.hull, nullptr, 0);
		break;
	case ShaderType::DOMAIN_SHADER:
		context->DSSetShader(this->shader.domain, nullptr, 0);
		break;
	case ShaderType::GEOMETRY_SHADER:
		context->GSSetShader(this->shader.geometry, nullptr, 0);
		break;
	case ShaderType::PIXEL_SHADER:
		context->PSSetShader(this->shader.pixel, nullptr, 0);
		break;
	case ShaderType::COMPUTE_SHADER:
		context->CSSetShader(this->shader.compute, nullptr, 0);
		break;
	}
}
