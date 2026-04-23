#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {

	class VertexBuffer final :public D3DBuffer
	{
	private:
		D3D12_VERTEX_BUFFER_VIEW vb_view = {};
		std::vector<float> vertex_data;
		void* mapped_data = nullptr;
	public:
		VertexBuffer(std::vector<float>& vertices, unsigned int stride_in_counts, D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_UPLOAD);
		const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return vb_view; }
		const D3D12_VERTEX_BUFFER_VIEW* GetViewPtr() const { return &vb_view; }
	};
}
