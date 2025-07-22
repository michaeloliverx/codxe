#include "image_loader.h"
#include "common.h"

namespace
{
    void GPUEndianSwapTexture(std::vector<uint8_t> &pixelData, GPUENDIAN endianType)
    {
        switch (endianType)
        {
        case GPUENDIAN_8IN16:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN16, 2, pixelData.size() / 2);
            break;
        case GPUENDIAN_8IN32:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_8IN32, 4, pixelData.size() / 4);
            break;
        case GPUENDIAN_16IN32:
            XGEndianSwapMemory(pixelData.data(), pixelData.data(), XGENDIAN_16IN32, 4, pixelData.size() / 4);
            break;
        }
    }

    // Function to swap all necessary fields from little-endian to big-endian
    void SwapDDSHeaderEndian(DDSHeader &header)
    {
        header.magic = _byteswap_ulong(header.magic);
        header.size = _byteswap_ulong(header.size);
        header.flags = _byteswap_ulong(header.flags);
        header.height = _byteswap_ulong(header.height);
        header.width = _byteswap_ulong(header.width);
        header.pitchOrLinearSize = _byteswap_ulong(header.pitchOrLinearSize);
        header.depth = _byteswap_ulong(header.depth);
        header.mipMapCount = _byteswap_ulong(header.mipMapCount);

        for (int i = 0; i < 11; i++)
            header.reserved1[i] = _byteswap_ulong(header.reserved1[i]);

        header.pixelFormat.size = _byteswap_ulong(header.pixelFormat.size);
        header.pixelFormat.flags = _byteswap_ulong(header.pixelFormat.flags);
        header.pixelFormat.fourCC = _byteswap_ulong(header.pixelFormat.fourCC);
        header.pixelFormat.rgbBitCount = _byteswap_ulong(header.pixelFormat.rgbBitCount);
        header.pixelFormat.rBitMask = _byteswap_ulong(header.pixelFormat.rBitMask);
        header.pixelFormat.gBitMask = _byteswap_ulong(header.pixelFormat.gBitMask);
        header.pixelFormat.bBitMask = _byteswap_ulong(header.pixelFormat.bBitMask);
        header.pixelFormat.aBitMask = _byteswap_ulong(header.pixelFormat.aBitMask);

        header.caps = _byteswap_ulong(header.caps);
        header.caps2 = _byteswap_ulong(header.caps2);
        header.caps3 = _byteswap_ulong(header.caps3);
        header.caps4 = _byteswap_ulong(header.caps4);
        header.reserved2 = _byteswap_ulong(header.reserved2);
    }

    DDSImage ReadDDSFile(const std::string &filepath)
    {
        DDSImage ddsImage;
        std::ifstream file(filepath, std::ios::binary);

        if (!file.is_open())
        {
            DbgPrint("ERROR: Unable to open file: %s\n", filepath.c_str());
            return ddsImage; // Return empty DDSImage
        }

        // Read DDS header (raw, little-endian)
        file.read(reinterpret_cast<char *>(&ddsImage.header), sizeof(DDSHeader));

        // Swap only the magic number to big-endian for proper validation
        uint32_t magicSwapped = _byteswap_ulong(ddsImage.header.magic);

        if (magicSwapped != 0x20534444) // 'DDS ' in big-endian
        {
            DbgPrint("ERROR: Invalid DDS file: %s\n", filepath.c_str());
            file.close();
            return ddsImage;
        }

        // Swap header fields to big-endian for Xbox 360
        SwapDDSHeaderEndian(ddsImage.header);

        // Move to end of file to get total file size
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();

        // Ensure fileSize is valid before proceeding
        if (fileSize == std::streampos(-1))
        {
            DbgPrint("ERROR: Failed to determine file size.\n");
            file.close();
            return ddsImage;
        }

        // Move back to after the header
        file.seekg(sizeof(DDSHeader), std::ios::beg);

        // Compute data size safely
        size_t dataSize = static_cast<size_t>(fileSize) - sizeof(DDSHeader);

        // Read image data
        ddsImage.data.resize(dataSize);
        file.read(reinterpret_cast<char *>(ddsImage.data.data()), dataSize);

        file.close();

        // Debug output
        DbgPrint("INFO: DDS file '%s' loaded successfully.\n", filepath.c_str());
        DbgPrint("      Resolution: %ux%u\n", ddsImage.header.width, ddsImage.header.height);
        DbgPrint("      MipMaps: %u\n", ddsImage.header.mipMapCount);
        DbgPrint("      Data Size: %u bytes\n", static_cast<unsigned int>(dataSize));

        return ddsImage;
    }
}

