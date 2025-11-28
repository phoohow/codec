#include "codecImpl.h"

#include "NvEncoder/NvEncoderD3D12.h"
#include "Utils/NvCodecUtils.h"

#include <d3d12.h>

#include <iostream>
#include <stdexcept>

namespace cdc
{

DX12Encoder::DX12Encoder() : m_encoder(nullptr), m_initialized(false) {}

DX12Encoder::~DX12Encoder()
{
    Destroy();
}

bool DX12Encoder::Initialize(const CreateParams& params)
{
    if (params.deviceType != DEVICE_TYPE_DX12)
    {
        return false;
    }

    m_params = params;

    try
    {
        ID3D12Device* pD3D12Device = reinterpret_cast<ID3D12Device*>(params.device);
        m_encoder                  = new NvEncoderD3D12(pD3D12Device, params.width, params.height, NV_ENC_BUFFER_FORMAT_ARGB);

        // Setup encoding parameters
        NV_ENC_INITIALIZE_PARAMS initializeParams = {NV_ENC_INITIALIZE_PARAMS_VER};
        NV_ENC_CONFIG            encodeConfig     = {NV_ENC_CONFIG_VER};

        initializeParams.encodeConfig = &encodeConfig;
        m_encoder->CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_P4_GUID);
        initializeParams.tuningInfo = NV_ENC_TUNING_INFO_HIGH_QUALITY;

        // Initialize encoder
        m_encoder->CreateEncoder(&initializeParams);
        m_initialized = true;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize DX12 encoder: " << e.what() << std::endl;
        return false;
    }
}

bool DX12Encoder::CopyTextureToEncoder(ID3D12Resource* src_tex, ID3D12Resource* dst_tex)
{
    ID3D12Device* device = reinterpret_cast<ID3D12Device*>(m_params.device);
    if (!device || !src_tex || !dst_tex)
    {
        std::cerr << "DX12Encoder: invalid D3D12 pointers" << std::endl;
        return false;
    }

    // Create a temporary command queue, allocator and list for the copy.
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        cmd_queue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    cmd_alloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd;
    Microsoft::WRL::ComPtr<ID3D12Fence>               fence;
    HANDLE                                            event       = nullptr;
    UINT64                                            fence_value = 1;

    D3D12_COMMAND_QUEUE_DESC qdesc = {};
    qdesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    qdesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&cmd_queue))))
    {
        std::cerr << "DX12Encoder: failed to create command queue" << std::endl;
        return false;
    }

    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_alloc))))
    {
        std::cerr << "DX12Encoder: failed to create command allocator" << std::endl;
        return false;
    }

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_alloc.Get(), nullptr, IID_PPV_ARGS(&cmd))))
    {
        std::cerr << "DX12Encoder: failed to create command list" << std::endl;
        return false;
    }

    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        std::cerr << "DX12Encoder: failed to create fence" << std::endl;
        return false;
    }

    event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event)
    {
        std::cerr << "DX12Encoder: failed to create event" << std::endl;
        return false;
    }

    // Transition resources to copy states. Assume source is in PRESENT and dst is COMMON.
    D3D12_RESOURCE_BARRIER src_barrier = {};
    src_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    src_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    src_barrier.Transition.pResource   = src_tex;
    src_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    src_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    src_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;

    D3D12_RESOURCE_BARRIER dst_barrier = {};
    dst_barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    dst_barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    dst_barrier.Transition.pResource   = dst_tex;
    dst_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dst_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    dst_barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;

    cmd->ResourceBarrier(1, &src_barrier);
    cmd->ResourceBarrier(1, &dst_barrier);

    // Perform the copy. Use CopyTextureRegion with a source box sized to the
    // destination (encoder input) to avoid mismatched-dimension errors.
    D3D12_RESOURCE_DESC srcDesc = src_tex->GetDesc();
    D3D12_RESOURCE_DESC dstDesc = dst_tex->GetDesc();

    uint32_t srcWidth  = static_cast<uint32_t>(srcDesc.Width);
    uint32_t srcHeight = srcDesc.Height;
    uint32_t dstWidth  = static_cast<uint32_t>(dstDesc.Width);
    uint32_t dstHeight = dstDesc.Height;

    uint32_t copyWidth  = (dstWidth <= srcWidth) ? dstWidth : srcWidth;
    uint32_t copyHeight = (dstHeight <= srcHeight) ? dstHeight : srcHeight;

    D3D12_BOX srcBox = {};
    srcBox.left      = 0;
    srcBox.top       = 0;
    srcBox.front     = 0;
    srcBox.right     = copyWidth;
    srcBox.bottom    = copyHeight;
    srcBox.back      = 1;

    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource                   = src_tex;
    srcLoc.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLoc.SubresourceIndex            = 0;

    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource                   = dst_tex;
    dstLoc.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex            = 0;

    cmd->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, &srcBox);

    // Transition back
    std::swap(src_barrier.Transition.StateBefore, src_barrier.Transition.StateAfter);
    std::swap(dst_barrier.Transition.StateBefore, dst_barrier.Transition.StateAfter);
    cmd->ResourceBarrier(1, &dst_barrier);
    cmd->ResourceBarrier(1, &src_barrier);

    cmd->Close();

    ID3D12CommandList* cmd_list[] = {cmd.Get()};
    cmd_queue->ExecuteCommandLists(1, cmd_list);

    // Signal and wait
    if (FAILED(cmd_queue->Signal(fence.Get(), fence_value)))
    {
        std::cerr << "DX12Encoder: failed to signal fence" << std::endl;
        CloseHandle(event);
        return false;
    }

    if (fence->GetCompletedValue() < fence_value)
    {
        if (FAILED(fence->SetEventOnCompletion(fence_value, event)))
        {
            std::cerr << "DX12Encoder: failed to set event on fence" << std::endl;
            CloseHandle(event);
            return false;
        }
        WaitForSingleObject(event, INFINITE);
    }

    CloseHandle(event);
    return true;
}

