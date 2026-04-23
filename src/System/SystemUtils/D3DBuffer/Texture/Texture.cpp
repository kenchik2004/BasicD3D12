#include "Texture.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DeviceContext/ID3D12DeviceContext.h"
#include "System/SystemUtils/CommandQueue/CommandQueue.h"
#include "System/SystemUtils/Descriptors/View/View.h"

namespace System {
	unsigned int GetBytesPerPixel(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 16;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 12;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 8;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return 4;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
			return 2;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			return 1;

		default:
			return 0; // 未対応のフォーマットの場合は0を返す
		}
	}


	Texture::Texture(ComPtr<ID3D12Resource>& resource, std::unique_ptr<ShaderResourceView> srv_, std::unique_ptr<RenderTargetView> rtv_, std::unique_ptr<DepthStencilView> dsv_)
	{
		if (!resource) {
			return;
		}
		if (!rtv_ && !dsv_ && !srv_) {
			return;
		}
		d3d_resource.Swap(resource);
		rtv = std::move(rtv_);
		dsv = std::move(dsv_);
		srv = std::move(srv_);
		resource_desc = d3d_resource->GetDesc();
		is_valid = true;
	}

	HRESULT Texture::TextureLoader::UploadTextureData(const void* data, size_t size, ID3D12Resource* texture_resource)
	{
		void* mapped_data = nullptr;
		HRESULT hr = texture_resource->Map(0, nullptr, &mapped_data);
		if (FAILED(hr)) {
			return hr;
		}
		const unsigned char* src_data = static_cast<const unsigned char*>(data);
		const unsigned char* end_data = src_data + size;
		unsigned char* dst_data = static_cast<unsigned char*>(mapped_data);


		std::copy(src_data, end_data, dst_data);

		texture_resource->Unmap(0, nullptr);
		return S_OK;
	}
	HRESULT Texture::TextureLoader::CopyUploadBufferToTexture(ID3D12Resource* upload_buffer, ID3D12Resource* texture_resource)
	{
		D3D12_RESOURCE_DESC desc = texture_resource->GetDesc();

		D3D12_TEXTURE_COPY_LOCATION dst_location = {};
		dst_location.pResource = texture_resource;
		dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst_location.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src_location = {};
		src_location.pResource = upload_buffer;
		src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src_location.PlacedFootprint.Offset = 0;
		src_location.PlacedFootprint.Footprint.Format = desc.Format;
		src_location.PlacedFootprint.Footprint.Width = static_cast<UINT>(desc.Width);
		src_location.PlacedFootprint.Footprint.Height = desc.Height;
		src_location.PlacedFootprint.Footprint.Depth = desc.DepthOrArraySize;
		src_location.PlacedFootprint.Footprint.RowPitch = GetBytesPerPixel(desc.Format) * static_cast<unsigned int>(desc.Width);

		ID3D12DeviceContext* context = DirectX12Manager::Instance()->GetCopyContext();
		if (!context || !context->IsValid())
		{
			return E_FAIL;
		}
		ID3D12GraphicsCommandList* cmd_list = context->GetCommandList();
		std::vector<ID3D12DeviceContext*> contexts = { context };

		CommandQueue* copy_queue = DirectX12Manager::Instance()->GetCopyQueue();

		context->ResetCommandList();

		cmd_list->CopyTextureRegion(&dst_location, 0, 0, 0, &src_location, nullptr);


		context->CloseCommandList();
		copy_queue->Execute(contexts);
		//実行を待たなければ、コピー中にアップロードバッファが破棄されてしまう可能性があるため、コピーキューの完了を待つ
		copy_queue->WaitForCompletion(context);

		return S_OK;
	}
	HRESULT Texture::TextureLoader::CreateViewsForTexture(ID3D12Resource* texture_resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::unique_ptr<ShaderResourceView>& out_srv, std::unique_ptr<RenderTargetView>& out_rtv, std::unique_ptr<DepthStencilView>& out_dsv)
	{
		//とりあえずSRVだけ作っておく
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = format;
			srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MipLevels = texture_resource->GetDesc().MipLevels;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.PlaneSlice = 0;
			if (format == DXGI_FORMAT_D32_FLOAT) {
				//深度フォーマットの場合、SRVはR32_FLOATとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			if (format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
				//深度ステンシルフォーマットの場合、SRVはR32_FLOAT_X8X24_TYPELESSとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
			}
			if (format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
				//深度ステンシルフォーマットの場合、SRVはR24_UNORM_X8_TYPELESSとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			if (format == DXGI_FORMAT_D16_UNORM) {
				//深度フォーマットの場合、SRVはR16_UNORMとして作成する必要がある
				srv_desc.Format = DXGI_FORMAT_R16_UNORM;
			}
			out_srv = DirectX12Manager::Instance()->CreateShaderResourceView(texture_resource, &srv_desc);
			if (!out_srv) {
				return E_FAIL;
			}
		}
		//レンダーターゲットとして使用する場合はRTVも作成しておく
		if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
			rtv_desc.Format = format;
			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtv_desc.Texture2D.MipSlice = 0;
			rtv_desc.Texture2D.PlaneSlice = 0;
			out_rtv = DirectX12Manager::Instance()->CreateRenderTargetView(texture_resource, &rtv_desc);
			if (!out_rtv) {
				return E_FAIL;
			}
		}
		//無いとは思うが、デプスステンシルとして使用する場合はDSVも作成しておく
		if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
			dsv_desc.Format = format;
			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
			out_dsv = DirectX12Manager::Instance()->CreateDepthStencilView(texture_resource, &dsv_desc);
			if (!out_dsv) {
				return E_FAIL;
			}
		}
		return S_OK;
	}
	HRESULT Texture::TextureLoader::CreateUploadBuffer(size_t size, ComPtr<ID3D12Resource>& upload_buffer)
	{
		D3D12_RESOURCE_DESC resource_desc = {};
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resource_desc.Alignment = 0;
		resource_desc.Width = size;
		resource_desc.Height = 1;
		resource_desc.DepthOrArraySize = 1;
		resource_desc.MipLevels = 1;
		resource_desc.Format = DXGI_FORMAT_UNKNOWN;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		D3D12_HEAP_PROPERTIES heap_properties = {};
		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;
		HRESULT hr = DirectX12Manager::Instance()->GetDevice()
			->CreateCommittedResource(&heap_properties,
				D3D12_HEAP_FLAG_NONE, &resource_desc,
				D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr,
				IID_PPV_ARGS(upload_buffer.GetAddressOf()));
		return hr;
	}
	HRESULT Texture::TextureLoader::CreateEmptyTexture(unsigned int width, unsigned int height, unsigned short array_size, unsigned short mip_levels, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, ComPtr<ID3D12Resource>& texture_resource)
	{

		D3D12_RESOURCE_DESC resource_desc = {};
		resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resource_desc.Alignment = 0;
		resource_desc.Width = width;
		resource_desc.Height = height;
		resource_desc.DepthOrArraySize = array_size;
		resource_desc.MipLevels = mip_levels;
		resource_desc.Format = format;
		resource_desc.SampleDesc.Count = 1;
		resource_desc.SampleDesc.Quality = 0;
		resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resource_desc.Flags = flags;
		D3D12_HEAP_PROPERTIES heap_properties = {};
		heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;
		D3D12_CLEAR_VALUE clear_value = {};
		D3D12_CLEAR_VALUE* p_clear_value = nullptr;
		clear_value.Format = format;
		D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COMMON;
		if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
			clear_value.Color[0] = 0.0f;
			clear_value.Color[1] = 0.0f;
			clear_value.Color[2] = 0.0f;
			clear_value.Color[3] = 1.0f;
			p_clear_value = &clear_value;
			initial_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}
		else if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
			clear_value.DepthStencil.Depth = 1.0f;
			clear_value.DepthStencil.Stencil = 0;
			p_clear_value = &clear_value;
			initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		HRESULT hr = DirectX12Manager::Instance()->GetDevice()
			->CreateCommittedResource(&heap_properties,
				D3D12_HEAP_FLAG_NONE, &resource_desc,
				initial_state, p_clear_value,
				IID_PPV_ARGS(texture_resource.GetAddressOf()));
		return hr;
	}
	std::unique_ptr<Texture> Texture::TextureLoader::LoadFromFile(const std::wstring& path, D3D12_RESOURCE_FLAGS flags)
	{
		if (path.empty() || !std::filesystem::exists(path)) {
			return nullptr;
		}

		std::vector<unsigned char> file_data;
		//あとコピー用のContextも作ってアップロードバッファからGPU上のテクスチャリソースにデータを転送する処理も必要になる(あとでやります)
		DirectX::TexMetadata metadata = {};
		DirectX::ScratchImage scratch = {};
		//とりあえず一般的なフォーマットのテクスチャのみ対応
		HRESULT hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratch);

		if (FAILED(hr)) {
			return nullptr;
		}

		ComPtr<ID3D12Resource> texture_resource;
		hr = CreateEmptyTexture(static_cast<unsigned int>(metadata.width), static_cast<unsigned int>(metadata.height), static_cast<unsigned short>(metadata.arraySize), static_cast<unsigned short>(metadata.mipLevels), metadata.format, flags, texture_resource);
		if (FAILED(hr)) {
			return nullptr;
		}

		unsigned int row_pitch = static_cast<unsigned int>(GetBytesPerPixel(metadata.format) * metadata.width);
		row_pitch = (row_pitch + 255) & ~255; // 256バイト境界に揃える
		size_t total_bytes = row_pitch * metadata.height;

		ComPtr<ID3D12Resource> upload_buffer;
		hr = CreateUploadBuffer(total_bytes, upload_buffer);
		if (FAILED(hr)) {
			return nullptr;
		}
		hr = UploadTextureData(scratch.GetImage(0, 0, 0)->pixels, scratch.GetImage(0, 0, 0)->slicePitch, upload_buffer.Get());
		if (FAILED(hr)) {
			return nullptr;
		}
		hr = CopyUploadBufferToTexture(upload_buffer.Get(), texture_resource.Get());
		if (FAILED(hr)) {
			return nullptr;
		}

		std::unique_ptr<ShaderResourceView> srv = nullptr;
		std::unique_ptr<RenderTargetView> rtv = nullptr;
		std::unique_ptr<DepthStencilView> dsv = nullptr;
		hr = CreateViewsForTexture(texture_resource.Get(), metadata.format, flags, srv, rtv, dsv);
		if (FAILED(hr)) {
			return nullptr;
		}
		return std::make_unique<Texture>(texture_resource, std::move(srv), std::move(rtv), std::move(dsv));
	}

	std::unique_ptr<Texture> Texture::TextureLoader::CreateEmpty(unsigned int width, unsigned int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
	{
		ComPtr<ID3D12Resource> texture_resource;

		HRESULT hr = CreateEmptyTexture(width, height, 1, 1, format, flags, texture_resource);
		if (FAILED(hr)) {
			return nullptr;
		}
		std::unique_ptr<ShaderResourceView> srv = nullptr;
		std::unique_ptr<RenderTargetView> rtv = nullptr;
		std::unique_ptr<DepthStencilView> dsv = nullptr;
		hr = CreateViewsForTexture(texture_resource.Get(), format, flags, srv, rtv, dsv);
		if (FAILED(hr)) {
			return nullptr;
		}
		return std::make_unique<Texture>(texture_resource, std::move(srv), std::move(rtv), std::move(dsv));
	}

}