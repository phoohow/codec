# Codec Library

A unified interface for video encoding and decoding using NVIDIA hardware acceleration with support for both DirectX 12 and CUDA.

## Features

- Unified interface for video encoding/decoding
- Support for DirectX 12 and CUDA
- Hardware-accelerated encoding and decoding
- Easy-to-use factory pattern for creating encoders and decoders

## Building

This project uses xmake as the build system.

### Prerequisites

- xmake (https://xmake.io/)
- NVIDIA GPU with NVENC/NVDEC support
- DirectX 12 SDK (for DX12 support)
- CUDA Toolkit (for CUDA support)

### Build Steps

1. Clone the repository
2. Navigate to the project directory
3. Run the following commands:

```bash
# Configure the project
xmake

# Build the project
xmake build

# Or build in release mode
xmake build -m release
```

## Usage

Include the codec.h header in your project:

```cpp
#include "codec.h"

// Create an encoder
cdc::CreateParams params = {};
params.width = 1920;
params.height = 1080;
params.deviceType = cdc::DEVICE_TYPE_DX12;  // or DEVICE_TYPE_CUDA
params.codecType = cdc::CODEC_TYPE_H264; // or other encoder types

auto encoder = cdc::CreateEncoder(params);

// Initialize the encoder
encoder->Initialize(params);

// Encode frames
cdc::FrameData frameData;
// Fill frameData with your texture or pixel data
std::vector<cdc::CodecPacket> packets;
encoder->EncodeFrame(frameData, packets);
```

## API Overview

### Encoder Types

- CODEC_TYPE_H264
- ENCODER_TYPE_HEVC

### Decoder Types

- CODEC_TYPE_H264
- DECODER_TYPE_HEVC

### Device Types

- DEVICE_TYPE_DX12
- DEVICE_TYPE_CUDA

## Example

See `src/example.cpp` for a complete example of how to use the codec library.

## License

MIT License