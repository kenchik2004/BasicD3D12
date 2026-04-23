#include "PipeLineState.h"

namespace System {
	PipeLineState::PipeLineState(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc)
	{

		HRESULT hr = device->CreateGraphicsPipelineState(desc, IID_PPV_ARGS(pipeline_state.GetAddressOf()));
		if (FAILED(hr)) {
			pipeline_state.Reset();
			return;
		}
	}
}