#include "codecImpl.h"

#include "NvEncoder/NvEncoderCuda.h"
#include "Utils/NvCodecUtils.h"

#include <cuda.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace cdc
{

    CudaEncoder::CudaEncoder() : m_pEncoder(nullptr), m_initialized(false) {}

    CudaEncoder::~CudaEncoder()
    {
        Destroy();
    }

    bool CudaEncoder::Initialize(const CreateParams &params)
    {
        if (params.deviceType != DEVICE_TYPE_CUDA)
        {
            return false;
        }

        m_params = params;

        try
        {
            CUcontext cuContext = reinterpret_cast<CUcontext>(params.device);
            m_pEncoder = new NvEncoderCuda(cuContext, params.width, params.height,
                                           static_cast<NV_ENC_BUFFER_FORMAT>(params.specifics.cuda.format));

            // Setup encoding parameters
            NV_ENC_INITIALIZE_PARAMS initializeParams = {NV_ENC_INITIALIZE_PARAMS_VER};
            NV_ENC_CONFIG encodeConfig = {NV_ENC_CONFIG_VER};

            initializeParams.encodeConfig = &encodeConfig;
            m_pEncoder->CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_P4_GUID);

            // Initialize encoder
            m_pEncoder->CreateEncoder(&initializeParams);
            m_initialized = true;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to initialize CUDA encoder: " << e.what() << std::endl;
            return false;
        }
    }

    bool CudaEncoder::EncodeFrame(void *pData, CodecPacket &packet)
    {
        if (!m_initialized || !m_pEncoder)
        {
            return false;
        }

        try
        {
            CUdeviceptr pDevidePtr = reinterpret_cast<CUdeviceptr>(pData);

            // Use the correct NvEncoder function signature
            std::vector<NvEncOutputFrame> vPacket;
            m_pEncoder->EncodeFrame(vPacket, nullptr);

            // Convert to our format
            if (!vPacket.empty())
            {
                // For simplicity, we're taking the first packet
                packet.size = static_cast<uint32_t>(vPacket[0].frame.size());
                packet.data = new uint8_t[packet.size];
                memcpy(packet.data, vPacket[0].frame.data(), packet.size);
                packet.timestamp = static_cast<uint64_t>(vPacket[0].timeStamp);
                packet.keyFrame = (vPacket[0].pictureType == NV_ENC_PIC_TYPE_IDR);
                return true;
            }

            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to encode frame: " << e.what() << std::endl;
            return false;
        }
    }

    bool CudaEncoder::Flush(CodecPacket &packet)
    {
        if (!m_initialized || !m_pEncoder)
        {
            return false;
        }

        try
        {
            // Use the correct NvEncoder function signature
            std::vector<NvEncOutputFrame> vPacket;
            m_pEncoder->EndEncode(vPacket);

            // Convert to our format
            if (!vPacket.empty())
            {
                // For simplicity, we're taking the first packet
                packet.size = static_cast<uint32_t>(vPacket[0].frame.size());
                packet.data = new uint8_t[packet.size];
                memcpy(packet.data, vPacket[0].frame.data(), packet.size);
                packet.timestamp = static_cast<uint64_t>(vPacket[0].timeStamp);
                packet.keyFrame = (vPacket[0].pictureType == NV_ENC_PIC_TYPE_IDR);
                return true;
            }

            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to flush encoder: " << e.what() << std::endl;
            return false;
        }
    }

    void CudaEncoder::Destroy()
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