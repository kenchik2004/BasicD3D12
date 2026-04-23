#include "ApplicationManager.h"
#include "System/Managers/WindowManager/WindowManager.h"
#include "System/Managers/DirectX12Manager/DirectX12Manager.h"
#include "System/SystemUtils/DescriptorHeaps/DescriptorHeap/DescriptorHeap.h"
#include "System/SystemUtils/D3DBuffer/D3DBuffer/D3DBuffer.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace SystemGUI {

	int InitImGui(HWND hwnd, ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Multi-Viewport / Platform Windows


		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		System::CSUHeap* csu_heap = System::DirectX12Manager::Instance()->GetCBVSRVUAVHeap();
		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = pd3dDevice;
		init_info.CommandQueue = pd3dCommandQueue;
		init_info.NumFramesInFlight = 5;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		init_info.SrvDescriptorHeap = csu_heap->GetHeap();
		init_info.UserData = csu_heap;

		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* init_info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
			{
				System::CSUHeap* allocator = (System::CSUHeap*)init_info->UserData;
				return allocator->AllocateWithoutView(out_cpu_desc_handle, out_gpu_desc_handle);
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* init_info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
			{
				return; // 今のところ解放はしない
			};

		ImGui_ImplDX12_Init(&init_info);
		std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> custom_window_proc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT
			{
				if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
					return true; // ImGuiがメッセージを処理した場合は1を返す
				return false; // ImGuiがメッセージを処理しなかった場合は0を返す
			};
		System::WindowManager::Instance()->SetCustomWindowProc(std::move(custom_window_proc));
		return 0;
	}

	int ImGuiDrawEnd() {
		ImGui::Render();
		return 0;
	}
	int ImGuiDrawBegin() {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		return 0;
	}
	int PresentImGui(ID3D12GraphicsCommandList* pd3dCommandList) {
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pd3dCommandList);
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		return 0;
	}
	int DestroyImGui() {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		return 0;
	}
}


class ShaderCompiler {
public:
	typedef enum TargetShader {
		VertexShader,
		HullShader,
		DomainShader,
		GeometryShader,
		PixelShader,
		ComputeShader,
		TargetShaderCount
	} TargetShader;
private:

	static inline std::unordered_map<TargetShader, std::string> target_shader_to_string_map = {
	{TargetShader::VertexShader,"vs_5_1"},
	{TargetShader::HullShader,"hs_5_1"},
	{TargetShader::DomainShader,"ds_5_1"},
	{TargetShader::GeometryShader,"gs_5_1"},
	{TargetShader::PixelShader,"ps_5_1"},
	{TargetShader::ComputeShader,"cs_5_1"}
	};

public:


	static HRESULT CompileShader(const std::wstring& file_path, const D3D_SHADER_MACRO* defines, const char* entry_point, TargetShader target, ComPtr<ID3DBlob>& shader_blob) {
		ComPtr<ID3DBlob> error_blob;

		HRESULT hr = D3DCompileFromFile(file_path.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, target_shader_to_string_map[target].c_str(), D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, 0, shader_blob.GetAddressOf(), error_blob.GetAddressOf());
		if (FAILED(hr)) {
			if (error_blob) {
				OutputDebugStringA((char*)error_blob->GetBufferPointer());
			}
			return hr;
		}
		return S_OK;
	}
};

//マテリアルを外に出して抽象化したいので、マテリアルクラスに必要な情報を挙げておこう
//1.シェーダー(最低でも頂点シェーダーとピクセルシェーダー)
//2.ルートシグネチャ<-(実はいらないかも?)
//3.定数バッファの情報
//4.テクスチャの情報
//5.ブレンドステートやラスタライザーステートなどのグラフィックスステートの情報
class RootSignature {
private:
	ComPtr<ID3D12RootSignature> root_signature;
	static constexpr unsigned int STRUCTURED_BUFFER_COUNTS = 5; //構造化バッファの数
	static constexpr unsigned int GENERAL_SRV_COUNTS = 1; //Bindless Resourceに使うSRVの数
	static constexpr unsigned int ROOT_CONSTANT_COUNTS = 4; //ルート定数の数
	static constexpr unsigned int GENERAL_CBV_COUNTS = 16;

public:
	typedef enum RootParameterStartSlot {
		StructuredBufferSlot = 0,
		SRVSlot = StructuredBufferSlot + STRUCTURED_BUFFER_COUNTS,
		RootConstantSlot = SRVSlot + GENERAL_SRV_COUNTS,
		CBVSlot = RootConstantSlot + ROOT_CONSTANT_COUNTS,
		RootParameterCount = CBVSlot + GENERAL_CBV_COUNTS
	} RootParameterStartSlot;


