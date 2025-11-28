#pragma once

#include <codec/codec.h>

class NvEncoderCuda;
class NvEncoderD3D12;
class NvDecoder;

namespace cdc
{

class CudaEncoder : public Encoder
{
    NvEncoderCuda* m_encoder;
    CreateParams   m_params;
    bool           m_initialized;

public:
    CudaEncoder();
    virtual ~CudaEncoder();

    bool Initialize(const CreateParams& params) override;
    bool EncodeFrame(void* pData, CodecPacket& packet) override;
    bool Flush(CodecPacket& packet) override;
    void Destroy() override;
};

class DX12Encoder : public Encoder
{
    NvEncoderD3D12* m_encoder;
    CreateParams    m_params;
    bool            m_initialized;

public:
    DX12Encoder();
    virtual ~DX12Encoder();

    bool Initialize(const CreateParams& params) override;
    bool EncodeFrame(void* pData, CodecPacket& packet) override;
    bool Flush(CodecPacket& packet) override;
    void Destroy() override;
};

class CudaDecoder : public Decoder
{
    NvDecoder*   m_decoder;
    CreateParams m_params;
    bool         m_initialized;

public:
    CudaDecoder();
    virtual ~CudaDecoder();

    bool Initialize(const CreateParams& params) override;
    bool DecodePacket(const CodecPacket& packet, FrameData& frame) override;
    bool Flush(FrameData& frame) override;
    void Destroy() override;
};

class DX12Decoder : public Decoder
{
    bool m_initialized;

public:
    DX12Decoder();
    virtual ~DX12Decoder();

    bool Initialize(const CreateParams& params) override;
    bool DecodePacket(const CodecPacket& packet, FrameData& frame) override;
    bool Flush(FrameData& frame) override;
    void Destroy() override;
};

} // namespace cdc