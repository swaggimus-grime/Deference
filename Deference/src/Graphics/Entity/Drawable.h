#pragma once

class Transform;

class Drawable
{
public:
	inline UINT DiffuseIndex() const { return m_DiffuseIndex; }
	inline auto& GetCBVHeap() const { return *m_CBVHeap; }
	inline auto& GetTextureHeap() const { return *m_TextureHeap; }
	inline auto& BLAS() const { return m_BLAS; }
	void Rasterize(Graphics& g);

private:
	void Update(Graphics& g);

protected:
	Shared<VertexBuffer> m_VB;
	Shared<IndexBuffer> m_IB;
	std::vector<Shared<Bindable>> m_Bindables;
	Unique<SucHeap> m_CBVHeap;
	Unique<SucHeap> m_TextureHeap;
	Shared<Transform> m_Transform;
	ComPtr<ID3D12Resource> m_BLAS;
	UINT m_DiffuseIndex;
};

class DrawableCollection
{
public:
	inline auto& Drawables() const { return m_Drawables; }

protected:
	std::vector<Shared<Drawable>> m_Drawables;
};