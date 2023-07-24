#include "Model.h"
#include "Graphics.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/Exception.h"
#include "Bindable/Pipeline/VertexBuffer.h"
#include "Bindable/Pipeline/InputLayout.h"
#include "Frame/GeometryGraph.h"
#include "Bindable/Heap/AccelStruct.h"

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
        aiProcess_CalcTangentSpace);
    BR (scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    INT texIdx = 0;
    const auto dir = filePath.substr(0, filePath.find_last_of('\\'));
    InputLayout layout(INPUT_LAYOUT_CONFIG::GEOMETRY_PIPELINE);
    for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) 
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        
        VertexStream stream(std::move(layout), mesh->mNumVertices);
        for (UINT vert = 0; vert < mesh->mNumVertices; vert++) {
            stream.Pos(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mVertices[vert].x);
            for (UINT i = 0; i < 8; i++)
            {
                if (mesh->HasTextureCoords(i))
                {
                    stream.Tex(vert) = *reinterpret_cast<XMFLOAT2*>(&mesh->mTextureCoords[i][vert].x);
                    break;
                }
            }

            stream.Norm(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mNormals[vert].x);
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

        m_Buffers.emplace_back(MakeShared<VertexBuffer>(g, std::move(stream)) , MakeShared<IndexBuffer>(g, indices.size(), indices.data()));
    
        auto getTexture = [&](aiTextureType type)
        {
            aiString texName;
            mat->GetTexture(type, 0, &texName);
            if (texName.length == NULL)
                return -1;
            std::string texPath = std::string(texName.C_Str());
            texPath = dir + "\\" + texPath;
            m_Textures.push_back(MakeShared<Texture2D>(g, std::wstring(texPath.begin(), texPath.end())));
            return texIdx++;
        };

        m_TextureIndexes.emplace_back(
            getTexture(aiTextureType_DIFFUSE),
            getTexture(aiTextureType_SPECULAR),
            getTexture(aiTextureType_NORMALS));
    }

    m_BLAS = TLAS::BLAS(g, m_Buffers);
}
