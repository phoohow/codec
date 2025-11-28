#include "codecImpl.h"

#include "NvDecoder/NvDecoder.h"
#include "Utils/NvCodecUtils.h"

#include <cuda.h>

#include <iostream>
#include <vector>

namespace cdc
{

CudaDecoder::CudaDecoder() : m_decoder(nullptr), m_initialized(false) {}

CudaDecoder::~CudaDecoder()
{
    Destroy();
}

bool CudaDecoder::Initialize(const CreateParams& params)
{
    if (params.deviceType != DEVICE_TYPE_CUDA)
    {
        return false;
    }

    m_params = params;

    try
    {
        CUcontext cuContext = reinterpret_cast<CUcontext>(params.device);
        // Convert codecType to cudaVideoCodec
        cudaVideoCodec codec = cudaVideoCodec_H264; // Default to H264
        switch (m_params.codecType)
        {
            case CODEC_TYPE_H264:
                codec = cudaVideoCodec_H264;
                break;
            case CODEC_TYPE_H265:
                codec = cudaVideoCodec_HEVC;
                break;
            default:
                codec = cudaVideoCodec_H264; // Default fallback
                break;
        }

        // Initialize NvDecoder with proper parameters including max width and height
        m_decoder     = new NvDecoder(cuContext, false, codec, false, false, nullptr, nullptr, false, params.width, params.height);
        m_initialized = true;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize CUDA decoder: " << e.what() << std::endl;
        return false;
    }
}

bool CudaDecoder::DecodePacket(const CodecPacket& packet, FrameData& frame)
{
    if (!m_initialized || !m_decoder)
    {
        return false;
    }

    try
    {
        // Use NvDecoder's Decode API properly
        const uint8_t* pData = static_cast<const uint8_t*>(packet.data);
        int            nSize = static_cast<int>(packet.size);

        // Decode the data
        int nFrameReturned = m_decoder->Decode(pData, nSize, 0, packet.timestamp);

        if (nFrameReturned > 0)
        {
            // Get the decoded frame
            int64_t  timestamp;
            uint8_t* pFrame = m_decoder->GetFrame(&timestamp);
            if (pFrame != nullptr)
            {
                // Copy decoded frame data
                size_t frameSize = m_decoder->GetFrameSize();
                frame.data       = new uint8_t[frameSize];
                memcpy(frame.data, pFrame, frameSize);
                frame.size      = static_cast<uint32_t>(frameSize);
                frame.timestamp = static_cast<uint64_t>(timestamp);
                return true;
            }
        }
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to decode packet: " << e.what() << std::endl;
        return false;
    }
}

bool CudaDecoder::Flush(FrameData& frame)
{
    // Flush remaining frames from decoder
    try
    {
        int nFrameReturned = m_decoder->Decode(nullptr, 0, 0, 0);

        if (nFrameReturned > 0)
        {
            // Get the decoded frame
            int64_t  timestamp;
            uint8_t* pFrame = m_decoder->GetFrame(&timestamp);
            if (pFrame != nullptr)
            {
                size_t frameSize = m_decoder->GetFrameSize();
                frame.data       = new uint8_t[frameSize];
                memcpy(frame.data, pFrame, frameSize);
                frame.size      = static_cast<uint32_t>(frameSize);
                frame.timestamp = static_cast<uint64_t>(timestamp);
                return true;
            }
        }
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to flush decoder: " << e.what() << std::endl;
        return false;
    }
}

void CudaDecoder::Destroy()
{
    if (m_decoder)
    {
        delete m_decoder;
        m_decoder = nullptr;
    }
    m_initialized = false;
}

} // namespace cdc