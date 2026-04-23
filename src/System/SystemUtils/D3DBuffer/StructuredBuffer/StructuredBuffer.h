#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {

	class ShaderResourceView;
	class StructuredBuffer :public MappableBuffer
	{
	private:
		std::unique_ptr<ShaderResourceView> srv;
		size_t element_size = 0;
		size_t element_count = 0;
	public:
		StructuredBuffer(size_t element_size_, size_t element_count_);
		ShaderResourceView* Srv() const { return srv.get(); }
		size_t GetElementSize() const { return element_size; }
		size_t GetElementCount() const { return element_count; }
		size_t GetBufferSize() const { return element_size * element_count; }

	};

	template <class T>
	class StructuredBufferTyped final :public StructuredBuffer
	{
	public:
		StructuredBufferTyped(size_t element_count) : StructuredBuffer(sizeof(T), element_count) {}
		T* Map() {
			return static_cast<T*>(MappableBuffer::Map());
		}
		T* At(size_t index) {
			T* data = Map();
			if (data) {
				return data + index;
			}
			return nullptr;
		}
	};
}