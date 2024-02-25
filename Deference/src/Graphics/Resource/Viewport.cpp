#include "Viewport.h"

namespace Def
{
    Viewport::Viewport(UINT w, UINT h)
    {
        // Fill out the Viewport
        m_VP.TopLeftX = 0;
        m_VP.TopLeftY = 0;
        m_VP.Width = w;
        m_VP.Height = h;
        m_VP.MinDepth = 0.0f;
        m_VP.MaxDepth = 1.0f;

        // Fill out a scissor rect
        m_SR.left = 0;
        m_SR.top = 0;
        m_SR.right = w;
        m_SR.bottom = h;
    }

    void Viewport::Bind(Graphics& g)
    {
        g.CL().RSSetViewports(1, &m_VP);
        g.CL().RSSetScissorRects(1, &m_SR);
    }

    void Viewport::Resize(Graphics& g, UINT w, UINT h)
    {
        m_VP.Width = w;
        m_VP.Height = h;
        m_SR.right = w;
        m_SR.bottom = h;
    }

}