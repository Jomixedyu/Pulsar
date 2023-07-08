#include <CoreLib/Guid.h>
#include <cassert>

using namespace jxcorlib;
using namespace std;

void TestGuid()
{
    guid_t guid = guid_t::parse("4d9922c82335473ba7a338c76252e976");

    assert(guid == guid_t::parse("4d9922c8-2335-473b-a7a338c76252e976"));
    assert(guid == guid_t::parse("4D9922C82335473BA7A338C76252E976"));
}