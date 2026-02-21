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
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc;
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		};
	};

	//-------------------------------------------------------------
	// @brief ディスクリプタをラップしたインターフェースクラス
	//-------------------------------------------------------------
	class View
	{
	public:
		virtual ~View() = default;
		View(const D3D12_CPU_DESCRIPTOR_HANDLE& cpu_handle_, ID3D12Resource* resource_)
			: cpu_handle(cpu_handle_), resource(resource_) {
		}

		ID3D12Resource* GetResource() const { return resource; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return cpu_handle; }
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
		ID3D12Resource* resource;
	};

	class ShaderResourceView : public View
	{
	public:
		ShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& gpu_handle, ID3D12Resource* resource)
			: View(gpu_handle, resource), desc(desc_) {
		}

		const D3D12_SHADER_RESOURCE_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	};

	class RenderTargetView : public View
	{
	public:
		RenderTargetView(const D3D12_RENDER_TARGET_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& gpu_handle, ID3D12Resource* resource)
			: View(gpu_handle, resource), desc(desc_) {
		}

		const D3D12_RENDER_TARGET_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_RENDER_TARGET_VIEW_DESC desc;
	};

	class DepthStencilView : public View
	{
	public:
		DepthStencilView(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc_, const D3D12_CPU_DESCRIPTOR_HANDLE& gpu_handle, ID3D12Resource* resource)
			: View(gpu_handle, resource), desc(desc_) {
		}

		const D3D12_DEPTH_STENCIL_VIEW_DESC& GetDesc() const { return desc; }
	private:
		D3D12_DEPTH_STENCIL_VIEW_DESC desc;
	};


}