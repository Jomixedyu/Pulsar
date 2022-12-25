#include "BasicTypes.h"
#include "CommonException.h"
#include "Reflection.h"



namespace jxcorlib
{



    void ObjectUtil::DeepCopyObject(const sptr<Object>& from, sptr<Object>& to)
    {
        using namespace ::jxcorlib;

        if (from->GetType() != to->GetType())
        {
            throw ArgumentException();
        }
        Type* type = from->GetType();
        for (auto& field : type->get_fieldinfos())
        {
            Type* field_type = field->get_field_type();

            //create new object
            auto obj = field_type->CreateSharedInstance({});

            if (field_type->IsImplementedInterface(cltypeof<IList>()))
            {
                IList_sp fromlist = interface_sptr_cast<IList>(field->GetValue(from.get()));
                if (fromlist == nullptr) //null pointer
                {
                    continue;
                }
                IList* tolist = interface_cast<IList>(obj.get());

                Type* item_type = fromlist->GetIListElementType();
                int32_t len = fromlist->GetCount();

                for (int i = 0; i < len; i++)
                {
                    if (item_type->is_boxing_type())
                    {
                        tolist->Add(fromlist->At(i));
                    }
                    else
                    {
                        auto new_item = item_type->CreateSharedInstance({});
                        DeepCopyObject(fromlist->At(i), new_item);
                        tolist->Add(new_item);
                    }
                }
            }

            //assign
            field->SetValue(to.get(), obj);

        }
    }

    sptr<Object> ObjectUtil::DeepCopyObject(const sptr<Object>& from)
    {
        auto obj = from->GetType()->CreateSharedInstance({});
        DeepCopyObject(from, obj);
        return std::move(obj);
    }

}
