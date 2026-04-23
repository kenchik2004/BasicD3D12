#pragma once
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

namespace System {
	class ConstantBufferView;
	class ConstantBuffer :public MappableBuffer
	{
	private:

		std::unique_ptr<ConstantBufferView> cbv;
		size_t class_size = 0;
	public:
		ConstantBuffer(size_t class_size_);
		ConstantBufferView* Cbv() { return cbv.get(); }
		size_t GetBufferSize() const { return class_size; }

	};
	template <class T>
	class ConstantBufferTyped final :public ConstantBuffer
	{
	private:
	public:
		ConstantBufferTyped() : ConstantBuffer(sizeof(T)) {}
		T* Map() {
			return static_cast<T*>(MappableBuffer::Map());
		}
	};

}

