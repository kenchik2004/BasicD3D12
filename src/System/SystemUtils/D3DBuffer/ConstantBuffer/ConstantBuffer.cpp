#include "ConstantBuffer.h"

#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
#include "System/SystemUtils/Descriptors/View/View.h"

namespace System {

	ConstantBuffer::ConstantBuffer(size_t class_size_) {
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Alignment = 0;
		resource_desc.Width = (class_size_ + 255) & ~255; // 256バイト境界に揃える
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
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
		cbv_desc.BufferLocation = d3d_resource->GetGPUVirtualAddress();
		cbv_desc.SizeInBytes = static_cast<UINT>(resource_desc.Width);
		cbv = DirectX12Manager::Instance()->CreateConstantBufferView(d3d_resource.Get(), &cbv_desc);
		if (!cbv) {
			return;
		}
		class_size = class_size_;
		is_valid = true;
	}

}