namespace t4
{
    namespace mp
    {
        void ImageLoader::DumpGfxImage(const GfxImage *image)
        {
            DbgPrint("Image_Dump: Dumping image '%s'\n", image->name);

            if (!image)
            {
                DbgPrint("Image_Dump: Null GfxImage!\n");
                return;
            }

            if (!image->pixels || image->baseSize == 0)
            {
                DbgPrint("Image_Dump: Image '%s' has no valid pixel data!\n", image->name);
                return;
            }

            if (image->mapType != MAPTYPE_2D && image->mapType != MAPTYPE_CUBE)
            {
                DbgPrint("Image_Dump: Unsupported map type %d!\n", image->mapType);
                return;
            }

            UINT BaseSize;
            XGGetTextureLayout(image->texture.basemap, 0, &BaseSize, 0, 0, 0, 0, 0, 0, 0, 0);

            DDSHeader header;
            memset(&header, 0, sizeof(DDSHeader));

            header.magic = _byteswap_ulong(DDS_MAGIC);
            header.size = _byteswap_ulong(DDS_HEADER_SIZE);
            header.flags = _byteswap_ulong(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE);
            header.height = _byteswap_ulong(image->height);
            header.width = _byteswap_ulong(image->width);
            header.depth = _byteswap_ulong(image->depth);
            header.mipMapCount = _byteswap_ulong(image->texture.basemap->GetLevelCount());

            auto format = image->texture.basemap->Format.DataFormat;
            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT1_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXT2_3:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT3_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXT4_5:
                header.pixelFormat.fourCC = _byteswap_ulong(DXT5_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_DXN:
                header.pixelFormat.fourCC = _byteswap_ulong(DXN_FOURCC);
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(8);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.gBitMask = 0;
                header.pixelFormat.bBitMask = 0;
                header.pixelFormat.aBitMask = 0;
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_LUMINANCE | DDPF_ALPHAPIXELS);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(16);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                header.pixelFormat.bBitMask = 0;
                header.pixelFormat.aBitMask = 0;
                header.pitchOrLinearSize = BaseSize;
                break;
            case GPUTEXTUREFORMAT_8_8_8_8:
                header.pixelFormat.flags = _byteswap_ulong(DDPF_RGB | DDPF_ALPHAPIXELS);
                header.pixelFormat.rgbBitCount = _byteswap_ulong(32);
                header.pixelFormat.rBitMask = _byteswap_ulong(0x00FF0000);
                header.pixelFormat.gBitMask = _byteswap_ulong(0x0000FF00);
                header.pixelFormat.bBitMask = _byteswap_ulong(0x000000FF);
                header.pixelFormat.aBitMask = _byteswap_ulong(0xFF000000);
                header.pitchOrLinearSize = BaseSize;
                break;
            default:
                DbgPrint("Image_Dump: Unsupported texture format %d!\n", format);
                return;
            }

            // Set texture capabilities
            header.caps = _byteswap_ulong(DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

            // Handle Cubemaps
            if (image->mapType == mp::MAPTYPE_CUBE)
            {
                header.caps2 = _byteswap_ulong(DDSCAPS2_CUBEMAP |
                                               DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                               DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                               DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);
            }

            std::string filename = "game:\\_codxe\\dump\\images\\";
            std::string sanitized_name = image->name;

            // Remove invalid characters
            sanitized_name.erase(std::remove_if(sanitized_name.begin(), sanitized_name.end(), [](char c)
                                                { return c == '*'; }),
                                 sanitized_name.end());

            filename += sanitized_name + ".dds";

            std::ofstream file(filename, std::ios::binary);
            if (!file)
            {
                DbgPrint("Image_Dump: Failed to open file: %s\n", filename.c_str());
                return;
            }

            if (image->mapType == MAPTYPE_CUBE)
            {
                file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

                unsigned int face_size = 0;
                unsigned int rowPitch = 0;
                const GPUTEXTUREFORMAT format = static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat);

                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                    face_size = (image->width / 4) * (image->height / 4) * 8;
                    rowPitch = (image->width / 4) * 8; // 8 bytes per 4x4 block
                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    face_size = image->width * image->height * 4;
                    rowPitch = image->width * 4; // 4 bytes per pixel
                    break;
                default:
                    DbgPrint("Image_Dump: Unsupported cube map format %d!\n", format);
                    return;
                }

