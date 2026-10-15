#include <CoreLib/Property.hpp>
#include <CoreLib/CoreLib.h>
#include <cassert>

using namespace jxcorlib;

class PropertyClass
{
public:
    int i_;
public:
    Property<int> i
    {
        PROP_GET(int) 
        {
            return this->i_;
        },
        PROP_SET(int) 
        {
            this->i_ = value;
        }
    };
};

void TestProperty()
{
    PropertyClass c;
    
    c.i = 3;
    assert(c.i_ == 3);

    int num = c.i;
    assert(num == 3);
}