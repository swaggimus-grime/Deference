#include "Texture.h"

Texture::Texture(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
    :Resource(handle, nullptr, D3D12_RESOURCE_STATE_COPY_DEST)
{
}

void Texture::CreateResourceAndView(Graphics& g, DirectX::ScratchImage& image)
{
    D3D12_RESOURCE_DESC rd = {};
    rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rd.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
    rd.Width = image.GetMetadata().width; // width of the texture
    rd.Height = image.GetMetadata().height; // height of the texture
    rd.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
    rd.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
    rd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // This is the dxgi format of the image (format of the pixels)
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
            m_State, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&m_Res));
    }
    ComPtr<ID3D12Resource> uptex;
    {
        const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        UINT64 numBytes = 0;
        g.Device().GetCopyableFootprints(&rd, 0, 1, 0, nullptr, nullptr, nullptr, &numBytes);
        const auto res = CD3DX12_RESOURCE_DESC::Buffer(numBytes);
        HR g.Device().CreateCommittedResource(
            &heap, // upload heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &res, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
            D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
            nullptr,
            IID_PPV_ARGS(&uptex));
        uptex->SetName(L"Texture upload buffer");
    }
    {
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = image.GetPixels(); // pointer to our image data
        textureData.RowPitch = image.GetImage(0, 0, 0)->rowPitch; // size of all our triangle vertex data
        textureData.SlicePitch = textureData.RowPitch * rd.Height; // also the size of our triangle vertex data

        // Now we copy the upload buffer contents to the default heap
        UpdateSubresources(&g.CL(), m_Res.Get(), uptex.Get(), 0, 0, 1, &textureData);
        g.Flush();
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = image.GetMetadata().format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    g.Device().CreateShaderResourceView(m_Res.Get(), &srvDesc, m_Handle);
}

Texture2D::Texture2D(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path)
    :Texture(handle)
{
    DirectX::ScratchImage image;
    HR DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_IGNORE_SRGB, nullptr, image);
    if (image.GetMetadata().format != DXGI_FORMAT_R32G32B32A32_FLOAT) {
        DirectX::ScratchImage converted;
        DirectX::Convert(
            *image.GetImage(0, 0, 0),
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            DirectX::TEX_FILTER_DEFAULT,
            DirectX::TEX_THRESHOLD_DEFAULT,
            converted
        );
        image = std::move(converted);
    }

    CreateResourceAndView(g, image);
    Transition(g, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

EnvironmentMap::EnvironmentMap(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::wstring& path)
    :Texture(handle)
{
    DirectX::TexMetadata meta;
    HR DirectX::GetMetadataFromHDRFile(path.c_str(), meta);

    DirectX::ScratchImage image;
    HR DirectX::LoadFromHDRFile(path.c_str(), &meta, image);

    CreateResourceAndView(g, image);
}