#include <Pulsar/Application.h>
#include <Pulsar/EngineAppInstance.h>
#include <PulsarEd/EditorAppInstance.h>
#include <PulsarEd/EditorUIConfig.h>
#include <CoreLib/File.h>
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace pulsar;
using namespace pulsared;

#include <Pulsar/TransformUtil.h>

int main()
{
    // auto tri = jmath::Triangle3f{{0.0f, 0.0, 0}, {1.f, 0, 0}, {0.f, 1.f, 0}};
    // cout << to_string(tri.BarycentricCoordinates({-0.5f,0.1f,0}));
    //
    // Vector3f inter{};
    // jmath::Edge3<float> edge { {0.01f, 0.01, -1}, {0.01f, 0.01, 1} };
    // jmath::Intersect(tri.GetPlane(), edge, inter);

    return Application::Exec(new EditorAppInstance, "Pulsar Editor", { 1280,720 });
}