                // TODO: handle mip levels per face for cubemaps
                for (int i = 0; i < 6; i++)
                {
                    unsigned char *face_pixels = image->pixels + (i * face_size); // Offset for each face

                    std::vector<uint8_t> swappedFace(face_pixels, face_pixels + face_size);
                    GPUEndianSwapTexture(swappedFace, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                    // Create buffer for linear texture data
                    std::vector<uint8_t> linearFace(face_size);

                    // Convert tiled texture to linear layout using XGUntileTextureLevel
                    XGUntileTextureLevel(
                        image->width,               // Width
                        image->height,              // Height
                        0,                          // Level (base level)
                        static_cast<DWORD>(format), // GpuFormat
                        0,                          // Flags (no special flags)
                        linearFace.data(),          // pDestination (linear output)
                        rowPitch,                   // RowPitch
                        nullptr,                    // pPoint (no offset)
                        swappedFace.data(),         // pSource (tiled input)
                        nullptr                     // pRect (entire texture)
                    );

                    file.write(reinterpret_cast<const char *>(linearFace.data()), linearFace.size());
                }

                file.close();
            }
            else if (image->mapType == MAPTYPE_2D)
            {
                // TODO: write mip levels
                file.write(reinterpret_cast<const char *>(&header), sizeof(DDSHeader));

                std::vector<uint8_t> pixelData(image->pixels, image->pixels + image->baseSize);

                GPUEndianSwapTexture(pixelData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                // Create a linear data buffer to hold the untiled texture
                std::vector<uint8_t> linearData(image->baseSize);

                // Calculate row pitch based on format
                UINT rowPitch;
                auto format = image->texture.basemap->Format.DataFormat;

                switch (format)
                {
                case GPUTEXTUREFORMAT_DXT1:
                case GPUTEXTUREFORMAT_DXT2_3:
                case GPUTEXTUREFORMAT_DXT4_5:
                case GPUTEXTUREFORMAT_DXN:
                    // Block compressed formats use 4x4 blocks
                    rowPitch = ((image->width + 3) / 4) * (format == GPUTEXTUREFORMAT_DXT1 ? 8 : 16);
                    break;
                case GPUTEXTUREFORMAT_8:
                    rowPitch = image->width;
                    break;
                case GPUTEXTUREFORMAT_8_8:
                    rowPitch = image->width * 2;
                    break;
                case GPUTEXTUREFORMAT_8_8_8_8:
                    rowPitch = image->width * 4;
                    break;
                default:
                    DbgPrint("Image_Dump: Unsupported texture format %d!\n", format);
                    return;
                }

                // Call XGUntileTextureLevel to convert the tiled texture to linear format
                XGUntileTextureLevel(
                    image->width,               // Width
                    image->height,              // Height
                    0,                          // Level (base level 0)
                    static_cast<DWORD>(format), // GpuFormat
                    XGTILE_NONPACKED,           // Flags (no special flags)
                    linearData.data(),          // pDestination
                    rowPitch,                   // RowPitch (calculated based on format)
                    nullptr,                    // pPoint (no offset)
                    pixelData.data(),           // pSource
                    nullptr                     // pRect (entire texture)
                );

                file.write(reinterpret_cast<const char *>(linearData.data()), linearData.size());

                file.close();
            }
            else
            {
                DbgPrint("Image_Dump: Unsupported map type %d!\n", image->mapType);
                return;
            }
        }

