#include "Pulsar/Assets/CurveLinearColorAtlas.h"

namespace pulsar
{

    CurveLinearColorAtlas::CurveLinearColorAtlas() : m_textureWidth(256)
    {
        init_sptr_member(m_curves);
    }

    void CurveLinearColorAtlas::Generate()
    {
        auto curveCount = m_curves->size();

        for (int row = 0; row < curveCount; ++row)
        {
            auto curve = m_curves->at(row).GetPtr();
            if (curve)
            {
                for (int column = 0; column < m_textureWidth; ++column)
                {
                    auto t = (float)column / (float)m_textureWidth;
                    auto color = curve->SampleColor(t);

                }
            }
        }

        // fill texture
    }
} // namespace pulsar