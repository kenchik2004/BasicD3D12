#pragma once
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"

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

	class Texture final :public D3DBuffer
	{
		friend class TextureLoader;
	private:
		struct TextureIndexBuffer {
			unsigned int my_index;
		};
		std::unique_ptr<RenderTargetView> rtv;
		std::unique_ptr<DepthStencilView> dsv;
		std::unique_ptr<ShaderResourceView> srv;
	public:
		Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_);
		RenderTargetView* Rtv() const { return rtv.get(); }
		DepthStencilView* Dsv() const { return dsv.get(); }
		ShaderResourceView* Srv() const { return srv.get(); }
#if 1
		class TextureLoader final
		{
		private:
			static HRESULT CreateEmptyTexture(unsigned int width, unsigned int height, unsigned short array_size, unsigned short mip_levels, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, ComPtr<ID3D12Resource>& texture_resource);
			static HRESULT CreateUploadBuffer(size_t size, ComPtr<ID3D12Resource>& upload_buffer);
			static HRESULT UploadTextureData(const void* data, size_t size, ID3D12Resource* texture_resource);
			static HRESULT CopyUploadBufferToTexture(ID3D12Resource* upload_buffer, ID3D12Resource* texture_resource);
			static HRESULT CreateViewsForTexture(ID3D12Resource* texture_resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::unique_ptr<ShaderResourceView>& out_srv, std::unique_ptr<RenderTargetView>& out_rtv, std::unique_ptr<DepthStencilView>& out_dsv);
		public:
			static std::unique_ptr<Texture> LoadFromFile(const std::wstring& path, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
			static std::unique_ptr<Texture> CreateEmpty(unsigned int width, unsigned int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
		};
#endif

	};
}
#endif