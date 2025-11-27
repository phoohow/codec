#include "codec/codec.h"
#include <iostream>
#include <memory>

#ifdef _WIN32
#    include <d3d12.h>
#endif

int main()
{
    std::cout << "Codec example application" << std::endl;

    // Example 1: Creating a DX12 encoder
    {
        cdc::CreateParams params = {};
        params.width             = 1920;
        params.height            = 1080;
        params.deviceType        = cdc::DEVICE_TYPE_DX12;
        params.codecType         = cdc::CODEC_TYPE_H264;
        // In a real application, you would set the device pointer here
        params.device      = nullptr;                 // Placeholder
        params.pixelFormat = cdc::PIXEL_FORMAT_BGRA8; // Placeholder for DXGI_FORMAT

        auto encoder = cdc::CreateEncoder(params);
        if (encoder)
        {
            std::cout << "Successfully created DX12 H.264 encoder" << std::endl;

            if (encoder->Initialize(params))
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
        cdc::CreateParams params = {};
        params.width             = 1920;
        params.height            = 1080;
        params.deviceType        = cdc::DEVICE_TYPE_DX12;
        params.codecType         = cdc::CODEC_TYPE_H264;
        // In a real application, you would set the device pointer here
        params.device      = nullptr;                 // Placeholder
        params.pixelFormat = cdc::PIXEL_FORMAT_BGRA8; // Placeholder for DXGI_FORMAT

        auto decoder = cdc::CreateDecoder(params);
        if (decoder)
        {
            std::cout << "Successfully created DX12 H.264 decoder" << std::endl;

            if (decoder->Initialize(params))
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