#pragma once

#include <codec/codec.h>
#include <Interface/nvEncodeAPI.h>

namespace cdc
{

// Helper function to convert PixelFormat to NV_ENC_BUFFER_FORMAT
inline NV_ENC_BUFFER_FORMAT to_nvEncFormat(PixelFormat format)
{
    switch (format)
    {
        // case PIXEL_FORMAT_RGBA8: return NV_ENC_BUFFER_FORMAT_RGBA;
        case PIXEL_FORMAT_ARGB8: return NV_ENC_BUFFER_FORMAT_ARGB;
        // case PIXEL_FORMAT_BGRA8: return NV_ENC_BUFFER_FORMAT_BGRA;
        case PIXEL_FORMAT_NV12: return NV_ENC_BUFFER_FORMAT_NV12;
        default: return NV_ENC_BUFFER_FORMAT_UNDEFINED;
    }

    return NV_ENC_BUFFER_FORMAT_NV12;
}
} // namespace cdc