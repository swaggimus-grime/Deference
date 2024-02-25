#include "Model.h"
#include "Graphics.h"
#include "Debug/Exception.h"
#include "Resource/InputLayout.h"
#include "Resource/AccelStruct.h"
#include <ranges>

namespace Def
{
    static SIZE_T GltfStride(int type, int compType)
    {
        SIZE_T mult = 0;
        switch (type)
        {
        case TINYGLTF_TYPE_SCALAR:
            mult = 1;
            break;
        case TINYGLTF_TYPE_VEC4:
            mult = 4;
            break;
        case TINYGLTF_TYPE_VEC3:
            mult = 3;
            break;
        case TINYGLTF_TYPE_VEC2:
            mult = 2;
            break;
        }

        SIZE_T bytes = 0;
        switch (compType)
        {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            bytes = 1;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            bytes = 4;
            break;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            bytes = 2;
            break;
        }

        return mult * bytes;
    }

    static std::string GetFilePathExtension(const std::string& FileName) {
        if (FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".") + 1);
        return "";
    }

    Model::Model(Graphics& g, const std::string& filePath)
        :m_NumSubMeshes(0)
    {
        tinygltf::TinyGLTF gltf_ctx;
        std::string err;
        std::string warn;
        std::string ext = GetFilePathExtension(filePath);

        if (ext.compare("glb") == 0)
        {
            if (!gltf_ctx.LoadBinaryFromFile(&m_Model, &err, &warn, filePath.c_str()))
                throw new DefException(err);
        }
        else if (ext.compare("gltf") == 0)
        {
            if (!gltf_ctx.LoadASCIIFromFile(&m_Model, &err, &warn, filePath.c_str()))
                throw new DefException(err);
        }

        m_RootNode = MakeShared<SceneNode>();

        for (int childId : m_Model.scenes[m_Model.defaultScene].nodes)
            m_RootNode->Children.push_back(ParseScene(childId));

        ParseVertices(g);
        ParseMaterials(g);
        ParseTextures(g, filePath.substr(0, filePath.find_last_of('\\')));
        ParseSamplers();

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometries;
        for (auto& mesh : m_Meshes)
            for (auto& sm : mesh.second.m_SubMeshes)
            {
                auto pos = sm["POSITION"];
                auto idx = sm.GetIndexAttrib();
                D3D12_RAYTRACING_GEOMETRY_DESC desc{};
                desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                desc.Triangles.VertexBuffer.StartAddress = pos.Location;
                desc.Triangles.VertexBuffer.StrideInBytes = pos.Stride;
                desc.Triangles.VertexCount = pos.Count;
                desc.Triangles.VertexFormat = pos.Format;
                desc.Triangles.IndexBuffer = idx.Location;
                desc.Triangles.IndexCount = idx.Count;
                desc.Triangles.IndexFormat = idx.Format;
                desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
                geometries.push_back(std::move(desc));
            }
        m_BLAS = TLAS::BLAS(g, std::move(geometries));
    }

    Shared<SceneNode> Model::ParseScene(int sceneId)
    {
        tinygltf::Node currentNode = m_Model.nodes[sceneId];

        Shared<SceneNode> sceneNode = MakeShared<SceneNode>();
        sceneNode->Id = currentNode.mesh;

        if (!currentNode.matrix.empty())
        {
            XMFLOAT4X4 m;
            for (size_t i = 0; i < 4; i++) for (size_t j = 0; j < 4; j++)
            {
                m(i, j) = static_cast<float>(currentNode.matrix[4 * i + j]);
            }
            sceneNode->Transform = XMLoadFloat4x4(&m);
        }

        /*XMMATRIX M, T, R, S;
        M = T = R = S = DirectX::XMMatrixIdentity();

        std::vector<double> t = currentNode.translation;
        std::vector<double> r = currentNode.rotation;
        std::vector<double> s = currentNode.scale;

        if (!t.empty()) { T = XMMatrixTranslation(static_cast<float>(t[0]), static_cast<float>(t[1]), static_cast<float>(t[2])); }
        if (!r.empty()) { R = XMMatrixRotationQuaternion(XMLoadFloat4(&XMFLOAT4(static_cast<float>(r[0]), static_cast<float>(r[1]), static_cast<float>(r[2]), static_cast<float>(r[3])))); }
        if (!s.empty()) { S = XMMatrixScaling(static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2])); }
        M = XMMatrixMultiply(S, XMMatrixMultiply(R, T));*/

        // Check if the transform is already specified with one matrix

        for (int childId : currentNode.children)
        {
            sceneNode->Children.push_back(ParseScene(childId));
        }

        return sceneNode;
    }

    void Model::ParseVertices(Graphics& g)
    {
        const tinygltf::Scene& scene = m_Model.scenes[m_Model.defaultScene];

        for (const auto& buff : m_Model.buffers)
        {
            RawBuffer buffer(g, D3D12_HEAP_TYPE_UPLOAD, buff.data.size(), D3D12_RESOURCE_STATE_COMMON);
            std::memcpy(buffer.Map(), buff.data.data(), buffer.Size());
            buffer.Unmap();
            m_GPUBuffers.push_back(std::move(buffer));
        }

        UINT meshIdx = 0;
        DXGI_FORMAT posFmt;
        DXGI_FORMAT normFmt;
        DXGI_FORMAT texFmt;
        DXGI_FORMAT tanFmt;
        for (const tinygltf::Mesh& mesh : m_Model.meshes)
        {
            Mesh m;
            for (tinygltf::Primitive primitive : mesh.primitives)
            {
                SubMesh sm;
                if (primitive.attributes.find("POSITION") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["POSITION"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = std::max(bv.byteStride, GltfStride(accessor.type, accessor.componentType));
                    attr.Count = accessor.count;
                    posFmt = DXGI_FORMAT_R32G32B32_FLOAT;
                    attr.Format = posFmt;
                    attr.ByteLength = attr.Stride * attr.Count;
                    sm.m_NumVertices = attr.Count;
                    sm.m_PosStart = attr.Location;

                    DirectX::XMFLOAT3* vp = (DirectX::XMFLOAT3*)(m_Model.buffers[0].data.data() + bv.byteOffset);
                    for (int i = 0; i < attr.Count; i++, vp++)
                    {
                        //// Compute the radius of the scene
                        if (abs(vp->x) > m_BBox.max.x) m_BBox.max.x = abs(vp->x);
                        if (abs(vp->y) > m_BBox.max.y) m_BBox.max.y = abs(vp->y);
                        if (abs(vp->z) > m_BBox.max.z) m_BBox.max.z = abs(vp->z);
                    }

                    sm.AddVertexAttrib("POSITION", attr);
                }

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["NORMAL"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = std::max(bv.byteStride, GltfStride(accessor.type, accessor.componentType));
                    attr.Count = accessor.count;
                    normFmt = DXGI_FORMAT_R32G32B32_FLOAT;
                    attr.Format = normFmt;
                    attr.ByteLength = attr.Count * attr.Stride;

                    sm.AddVertexAttrib("NORMAL", attr);
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["TEXCOORD_0"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset;
                    attr.Stride = std::max(bv.byteStride, GltfStride(accessor.type, accessor.componentType));
                    attr.Count = accessor.count;
                    attr.ByteLength = attr.Stride * attr.Count;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        texFmt = DXGI_FORMAT_R32G32_FLOAT;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        texFmt = DXGI_FORMAT_R8G8_UNORM;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        texFmt = DXGI_FORMAT_R16G16_FLOAT;
                    attr.Format = texFmt;
                    sm.AddVertexAttrib("TEXCOORD_0", attr);
                }

                if (primitive.indices != -1)
                {
                    GltfAttribute attr;
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.indices];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset;
                    sm.m_IndexStart = attr.Location;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset;
                    attr.Stride = std::max(bv.byteStride, GltfStride(accessor.type, accessor.componentType));
                    attr.Count = accessor.count;
                    attr.ByteLength = attr.Stride * attr.Count;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        attr.Format = DXGI_FORMAT_R32_UINT;
                        /*uint32_t* indexes = (uint32_t*)(m_Model.buffers[0].data.data() + bv.byteOffset);

                        size_t indexesCount = m_Model.accessors[primitive.indices].count;
                        for (int i = 0; i < indexesCount; i += 3)
                        {
                            int tmp = indexes[i];
                            indexes[i] = indexes[i + 2];
                            indexes[i + 2] = tmp;
                        }*/
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        attr.Format = DXGI_FORMAT_R16_UINT;
                        uint16_t* indexes = (uint16_t*)(m_Model.buffers[0].data.data() + bv.byteOffset);

                        /*size_t indexesCount = m_Model.accessors[primitive.indices].count;
                        for (int i = 0; i < indexesCount; i += 3)
                        {
                            int tmp = indexes[i];
                            indexes[i] = indexes[i + 2];
                            indexes[i + 2] = tmp;
                        }*/
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        attr.Format = DXGI_FORMAT_R8_UINT;
                        /*uint8_t* indexes = (uint8_t*)(m_Model.buffers[0].data.data() + bv.byteOffset);

                        size_t indexesCount = m_Model.accessors[primitive.indices].count;
                        for (int i = 0; i < indexesCount; i += 3)
                        {
                            int tmp = indexes[i];
                            indexes[i] = indexes[i + 2];
                            indexes[i + 2] = tmp;
                        }*/
                    }

                    sm.SetIndexAttrib(attr);
                }

                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["TANGENT"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = std::max(bv.byteStride, GltfStride(accessor.type, accessor.componentType));
                    attr.Count = accessor.count;
                    attr.ByteLength = attr.Stride * attr.Count;
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        tanFmt = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        tanFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        tanFmt = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    attr.Format = tanFmt;
                    sm.AddVertexAttrib("TANGENT", attr);
                }
                else
                {
                    GltfAttribute attr{};
                    attr.ByteOffset = 0;
                    attr.Count = sm.m_NumVertices;
                    attr.Stride = sizeof(DirectX::XMFLOAT4);
                    attr.ByteLength = attr.Stride * attr.Count;
                    attr.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    tanFmt = attr.Format;
                    
                    switch (sm.GetIndexAttrib().Format)
                    {
                    case DXGI_FORMAT_R32_UINT:
                        CalculateTangents<UINT32>(g, primitive, sm, attr);
                        break;
                    case DXGI_FORMAT_R16_UINT:
                        CalculateTangents<UINT16>(g, primitive, sm, attr);
                        break;
                    case DXGI_FORMAT_R8_UINT:
                        CalculateTangents<UINT8>(g, primitive, sm, attr);
                        break;
                    }
                    
                    sm.AddVertexAttrib("TANGENT", attr);
                }
                
                switch (primitive.mode)
                {
                case TINYGLTF_MODE_POINTS:
                    sm.m_Topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                    break;
                case TINYGLTF_MODE_TRIANGLES:
                    sm.m_Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    break;
                }

                sm.m_Material = primitive.material;

                m_NumSubMeshes++;
                m.m_SubMeshes.push_back(std::move(sm));
            }

            m_Meshes.insert({ meshIdx++, std::move(m) });
        }

        m_Layout.AddElement<VERTEX_ATTRIBUTES::POS>(posFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::TEX>(texFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::NORM>(normFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::TAN>(tanFmt);
    }

    void Model::ParseMaterials(Graphics& g)
    {
        for (const tinygltf::Material& material : m_Model.materials)
        {
            Material mat{};
            mat.BaseColor =
            {
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3])
            };
            mat.Roughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
            mat.Metallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
            mat.BaseID     = material.pbrMetallicRoughness.baseColorTexture.index;
            mat.RMID       = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            mat.NormID     = material.normalTexture.index;
            mat.OccID      = material.occlusionTexture.index;
            mat.EmissiveID = material.emissiveTexture.texCoord;

            m_Materials.push_back(std::move(mat));
        }
    }

    void Model::ParseTextures(Graphics& g, const std::string& baseDir)
    {
        for (const tinygltf::Texture& texture : m_Model.textures)
        {
            tinygltf::Image image = m_Model.images[texture.source];
            if (image.bufferView != -1)
            {
                tinygltf::BufferView imageBufferView = m_Model.bufferViews[image.bufferView];
                tinygltf::Buffer imageBuffer = m_Model.buffers[imageBufferView.buffer];
                uint8_t* imageBufferBegin = imageBuffer.data.data() + imageBufferView.byteOffset;
                m_Textures.push_back(MakeShared<Texture2D>(g, imageBufferBegin, imageBufferView.byteLength));
            }
            else
                m_Textures.push_back(MakeShared<Texture2D>(g, baseDir + "\\" + image.uri));
        }
    }

    void Model::ParseSamplers()
    {
        if (m_Model.samplers.empty())
            m_Samplers.push_back(MakeShared<Sampler>());
        else
        {
            for (tinygltf::Sampler sampler : m_Model.samplers)
            {
                Shared<Sampler> s = MakeShared<Sampler>();
                D3D12_TEXTURE_ADDRESS_MODE mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

                switch (sampler.wrapS)
                {
                case TINYGLTF_TEXTURE_WRAP_REPEAT:
                    break;
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    break;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    break;
                }
                s->SetAddressU(mode);

                switch (sampler.wrapT)
                {
                case TINYGLTF_TEXTURE_WRAP_REPEAT:
                    break;
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    break;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    break;
                }
                s->SetAddressV(mode);

                m_Samplers.push_back(std::move(s));
            }
        }
    }
    
}