	//作成するルートシグネチャのイメージ図
	// 
	//+------------------------------------------------+
	//| ルートパラメータ0~15: SRV(StructuredBuffer)      |
	//|                                                |
	//|                                                |
	//|                                                |
	//|                                                |
	//|                                                |
	//+------------------------------------------------+
	//| ルートパラメータ16: SRV(Bindless Resource)        |
	//| 通常使用を目的としたSRVの巨大配列                   |
	//| 定数バッファやStructuredBufferでインデックスを渡し、 |
	//| それを通してアクセスする                           |
	//+------------------------------------------------+
	//| ルートパラメータ17~20: Root Constant              |
	//| 使用するSRVやStructuredBufferのインデックスを      |
	//| シェーダーに渡すためのバッファ                      |
	//|                                                |
	//+------------------------------------------------+
	//| ルートパラメータ21~36: CBV(通常定数バッファ)        |
	//|                                                |
	//|                                                |
	//|                                                |
	//|                                                |
	//|                                                |
	//+------------------------------------------------+
	//| 静的サンプラー0:バイリニアフィルタリング、ラップ      |
	//| 静的サンプラー1:バイリニアフィルタリング、クランプ    |
	//| 静的サンプラー2:バイリニアフィルタリング、ミラー      |
	//| 静的サンプラー3:バイリニアフィルタリング、ボーダー    |
	//|                                                |
	//| 現在はこれだけだが、サンプラーはそこまで沢山必要ない。 |
	//| 予め複数用意しておいて、使い分けることを想定している   |
	//+------------------------------------------------+

	int CreateRootSignature() {
		D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
		std::vector<D3D12_ROOT_PARAMETER> root_parameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC> static_samplers;

		D3D12_DESCRIPTOR_RANGE srv_range = {};
		D3D12_DESCRIPTOR_RANGE srv_range2 = {};
		D3D12_ROOT_PARAMETER cbv_parameter = {};

		for (unsigned int i = StructuredBufferSlot; i < SRVSlot; i++)
			SetStructuredBufferParameter(root_signature_desc, root_parameters, i);

		SetSRVParameter(root_signature_desc, root_parameters, &srv_range, SRVSlot);

		for (unsigned int i = 0; i < CBVSlot - RootConstantSlot; i++)
			SetRootConstatntParameter(root_signature_desc, root_parameters, i);

		for (unsigned int i = CBVSlot - RootConstantSlot; i < RootParameterCount - RootConstantSlot; i++)
			SetCBVParameter(root_signature_desc, root_parameters, i);

		SetStaticSamplers(root_signature_desc, static_samplers);
		SetParameters(root_signature_desc, root_parameters);

		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		ComPtr<ID3DBlob> signature_blob;
		ComPtr<ID3DBlob> error_blob;
		HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature_blob, &error_blob);
		if (FAILED(hr)) {
			if (error_blob) {
				OutputDebugStringA((char*)error_blob->GetBufferPointer());
			}
			return -1;
		}
		hr = System::DirectX12Manager::Instance()->GetDevice()->CreateRootSignature(0, signature_blob->GetBufferPointer(), signature_blob->GetBufferSize(), IID_PPV_ARGS(root_signature.GetAddressOf()));
		if (FAILED(hr)) {
			return -1;
		}
		return 0;
	}
	int SetSRVParameter(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector<D3D12_ROOT_PARAMETER>& root_parameters,
		D3D12_DESCRIPTOR_RANGE* srv_range, unsigned int base_register) {

		//SRVをルートパラメータに追加する
		//ここではSRVを1つだけ追加しているが、
		//シェーダーのバージョンを5.1にしているので実質無限のSRVを使用することができる(Bindless Resourceという方式)
		D3D12_ROOT_PARAMETER srv_parameter = {};
		srv_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		srv_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		srv_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srv_range->BaseShaderRegister = base_register;
		srv_range->NumDescriptors = 1;

		srv_parameter.DescriptorTable.NumDescriptorRanges = 1;
		srv_parameter.DescriptorTable.pDescriptorRanges = srv_range;
#if 1
		{

		}
#endif
		root_parameters.push_back(srv_parameter);
		rs_desc.NumParameters++;
		return 0;

	}
	int SetRootConstatntParameter(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector<D3D12_ROOT_PARAMETER>& root_parameters,
		unsigned int start_slot) {
		//ルート定数をルートパラメータに追加する
		D3D12_ROOT_PARAMETER root_constant_parameter = {};
		root_constant_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		root_constant_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		root_constant_parameter.Constants.Num32BitValues = 1; //テクスチャインデックスだけで十分
		root_constant_parameter.Constants.ShaderRegister = start_slot;
		root_constant_parameter.Constants.RegisterSpace = 0;
		root_parameters.push_back(root_constant_parameter);
		rs_desc.NumParameters++;
		return 0;
	}
	int SetCBVParameter(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector<D3D12_ROOT_PARAMETER>& root_parameters,
		unsigned int start_slot) {
		//CBVをルートパラメータに追加する

		D3D12_ROOT_PARAMETER my_cbv_parameter;
		my_cbv_parameter = {};
		my_cbv_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		my_cbv_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		my_cbv_parameter.Constants.Num32BitValues = 256;
		my_cbv_parameter.Constants.ShaderRegister = start_slot;
		my_cbv_parameter.Constants.RegisterSpace = 0;
		root_parameters.push_back(my_cbv_parameter);
		rs_desc.NumParameters++;

		return 0;
	}
	int SetStructuredBufferParameter(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector<D3D12_ROOT_PARAMETER>& root_parameters, unsigned int start_slot) {
		//StructuredBufferをルートパラメータに追加する
		D3D12_ROOT_PARAMETER structured_buffer_parameter = {};
		structured_buffer_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		structured_buffer_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		structured_buffer_parameter.Descriptor.ShaderRegister = start_slot;
		root_parameters.push_back(structured_buffer_parameter);
		rs_desc.NumParameters++;
		return 0;
	}
	int SetStaticSamplers(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector< D3D12_STATIC_SAMPLER_DESC>& static_samplers) {
		//静的サンプラーをルートシグネチャに追加する
		//ここでは、1つのサンプラーを追加しているが、必要に応じて複数のサンプラーを追加することができる
		D3D12_STATIC_SAMPLER_DESC linear_wrap_sampler_desc = {};
		D3D12_STATIC_SAMPLER_DESC linear_clamp_sampler_desc = {};
		D3D12_STATIC_SAMPLER_DESC linear_mirror_sampler_desc = {};
		D3D12_STATIC_SAMPLER_DESC linear_border_sampler_desc = {};
		{
			linear_wrap_sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			linear_wrap_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			linear_wrap_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			linear_wrap_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			linear_wrap_sampler_desc.MipLODBias = 0.0f;
			linear_wrap_sampler_desc.MaxAnisotropy = 1;
			linear_wrap_sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			linear_wrap_sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			linear_wrap_sampler_desc.ShaderRegister = 0;

			linear_wrap_sampler_desc.MinLOD = 0.0f;
			linear_wrap_sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
			static_samplers.push_back(linear_wrap_sampler_desc);
		}
		{
			linear_clamp_sampler_desc = linear_wrap_sampler_desc;

			linear_clamp_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			linear_clamp_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			linear_clamp_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			linear_clamp_sampler_desc.ShaderRegister = 1;
			static_samplers.push_back(linear_clamp_sampler_desc);
		}
		{
			linear_mirror_sampler_desc = linear_wrap_sampler_desc;
			linear_mirror_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			linear_mirror_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			linear_mirror_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			linear_mirror_sampler_desc.ShaderRegister = 2;
			static_samplers.push_back(linear_mirror_sampler_desc);
		}
		{
			linear_border_sampler_desc = linear_wrap_sampler_desc;
			linear_border_sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			linear_border_sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			linear_border_sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			linear_border_sampler_desc.ShaderRegister = 3;
			static_samplers.push_back(linear_border_sampler_desc);
		}

		rs_desc.NumStaticSamplers = 4;
		rs_desc.pStaticSamplers = static_samplers.data();
		return 0;
	}
	int SetParameters(D3D12_ROOT_SIGNATURE_DESC& rs_desc, std::vector<D3D12_ROOT_PARAMETER>& root_parameters) {
		rs_desc.NumParameters = (UINT)root_parameters.size();
		rs_desc.pParameters = root_parameters.data();
		return 0;
	}
	ID3D12RootSignature* GetRootSignature() const { return root_signature.Get(); }

};

