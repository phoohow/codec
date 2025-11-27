#include "codec/codec.h"
#include <iostream>
#include <memory>

#ifdef _WIN32
#include <d3d12.h>
#endif

int main()
{
    std::cout << "Codec example application" << std::endl;

    // Example 1: Creating a DX12 encoder
    {
        cdc::CreateParams encoderParams = {};
        encoderParams.width = 1920;
        encoderParams.height = 1080;
        encoderParams.deviceType = cdc::DEVICE_TYPE_DX12;
        encoderParams.codecType = cdc::CODEC_TYPE_H264;
        // In a real application, you would set the device pointer here
        encoderParams.device = nullptr;          // Placeholder
        encoderParams.specifics.dx12.format = 0; // Placeholder for DXGI_FORMAT

        auto encoder = cdc::CreateEncoder(encoderParams);
        if (encoder)
        {
            std::cout << "Successfully created DX12 H.264 encoder" << std::endl;

            if (encoder->Initialize(encoderParams))
            {
                std::cout << "Successfully initialized DX12 encoder" << std::endl;
            }
            else
            {
                std::cout << "Failed to initialize DX12 encoder" << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to create DX12 encoder" << std::endl;
        }
    }

    // Example 2: Creating a DX12 decoder
    {
        cdc::CreateParams decoderParams = {};
        decoderParams.width = 1920;
        decoderParams.height = 1080;
        decoderParams.deviceType = cdc::DEVICE_TYPE_DX12;
        decoderParams.codecType = cdc::CODEC_TYPE_H264;
        // In a real application, you would set the device pointer here
        decoderParams.device = nullptr;          // Placeholder
        decoderParams.specifics.dx12.format = 0; // Placeholder for DXGI_FORMAT

        auto decoder = cdc::CreateDecoder(decoderParams);
        if (decoder)
        {
            std::cout << "Successfully created DX12 H.264 decoder" << std::endl;

            if (decoder->Initialize(decoderParams))
            {
                std::cout << "Successfully initialized DX12 decoder" << std::endl;
            }
            else
            {
                std::cout << "Failed to initialize DX12 decoder" << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to create DX12 decoder" << std::endl;
        }
    }

    return 0;
}