#ifndef ENTRANCE_H
#define ENTRANCE_H

#include "Graphics/Renderer.h"

#include <memory>
#include <thread>
#include <mutex>
#include <string>


namespace ViniviaSDK
{
    struct EntrancePrivate;

    /**
     * Vinivia SDK class;
     * Wrapping all functionality of Vinivia Engine to Android Platforms
     * Also controlling thread mechanism
     */
    class Entrance {
    private:
        /**
         * C-tor of sdk engine
         * storing all application thread states and basic configurations
         */
        Entrance(void);

        /**
         * Deleted copy c-tor
         */
        Entrance(const Entrance &) = delete;

        /**
         * Deleted = operator for singleton object
         * @return reference of ViniviaSDK object
         */
        Entrance &operator=(const Entrance &) = delete;

        /**
        * D-tor of vinivia sdk engine
        */
        ~Entrance(void) = default;

        /**
         * Initialize ViniviaEngine instance which is coming from ViniviaEngine Library
         * @param:
         */
        void initializeEngine();

        /**
         * Create Renderer that is working with OpenGL ES 2
         * @return
         */
        Renderer* CreateES2Renderer();

    private:
        static Entrance *instance;
        static std::mutex mutex;

    public:
        /**
        * retrieve global singleton ViniviaSDK instance
        * @return the only instance of ViniviaSDK in the app
        */
        static Entrance *getInstance(void);

        /**
         * Get alias of renderer
         * @return : Renderer
         */
        Renderer* getRenderer();

    protected:
        std::unique_ptr<Renderer> m_renderer;
    };
}

#endif  // ENTRANCE_H
