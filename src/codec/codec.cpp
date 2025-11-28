#include <codec/codec.h>

#include "codecImpl.h"

#include <Utils/Logger.h>

simplelogger::Logger* logger = simplelogger::LoggerFactory::CreateConsoleLogger();

namespace cdc
{

Encoder* CreateEncoder(const CreateParams& params)
{
    switch (params.deviceType)
    {
        case DEVICE_TYPE_DX12: return new DX12Encoder();
        case DEVICE_TYPE_CUDA: return new CudaEncoder();
        default: return nullptr;
    }
}

Decoder* CreateDecoder(const CreateParams& params)
{
    switch (params.deviceType)
    {
        case DEVICE_TYPE_DX12: return new DX12Decoder();
        case DEVICE_TYPE_CUDA: return new CudaDecoder();
        default: return nullptr;
    }
}

} // namespace cdc