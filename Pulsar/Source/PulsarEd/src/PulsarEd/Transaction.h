#pragma once
#include <CoreLib/Object.h>

class Transaction
{
public:
    virtual void Do() {}
    virtual void Undo() {}
    virtual ~Transaction() = default;
};
class Transactor
{
public:
    void Begin();
    void End();
};
class PropertyChangeTransaction : public Transaction
{
public:
    void Begin(Transactor* t, jxcorlib::Object_wp obj)
    {

    }
    void End()
    {

    }
};

void A()
{
    PropertyChangeTransaction t;


}


