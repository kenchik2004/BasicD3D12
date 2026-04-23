#include "D3DBuffer.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/Descriptors/View/View.h"

namespace System {
	


	void* MappableBuffer::Map() {
		if (!is_valid) {
			return nullptr;
		}
		if (is_mapped) {
			return mapped_data;
		}
		HRESULT hr = d3d_resource->Map(0, nullptr, &mapped_data);
		if (FAILED(hr)) {
			return nullptr;
		}
		is_mapped = true;
		return mapped_data;
	}

	void MappableBuffer::Unmap() {
		if (!is_valid || !is_mapped) {
			return;
		}
		d3d_resource->Unmap(0, nullptr);
		mapped_data = nullptr;
		is_mapped = false;
	}


}