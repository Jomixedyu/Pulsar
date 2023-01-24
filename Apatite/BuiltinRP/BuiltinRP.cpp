#include "BuiltinRP.h"

static const char* vert = R"_(

)_";

namespace apatite::builtinrp
{
    BultinRP::BultinRP()
    {
        this->default_lit_ = mksptr(new Material);
        this->default_lit_->Construct();
        this->default_lit_->set_name("BuiltinRP/DefaultLit");

        //this->default_lit_->set_shader();
    }
}