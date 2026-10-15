#include <CoreLib/Events.hpp>

#include <iostream>

using namespace jxcorlib;

class TestEventsClass
{
public:
    int cb_index;

    Action<int> e;
    void lambda_inst(int c) {
        std::cout << "lambda_inst callback" << std::endl;
    }
    void bind_inst(int c) {
        std::cout << "bind_inst callback" << std::endl;
    }
    static void static_method(int c) {
        std::cout << "static_method callback" << std::endl;
    }

    TestEventsClass()
    {
        //静态lambda两种方式
        e.AddListener([](int c) { std::cout << "static lambda1 callback" << std::endl; });
        e += [](int c) { std::cout << "static lambda2 callback" << std::endl; };

        //静态函数
        e += static_method; //或 e.AddListener(static_method);
        //e -= static_method; //或 e.RemoveListener(static_method);

        //添加与移除闭包lambda方法，可以把lambda托管给this，然后最后按实例移除
        this->cb_index = e.AddListener(this, [this](int c) { this->lambda_inst(c); });
        //e.RemoveListenerByIndex(this->cb_index);

        //添加与移除成员方法
        e.AddListener(this, &TestEventsClass::bind_inst);
        //e.RemoveListener(this, &EventTest::bind_inst);

        //执行
        e.Invoke(3);

        //移除实例中的所有事件
        e.RemoveListenerByInstance(this);

        //移除全部事件
        e.RemoveAllListener();
    }
};

void TestEvents()
{
    TestEventsClass c;

    
}