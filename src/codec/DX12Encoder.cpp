#include "codecImpl.h"

#include "NvEncoder/NvEncoderD3D12.h"
#include "Utils/NvCodecUtils.h"

#include <d3d12.h>

#include <iostream>
#include <stdexcept>

namespace cdc
{

DX12Encoder::DX12Encoder() : m_pEncoder(nullptr), m_initialized(false) {}

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
        m_pEncoder                 = new NvEncoderD3D12(pD3D12Device, params.width, params.height, NV_ENC_BUFFER_FORMAT_ARGB);

        // Setup encoding parameters
        NV_ENC_INITIALIZE_PARAMS initializeParams = {NV_ENC_INITIALIZE_PARAMS_VER};
        NV_ENC_CONFIG            encodeConfig     = {NV_ENC_CONFIG_VER};

        initializeParams.encodeConfig = &encodeConfig;
        m_pEncoder->CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_P4_GUID);
        initializeParams.tuningInfo = NV_ENC_TUNING_INFO_HIGH_QUALITY;

        // Initialize encoder
        m_pEncoder->CreateEncoder(&initializeParams);
        m_initialized = true;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize DX12 encoder: " << e.what() << std::endl;
        return false;
    }
}

bool DX12Encoder::EncodeFrame(void* pData, CodecPacket& packet)
{
    if (!m_initialized || !m_pEncoder)
    {
        return false;
    }

    try
    {
        ID3D12Resource* pTex2D = reinterpret_cast<ID3D12Resource*>(pData);

        // Use the correct NvEncoder function signature
        std::vector<NvEncOutputFrame> vOutputFrames;
        m_pEncoder->EncodeFrame(vOutputFrames, nullptr);

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
    if (!m_initialized || !m_pEncoder)
    {
        return false;
    }

    try
    {
        // Use the correct NvEncoder function signature
        std::vector<NvEncOutputFrame> vOutputFrames;
        m_pEncoder->EndEncode(vOutputFrames);

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
    if (m_pEncoder)
    {
        m_pEncoder->DestroyEncoder();
        delete m_pEncoder;
        m_pEncoder = nullptr;
    }
    m_initialized = false;
}

} // namespace cdc
