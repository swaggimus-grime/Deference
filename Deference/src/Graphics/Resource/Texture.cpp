#include "Texture.h"
#include <ranges>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>
#include <d3d12.h>
#include <DirectXTex/DirectXTex.h>
#include "Buffer.h"

namespace Def
{
    Texture2D::Texture2D(Graphics& g, UINT8* start, SIZE_T byteLength)
        :SRV(this)
    {
        CreateFromMemory(g, start, byteLength);
    }

    Texture2D::Texture2D(Graphics& g, const std::wstring& path)
        :SRV(this)
    {
        CreateFromFile(g, path);
    }

    Texture2D::Texture2D(Graphics& g, const std::string& path)
        :Texture2D(g, std::wstring(path.begin(), path.end()))
    {
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& Texture2D::SRVDesc() const
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = GetFormat();
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = m_Res->GetDesc().MipLevels;
        
        return std::move(desc);
    }

    void Texture2D::CreateFromFile(Graphics& g, const std::wstring& path)
    {
        auto device = g.GetBaseDevice();
        DirectX::ResourceUploadBatch batch(device.Get());
        batch.Begin();
        HR DirectX::CreateWICTextureFromFile(device.Get(), batch,
            path.c_str(), &m_Res, true);
        auto finish = batch.End(&g.CQ());
        finish.wait();

        m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    void Texture2D::CreateFromMemory(Graphics& g, UINT8* start, SIZE_T byteLength)
    {
        auto device = g.GetBaseDevice();
        DirectX::ResourceUploadBatch batch(device.Get());
        batch.Begin();
        HR DirectX::CreateWICTextureFromMemory(device.Get(), batch,
            start, byteLength, &m_Res, true);
        auto finish = batch.End(&g.CQ());
        finish.wait();

        m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    EnvironmentMap::EnvironmentMap(Graphics& g, const std::wstring& path)
        :SRV(this)
    {
        DirectX::TexMetadata meta;
        HR DirectX::GetMetadataFromHDRFile(path.c_str(), meta);

        DirectX::ScratchImage image;
        HR DirectX::LoadFromHDRFile(path.c_str(), &meta, image);

        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        rd.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
        rd.Width = image.GetMetadata().width; // width of the texture
        rd.Height = image.GetMetadata().height; // height of the texture
        rd.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
        rd.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
        rd.Format = image.GetMetadata().format; // This is the dxgi format of the image (format of the pixels)
        rd.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
        rd.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
        rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
        rd.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

        {
            const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            HR g.Device().CreateCommittedResource(
                &heap, // this heap will be used to upload the constant buffer data
                D3D12_HEAP_FLAG_NONE, // no flags
                &rd, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
                D3D12_RESOURCE_STATE_COPY_DEST, // will be data that is read from so we keep it in the generic read state
                nullptr, // we do not have use an optimized clear value for constant buffers
                IID_PPV_ARGS(&m_Res));
        }

        UINT64 numBytes = 0;
        g.Device().GetCopyableFootprints(&rd, 0, 1, 0, nullptr, nullptr, nullptr, &numBytes);
        RawBuffer uptex(g, D3D12_HEAP_TYPE_UPLOAD, numBytes, D3D12_RESOURCE_STATE_GENERIC_READ);

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = image.GetPixels(); // pointer to our image data
        textureData.RowPitch = image.GetImage(0, 0, 0)->rowPitch; // size of all our triangle vertex data
        textureData.SlicePitch = textureData.RowPitch * rd.Height; // also the size of our triangle vertex data

        // Now we copy the upload buffer contents to the default heap
        UpdateSubresources(&g.CL(), m_Res.Get(), *uptex, 0, 0, 1, &textureData);
        auto barrier = Transition(D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        g.CL().ResourceBarrier(1, &barrier);
        g.Flush();
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& EnvironmentMap::SRVDesc() const
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = GetFormat();
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = 1;

        return std::move(desc);
    }

    CubeMap::CubeMap(Graphics& g, const std::wstring& path)
        :SRV(this)
    {
        g.Flush();

        auto device = g.GetBaseDevice();
        DirectX::ResourceUploadBatch batch(device.Get());
        batch.Begin();
        HR DirectX::CreateDDSTextureFromFile(device.Get(), batch,
            path.c_str(), &m_Res);
        auto finish = batch.End(&g.CQ());
        finish.wait();

        m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& CubeMap::SRVDesc() const
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        desc.TextureCube.MostDetailedMip = 0;
        desc.TextureCube.MipLevels = m_Res->GetDesc().MipLevels;
        desc.TextureCube.ResourceMinLODClamp = 0.0f;
        desc.Format = m_Res->GetDesc().Format;

        return std::move(desc);
    }
}