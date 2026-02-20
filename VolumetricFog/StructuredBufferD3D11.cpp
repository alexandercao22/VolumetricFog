#include "StructuredBufferD3D11.h"

StructuredBufferD3D11::StructuredBufferD3D11(ID3D11Device* device, UINT sizeOfElement,
	size_t nrOfElementsInBuffer, void* bufferData, bool dynamic)
{
	Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic);
}

StructuredBufferD3D11::~StructuredBufferD3D11()
{
	if (this->buffer != nullptr)
		this->buffer->Release();
	if (this->srv != nullptr)
		this->srv->Release();
	if (this->uav != nullptr)
		this->uav->Release();
}

void StructuredBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfElement,
	size_t nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasSRV, bool hasUAV)
{
	this->elementSize = sizeOfElement;
	this->nrOfElements = nrOfElementsInBuffer;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = this->elementSize * this->nrOfElements;
	bufferDesc.Usage = dynamic ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	bufferDesc.BindFlags |= hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
	bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = this->elementSize;

	HRESULT hr;
	if (bufferData != nullptr)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = bufferData;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		
		hr = device->CreateBuffer(&bufferDesc, &data, &this->buffer);
	}
	else
	{
		hr = device->CreateBuffer(&bufferDesc, nullptr, &this->buffer);
	}

	if (FAILED(hr))
	{
		std::cerr << "Failed to create structured buffer\n";
	}
	
	if (SUCCEEDED(hr) && hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = this->nrOfElements;

		hr = device->CreateShaderResourceView(this->buffer, &srvDesc, &this->srv);

		if (FAILED(hr))
		{
			std::cerr << "Failed to create structured buffer SRV\n";
		}
	}

	if (SUCCEEDED(hr) && hasUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = this->nrOfElements;
		uavDesc.Buffer.Flags = 0;

		hr = device->CreateUnorderedAccessView(this->buffer, &uavDesc, &this->uav);

		if (FAILED(hr))
		{
			std::cerr << "Failed to create structured buffer UAV\n";
		}
	}
}

void StructuredBufferD3D11::UpdateBuffer(ID3D11DeviceContext* context, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = context->Map(this->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Deny GPU access to mappedResource
	CopyMemory(mappedResource.pData, data, this->elementSize * this->nrOfElements);
	context->Unmap(this->buffer, 0); // Reenable GPU access to mappedResource
}

UINT StructuredBufferD3D11::GetElementSize() const
{
	return this->elementSize;
}

size_t StructuredBufferD3D11::GetNrOfElements() const
{
	return this->nrOfElements;
}

ID3D11ShaderResourceView* StructuredBufferD3D11::GetSRV() const
{
	return this->srv;
}

ID3D11UnorderedAccessView* StructuredBufferD3D11::GetUAV() const
{
	return this->uav;
}
