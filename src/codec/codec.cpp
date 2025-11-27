#include <codec/codec.h>

#include "codecImpl.h"

#include <Utils/Logger.h>

simplelogger::Logger *logger = simplelogger::LoggerFactory::CreateConsoleLogger();

namespace cdc
{

    std::unique_ptr<Encoder> CreateEncoder(const CreateParams &params)
    {
        switch (params.deviceType)
        {
        case DEVICE_TYPE_DX12:
            return std::make_unique<DX12Encoder>();
        case DEVICE_TYPE_CUDA:
            return std::make_unique<CudaEncoder>();
        default:
            return nullptr;
        }
    }

    std::unique_ptr<Decoder> CreateDecoder(const CreateParams &params)
    {
        switch (params.deviceType)
        {
        case DEVICE_TYPE_DX12:
            return std::make_unique<DX12Decoder>();
        case DEVICE_TYPE_CUDA:
            return std::make_unique<CudaDecoder>();
        default:
            return nullptr;
        }
    }

} // namespace cdc