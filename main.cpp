#include <Apatite/Application.h>
#include <Apatite/EngineAppInstance.h>
#include <ApatiteEd/EditorAppInstance.h>
#include <ApatiteEd/EditorUIConfig.h>
#include <CoreLib/File.h>

using namespace apatite;
using namespace apatiteed;

#include <Apatite/TransformUtil.h>

int main()
{
    /*Matrix4f mat = Matrix4f::StaticScalar();
    Quat4f q = { -0.713 , 0.002 , 0.665, -0.222 };
    transutil::Rotate(&mat, q);
    std::cout << to_string(q.GetEuler()) << std::endl;
    q.SetEulerZYX({ -68.8344f,-71.3343f,86.9160 });
    std::cout << to_string(q.GetEuler()) << std::endl;*/
    //std::cout << to_string(mat);
    return Application::Exec(new EditorAppInstance, "apatite", { 1280,720 });
}