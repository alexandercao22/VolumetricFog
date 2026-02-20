#include "InputLayoutD3D11.h"

InputLayoutD3D11::~InputLayoutD3D11()
{
	if (this->inputLayout != nullptr)
		this->inputLayout->Release();
}

void InputLayoutD3D11::AddInputElement(const std::string& semanticName, DXGI_FORMAT format)
{
	this->semanticNames.push_back(semanticName);

	D3D11_INPUT_ELEMENT_DESC desc;
	desc.SemanticName = semanticName.c_str();
	desc.SemanticIndex = 0;
	desc.Format = format;
	desc.InputSlot = 0;
	desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc.InstanceDataStepRate = 0;
	this->elements.push_back(desc);
}

void InputLayoutD3D11::FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize)
{
	const size_t totalElements = this->elements.size();
	HRESULT hr = device->CreateInputLayout(this->elements.data(), static_cast<UINT>(totalElements), vsDataPtr, vsDataSize, &this->inputLayout);
}

ID3D11InputLayout* InputLayoutD3D11::GetInputLayout() const
{
	return this->inputLayout;
}