class PipelineState {
public:
	enum PipelineStateFlags {
		AlphaBlendEnable = 1 << 0,
		AlphaCheckEnable = 1 << 1,
		WireFrameEnable = 1 << 2,
		DepthTestEnable = 1 << 3,
		DepthWriteEnable = 1 << 4,
		CullBack = 1 << 5,
		CullFront = 1 << 6,

	};
private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> input_element_descs;
	D3D12_INPUT_LAYOUT_DESC input_layout_desc;

	ComPtr<ID3D12PipelineState> pipeline_state;

	RootSignature* root_signature;
	ComPtr<ID3DBlob> vs_blob;
	ComPtr<ID3DBlob> ps_blob;

public:
	ID3D12PipelineState* GetPipelineState() const { return pipeline_state.Get(); }
	bool IsValid() const { return pipeline_state != nullptr; }

	PipelineState(RootSignature* root_sig, const std::wstring& vs, const std::string& vs_entry, const std::wstring& ps, const std::string& ps_entry, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputs, unsigned int flags) {
		root_signature = root_sig;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

		pso_desc.pRootSignature = root_signature->GetRootSignature();


		ShaderCompiler::CompileShader(vs, nullptr, vs_entry.c_str(), ShaderCompiler::TargetShader::VertexShader, vs_blob);
		ShaderCompiler::CompileShader(ps, nullptr, ps_entry.c_str(), ShaderCompiler::TargetShader::PixelShader, ps_blob);
		pso_desc.VS.pShaderBytecode = vs_blob->GetBufferPointer();
		pso_desc.VS.BytecodeLength = vs_blob->GetBufferSize();
		pso_desc.PS.pShaderBytecode = ps_blob->GetBufferPointer();
		pso_desc.PS.BytecodeLength = ps_blob->GetBufferSize();

		input_element_descs = inputs;
		input_layout_desc.NumElements = (UINT)input_element_descs.size();
		input_layout_desc.pInputElementDescs = input_element_descs.data();
		pso_desc.InputLayout = input_layout_desc;

		D3D12_RASTERIZER_DESC rasterizer_desc = {};
		{
			rasterizer_desc.FillMode = (flags & WireFrameEnable) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
			rasterizer_desc.CullMode = (flags & CullBack) ? D3D12_CULL_MODE_BACK : ((flags & CullFront) ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_NONE);
			rasterizer_desc.FrontCounterClockwise = FALSE;
			rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterizer_desc.DepthClipEnable = TRUE;
			rasterizer_desc.MultisampleEnable = FALSE;
			rasterizer_desc.AntialiasedLineEnable = TRUE;
			rasterizer_desc.ForcedSampleCount = 0;
			rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			pso_desc.RasterizerState = rasterizer_desc;
		}
		D3D12_BLEND_DESC blend_desc = {};
		{
			blend_desc.AlphaToCoverageEnable = (flags & AlphaCheckEnable) ? TRUE : FALSE;
			blend_desc.IndependentBlendEnable = FALSE;
			blend_desc.RenderTarget[0].BlendEnable = (flags & AlphaBlendEnable) ? TRUE : FALSE;
			blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			pso_desc.BlendState = blend_desc;
		}
		{
			pso_desc.DepthStencilState.DepthEnable = (flags & DepthTestEnable) ? TRUE : FALSE;
			pso_desc.DepthStencilState.DepthWriteMask = (flags & (DepthWriteEnable | DepthTestEnable)) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			pso_desc.DepthStencilState.StencilEnable = FALSE;
		}
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pso_desc.NumRenderTargets = 1;
		pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		DXGI_SAMPLE_DESC sample_desc = {};
		{
			sample_desc.Count = 1;
			sample_desc.Quality = 0;
			pso_desc.SampleDesc = sample_desc;
		}
		HRESULT hr = System::DirectX12Manager::Instance()->GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pipeline_state.GetAddressOf()));
		if (FAILED(hr)) {
			return;
		}



	}



};



