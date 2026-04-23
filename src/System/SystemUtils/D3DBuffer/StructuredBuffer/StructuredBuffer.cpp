#include "StructuredBuffer.h"

#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
#include "System/SystemUtils/Descriptors/View/View.h"
namespace System {

	StructuredBuffer::StructuredBuffer(size_t element_size_, size_t element_count_)
	{
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Alignment = 0;
		resource_desc.Width = element_size_ * element_count_;
		resource_desc.Height = 1;
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = 1;
		resource_desc.Format = DXGI_FORMAT_UNKNOWN;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;
		HRESULT hr = DirectX12Manager::Instance()->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(d3d_resource.GetAddressOf()));
		if (FAILED(hr)) {
			return;
		}
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = static_cast<UINT>(element_count_);
		srv_desc.Buffer.StructureByteStride = static_cast<UINT>(element_size_);
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srv = DirectX12Manager::Instance()->CreateShaderResourceView(d3d_resource.Get(), &srv_desc);
		if (!srv) {
			return;
		}
		element_size = element_size_;
		element_count = element_count_;
		is_valid = true;
	}
}