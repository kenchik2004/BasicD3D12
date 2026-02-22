#include "precompile.h"
#include "DirectX12Manager.h"

#include <stdexcept>

// デバイスとコマンドキューマネージャーを初期化する
bool DirectX12Manager::Initialize()
{
    // デバイスを作成
    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device_));
    if (FAILED(hr)) {
        return false;
    }

    // コマンドキューマネージャーを作成（デバイスを渡して内部でキューとフェンスを生成）
    try {
        command_queue_manager_ = std::make_unique<CommandQueueManager>(device_.Get());
    }
    catch (const std::runtime_error&) {
        return false;
    }

    return true;
}

// 保持するリソースを解放する
void DirectX12Manager::Finalize()
{
    command_queue_manager_.reset();
    device_.Reset();
}
