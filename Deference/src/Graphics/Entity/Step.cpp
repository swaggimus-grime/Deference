#include "Step.h"
#include "Bindable/IndexBuffer.h"

Step::Step(const std::string& passName, UINT idxCount)
    :m_PassName(std::move(passName)), m_IdxCount(idxCount)
{
}

void Step::Run(Graphics& g)
{
    for (auto& b : m_Bindables)
        b->Bind(g);
    g.CL().DrawIndexedInstanced(m_IdxCount, 1, 0, 0, 0);
}