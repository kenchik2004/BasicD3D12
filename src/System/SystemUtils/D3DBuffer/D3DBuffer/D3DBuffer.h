#pragma once

#if 1
namespace System {

	class D3DBuffer {
	public:
		virtual ~D3DBuffer() {
			unsigned long refs = d3d_resource.Reset();
			resource_desc = {};
			heap_properties = {};
			is_valid = false;
		}
		ID3D12Resource* GetResource() { return d3d_resource.Get(); }
		const D3D12_HEAP_PROPERTIES& GetHeapProperties() const { return heap_properties; }
		const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resource_desc; }
		bool IsValid() const { return is_valid; }

	protected:
		//基底クラスとして作成する上に、サイズがものによって大きく変わるため、コイツの実体化は禁止
		//ただし、派生先は作らせたいので、コンストラクタはprotectedにしておく
		D3DBuffer() = default;

		ComPtr<ID3D12Resource> d3d_resource;
		D3D12_RESOURCE_DESC resource_desc = {};
		D3D12_HEAP_PROPERTIES heap_properties = {};
		bool is_valid = false;

	};
	class MappableBuffer :public D3DBuffer
	{
	public:
		void* Map();
		void Unmap();
	private:
		void* mapped_data = nullptr;
		bool is_mapped = false;
	protected:
		MappableBuffer() = default;//Map機能を提供するだけのクラスなので、コンストラクタはprotectedにしておく
	};


}
#endif