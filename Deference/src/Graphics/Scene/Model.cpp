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
        default:
            break;
        }

        SIZE_T bytes = 0;
        switch (compType)
        {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
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
        default:
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
        CreateGeometry(g, geometries, XMMatrixIdentity(), m_RootNode);
           
        m_BLAS = TLAS::BLAS(g, std::move(geometries));
    }

    Shared<SceneNode> Model::ParseScene(int sceneId)
    {
        tinygltf::Node currentNode = m_Model.nodes[sceneId];

        Shared<SceneNode> sceneNode = MakeShared<SceneNode>();
        sceneNode->Id = currentNode.mesh;

        XMMATRIX M, T, R, S;
        M = T = R = S = DirectX::XMMatrixIdentity();

        std::vector<double> t = currentNode.translation;
        std::vector<double> r = currentNode.rotation;
        std::vector<double> s = currentNode.scale;

        // Compute the composed transform: Scale, then Rotate, than Translate
        if (!t.empty()) { T = XMMatrixTranslation(static_cast<float>(t[0]), static_cast<float>(t[1]), static_cast<float>(t[2])); }
        if (!r.empty()) { 
            auto v = XMFLOAT4(static_cast<float>(r[0]), static_cast<float>(r[1]), static_cast<float>(r[2]), static_cast<float>(r[3]));
            R = XMMatrixRotationQuaternion(XMLoadFloat4(&v)); 
        }
        if (!s.empty()) { S = XMMatrixScaling(static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2])); }
        M = XMMatrixMultiply(S, XMMatrixMultiply(R, T));

        // Check if the transform is already specified with one matrix
        if (!currentNode.matrix.empty())
        {
            XMFLOAT4X4 m;
            for (size_t i = 0; i < 4; i++) for (size_t j = 0; j < 4; j++) { m(i, j) = static_cast<float>(currentNode.matrix[4 * i + j]); }
            M = XMLoadFloat4x4(&m);
        }

        sceneNode->Transform = std::move(M);

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
            GenericBuffer buffer(g, D3D12_HEAP_TYPE_UPLOAD, buff.data.size(), D3D12_RESOURCE_STATE_COMMON);
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
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset + accessor.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = accessor.ByteStride(bv);
                    attr.Count = accessor.count;
                    posFmt = DXGI_FORMAT_R32G32B32_FLOAT;
                    attr.Format = posFmt;
                    attr.ByteLength = bv.byteLength;
                    sm.m_NumVertices = attr.Count;
                    sm.m_PosStart = attr.Location;

                    const std::vector<float> mins(accessor.minValues.begin(), accessor.minValues.end());
                    const std::vector<float> maxes(accessor.maxValues.begin(), accessor.maxValues.end());
                    const XMFLOAT3 min = { mins[0], mins[1], mins[2] };
                    const XMFLOAT3 max = { maxes[0], maxes[1], maxes[2] };
                    m_BBox = m_BBox.Union(BBox(min, max));

                    sm.AddVertexAttrib("POSITION", attr);
                }

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["NORMAL"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset + accessor.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = accessor.ByteStride(bv);
                    attr.Count = accessor.count;
                    normFmt = DXGI_FORMAT_R32G32B32_FLOAT;
                    attr.Format = normFmt;
                    attr.ByteLength = bv.byteLength;

                    sm.AddVertexAttrib("NORMAL", attr);
                }

                UINT tcSemIdx = 0;
                std::string tcSem = std::format("TEXCOORD_{}", tcSemIdx);
                for (;primitive.attributes.find(tcSem) != primitive.attributes.end();) {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes[tcSem]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset + accessor.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset;
                    attr.Stride = accessor.ByteStride(bv);
                    attr.Count = accessor.count;
                    attr.ByteLength = bv.byteLength;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        texFmt = DXGI_FORMAT_R32G32_FLOAT;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        texFmt = DXGI_FORMAT_R8G8_UNORM;
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        texFmt = DXGI_FORMAT_R16G16_FLOAT;
                    attr.Format = texFmt;
                    sm.AddVertexAttrib(tcSem, attr);
                    tcSem = std::format("TEXCOORD_{}", ++tcSemIdx);
                }

                if (primitive.indices != -1)
                {
                    GltfAttribute attr;
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.indices];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset + accessor.byteOffset;
                    sm.m_IndexStart = attr.Location;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset;
                    attr.Stride = accessor.ByteStride(bv);
                    attr.Count = accessor.count;
                    attr.ByteLength = bv.byteLength;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                    {
                        attr.Format = DXGI_FORMAT_R32_UINT;
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        attr.Format = DXGI_FORMAT_R16_UINT;
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        attr.Format = DXGI_FORMAT_R8_UINT;
                    }

                    sm.SetIndexAttrib(attr);
                }

                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    GltfAttribute attr{};
                    tinygltf::Accessor accessor = m_Model.accessors[primitive.attributes["TANGENT"]];
                    tinygltf::BufferView bv = m_Model.bufferViews[accessor.bufferView];
                    attr.BufferID = bv.buffer;
                    attr.Location = m_GPUBuffers[bv.buffer].GetGPUAddress() + bv.byteOffset + accessor.byteOffset;
                    attr.ByteOffset = bv.byteOffset + accessor.byteOffset; // Accessors defines an additional offset
                    attr.Stride = accessor.ByteStride(bv);
                    attr.Count = accessor.count;
                    attr.ByteLength = bv.byteLength;
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
                case TINYGLTF_MODE_TRIANGLE_STRIP:
                    sm.m_Topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                    break;
                }

                sm.m_Material = primitive.material;

                m_NumSubMeshes++;
                m.m_SubMeshes.push_back(std::move(sm));
            }

            m_Meshes.insert({ meshIdx++, std::move(m) });
        }

        m_Layout.AddElement<VERTEX_ATTRIBUTES::POS>(posFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::TEX_0>(texFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::NORM>(normFmt);
        m_Layout.AddElement<VERTEX_ATTRIBUTES::TAN>(tanFmt);
    }

    void Model::ParseMaterials(Graphics& g)
    {
        auto getIndexer = [&](const tinygltf::TextureInfo& info) {
            return Material::TextureIndexer{ info.index, info.texCoord };
        };

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
            mat.EmissiveColor =
            {
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2])
            };
            mat.Roughness = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
            mat.Metallic = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
            mat.BaseTex     = getIndexer(material.pbrMetallicRoughness.baseColorTexture);
            mat.RMTex       = getIndexer(material.pbrMetallicRoughness.metallicRoughnessTexture);
            mat.NormTex = Material::TextureIndexer{ material.normalTexture.index, material.normalTexture.texCoord };
            mat.OccTex = Material::TextureIndexer{ material.occlusionTexture.index, material.occlusionTexture.texCoord };
            mat.EmissiveTex = getIndexer(material.emissiveTexture);

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

    void Model::CreateGeometry(Graphics& g, std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& descs, XMMATRIX parentTransform, Shared<SceneNode> node)
    {
        XMMATRIX world = XMMatrixMultiply(node->Transform, parentTransform);
        XMFLOAT3X4 mat;
        XMStoreFloat3x4(&mat, world);
        GenericBuffer buffer(g, D3D12_HEAP_TYPE_UPLOAD, ALIGN(sizeof(XMFLOAT3X4), D3D12_RAYTRACING_TRANSFORM3X4_BYTE_ALIGNMENT), D3D12_RESOURCE_STATE_COMMON);
        std::memcpy(buffer.Map(), &mat, buffer.Size());
        buffer.Unmap();
        if (node->Id != -1)
        {
            auto& mesh = m_Meshes[node->Id];
            for (auto& sm : mesh.m_SubMeshes)
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
                desc.Triangles.Transform3x4 = buffer.GetGPUAddress();
                descs.push_back(std::move(desc));
            }
        }

        m_GPUBuffers.push_back(std::move(buffer));

        for (auto& child : node->Children)
            CreateGeometry(g, descs, world, child);
    }
    
}