        unsigned int ImageLoader::CalculateMipLevelSize(unsigned int width, unsigned int height, unsigned int mipLevel, GPUTEXTUREFORMAT format)
        {
            // Calculate dimensions for the requested mip level
            unsigned int mipWidth = max(1, width >> mipLevel);
            unsigned int mipHeight = max(1, height >> mipLevel);

            // Calculate size based on format
            unsigned int blockSize;
            switch (format)
            {
            case GPUTEXTUREFORMAT_DXT1:
                blockSize = 8; // 8 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXT2_3:
                blockSize = 16; // 16 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXT4_5:
                blockSize = 16; // 16 bytes per 4x4 block
                break;
            case GPUTEXTUREFORMAT_DXN:
                blockSize = 16; // 16 bytes per 4x4 block (two 8-byte channels)
                break;
            default:
                DbgPrint("CalculateMipLevelSize: Unsupported format %d\n", format);
                return 0;
            }

            // For block-compressed formats, calculate number of blocks
            // Each block is 4x4 pixels, so we need to round up to nearest block
            unsigned int blocksWide = (mipWidth + 3) / 4;
            unsigned int blocksHigh = (mipHeight + 3) / 4;

            // Calculate total size in bytes
            unsigned int sizeInBytes = blocksWide * blocksHigh * blockSize;

            return sizeInBytes;
        }

        void ImageLoader::ReplaceGfxImage2d(GfxImage *image, const DDSImage &ddsImage)
        {
            if (image->mapType != MAPTYPE_2D)
            {
                DbgPrint("Image '%s' is not a 2D map!\n", image->name);
                return;
            }

            // Get base texture layout
            UINT baseAddress, baseSize, mipAddress, mipSize;

            XGGetTextureLayout(image->texture.basemap,
                               &baseAddress, &baseSize, 0, 0, 0,
                               &mipAddress, &mipSize, 0, 0, 0);

            XGTEXTURE_DESC TextureDesc;
            XGGetTextureDesc(image->texture.basemap, 0, &TextureDesc);

            UINT mipTailBaseLevel = XGGetMipTailBaseLevel(TextureDesc.Width, TextureDesc.Height, XGIsBorderTexture(image->texture.basemap));

            UINT ddsOffset = 0;

            for (UINT mipLevel = 0; mipLevel < mipTailBaseLevel; mipLevel++)
            {
                UINT widthInBlocks = max(1, TextureDesc.WidthInBlocks >> mipLevel);
                UINT rowPitch = widthInBlocks * TextureDesc.BytesPerBlock;
                // UINT levelSize = rowPitch * heightInBlocks;
                UINT ddsMipLevelSize = CalculateMipLevelSize(image->width, image->height, mipLevel, static_cast<GPUTEXTUREFORMAT>(image->texture.basemap->Format.DataFormat));

                if (ddsMipLevelSize == 0)
                {
                    DbgPrint("  [ERROR] Unsupported format %d for mip level %u! Skipping...\n", image->texture.basemap->Format.DataFormat, mipLevel);
                    break;
                }

                // Ensure we're not reading out of bounds
                if (ddsOffset + ddsMipLevelSize > ddsImage.data.size())
                {
                    DbgPrint("  [ERROR] Mip Level %u exceeds DDS data size! Skipping...\n", mipLevel);
                    break;
                }

                std::vector<uint8_t> levelData(ddsImage.data.begin() + ddsOffset, ddsImage.data.begin() + ddsOffset + ddsMipLevelSize);

                GPUEndianSwapTexture(levelData, static_cast<GPUENDIAN>(image->texture.basemap->Format.Endian));

                DbgPrint("Image_Replace_2D: Mip Level %d - Row Pitch=%u\n", mipLevel, rowPitch);

                UINT address = baseAddress;
                if (mipLevel > 0)
                {
                    UINT mipLevelOffset = XGGetMipLevelOffset(image->texture.basemap, 0, mipLevel);
                    address = mipAddress + mipLevelOffset;
                }

                DbgPrint("Image_Replace_2D: Writing mip level %d to address 0x%08X - levelSize=%u\n", mipLevel, address, ddsMipLevelSize);

                // // Write the base level
                XGTileTextureLevel(
                    TextureDesc.Width,
                    TextureDesc.Height,
                    mipLevel,
                    image->texture.basemap->Format.DataFormat,
                    XGTILE_NONPACKED,                  // Use non-packed mode (likely required for this texture)
                    reinterpret_cast<void *>(address), // Destination (tiled GPU memory for Base)
                    nullptr,                           // No offset (tile the whole image)
                    levelData.data(),                  // Source mip level data
                    rowPitch,                          // Row pitch of source image (should match DDS format)
                    nullptr                            // No subrectangle (tile the full image)
                );

                ddsOffset += ddsMipLevelSize;
            }
        }

