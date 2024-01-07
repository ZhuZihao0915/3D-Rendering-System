#include "server/Server.hpp"
#include "scene/Scene.hpp"
#include "component/RenderComponent.hpp"
#include "Camera.hpp"

#include "PhotonMapRenderer.hpp"

using namespace std;
using namespace NRenderer;

namespace PhotonMap
{
    class Adapter : public RenderComponent
    {
        void render(SharedScene spScene) {
            PhotonMapRenderer renderer{spScene};
            auto renderResult = renderer.render();
            auto [ pixels, width, height ]  = renderResult;
            getServer().screen.set(pixels, width, height);
            renderer.release(renderResult);
        }
    };
}

const static string description = 
    "A Photon Mapping. "
    "Lambertian and Dielectric are supported."
    "\nPlease use scene file : path_tracing_cornel.scn";

REGISTER_RENDERER(PhotonMap, description, PhotonMap::Adapter);