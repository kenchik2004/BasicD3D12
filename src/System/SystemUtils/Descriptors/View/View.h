#pragma once
namespace System {

	// DirectX11では、ビューという概念が存在していた。
	// その頃のビューは、それぞれのクラスとして別々に存在していた。
	//例：
	// ・ID3D11ShaderResourceView：シェーダーリソースビュー
	// ・ID3D11RenderTargetView：レンダーターゲットビュー
	// ・ID3D11DepthStencilView：デプスステンシルビュー
	// 
	// D3D12では、ビューという概念が存在しない。
	// 全て、他のビューと統合され「ディスクリプタ」と呼ばれている。
	// ここでは、テクスチャ用のディスクリプタを明確に区別するために、
	// テクスチャ用のディスクリプタをラップしたクラスを作ることにする。


	//-------------------------------------------------------------
	// @brief ビューの説明構造体
	// @brief ビューの説明構造体。シェーダーリソースビュー、レンダーターゲットビュー、デプスステンシルビューの説明をまとめて管理するための構造体。
	//-------------------------------------------------------------
	struct VIEW_DESC
	{
		enum VIEW_TYPE
		{
			SRV,
			CBV,
			UAV,
			RTV,
			DSV
		} type;
		union
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC* srv_desc;
			D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv_desc;
			D3D12_UNORDERED_ACCESS_VIEW_DESC* uav_desc;
			D3D12_RENDER_TARGET_VIEW_DESC* rtv_desc;
			D3D12_DEPTH_STENCIL_VIEW_DESC* dsv_desc;
		};
	};

	struct DEFAULT_VIEW_DESC_HELPER {

		static D3D12_SHADER_RESOURCE_VIEW_DESC GetDefaultSRVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

			srvDesc.Format = resDesc.Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			switch (resDesc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.NumElements = (UINT)(resDesc.Width / resDesc.Alignment); // 要調整
				srvDesc.Buffer.StructureByteStride = 0;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (resDesc.DepthOrArraySize > 1) {
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					srvDesc.Texture1DArray.MipLevels = resDesc.MipLevels;
					srvDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					srvDesc.Texture1D.MipLevels = resDesc.MipLevels;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				// キューブマップ判定（フラグがある場合）

				if (resDesc.DepthOrArraySize % 6 == 0) {
					// ここはエンジンの運用ルールによりますが、基本はTEXTURE2D
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
				}
				else if (resDesc.DepthOrArraySize > 1) {
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MipLevels = resDesc.MipLevels;
					srvDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MipLevels = resDesc.MipLevels;
				break;
			}
			return srvDesc;
		}

		static D3D12_RENDER_TARGET_VIEW_DESC GetDefaultRTVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = resDesc.Format;

			switch (resDesc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				// バッファはRTVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要があります。
				// ここでは、エラー処理として、無効なビューを返すことにします。
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (resDesc.DepthOrArraySize > 1) {
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
					rtvDesc.Texture1DArray.MipSlice = 0;
					rtvDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
					rtvDesc.Texture1D.MipSlice = 0;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (resDesc.DepthOrArraySize > 1) {
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = 0;
					rtvDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtvDesc.Texture2D.MipSlice = 0;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				// テクスチャ3DはRTVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要があります。
				// ここでは、エラー処理として、無効なビューを返すことにします。
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			}
			return rtvDesc;
		}
		static D3D12_DEPTH_STENCIL_VIEW_DESC GetDefaultDSVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = resDesc.Format;
			switch (resDesc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				// バッファはDSVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要があります。
				// ここでは、エラー処理として、無効なビューを返すことにします。
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (resDesc.DepthOrArraySize > 1) {
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					dsvDesc.Texture1DArray.MipSlice = 0;
					dsvDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
					dsvDesc.Texture1D.MipSlice = 0;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (resDesc.DepthOrArraySize > 1) {
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = 0;
					dsvDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
				}
				else {
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				// テクスチャ3DはDSVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要があります。
				// ここでは、エラー処理として、無効なビューを返すことにします。
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			}
			return dsvDesc;
		}
		static D3D12_CONSTANT_BUFFER_VIEW_DESC GetDefaultCBVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = (UINT)(resDesc.Width + 255) & ~255; // 256バイトアラインメント
			return cbvDesc;
		}
	};

	class DescriptorHeap; // 前方宣言

	//-------------------------------------------------------------
	// @brief ディスクリプタをラップしたインターフェースクラス
	//-------------------------------------------------------------
	class View
	{
	public:
		virtual ~View() = default;
		View(const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle_, ID3D12Resource* resource_, DescriptorHeap* parent_heap_)
			: cpu_handle(cpu_handle_), resource(resource_), parent_heap(parent_heap_) {
		}

		ID3D12Resource* GetResource() const { return resource; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return cpu_handle; }
		const DescriptorHeap* GetParentHeap() const { return parent_heap; }
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
		ID3D12Resource* resource;
		DescriptorHeap* parent_heap; // 所属するディスクリプタヒープへのポインタ
	};

	class ShaderResourceView : public View
	{
	public:
		ShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle_, const D3D12_GPU_DESCRIPTOR_HANDLE& gpu_handle_, unsigned int index_, ID3D12Resource* resource, DescriptorHeap* parent_heap_)
			: View(cpu_handle_, resource, parent_heap_), desc(desc_), index(index_) {
			gpu_handle = gpu_handle_;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return gpu_handle; }
		unsigned int GetIndex() const { return index; }

		const D3D12_SHADER_RESOURCE_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
		unsigned int index; // ディスクリタヒープ内のインデックス
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	};

	class ConstantBufferView : public View
	{
	public:
		ConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle, const D3D12_GPU_DESCRIPTOR_HANDLE& gpu_handle_, ID3D12Resource* resource, DescriptorHeap* parent_heap_)
			: View(cpu_handle, resource, parent_heap_), desc(desc_) {
			gpu_handle = gpu_handle_;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return gpu_handle; }
		const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
	};

	class RenderTargetView : public View
	{
	public:
		RenderTargetView(const D3D12_RENDER_TARGET_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle, ID3D12Resource* resource, DescriptorHeap* parent_heap_)
			: View(cpu_handle, resource, parent_heap_), desc(desc_) {
		}

		const D3D12_RENDER_TARGET_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_RENDER_TARGET_VIEW_DESC desc;
	};

	class DepthStencilView : public View
	{
	public:
		DepthStencilView(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle, ID3D12Resource* resource, DescriptorHeap* parent_heap_)
			: View(cpu_handle, resource, parent_heap_), desc(desc_) {
		}

		const D3D12_DEPTH_STENCIL_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	};


}