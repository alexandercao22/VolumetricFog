#include "ConstantBufferD3D11.h"

ConstantBufferD3D11::ConstantBufferD3D11(ID3D11Device* device, size_t byteSize, void* initialData)
{
	this->Initialize(device, byteSize, initialData);
}

ConstantBufferD3D11::~ConstantBufferD3D11()
{
	if (this->buffer != nullptr)
		this->buffer->Release();
}

ConstantBufferD3D11::ConstantBufferD3D11(ConstantBufferD3D11&& other) noexcept // Move constructor
{
	this->buffer = other.buffer;
	this->buffer->AddRef();
	this->bufferSize = other.bufferSize;
}

ConstantBufferD3D11& ConstantBufferD3D11::operator=(ConstantBufferD3D11&& other) noexcept // Move assignment operator
{
	this->buffer = other.buffer;
	this->bufferSize = other.bufferSize;
	return other;
}

void ConstantBufferD3D11::Initialize(ID3D11Device* device, size_t byteSize, void* initialData)
{
	this->bufferSize = byteSize;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = this->bufferSize;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // Update the resource. Can only be read by GPU and can only be written by CPU.
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU updates the resource
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = initialData;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &resourceData, &this->buffer);
	if (FAILED(hr))
	{
		std::cerr << "Could not create constant buffer\n";
		return;
	}

	this->buffer->AddRef();
}

size_t ConstantBufferD3D11::GetSize() const
{
	return this->bufferSize;
}

ID3D11Buffer* ConstantBufferD3D11::GetBuffer() const
{
	return this->buffer;
}

void ConstantBufferD3D11::UpdateBuffer(ID3D11DeviceContext* context, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	HRESULT hr = context->Map(this->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Deny GPU access to mappedResource
	CopyMemory(mappedResource.pData, data, this->bufferSize);
	context->Unmap(this->buffer, 0); // Reenable GPU access to mappedResource
}