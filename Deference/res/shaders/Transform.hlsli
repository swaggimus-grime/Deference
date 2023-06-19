struct Transform
{
    matrix view;
    matrix proj;
    matrix viewI;
    matrix projI;
};

ConstantBuffer<Transform> transform : register(b0);