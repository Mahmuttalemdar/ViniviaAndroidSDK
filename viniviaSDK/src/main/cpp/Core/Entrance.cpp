#include "Entrance.h"
#include "Entrance_p.h"

#include "Graphics/Graphics.h"
#include "Graphics/Renderer.h"
#include "Log/Log.h"
#include "Graphics/RendererES2.h"

namespace ViniviaSDK
{
    Entrance *Entrance::instance = nullptr;
    std::mutex Entrance::mutex;

    Entrance::Entrance()
            : m_renderer(nullptr) {
        initializeEngine();
    }

    void Entrance::initializeEngine() {
        if (m_renderer) {
            m_renderer.reset();;
        }

        m_renderer.reset(CreateES2Renderer());
    }

    Renderer *Entrance::CreateES2Renderer() {
        RendererES2 *renderer = new RendererES2;
        return renderer;
    }

    Entrance *Entrance::getInstance(void) {
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex);
            if (instance == nullptr) {
                instance = new Entrance();
            }
        }
        return instance;
    }

    Renderer *Entrance::getRenderer() {
        return m_renderer.get();
    }
}