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
			D3D12_RESOURCE_DESC res_desc = resource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};

			srv_desc.Format = res_desc.Format;
			if (res_desc.Format == DXGI_FORMAT_D32_FLOAT) {
				//深度フォーマットの場合、SRVはR32_FLOATとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			if (res_desc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
				//深度ステンシルフォーマットの場合、SRVはR32_FLOAT_X8X24_TYPELESSとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			}
			if (res_desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
				//深度ステンシルフォーマットの場合、SRVはR24_UNORM_X8_TYPELESSとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			if (res_desc.Format == DXGI_FORMAT_D16_UNORM) {
				//深度フォーマットの場合、SRVはR16_UNORMとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R16_UNORM;
			}
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (res_desc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srv_desc.Buffer.FirstElement = 0;
				srv_desc.Buffer.NumElements = (UINT)(res_desc.Width / res_desc.Alignment); // 要調整
				srv_desc.Buffer.StructureByteStride = 0;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (res_desc.DepthOrArraySize > 1) {
					srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
					srv_desc.Texture1DArray.MipLevels = res_desc.MipLevels;
					srv_desc.Texture1DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
					srv_desc.Texture1D.MipLevels = res_desc.MipLevels;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				// キューブマップ判定（フラグがある場合）

				if (res_desc.DepthOrArraySize % 6 == 0) {
					// ここはエンジンの運用ルールによるが、基本はTEXTURE2D
					srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srv_desc.Texture2D.MipLevels = res_desc.MipLevels;
				}
				else if (res_desc.DepthOrArraySize > 1) {
					srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srv_desc.Texture2DArray.MipLevels = res_desc.MipLevels;
					srv_desc.Texture2DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					srv_desc.Texture2D.MipLevels = res_desc.MipLevels;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srv_desc.Texture3D.MipLevels = res_desc.MipLevels;
				break;
			}
			return srv_desc;
		}

		static D3D12_RENDER_TARGET_VIEW_DESC GetDefaultRTVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC res_desc = resource->GetDesc();
			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
			rtv_desc.Format = res_desc.Format;

			switch (res_desc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				// バッファはRTVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要がある。
				// ここでは、エラー処理として、無効なビューを返すことにする。
				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (res_desc.DepthOrArraySize > 1) {
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
					rtv_desc.Texture1DArray.MipSlice = 0;
					rtv_desc.Texture1DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
					rtv_desc.Texture1D.MipSlice = 0;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (res_desc.DepthOrArraySize > 1) {
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtv_desc.Texture2DArray.MipSlice = 0;
					rtv_desc.Texture2DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtv_desc.Texture2D.MipSlice = 0;
				}
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				// テクスチャ3DはRTVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要がある。
				// ここでは、エラー処理として、無効なビューを返すことにする。
				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			}
			return rtv_desc;
		}
		static D3D12_DEPTH_STENCIL_VIEW_DESC GetDefaultDSVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC res_desc = resource->GetDesc();
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
			dsv_desc.Format = res_desc.Format;
			switch (res_desc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				// バッファはDSVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要がある。
				// ここでは、エラー処理として、無効なビューを返すことにする。
				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (res_desc.DepthOrArraySize > 1) {
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					dsv_desc.Texture1DArray.MipSlice = 0;
					dsv_desc.Texture1DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
					dsv_desc.Texture1D.MipSlice = 0;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (res_desc.DepthOrArraySize > 1) {
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsv_desc.Texture2DArray.MipSlice = 0;
					dsv_desc.Texture2DArray.ArraySize = res_desc.DepthOrArraySize;
				}
				else {
					dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsv_desc.Texture2D.MipSlice = 0;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				// テクスチャ3DはDSVにできないため、ここではエラー処理を行うか、デフォルトのビューを返すかを選択する必要がある。
				// ここでは、エラー処理として、無効なビューを返すことにする。
				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			}
			return dsv_desc;
		}
		static D3D12_CONSTANT_BUFFER_VIEW_DESC GetDefaultCBVDesc(ID3D12Resource* resource) {
			D3D12_RESOURCE_DESC res_desc = resource->GetDesc();
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
			cbv_desc.BufferLocation = resource->GetGPUVirtualAddress();
			cbv_desc.SizeInBytes = (UINT)(res_desc.Width + 255) & ~255; // 256バイトアラインメント
			return cbv_desc;
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