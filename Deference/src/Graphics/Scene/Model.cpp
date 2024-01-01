#include "Model.h"
#include "Graphics.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/Exception.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/InputLayout.h"
#include "Bindable/Heap/AccelStruct.h"
#include <ranges>

aiVector3D max(const aiVector3D& v1, const aiVector3D& v2)
{
    return v1.Length() > v2.Length() ? v1 : v2;
}

aiVector3D min(const aiVector3D& v1, const aiVector3D& v2)
{
    return v1.Length() <= v2.Length() ? v1 : v2;
}

Model::Model(Graphics& g, const std::string& filePath)
    :m_World(XMMatrixIdentity())
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_GenBoundingBoxes);
    BR (scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    aiVector3D modelMax;
    aiVector3D modelMin;

    INT texIdx = 0;
    const auto dir = filePath.substr(0, filePath.find_last_of('\\'));
    InputLayout layout(INPUT_LAYOUT_CONFIG::GEOMETRY_PIPELINE);
    for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) 
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        
        VertexStream stream(std::move(layout), mesh->mNumVertices);
        for (UINT vert = 0; vert < mesh->mNumVertices; vert++) {
            stream.Pos(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mVertices[vert].x) / 10.f;
            if (mesh->HasVertexColors(0))
                stream.Color(vert) = *reinterpret_cast<XMFLOAT4*>(&mesh->mColors[vert][0].r);
            else
                stream.Color(vert) = XMFLOAT4(0.f, 0.f, 0.f, 0.f);

            for (UINT i = 0; i < 8; i++)
            {
                if (mesh->HasTextureCoords(i))
                {
                    stream.Tex(vert) = *reinterpret_cast<XMFLOAT2*>(&mesh->mTextureCoords[i][vert].x);
                    break;
                }
            }

            if (mesh->HasNormals())
                stream.Norm(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mNormals[vert].x);
            else
                stream.Norm(vert) = { 0, 0, 0 };
                
            if (mesh->HasTangentsAndBitangents())
            {
                stream.Tan(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mTangents[vert].x);
                stream.Bitan(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mBitangents[vert].x);
            }
            else
            {
                stream.Tan(vert) = { 0, 0, 0 };
                stream.Bitan(vert) = { 0, 0, 0 };
            }
        }

        std::vector<UINT> indices;
        indices.reserve(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);
        for (UINT faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
        {
            const aiFace& face = mesh->mFaces[faceIndex];
            for (UINT indIndex = 0; indIndex < face.mNumIndices; indIndex++)
                indices.push_back(face.mIndices[indIndex]);
        }

        auto getTexture = [&](aiTextureType type)
        {
            aiString texName;
            mat->GetTexture(type, 0, &texName);
            if (texName.length == NULL)
                return MakeShared<Texture2D>();
            std::string texPath = std::string(texName.C_Str());
            texPath = dir + "\\" + texPath;
            return MakeShared<Texture2D>(g, std::wstring(texPath.begin(), texPath.end()));
        };

        const auto bb = mesh->mAABB;

        modelMin = min(modelMin, bb.mMin);
        modelMax = max(modelMax, bb.mMax);

        Mesh m;
        m.m_VB = MakeShared<VertexBuffer>(g, std::move(stream));
        m.m_IB = MakeShared<IndexBuffer>(g, indices.size(), indices.data());
        m.m_DiffuseMap = getTexture(aiTextureType_DIFFUSE);
        m.m_NormalMap = getTexture(aiTextureType_NORMALS);
        m.m_SpecularMap = getTexture(aiTextureType_SPECULAR);
        m.m_EmissiveMap = getTexture(aiTextureType_EMISSIVE);
        ConstantBufferLayout layout;
        layout.Add<CONSTANT_TYPE::XMFLOAT4>("albedo");
        layout.Add<CONSTANT_TYPE::XMFLOAT4>("specular");
        layout.Add<CONSTANT_TYPE::XMFLOAT4>("emissive");
        m.m_Materials = MakeShared<ConstantBuffer>(g, std::move(layout));
        aiColor4D col;
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &col);
        (*m.m_Materials)["albedo"] = XMFLOAT4(col.r, col.g, col.b, col.a);
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &col);
        (*m.m_Materials)["specular"] = XMFLOAT4(col.r, col.g, col.b, col.a);
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &col);
        (*m.m_Materials)["emissive"] = XMFLOAT4(col.r, col.g, col.b, col.a);
        m_Meshes.push_back(std::move(m));
    }

    std::memcpy(&m_BBox.min, &modelMin, sizeof(XMFLOAT3));
    std::memcpy(&m_BBox.max, &modelMax, sizeof(XMFLOAT3));

    const auto& pairs =
        std::views::iota(0u, (UINT)m_Meshes.size()) |
        std::views::transform([&](UINT i) {
            return std::make_pair(m_Meshes[i].m_VB, m_Meshes[i].m_IB);
        }) |
        std::ranges::to<std::vector>();
    m_BLAS = TLAS::BLAS(g, pairs);
}
