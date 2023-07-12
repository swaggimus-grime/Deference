//struct CamData
//{
//    matrix view;
//    matrix proj;
//};

struct ModelTransform
{
    matrix mvp;
    float3x3 normMat;
    matrix model;
};

ConstantBuffer<ModelTransform> modelTransform : register(b0);
//ConstantBuffer<ModelData> modelData : register(b1);