class Material {
private:
	ComPtr<ID3DBlob> vs_blob;
	ComPtr<ID3DBlob> ps_blob;
	ComPtr<ID3D12PipelineState> pipeline_state;

public:
	int CreatePipeLineState() {
	}
	struct InputElement {
		const char* semantic_name;
		unsigned int index;
		DXGI_FORMAT format;
	};

	int CreateInputLayout(const std::vector<InputElement>& input_elements, std::vector<D3D12_INPUT_ELEMENT_DESC>& input_element_descs) {
		input_element_descs.clear();
		for (size_t i = 0; i < input_elements.size(); i++) {
			D3D12_INPUT_ELEMENT_DESC element_desc = {};
			element_desc.SemanticName = input_elements[i].semantic_name;
			element_desc.SemanticIndex = input_elements[i].index;
			element_desc.Format = input_elements[i].format;
			element_desc.InputSlot = 0;
			element_desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			element_desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element_desc.InstanceDataStepRate = 0;
			input_element_descs.push_back(element_desc);
		}
		return 0;
	}

	int CompileShaderVS(const std::wstring& file_path) {
		return ShaderCompiler::CompileShader(file_path, nullptr, "main", ShaderCompiler::TargetShader::VertexShader, vs_blob);
	}
	int CompileShaderPS(const std::wstring& file_path) {
		return ShaderCompiler::CompileShader(file_path, nullptr, "main", ShaderCompiler::TargetShader::PixelShader, ps_blob);
	}
};


namespace System {
	struct ConstantBufferData {
		DirectX::XMMATRIX world_matrix;
		DirectX::XMMATRIX view_matrix;
		DirectX::XMMATRIX projection_matrix;
		DirectX::XMFLOAT3 eye_position;
		float system_time;
	};
	struct TextureIndices {
		unsigned int slot[10];
	};
	struct MaterialData {
		DirectX::XMFLOAT4 diffuse_color;
		float metallic;
		float roughness;
		TextureIndices texture_indices;
	};
	struct ObjectCBuffer {
		DirectX::XMMATRIX world_matrix;
	};
	struct CameraBuffer {
		DirectX::XMMATRIX view_matrix;
		DirectX::XMMATRIX projection_matrix;
	};

	std::unique_ptr<RootSignature> root_signature;
	std::unique_ptr<PipelineState> pipeline_state;


	std::array<std::unique_ptr<ConstantBufferTyped<ConstantBufferData>>, DirectX12Manager::DRAW_CONTEXT_FRAME_COUNT> frame_constant_buffers;
	std::unique_ptr<StructuredBufferTyped<MaterialData>> material_buffer;
	std::unique_ptr<StructuredBufferTyped<ObjectCBuffer>> objs_buffer;
	std::unique_ptr<StructuredBufferTyped<CameraBuffer>> camera_buffer;
	std::array<unsigned int, 4> tex_indices;

	std::unique_ptr<Texture> diffuse_texture;
	std::unique_ptr<Texture> normal_texture;
	std::unique_ptr<Texture> roughness_texture;
	std::unique_ptr<Texture> metallic_texture;
	std::unique_ptr<Texture> emission_texture;
	std::unique_ptr<Texture> depth_texture;
	struct MeshInfo {
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		unsigned int material_index;
	};
	std::vector<MeshInfo> meshes;

	std::vector<std::unique_ptr<VertexBuffer>> vertex_buffers;
	std::vector<std::unique_ptr<IndexBuffer>> index_buffers;

	//パイプライン周りの用語について
	//そもそもパイプラインとは何か
	//ギターやベースをやる人にわかりやすく伝えるときに、パイプライン全体をエフェクターやアンプで例えたことがあるが
	//その例えではシェーダーがエフェクター、ギターからアンプまで流れる生の信号をインプットと言える。

	//パイプラインステートオブジェクト(PSO)
	//PSOは、シェーダーやブレンドステート、ラスタライザーステートなどのグラフィックスパイプラインの設定をまとめたオブジェクト
	//上記の例えでいうと、「エフェクターボード」だと言ってよいだろう。
	//ギター、エフェクター、アンプの全てを1つの設定として予めて保存しておくことができるのがPSOである。