        void ImageLoader::ReplaceGfxImage(GfxImage *image)
        {
            const std::string replacement_base_dir = "game:\\_codxe\\raw\\images";
            const std::string replacement_path = replacement_base_dir + "\\" + image->name + ".dds";

            if (!filesystem::file_exists(replacement_path))
            {
                DbgPrint("File does not exist: %s\n", replacement_path.c_str());
                return;
            }

            DDSImage ddsImage = ReadDDSFile(replacement_path.c_str());
            if (ddsImage.data.empty())
            {
                DbgPrint("Failed to load DDS file: %s\n", replacement_path.c_str());
                return;
            }

            if (image->width != ddsImage.header.width || image->height != ddsImage.header.height)
            {
                DbgPrint("Image '%s' dimensions do not match DDS file: %s\n", image->name, replacement_path.c_str());
                return;
            }

            GPUTEXTUREFORMAT ddsFormat;
            switch (ddsImage.header.pixelFormat.fourCC)
            {
            case DXT1_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT1;
                break;
            case DXT3_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT2_3;
                break;
            case DXT5_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXT4_5;
                break;
            case DXN_FOURCC:
                ddsFormat = GPUTEXTUREFORMAT_DXN;
                break;
            default:
                DbgPrint("Image '%s' has an unsupported DDS format: 0x%X\n", image->name, ddsImage.header.pixelFormat.fourCC);
                return;
            }

            if (static_cast<uint32_t>(image->texture.basemap->Format.DataFormat) != static_cast<uint32_t>(ddsFormat))
            {
                DbgPrint("Image '%s' format does not match DDS file: Expected %d, Got %d\n",
                         image->name, static_cast<uint32_t>(image->texture.basemap->Format.DataFormat),
                         static_cast<uint32_t>(ddsFormat));
                return;
            }

            if (image->mapType == MAPTYPE_2D)
            {
                ReplaceGfxImage2d(image, ddsImage);
            }
            // else if (image->mapType == MAPTYPE_CUBE)
            // {
            //     Image_Replace_Cube(image, ddsImage);
            // }
            else
            {
                DbgPrint("Image '%s' is not a 2D or cube map!\n", image->name);
                return;
            }
        }

        ImageLoader::ImageLoader()
        {
            Sleep(10000); // Simulate some initialization delay

            // Create directories for dumping images
            CreateDirectoryA("game:\\_codxe", nullptr);
            CreateDirectoryA("game:\\_codxe\\dump", nullptr);
            CreateDirectoryA("game:\\_codxe\\dump\\images", nullptr);
            CreateDirectoryA("game:\\_codxe\\dump\\highmip", nullptr);

            // // Dump all images from the database
            // const auto MAX_IMAGES = 2048;
            // XAssetHeader assets[MAX_IMAGES];
            // auto count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_IMAGE, assets, MAX_IMAGES);
            // DbgPrint("ImageLoader: Found %d images\n", count);
            // for (int i = 0; i < count; i++)
            // {
            //     DumpGfxImage(assets[i].image);
            // }

            // Replace images
            const auto MAX_IMAGES = 2048;
            XAssetHeader assets[MAX_IMAGES];
            auto count = DB_GetAllXAssetOfType_FastFile(ASSET_TYPE_IMAGE, assets, MAX_IMAGES);
            DbgPrint("Cmd_imageload2_f: Found %d images\n", count);
            for (int i = 0; i < count; i++)
                ReplaceGfxImage(assets[i].image);
        }

        ImageLoader::~ImageLoader()
        {
            DbgPrint("ImageLoader shutdown\n");
        }
    }
}
