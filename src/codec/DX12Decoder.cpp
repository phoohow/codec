#include "codecImpl.h"

#include <iostream>

namespace cdc
{

DX12Decoder::DX12Decoder() : m_initialized(false) {}

DX12Decoder::~DX12Decoder()
{
    Destroy();
}

bool DX12Decoder::Initialize(const CreateParams& params)
{
    // TODO: Implement DX12 decoder initialization
    std::cerr << "DX12 decoder not implemented yet" << std::endl;
    return false;
}

bool DX12Decoder::DecodePacket(const CodecPacket& packet, FrameData& frame)
{
    // TODO: Implement DX12 decoding
    std::cerr << "DX12 decoder not implemented yet" << std::endl;
    return false;
}

bool DX12Decoder::Flush(FrameData& frame)
{
    // TODO: Implement DX12 decoder flush
    return false;
}

void DX12Decoder::Destroy()
{
    m_initialized = false;
}

} // namespace cdc
