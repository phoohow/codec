#pragma once

#include <stdint.h>
#include <memory>

namespace cdc
{

    // Device type enumeration
    enum DeviceType
    {
        DEVICE_TYPE_UNKNOWN = 0,
        DEVICE_TYPE_DX12,
        DEVICE_TYPE_CUDA
    };

    // Encoder/Decoder types
    enum CodecType
    {
        CODEC_TYPE_H264 = 0,
        CODEC_TYPE_H265,
        CODEC_TYPE_AV1
    };

    // Creation parameters for encoder/decoder
    struct CreateParams
    {
        void *device;          // Device pointer (ID3D12Device* for DX12, CUcontext for CUDA)
        uint32_t width;        // Width of the frame
        uint32_t height;       // Height of the frame
        DeviceType deviceType; // Type of device (DX12 or CUDA)
        CodecType codecType;   // Type of encoder/decoder (H264, H265, AV1)
        union
        {
            struct
            {
                // DX12 specific parameters
                uint32_t format; // DXGI_FORMAT
            } dx12;
            struct
            {
                // CUDA specific parameters
                uint32_t format; // cudaVideoSurfaceFormat
            } cuda;
        } specifics;
    };

    // Frame data structure
    struct FrameData
    {
        void *data;         // Pointer to frame data
        uint32_t size;      // Size of frame data
        uint64_t timestamp; // Timestamp of the frame
    };

    // Encoded/Decoded packet structure
    struct CodecPacket
    {
        void *data;         // Pointer to encoded/decode data
        uint32_t size;      // Size of encoded/decode data
        uint64_t timestamp; // Timestamp of the packet
        bool keyFrame;      // Is this a key frame?
    };

    // Abstract base class for encoder
    class Encoder
    {
    public:
        virtual ~Encoder() {}

        // Initialize the encoder with given parameters
        virtual bool Initialize(const CreateParams &params) = 0;

        // Encode a frame
        // For DX12, pData should be ID3D12Resource*
        // For CUDA, pData should be CUdeviceptr
        virtual bool EncodeFrame(void *pData, CodecPacket &packet) = 0;

        // Flush any remaining encoded frames
        virtual bool Flush(CodecPacket &packet) = 0;

        // Destroy the encoder
        virtual void Destroy() = 0;
    };

    // Abstract base class for decoder
    class Decoder
    {
    public:
        virtual ~Decoder() {}

        // Initialize the decoder with given parameters
        virtual bool Initialize(const CreateParams &params) = 0;

        // Decode a packet
        virtual bool DecodePacket(const CodecPacket &packet, FrameData &frame) = 0;

        // Flush any remaining decoded frames
        virtual bool Flush(FrameData &frame) = 0;

        // Destroy the decoder
        virtual void Destroy() = 0;
    };

    // Factory functions
    std::unique_ptr<Encoder> CreateEncoder(const CreateParams &params);
    std::unique_ptr<Decoder> CreateDecoder(const CreateParams &params);

} // namespace cdc