	//インプットアセンブラ
	//インプットアセンブラは、頂点バッファやインデックスバッファなどの入力データをGPUに渡す役割を持つステージ
	//ギターから入力される生の信号を、「どのような形式で入力されるのか」を定義するのがインプットアセンブラである。

	//シェーダー
	//シェーダーは、GPU上で実行されるプログラムのこと
	//エフェクターでいうと、「歪みエフェクター」や「ディレイエフェクター」などの個々のエフェクターに相当する
	//インプットで入ってきた生データを加工し、次のエフェクターないしはアンプに出力する役割を持つのがシェーダーである。

	//ルートシグネチャ
	//ルートシグネチャとは、シェーダーで使用するリソースを定義するための概念
	//ルートシグネチャは、エフェクターについている「ツマミ」のようなものだと解釈できる。
	//エフェクターが使用するパラメータ(歪みやディレイの量)において、「何を使用するのか」を定義するのがルートシグネチャ

	//では、なぜこれらをわざわざ予め作っているのか
	//D3D11では、シェーダーやブレンドステートなどの設定を個別に作成して、描画のたびにそれらをセットしていた。
	//上記の例えだと、音を一回鳴らす度に、使うエフェクターを刺し直して、個別にツマミを弄って、使うアンプ選び直すようなものだ。
	//色々なバリエーションの音を出すことができる反面、一音鳴らす度にこんな時間のかかることをやっていたのでは、リアルタイムで音を出すことができない。

	//D3D12では、シェーダーやブレンドステートなどの設定をまとめてPSOとして作成しておくことができる。
	//エフェクターやその設定を、「ボードごと」保存しておいて、使うボードを刺すだけで、エフェクターの選択もツマミの設定も一気に切り替えることができるようになった。
	//これにより、リアルタイムで様々なバリエーションの音を出すことができるようになった。
	//ただし欠点として、ボードの構築は非常に時間がかかるため、リアルタイムでボードを構築することはできない。
	//その上、ライブ会場に大量のボードを持ち込むと非常に場所を取ってしまう。これはメモリの使用量が増えることに相当する。

	//ルートシグネチャも同様で、シェーダーで使用するリソースをまとめて定義しておくことができる。







	ApplicationManager* ApplicationManager::Instance()
	{
		static ApplicationManager manager;
		return &manager;
	}

