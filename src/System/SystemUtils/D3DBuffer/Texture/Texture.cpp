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
		width_32 = static_cast<unsigned int>(resource_desc.Width);
		is_valid = true;
	}
	HRESULT Texture::Loader::CreateUploadBuffer(size_t size, ComPtr<ID3D12Resource>& upload_buffer)
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

	HRESULT Texture::Loader::UploadTextureData(ID3D12Resource* upload_buffer, void* data, unsigned int width_in_bytes, unsigned int height, unsigned short depth)
	{
		unsigned int row_pitch = (width_in_bytes + 255) & ~255; //行のピッチは256バイトアラインメントでなければならないため、256の倍数に切り上げる
		void* mapped_data = nullptr;
		HRESULT hr = upload_buffer->Map(0, nullptr, &mapped_data);
		if (FAILED(hr)) {
			return hr;
		}
		unsigned char* src_data = static_cast<unsigned char*>(data);
		unsigned char* dst_data = static_cast<unsigned char*>(mapped_data);

		for (unsigned short z = 0; z < depth; ++z) {
			for (unsigned int y = 0; y < height; ++y) {
				std::copy(src_data, src_data + row_pitch, dst_data);
				//元データは1行分だけ進める
				src_data += width_in_bytes;
				//アップロードバッファは切り上げた分も含めて1行分進める
				dst_data += row_pitch;
			}
		}

		upload_buffer->Unmap(0, nullptr);
		return S_OK;
	}
	HRESULT Texture::Loader::CopyUploadBufferToTexture(ID3D12Resource* upload_buffer, ID3D12Resource* texture_resource)
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
		src_location.PlacedFootprint.Footprint.Width = static_cast<unsigned int>(desc.Width);
		src_location.PlacedFootprint.Footprint.Height = static_cast<unsigned int>(desc.Height);
		src_location.PlacedFootprint.Footprint.Depth = static_cast<unsigned int>(desc.DepthOrArraySize);
		src_location.PlacedFootprint.Footprint.RowPitch = (GetBytesPerPixel(desc.Format) * static_cast<unsigned int>(desc.Width) + 255) & ~255;

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
	HRESULT Texture::Loader::CreateViewsForTexture(ID3D12Resource* texture_resource, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::unique_ptr<ShaderResourceView>& out_srv, std::unique_ptr<RenderTargetView>& out_rtv, std::unique_ptr<DepthStencilView>& out_dsv)
	{
		//とりあえずSRVだけ作っておく
		{


			out_srv = DirectX12Manager::Instance()->CreateShaderResourceView(texture_resource, nullptr);
			if (!out_srv) {
				return E_FAIL;
			}
		}
		//レンダーターゲットとして使用する場合はRTVも作成しておく
		if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
			out_rtv = DirectX12Manager::Instance()->CreateRenderTargetView(texture_resource, nullptr);
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
			out_dsv = DirectX12Manager::Instance()->CreateDepthStencilView(texture_resource, nullptr);
			if (!out_dsv) {
				return E_FAIL;
			}
		}
		return S_OK;
	}

	HRESULT Texture::Loader::CreateEmptyTexture(const D3D12_RESOURCE_DESC& desc, ComPtr<ID3D12Resource>& texture_resource, D3D12_CLEAR_VALUE* p_clear_value)
	{
		D3D12_HEAP_PROPERTIES heap_properties = {};
		heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 0;
		heap_properties.VisibleNodeMask = 0;
		D3D12_CLEAR_VALUE clear_value = {};
		clear_value.Format = desc.Format;
		D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COMMON;
		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
			clear_value.Color[0] = 0.0f;
			clear_value.Color[1] = 0.0f;
			clear_value.Color[2] = 0.0f;
			clear_value.Color[3] = 1.0f;
			if (!p_clear_value)
				p_clear_value = &clear_value;
			initial_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
		}
		else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
			clear_value.DepthStencil.Depth = 1.0f;
			clear_value.DepthStencil.Stencil = 0;
			if (!p_clear_value)
				p_clear_value = &clear_value;
			initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		HRESULT hr = DirectX12Manager::Instance()->GetDevice()
			->CreateCommittedResource(&heap_properties,
				D3D12_HEAP_FLAG_NONE, &desc,
				initial_state, p_clear_value,
				IID_PPV_ARGS(texture_resource.GetAddressOf()));
		return hr;
	}
	std::unique_ptr<Texture> Texture::Loader::LoadFromFile(const std::wstring& path, D3D12_RESOURCE_FLAGS flags)
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

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
		desc.Alignment = 0;
		desc.Width = static_cast<unsigned int>(metadata.width);
		desc.Height = static_cast<unsigned int>(metadata.height);
		desc.DepthOrArraySize = static_cast<unsigned short>(metadata.arraySize);
		desc.MipLevels = static_cast<unsigned short>(metadata.mipLevels);
		desc.Format = metadata.format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = flags;


		ComPtr<ID3D12Resource> texture_resource;
		hr = CreateEmptyTexture(desc, texture_resource);
		if (FAILED(hr)) {
			return nullptr;
		}

		size_t row_pitch = GetBytesPerPixel(desc.Format) * desc.Width;
		row_pitch = (row_pitch + 255) & ~255; // 256バイト境界に揃える
		size_t total_bytes = row_pitch * desc.Height * desc.DepthOrArraySize;

		ComPtr<ID3D12Resource> upload_buffer;
		hr = CreateUploadBuffer(total_bytes, upload_buffer);
		if (FAILED(hr)) {
			return nullptr;
		}
		hr = UploadTextureData(upload_buffer.Get(), scratch.GetImage(0, 0, 0)->pixels, GetBytesPerPixel(desc.Format) * desc.Width, desc.Height, desc.DepthOrArraySize);
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
		hr = CreateViewsForTexture(texture_resource.Get(), desc.Format, flags, srv, rtv, dsv);
		if (FAILED(hr)) {
			return nullptr;
		}
		return std::make_unique<Texture>(texture_resource, std::move(srv), std::move(rtv), std::move(dsv));
	}

	std::unique_ptr<Texture> Texture::Loader::CreateEmpty(const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE* p_clear_value)
	{
		ComPtr<ID3D12Resource> texture_resource;

		HRESULT hr = CreateEmptyTexture(desc, texture_resource, p_clear_value);
		if (FAILED(hr)) {
			return nullptr;
		}
		std::unique_ptr<ShaderResourceView> srv = nullptr;
		std::unique_ptr<RenderTargetView> rtv = nullptr;
		std::unique_ptr<DepthStencilView> dsv = nullptr;
		hr = CreateViewsForTexture(texture_resource.Get(), desc.Format, desc.Flags, srv, rtv, dsv);
		if (FAILED(hr)) {
			return nullptr;
		}
		return std::make_unique<Texture>(texture_resource, std::move(srv), std::move(rtv), std::move(dsv));
	}

	enum SaveFormat {
		DDS,
		PNG,
		TGA,
		JPEG,
		BMP,
		TIFF,
		ICO,
		GIF,
		HDR,
		UNKNOWN
	};
	SaveFormat GetSaveFormatFromExtension(const std::wstring& extension) {
		std::wstring ext = extension;
		std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
		if (ext == L".dds") {
			return SaveFormat::DDS;
		}
		else if (ext == L".png") {
			return SaveFormat::PNG;
		}
		else if (ext == L".tga") {
			return SaveFormat::TGA;
		}
		else if (ext == L".jpg" || ext == L".jpeg") {
			return SaveFormat::JPEG;
		}
		else if (ext == L".bmp") {
			return SaveFormat::BMP;
		}
		else if (ext == L".tiff" || ext == L".tif") {
			return SaveFormat::TIFF;
		}
		else if (ext == L".ico") {
			return SaveFormat::ICO;
		}
		else if (ext == L".gif") {
			return SaveFormat::GIF;
		}
		else if (ext == L".hdr") {
			return SaveFormat::HDR;
		}
		else {
			return SaveFormat::UNKNOWN;
		}
	}


	int Texture::Loader::SaveToFile(Texture* texture, const std::wstring& path, D3D12_RESOURCE_STATES cur_state)
	{
		std::wstring extension = std::filesystem::path(path).extension().wstring();
		//テクスチャがないまたは無効、パスが空、拡張子がない場合はエラー
		if (!texture || !texture->IsValid() || path.empty() || extension.empty()) {
			return -1;
		}
		ID3D12Resource* resource = texture->GetResource();
		if (!resource) {
			return -1;
		}
		DirectX::ScratchImage scratch = {};
		HRESULT hr = DirectX::CaptureTexture(DirectX12Manager::Instance()->GetDrawQueue()->GetCommandQueue(), resource, false, scratch, cur_state, cur_state);
		if (FAILED(hr)) {
			return -1;
		}
		SaveFormat format = GetSaveFormatFromExtension(extension);

		switch (format)
		{
		case SaveFormat::DDS: {

			hr = DirectX::SaveToDDSFile(scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(), DirectX::DDS_FLAGS_NONE, path.c_str());
			break;
		}
		case SaveFormat::PNG: {

			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), path.c_str());
			break;
		}
		case SaveFormat::TGA: {

			const DirectX::Image* image = scratch.GetImage(0, 0, 0);
			if (!image) {
				return -1;
			}
			hr = DirectX::SaveToTGAFile(*image, DirectX::TGA_FLAGS_NONE, path.c_str());
			break;
		}
		case SaveFormat::JPEG: {
			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG), path.c_str());
			break;
		}
		case SaveFormat::BMP: {
			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_BMP), path.c_str());
			break;
		}
		case SaveFormat::TIFF: {
			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF), path.c_str());
			break;
		}
		case SaveFormat::ICO: {
			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_ICO), path.c_str());
			break;
		}
		case SaveFormat::GIF: {
			hr = DirectX::SaveToWICFile(scratch.GetImages(), scratch.GetImageCount(), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_GIF), path.c_str());
			break;
		}
		case SaveFormat::HDR: {

			const DirectX::Image* image = scratch.GetImage(0, 0, 0);
			if (!image) {
				return -1;
			}
			hr = DirectX::SaveToHDRFile(*image, path.c_str());
			break;
		}
		default:
			return -1;
		}

		if (FAILED(hr)) {
			return -1;
		}
		return 0;
	}

}