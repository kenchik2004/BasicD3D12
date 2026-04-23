#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {

	class IndexBuffer final :public D3DBuffer
	{
	private:
		D3D12_INDEX_BUFFER_VIEW ib_view = {};
		std::vector<unsigned int> index_data;
		void* mapped_data = nullptr;
	private:
	public:
		IndexBuffer(std::vector<unsigned int>& indices, D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_INDEX_BUFFER_VIEW& GetView() const { return ib_view; }
		const D3D12_INDEX_BUFFER_VIEW* GetViewPtr() const { return &ib_view; }
	};

}