	int ApplicationManager::RunApplication()
	{

		//アプリケーションの初期化
		if (Initialize() != 0)
		{
			//初期化に失敗した場合は、アプリを終了する
			Finalize();
			return -1;
		}

		MainLoop();		// メインループ


		return Finalize();		// アプリケーションの終了処理

	}
	int ApplicationManager::Initialize()
	{
		//ウィンドウマネージャーの初期化
		if (WindowManager::Instance()->Initialize() != 0) return -1;
		if (DirectX12Manager::Instance()->Initialize() != 0) return -1;
		if (WindowManager::Instance()->CreateSwapChain() != 0) return -1;
		//if (SystemGUI::InitImGui(WindowManager::Instance()->GetWindowHandle(), DirectX12Manager::Instance()->GetDevice(), DirectX12Manager::Instance()->GetDrawQueue()) != 0) return -1;



		auto back_buffer = WindowManager::Instance()->GetCurrentBackBuffer();
		////コマンドリスト(コンテキスト)は、初期化段階でCloseされているので、最初のフレームの描画を始める前にResetしておく必要がある
		//if (DirectX12Manager::Instance()->GetDrawContext()->ResetCommandList() != 0) return -1;
		DirectX::XMFLOAT4 mat_diffuse_color[10] = {};

		//頂点バッファとインデックスバッファの作成(読み込みからバッファの作成、転送まで)
		{
			Assimp::Importer importer;
			//const aiScene* scene = importer.ReadFile("Assets/cube.obj", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
			//const aiScene* scene = importer.ReadFile("Assets/sphere.obj", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
			const aiScene* scene = importer.ReadFile("Assets/Y Bot LOD.fbx", aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
			//const aiScene* scene = importer.ReadFile("Assets/Fish.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
			if (!scene || !scene->HasMeshes()) {
				return -1;
			}
			for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
				const aiMaterial* material = scene->mMaterials[i];
				aiColor4D diffuse_color;
				material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
				mat_diffuse_color[i] = DirectX::XMFLOAT4(diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a);
			}
			for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
				const aiMesh* mesh = scene->mMeshes[i];
				MeshInfo info;
				info.vertices.reserve(mesh->mNumVertices * 12);
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					//頂点位置
					info.vertices.push_back(mesh->mVertices[j].x);
					info.vertices.push_back(mesh->mVertices[j].y);
					info.vertices.push_back(mesh->mVertices[j].z);
					//頂点カラー
					if (mesh->HasVertexColors(0)) {
						info.vertices.push_back(mesh->mColors[0][j].r);
						info.vertices.push_back(mesh->mColors[0][j].g);
						info.vertices.push_back(mesh->mColors[0][j].b);
						info.vertices.push_back(mesh->mColors[0][j].a);
					}
					else {
						info.vertices.push_back(1.0f);
						info.vertices.push_back(1.0f);
						info.vertices.push_back(1.0f);
						info.vertices.push_back(1.0f);
					}
					//テクスチャ座標
					if (mesh->HasTextureCoords(0)) {
						info.vertices.push_back(mesh->mTextureCoords[0][j].x);
						info.vertices.push_back(mesh->mTextureCoords[0][j].y);
					}
					else {
						info.vertices.push_back(0.0f);
						info.vertices.push_back(0.0f);
					}
					//法線ベクトル
					if (mesh->HasNormals()) {
						info.vertices.push_back(mesh->mNormals[j].x);
						info.vertices.push_back(mesh->mNormals[j].y);
						info.vertices.push_back(mesh->mNormals[j].z);
					}
					else {
						info.vertices.push_back(0.0f);
						info.vertices.push_back(0.0f);
						info.vertices.push_back(0.0f);
					}
				}
				info.indices.reserve(mesh->mNumFaces * 3);
				for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
					const aiFace& face = mesh->mFaces[i];
					for (unsigned int j = 0; j < face.mNumIndices; j++) {
						info.indices.push_back(face.mIndices[j]);
					}
				}
				info.material_index = mesh->mMaterialIndex;
				meshes.push_back(std::move(info));
			}
			if (vertex_buffers.empty()) {
				vertex_buffers.reserve(meshes.size());
				for (auto& mesh_info : meshes)
					vertex_buffers.push_back(std::make_unique<VertexBuffer>(mesh_info.vertices, 12, D3D12_HEAP_TYPE_DEFAULT));
			}
			if (index_buffers.empty()) {
				index_buffers.reserve(meshes.size());
				for (auto& mesh_info : meshes)
					index_buffers.push_back(std::make_unique<IndexBuffer>(mesh_info.indices, D3D12_HEAP_TYPE_DEFAULT));
			}
		}



		if (!root_signature) {
			root_signature = std::make_unique<RootSignature>();
			if (root_signature->CreateRootSignature() != 0) {
				return -1;
			}
		}
		if (!depth_texture) {
			//深度バッファを作成する
			depth_texture = Texture::TextureLoader::CreateEmpty(WindowManager::Instance()->GetBackBufferWidth(), WindowManager::Instance()->GetBackBufferHeight(), DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			if (!depth_texture->IsValid()) {
				return -1;
			}
		}

		if (!pipeline_state) {

			static constexpr unsigned int INPUT_ELEMENT_COUNT = 4;
			static D3D12_INPUT_LAYOUT_DESC input_layout_desc = {};
			input_layout_desc.NumElements = INPUT_ELEMENT_COUNT;
			std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements(INPUT_ELEMENT_COUNT);
			input_elements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			input_elements[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			input_elements[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			input_elements[3] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			pipeline_state = std::make_unique<PipelineState>(root_signature.get(), L"Assets/Shaders/simple_vs.fx", "main", L"Assets/Shaders/simple_ps.fx", "main", input_elements, PipelineState::DepthTestEnable | PipelineState::DepthWriteEnable | PipelineState::CullBack | PipelineState::AlphaBlendEnable);
			if (!pipeline_state->IsValid()) {
				return -1;
			}


		}
		if (!diffuse_texture) {
			diffuse_texture = Texture::TextureLoader::LoadFromFile(L"Assets/Textures/sample_diffuse.jpg");
			if (!diffuse_texture->IsValid()) {
				return -1;
			}
		}

		if (!normal_texture) {
			normal_texture = Texture::TextureLoader::LoadFromFile(L"Assets/Textures/sample_normal.png");
			if (!normal_texture->IsValid()) {
				return -1;
			}
		}
		if (!roughness_texture) {
			roughness_texture = Texture::TextureLoader::LoadFromFile(L"Assets/Textures/sample_roughness.jpg");
			if (!roughness_texture->IsValid()) {
				return -1;
			}
		}
		if (!metallic_texture) {
			metallic_texture = Texture::TextureLoader::LoadFromFile(L"Assets/Textures/sample_metallic.jpg");
			if (!metallic_texture->IsValid()) {
				return -1;
			}
		}
		//if (!emission_texture) {
		//	emission_texture = Texture::TextureLoader::LoadFromFile(L"Assets/Textures/sample_emission.jpg");
		//	if (!emission_texture->IsValid()) {
		//		return -1;
		//	}
		//}

		if (!frame_constant_buffers[0]) {
			for (size_t i = 0; i < DirectX12Manager::DRAW_CONTEXT_FRAME_COUNT; i++)
			{
				auto cb = std::make_unique<ConstantBufferTyped<ConstantBufferData>>();
				frame_constant_buffers[i] = (std::move(cb));

			}
			//CreateConstantBufer();

		}
		if (!material_buffer) {
			material_buffer = std::make_unique<StructuredBufferTyped<MaterialData>>(10);
			tex_indices = {
				diffuse_texture->Srv()->GetIndex(),
				normal_texture->Srv()->GetIndex(),
				roughness_texture->Srv()->GetIndex(),
				metallic_texture->Srv()->GetIndex(),
				//emission_texture->Srv()->GetIndex()
			};
			for (size_t i = 0; i < 10; i++) {
				material_buffer->At(i)->diffuse_color = mat_diffuse_color[i];
				material_buffer->At(i)->texture_indices.slot[0] = tex_indices[0];
				material_buffer->At(i)->texture_indices.slot[1] = tex_indices[1];
				material_buffer->At(i)->texture_indices.slot[2] = tex_indices[2];
				material_buffer->At(i)->texture_indices.slot[3] = tex_indices[3];
			}
		}
		if (!objs_buffer) {
			objs_buffer = std::make_unique<StructuredBufferTyped<ObjectCBuffer>>(10000);

		}





		return 0;
	}
	int ApplicationManager::MainLoop()
	{
		MSG msg;		// Windowsのメッセージを格納する構造体



		while (true) {
			static bool resize_flag = false;
			static bool size_up = true;
			static constexpr unsigned int upper_width = 1920 * 2;
			static constexpr unsigned int upper_height = 1080 * 2;
			static constexpr unsigned int lower_width = 1920 / 2;
			static constexpr unsigned int lower_height = 1080 / 2;
			if (resize_flag) {
				WindowManager::Instance()->ResizeBackBuffers(size_up ? upper_width : lower_width, size_up ? upper_height : lower_height);
				depth_texture = Texture::TextureLoader::CreateEmpty(WindowManager::Instance()->GetBackBufferWidth(), WindowManager::Instance()->GetBackBufferHeight(), DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
				resize_flag = false;
				size_up = !size_up;
			}

			// メッセージが来ているか確認。来ていればmsgに格納して、キューから削除する
			// 英語でPeek = 「覗く」という意味。PeekMessageは「メッセージを覗いてみる」という意味で、メッセージが来ているか確認する関数
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				// 来ていた場合、TranslateMessageでキーボード入力などのメッセージを翻訳して、DispatchMessageでウィンドウプロシージャに送る
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// WM_QUIT(「アプリ終わるでー」のメッセージ)が来ていたら、ループを抜けてアプリを終了する
			if (msg.message == WM_QUIT) {

				break;
			}



			if constexpr (true) {
				// ここにゲームの更新や描画のコードを入れることになる

				auto back_buffer = WindowManager::Instance()->GetCurrentBackBuffer();
				auto handle = back_buffer->Rtv()->GetCPUHandle();
				auto dsv_handle = depth_texture->Dsv()->GetCPUHandle();
				auto cmd_list = DirectX12Manager::Instance()->GetDrawContext()->GetCommandList();
				auto cmd_allocator = DirectX12Manager::Instance()->GetDrawContext()->GetCommandAllocator();

				if (DirectX12Manager::Instance()->DrawBegin() < 0) {
					return -1;
				}

				D3D12_RESOURCE_BARRIER begin_barrier = {};
				begin_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				begin_barrier.Transition.pResource = back_buffer->GetResource();
				begin_barrier.Transition.Subresource = 0;
				begin_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				begin_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				cmd_list->ResourceBarrier(1, &begin_barrier);

				cmd_list->OMSetRenderTargets(1, &handle, FALSE, &dsv_handle);
				float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				cmd_list->ClearRenderTargetView(handle, clear_color, 0, nullptr);
				cmd_list->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
				//試験的に虹色トライアングルの描画コマンドを入れてみる
				if constexpr (true) {

					//定数バッファにデータを転送する
					{
						ConstantBufferData* mapped_data = nullptr;
						mapped_data = frame_constant_buffers[DirectX12Manager::Instance()->GetFrameIndex()]->Map();
						if (!mapped_data) {
							return -1;
						}
						//以前はstd::copyを使っていたが、templateを使用することによってMap関数が定数バッファのデータ型に合わせて
						//適切なポインタを返すようになったので、直接メンバにアクセスして値をセットすることができるようになった。
						//また、template入れる型と同じ型を使えばバッファオーバーランを起こす心配もなくなった。
						//std::copy(&cb_data, &cb_data + 1, mapped_data);
						static float system_time = 0;
						system_time += 0.01f;
						mapped_data->system_time = system_time;
						{
							DirectX::XMMATRIX w_m = DirectX::XMMatrixRotationY(system_time) *
								//DirectX::XMMatrixRotationZ(system_time) *
								DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
								DirectX::XMMatrixTransformation(DirectX::XMVectorZero(), DirectX::XMQuaternionIdentity(), DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMVectorZero(), DirectX::XMQuaternionIdentity(), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
							mapped_data->world_matrix = w_m;
							DirectX::XMMATRIX v_m = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(-10.0f, 10.0f, -10.0f, 1.0f), DirectX::XMVectorSet(0.0f, 3.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
							mapped_data->view_matrix = v_m;
							DirectX::XMMATRIX p_m = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), static_cast<float>(WindowManager::Instance()->GetWindowWidth()) / WindowManager::Instance()->GetWindowHeight(), 0.1f, 1000.0f);
							mapped_data->projection_matrix = p_m;
							mapped_data->eye_position = DirectX::XMFLOAT3(-10.0f, 10.0f, -10.0f);
						}
						int elem_count = objs_buffer->GetElementCount();
						int x_count = 40;
						int z_count = 40;
						for (int i = 0; i < elem_count; i++) {
							objs_buffer->At(i)->world_matrix =
								DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
								DirectX::XMMatrixRotationY(system_time+0.05f*i) *
								//DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(90.0f)) *
								//DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(90.0f)) *
								DirectX::XMMatrixTranslation((i % x_count) * 0.5f, i / (x_count * z_count), (i / x_count - i / (x_count * z_count) * x_count) * 0.5f);
						}
						//constant_buffer->Unmap();

					}
					//このままではラスタライザーで全ての頂点がdiscardされてしまうため、ビューポートとシザー矩形を画面全体に設定しておく
					{
						D3D12_VIEWPORT viewport = {};
						viewport.TopLeftX = 0.0f;
						viewport.TopLeftY = 0.0f;
						viewport.Width = static_cast<float>(back_buffer->GetResourceDesc().Width);
						viewport.Height = static_cast<float>(back_buffer->GetResourceDesc().Height);
						viewport.MinDepth = 0.0f;
						viewport.MaxDepth = 1.0f;
						cmd_list->RSSetViewports(1, &viewport);
						D3D12_RECT scissor_rect = {};
						scissor_rect.left = 0;
						scissor_rect.top = 0;
						scissor_rect.right = static_cast<LONG>(back_buffer->GetResourceDesc().Width);
						scissor_rect.bottom = static_cast<LONG>(back_buffer->GetResourceDesc().Height);
						cmd_list->RSSetScissorRects(1, &scissor_rect);


					}

					cmd_list->SetGraphicsRootSignature(root_signature->GetRootSignature());
					cmd_list->SetPipelineState(pipeline_state->GetPipelineState());
					cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					ID3D12DescriptorHeap* descriptor_heaps[] = { System::DirectX12Manager::Instance()->GetCBVSRVUAVHeap()->GetHeap() };
					cmd_list->SetDescriptorHeaps(1, descriptor_heaps);


					cmd_list->SetGraphicsRootShaderResourceView(RootSignature::StructuredBufferSlot, material_buffer->GetResource()->GetGPUVirtualAddress());
					cmd_list->SetGraphicsRootShaderResourceView(RootSignature::StructuredBufferSlot + 1, objs_buffer->GetResource()->GetGPUVirtualAddress());
					cmd_list->SetGraphicsRootDescriptorTable(RootSignature::SRVSlot, System::DirectX12Manager::Instance()->GetCBVSRVUAVHeap()->GetStartGPUHandle());

					cmd_list->SetGraphicsRootConstantBufferView(RootSignature::CBVSlot, frame_constant_buffers[DirectX12Manager::Instance()->GetFrameIndex()]->GetResource()->GetGPUVirtualAddress());



					unsigned int idx_offset = 0;
					unsigned int vertex_offset = 0;

					for (size_t i = 0; i < meshes.size(); ++i) {
						//頂点バッファをセットして、描画コマンドを発行する
						{

							cmd_list->IASetVertexBuffers(0, 1, vertex_buffers[i]->GetViewPtr());
						}
						//インデックスバッファをセットする
						{
							cmd_list->IASetIndexBuffer(index_buffers[i]->GetViewPtr());
						}
						cmd_list->SetGraphicsRoot32BitConstant(RootSignature::RootConstantSlot, static_cast<UINT>(meshes[i].material_index), 0);


						//インスタンス描画を行う。
						//今回用意したモデルは無駄に50000ポリゴンあるが、
						//50000*1000体で
						//合計5000万ポリゴンを2ドローコールで描画することができる。
						cmd_list->DrawIndexedInstanced(meshes[i].indices.size(), 5000, idx_offset, vertex_offset, 0);
					}

				}



#if 0
				SystemGUI::ImGuiDrawBegin();
				{

					//仮でfps計測してみる
					auto now = std::chrono::high_resolution_clock::now();
					static auto last_time = now;
					auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time);
					if (delta.count() > 0) {
						double fps = 1000000.0 / delta.count();
						ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
						ImGui::Begin("preview");
						ImGui::Image((ImTextureID)random_noise_texture->Srv()->GetGPUHandle().ptr, ImVec2(static_cast<float>(random_noise_texture->GetResourceDesc().Width), static_cast<float>(random_noise_texture->GetResourceDesc().Height)));
						ImGui::End();
						ImGui::Begin("FPS_2");
						ImGui::End();
						ImGui::Begin("FPS");
						if (ImGui::Button("Toggle.."))
							ImGui::OpenPopup("FPSPopup");
						if (ImGui::BeginPopup("FPSPopup")) {
							std::string str = "Current FPS: " + std::to_string(fps);
							ImGui::InputText("Input", str.data(), str.size() + 1);
							ImGui::EndPopup();
						}
						ImGui::Text("FPS: %.2f", fps);
						ImGui::End();
					}
					last_time = now;
				}

				SystemGUI::ImGuiDrawEnd();
				SystemGUI::PresentImGui(cmd_list);
#endif
				static int press_counter = 0;
				if (GetKeyState(VK_SPACE) & 0x8000) {
					if (press_counter == 0) {
						resize_flag = true;
					}
					press_counter++;
				}
				else {
					press_counter = 0;
				}

				D3D12_RESOURCE_BARRIER end_barrier = {};
				end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				end_barrier.Transition.pResource = back_buffer->GetResource();
				end_barrier.Transition.Subresource = 0;
				end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				cmd_list->ResourceBarrier(1, &end_barrier);

				if (DirectX12Manager::Instance()->DrawEnd() < 0) {
					return -1;
				}

				if (WindowManager::Instance()->ScreenFlip() < 0)
					return -1;


			}


		}
		return 0;

	}
	int ApplicationManager::Finalize()
	{
		//SystemGUI::DestroyImGui();
		WindowManager::Instance()->ReleaseSwapChain();
		DirectX12Manager::Instance()->Finalize();
		WindowManager::Instance()->Finalize();

		return 0;
	}
}