#include <CoreLib.Math/Math.h>
#include <cassert>
#include <iostream>

using namespace jxcorlib::math;
using namespace jxcorlib;
using namespace std;

void TestMath()
{
    Vector3f v{ 3,3,3 };

    auto vec3_box = mkbox(v);

    Vector3f v4 = vec3_box->get_unboxing_value();

    //Quat4f(Vector3f{});
    //cout << to_string(Quat4f(1, 0, 0, 0)) << endl;
    //cout << to_string(Quat4f(1, 0, 0, 0).GetEuler()) << endl;

    //auto q = Quat4f(0.224f, 0.827f, -0.514f, -0.038f);
    //auto qe = q.GetEulerZYX();
    //auto q2 = Quat4f(qe);
    //cout << to_string(q ) << endl;
    //cout << to_string(qe ) << endl;
    //cout << to_string(q2 ) << endl;

    //cout << to_string(Quat4f(0.023, 0.003, 0.948, -0.316).GetEuler()) << endl;

    //assert(Quat4f(-1, 0, 0.023, -0.008).GetEuler() == Vector3f(0.020, -2.591, 0.864));
}