//struct CamData
//{
//    matrix view;
//    matrix proj;
//};

struct ModelTransform
{
    matrix model;
    matrix mvp;
    float3x3 normMat;
};

ConstantBuffer<ModelTransform> modelTransform : register(b0);
//ConstantBuffer<ModelData> modelData : register(b1);