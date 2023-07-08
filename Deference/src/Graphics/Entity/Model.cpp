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
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    BR (scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    m_Drawables.reserve(scene->mNumMeshes);

    UINT diffIdx = 0;
    for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) 
    {
        const aiMesh* pMesh = scene->mMeshes[meshIndex];
        const aiMaterial* pMat = scene->mMaterials[pMesh->mMaterialIndex];
        m_Drawables.push_back(MakeShared<Mesh>(g, this, diffIdx, pMesh, pMat, filePath.substr(0, filePath.find_last_of('\\'))));
    }
}

Model::Mesh::Mesh(Graphics& g, Model* parent, UINT& diffIdx, const aiMesh* mesh, const aiMaterial* mat, const std::string& dir)
{
    InputLayout layout(INPUT_LAYOUT_CONFIG::GEOMETRY_PIPELINE);
    VertexStream stream(std::move(layout), mesh->mNumVertices);
    for (UINT vert = 0; vert < mesh->mNumVertices; vert++) {
            stream.Pos(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mVertices[vert].x);
        //stream.Color(vert) = XMFLOAT4(1.f, 1.f, 0.f, 1.f);
            stream.Tex(vert) = *reinterpret_cast<XMFLOAT2*>(&mesh->mTextureCoords[0][vert].x);
            stream.Norm(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mNormals[vert].x);
        //stream.Tan(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mTangents[vert].x);
        //stream.Bitan(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mBitangents[vert].x);
    }
    m_VB = MakeShared<VertexBuffer>(g, stream);

    std::vector<UINT> indices;
    indices.reserve(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);
    for (UINT faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
    {
        const aiFace& face = mesh->mFaces[faceIndex];
        for (UINT indIndex = 0; indIndex < face.mNumIndices; indIndex++)
            indices.push_back(face.mIndices[indIndex]);
    }
    m_IB = MakeShared<IndexBuffer>(g, indices.size(), indices.data());

    std::vector<std::string> texturePaths;
    auto getTexture = [&](aiTextureType type)
    {
        aiString texName;
        mat->GetTexture(type, 0, &texName);
        if (texName.length == NULL)
            return;
        std::string texPath = std::string(texName.C_Str());
        texPath = dir + "\\" + texPath;
        texturePaths.push_back(std::move(texPath));
    };

    getTexture(aiTextureType_BASE_COLOR);

    m_CBVHeap = MakeUnique<SucHeap>(g, 2); // textures + transform + material cbuff
    m_Transform = m_CBVHeap->Add<Transform>(g);
    //{
    //    auto material = m_CBVHeap->Add<Material>(g);
    //    MaterialParams m;
    //    m.diffuseColor = XMFLOAT3(1.f, 0.5f, 1.f);
    //    m.diffTableIdx = meshIdx;
    //    material->Update(&m);
    //    m_Bindables.push_back(std::move(material));
    //}

    if (texturePaths.size() > 0)
    {
        m_DiffuseIndex = diffIdx++;
        m_TextureHeap = MakeUnique<SucHeap>(g, texturePaths.size());
        for (const auto& path : texturePaths)
            m_TextureHeap->Add<Texture2D>(g, std::wstring(path.begin(), path.end()));
    }
    else
        m_DiffuseIndex = 9999;

    m_BLAS = TLAS::BLAS(g, { m_VB }, { m_IB });
}