#include "Viewport.h"

Viewport::Viewport(Graphics& g)
{
    // Fill out the Viewport
    m_VP.TopLeftX = 0;
    m_VP.TopLeftY = 0;
    m_VP.Width = g.Width();
    m_VP.Height = g.Height();
    m_VP.MinDepth = 0.0f;
    m_VP.MaxDepth = 1.0f;

    // Fill out a scissor rect
    m_SR.left = 0;
    m_SR.top = 0;
    m_SR.right = g.Width();
    m_SR.bottom = g.Height();
}

void Viewport::Bind(Graphics& g)
{
    g.CL().RSSetViewports(1, &m_VP);
    g.CL().RSSetScissorRects(1, &m_SR);
}

void Viewport::Resize(UINT w, UINT h)
{
    m_VP.Width = w;
    m_VP.Height = h;
    m_SR.right = w;
    m_SR.bottom = h;
}
