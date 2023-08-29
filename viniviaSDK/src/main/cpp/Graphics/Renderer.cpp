#include "Renderer.h"
#include "Graphics/Graphics.h"
#include "Filters/NoFilter.h"
#include "Filters/GrayscaleFilter.h"
#include "Filters/CubeFilter.h"
#include "Filters/CubeWithDepthMapFilter.h"
#include "Filters/DepthMapFilter.h"
#include "Filters/NegativeFilter.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace ViniviaSDK
{
    Renderer::Renderer()
        : m_filter(Filter::NO_FILTER)
    {}

    Renderer::~Renderer() {}

    void Renderer::SetFilter(const Filter filter) {
        m_filter = filter;
        switch (filter) {
            case Filter::NO_FILTER:
                m_shaderFilter.reset(new NoFilter());
                break;
            case Filter::GRAYSCALE_FILTER:
                m_shaderFilter.reset(new GrayscaleFilter());
                break;
            case Filter::CUBE_FILTER:
                m_shaderFilter.reset(new CubeFilter());
                break;
            case Filter::CUBE_WITH_DEPTH_MAP_FILTER:
                m_shaderFilter.reset(new CubeWithDepthMapFilter());
                break;
            case Filter::DEPTH_MAP_FILTER:
                m_shaderFilter.reset(new DepthMapFilter());
                break;
            case Filter::NEGATIVE_FILTER:
                m_shaderFilter.reset(new NegativeFilter());
                break;
            default:
                m_shaderFilter.reset(new NoFilter());
                break;
        }
    }

    const Filter Renderer::GetFilter() const {
        return m_filter;
    }

    BaseFilter *const Renderer::ActiveShaderFilter() const {
        return m_shaderFilter.get();
    }
}