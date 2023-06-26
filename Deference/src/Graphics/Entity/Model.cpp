#include "Model.h"
#include "Graphics.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Debug/Exception.h"
#include "Bindable/VertexBuffer.h"
#include "Frame/FrameGraph.h"
#include "Bindable/InputLayout.h"

Model::Model(Graphics& g, const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded |
        aiProcess_PreTransformVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);
    BR (scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode);

    m_Meshes.reserve(scene->mNumMeshes);
    for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* pMesh = scene->mMeshes[meshIndex];
        const aiMaterial* pMat = scene->mMaterials[pMesh->mMaterialIndex];

        auto mat = MakeShared<Material>(g, pMat, filePath.substr(0, filePath.find_last_of('\\')));
        Mesh mesh(g, pMesh, mat, InputLayout::s_PosTexColor);
        AddDrawable(mesh);
        m_Meshes.push_back(std::move(mesh));
        m_Materials.push_back(std::move(mat));
    }
}

Model::Mesh::Mesh(Graphics& g, const aiMesh* mesh, Shared<Material> mat, const InputLayout& layout)
{
    using enum VERTEX_ATTRIBUTES;
    VertexStream stream(layout, mesh->mNumVertices);

    for (UINT vert = 0; vert < mesh->mNumVertices; vert++) {
        stream.Pos(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mVertices[vert].x);
        stream.Color(vert) = XMFLOAT4(1.f, 1.f, 0.f, 1.f);
        //stream.Norm(vert) = *reinterpret_cast<XMFLOAT3*>(&mesh->mNormals[vert].x);
        stream.Tex(vert) = *reinterpret_cast<XMFLOAT2*>(&mesh->mTextureCoords[0][vert].x);
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

    /*{
        Step step("lambertian", ib->NumIndices());
        step.AddBindable(vb);
        step.AddBindable(ib);
        step.AddBindable(mat);
        AddStep(std::move(step));
    }*/
    CreateBottomLevelAS(g, { m_VB }, { m_IB });
}

Model::Material::Material(Graphics& g, const aiMaterial* mat, const std::string& dir)
{
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

    getTexture(aiTextureType_DIFFUSE);
    if (texturePaths.size() > 0)
    {
        m_TextureHeap = MakeUnique<Heap<Texture2D>>(g, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, texturePaths.size(), true);
        for (const auto& path : texturePaths) 
            m_TextureHeap->AddResource<Texture2D>(g, std::wstring(path.begin(), path.end()));
    }
}

void Model::Material::Bind(Graphics& g)
{
    if (m_TextureHeap)
    {
        m_TextureHeap->Bind(g);
        g.CL().SetGraphicsRootDescriptorTable(1, m_TextureHeap->GPUHandle());
    }
}