bool DX12Encoder::EncodeFrame(void* pData, CodecPacket& packet)
{
    if (!m_initialized || !m_encoder)
    {
        return false;
    }

    try
    {
        const NvEncInputFrame* next_input = m_encoder->GetNextInputFrame();
        if (!next_input || !next_input->inputPtr)
        {
            std::cerr << "DX12Encoder: no internal input buffer available" << std::endl;
            return false;
        }

        ID3D12Resource* src_tex = reinterpret_cast<ID3D12Resource*>(pData);
        ID3D12Resource* dst_tex = reinterpret_cast<ID3D12Resource*>(next_input->inputPtr);

        // Use the new function to copy texture
        if (!CopyTextureToEncoder(src_tex, dst_tex))
        {
            return false;
        }

        // Now call encoder to encode the copied internal input buffer
        std::vector<NvEncOutputFrame> vOutputFrames;
        m_encoder->EncodeFrame(vOutputFrames, nullptr);

        // Convert to our format
        if (!vOutputFrames.empty())
        {
            // For simplicity, we're taking the first packet
            packet.size = static_cast<uint32_t>(vOutputFrames[0].frame.size());
            packet.data = new uint8_t[packet.size];
            memcpy(packet.data, vOutputFrames[0].frame.data(), packet.size);
            packet.timestamp = static_cast<uint64_t>(vOutputFrames[0].timeStamp);
            packet.keyFrame  = (vOutputFrames[0].pictureType == NV_ENC_PIC_TYPE_IDR);
            return true;
        }

        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to encode frame: " << e.what() << std::endl;
        return false;
    }
}

bool DX12Encoder::Flush(CodecPacket& packet)
{
    if (!m_initialized || !m_encoder)
    {
        return false;
    }

    try
    {
        // Use the correct NvEncoder function signature
        std::vector<NvEncOutputFrame> vOutputFrames;
        m_encoder->EndEncode(vOutputFrames);

        // Convert to our format
        if (!vOutputFrames.empty())
        {
            // For simplicity, we're taking the first packet
            packet.size = static_cast<uint32_t>(vOutputFrames[0].frame.size());
            packet.data = new uint8_t[packet.size];
            memcpy(packet.data, vOutputFrames[0].frame.data(), packet.size);
            packet.timestamp = static_cast<uint64_t>(vOutputFrames[0].timeStamp);
            packet.keyFrame  = (vOutputFrames[0].pictureType == NV_ENC_PIC_TYPE_IDR);
            return true;
        }

        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to flush encoder: " << e.what() << std::endl;
        return false;
    }
}

void DX12Encoder::Destroy()
{
    if (m_encoder)
    {
        m_encoder->DestroyEncoder();
        delete m_encoder;
        m_encoder = nullptr;
    }
    m_initialized = false;
}

} // namespace cdc
