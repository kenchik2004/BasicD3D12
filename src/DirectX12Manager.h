#pragma once

#include "CommandQueueManager.h"
#include <memory>

// DirectX 12 デバイスとコマンドキューマネージャーを管理するクラス
class DirectX12Manager : SINGLETON_CLASS(DirectX12Manager) {
public:
    // デバイスとコマンドキューマネージャーを初期化する
    bool Initialize();

    // 保持するリソースを解放する
    void Finalize();

    ID3D12Device*        GetDevice()             const { return device_.Get(); }
    CommandQueueManager* GetCommandQueueManager() const { return command_queue_manager_.get(); }

private:
    ComPtr<ID3D12Device>                 device_;
    std::unique_ptr<CommandQueueManager> command_queue_manager_;
};
