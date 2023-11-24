# jxcorlib

![](https://img.shields.io/github/license/JomiXedYu/jxcorlib?style=for-the-badge)
![](https://img.shields.io/github/v/release/JomiXedYu/jxcorlib?style=for-the-badge)
![](https://img.shields.io/github/release-date/JomiXedYu/jxcorlib?style=for-the-badge)
![](https://img.shields.io/badge/StdVer-C++20-blueviolet.svg?style=for-the-badge&logo=c%2B%2B)


- CoreLib总共分为4各部分：
  - CoreLib: 提供类型系统、运行时反射、值装拆箱、字符串工具，委托与事件模板等基础设施。
  - Serialization: 序列化系统，支持Json，二进制，Stream与读写等功能。
  - Platform: 提供操作系统平台的抽象API，如文件、进程、对话框等工具。目前暂支持Windows系统。
  - Math: 3D数学库，提供向量、矩阵类型模板与常见运算，提供装箱类型并支持序列化。

   

- [jxcorlib](#jxcodecorelib)
  - [使用本基本库与工具的规范](#使用本基本库与工具的规范)
  - [命名规范](#命名规范)
  - [万物基于Object](#万物基于object)
  - [装箱与拆箱](#装箱与拆箱)
  - [基元类型](#基元类型)
  - [String字符串](#string字符串)
    - [String与Char](#string与char)
    - [索引与访问](#索引与访问)
    - [编码转换](#编码转换)
    - [字符串工具类](#字符串工具类)
  - [类型元数据](#类型元数据)
  - [程序集](#程序集)
  - [类型定义](#类型定义)
    - [普通类型定义](#普通类型定义)
    - [模板类型定义](#模板类型定义)
    - [接口类型定义](#接口类型定义)
  - [反射系统](#反射系统)
    - [反射工厂动态创建对象](#反射工厂动态创建对象)
    - [参数包与变长验证模板函数](#参数包与变长验证模板函数)
    - [字段反射](#字段反射)
    - [方法反射](#方法反射)
  - [反射扩展](#反射扩展)
    - [Json序列化](#json序列化)
  - [强类型枚举位运算的支持](#强类型枚举位运算的支持)
  - [属性模板](#属性模板)
  - [事件发送器与委托](#事件发送器与委托)
    - [事件类](#事件类)
    - [添加与移除](#添加与移除)
    - [静态函数](#静态函数)
    - [Lambda](#lambda)
    - [成员函数](#成员函数)
    - [执行](#执行)
  - [异常类](#异常类)
  - [调试工具](#调试工具)

## 使用本基本库与工具的规范
* 所有类型只能使用单继承，但是可以继承纯虚类（接口）
* 类型继承与接口实现总是public继承
* 项目应采用Unicode字符集，使用UTF8编码来编译字符串
* struct实例应为值类型，class实例应在堆中分配。

**本规范非常重要，需强制执行，如不遵守规范可能会导致编译错误或异常结果。**

## 命名规范
* 成员方法使用下划线命名法，并以下划线结尾，如 `list_`
* 局部变量使用下划线命名法，如`all_item`
* 属性方法使用`get_field()`与`set_field()`命名
* 类名与方法名使用Pascal规则，如`class Renderer`、`void ShutDown()`
* 接口以大写I大写开头，如`ICompare`

## 万物基于Object
类型系统体系中，所有类型都应该单继承基于Object的类型，Object在绝大多数下应使用`sptr`来管理生命周期，`sptr`目前为`std::shared_ptr`的别名，可使用mksptr来新建一个对象，如：

```c++
sptr<Object> obj = mksptr(new Object());
```
如果在类型声明后使用`CORELIB_DECL_SHORTSPTR(Class)`宏，将会自动新增两个别名:

```c++
using Object_sp = sptr<Object>;
using Object_rsp = const Object_sp&;
```
这两个别名可以很好的在一些地方省去手敲sptr模板的时间，另外，模板应使用`CORELIB_DECL_TEMP_SHORTSPTR(Class)`宏来新增别名。

`Object`类型定义了四个虚函数
- Equals
- ToString
- GetType
- ~Object

其中GetType函数，若在用户不清楚类型系统如何运作时，始终不应该由用户重写。

## 装箱与拆箱
在该库中，如果想将值类型放到类型系统中去，应该为该值类型定义一个class并继承Object体系中的类型，如：
```c++
struct Value {};
class BoxingValue : public Object, public Value {
    using unboxing_type = Value;
};
template<> get_boxing_type<Value> { using type = BoxingValue; }
```
`Value`是值类型，`BoxingValue`是`Value`在类型系统中的装箱版本。在反射与序列化等功能上，使用统一的Object基类对象进行操作。

类库提供了显式装拆箱的工具：
```c++
Value v;
sptr<BoxingValue> bvalue = static_pointer_cast<BoxingValue>( BoxUtil::Box(v) ); //boxing
//or
/* sptr<BoxingValue> bvalue = mkbox(v) ); */ //boxing

Value ubvalue = UnboxUtil::Unbox<Value>(bvalue); //unboxing
```

如果想获取一个类型的值类型，可以使用`get_boxing_type<T>::type`来获得，每个值类型都应为装箱类型编写该模板的特化。

## 基元类型
在该类库中，将以下类型定义为基元类型，他们并没有继承Object，但是他们的装箱类型继承自Object：

| 原类型   | 装箱类型   |
| -------- | ---------- |
| int8_t   | Integer8   |
| uint8_t  | UInteger8  |
| int16_t  | Integer16  |
| uint16_t | UInteger16 |
| int32_t  | Integer32  |
| uint32_t | UInteger32 |
| int64_t  | Integer64  |
| uint64_t | UInteger64 |
| float    | Single32   |
| double   | Double64   |
| bool     | Boolean    |
| string   | String     |


## String字符串
### String与Char
使用`#include <CoreLib/UString.h>`引入  
```c++
string s("a word 一个字");
```
字符串使用了别名引用，它的原型为
```c++
using string = std::string;
```
由此可见string并不继承Object，这也是为了能和其他使用标准库的类库与工具可以同时使用。  
项目应采用的所有字符串都应该是UTF8的，可以使用StringUtil来查询UTF8字符串长度，索引字符，编码转换。  
因为UTF8是不定长的字符编码，所以在处理字符时采用的Char是一个八字节大小的类型。  
```c++
struct u8char
{
    char value[8]{ 0 };
    //...
};
```

### 索引与访问
使用工具类去索引一个UTF8字符
```c++
u8char c = StringUtil::CharAt(s, 9);
```
但是当字符串特别大时，并对这个字符串的随机访问次数多时，直接使用这个方法会特别的慢，
为解决UTF8的索引和随机访问慢的问题，采用字符串分块位置映射的空间换时间方式来提升速度。  
```c++
StringIndexMapping mapping(s, 2); // use cache
u8char c2 = StringUtil::CharAt(s, 9, mapping);
```
构造函数原型
```
StringIndexMapping(const string& str, size_t block_size);
```
第一个参数是字符串引用，第二个参数是块的大小：  
- 块越大，映射数据少，空间开销小，索引速度慢。
- 块越小，映射数据多，空间开销大，索引速度快。


### 编码转换
因为项目规范使用Unicode字符集，并且以UTF8以基础字符串，所以编码转换仅提供UTF8与UTF16的互相转换。
```c++
static std::u16string Utf8ToUtf16(const string& str);
static String Utf16ToUtf8(const std::u16string& str);
```

### 字符串工具类
StringUtil类中有常用的`Replace`，`Concat`等函数，具体查看`String.h`中的`StingUtil`类

## 类型元数据
类型系统中的每个class类都有一个`Type`类型实例，Type类型也继承了Object，该类型实例可以保存class类型的字段、函数、类型具体信息如名字、父类型等内容。任意继承了Object的类型都可以使用GetType()函数来获取运行时的Type实例，可以通过该实例在运行时动态判断对象之间继承关系以及为后续反射提供相关功能。如：
```c++
String* str = new String;
Object* obj = str;

obj->GetType() == cltypeof<String>("str"); //ok

obj->GetType()->IsSubclassOf(cltypeof<Object>()); //ok

obj->GetType()->get_base() == cltypeof<Object>(); //ok

String* new_str = cltypeof<String>()->CreateInstance({"new str"}); //ok

```

在程序启动时为每一个class创建一个Type实例，并将它们注册到程序集中。可以使用cltypeof<T>()来获取类型T的Type实例，如果你对unreal熟悉的话可能更喜欢使用T::StaticType()来获取，但更建议前者。


## 程序集
应用程序以程序集`Assembly`构建，一个`Assembly`应代表着一个lib、dll或者exe，每一个Assembly实例内储存着该模块的所有Type实例，如果知道一个外部程序集中的某个类型，那么就可以动态的获取到Type并创建实例。

程序集需要使用`CORELIB_DECL_ASSEMBLY(Name)`宏来定义。如：`CORELIB_DECL_ASSEMBLY(jxcorlib)`，就声明了一个名为jxcorlib的程序集，同时产生一个程序集实例给予类型定义使用，它的名字是`AssemblyObject_程序集名字`，所以`jxcorlib`的类型定义用实例名为`AssemblyObject_jxcorlib`。

## 类型定义

### 普通类型定义
首先需要引入头文件`CoreLib/CoreLib.h`，然后进行类型声明：
```c++
namespace space
{
    class ExampleClass : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, space::ExampleClass, Object);
    public:

    };
}
```

定义类型在类中使用`CORELIB_DEF_TYPE(AssemblyObject, Class, Base)`宏
- 第一个参数使用了之前上一小节中定义的程序集实例，代表该类型将会注册进jxcorlib程序集中。
- 第二个类型参数必需为完整路径。  
- 第三个参数是继承的基类，不必使用完整名。

另外重申规范：继承Object体系类型总是public继承

### 模板类型定义
除了普通的类型定义外，模板类型使用的定义宏与一些细节是不一样的。  
一个模板类的声明：
```c++
template<typename T>
class TemplateClass : public Object
{
    CORELIB_DEF_TEMPLATE_TYPE(AssemblyObject_jxcorlib, TemplateClass, Object, T);
public:

};
```
在普通的类型中使用`CORELIB_DEF_TYPE`去定义元数据，而模板类则使用`CORELIB_DEF_TEMPLATE_TYPE`来定义。  
类型定义的后面是一个变长列表，依次按照模板顺序添加。  

关于模板类型获取Type名字：
当获取模板类型`Type*`的`get_name()`时，这个名字并不会像普通类型固定，而是会到编译器的影响。  
如`TemplateClass<int>`类型，在msvc下，它的名字是`TemplateClass<int>`，而在gcc下则是`TemplateClass<i>`。  
模板类中的名字取决于类型的`std::type_info`中的`name()`。  
综上所述，因为编译器实现的不同，模板类的反射工厂无法通用。

### 接口类型定义
使用`CORELIB_DEF_INTERFACE(AssemblyObject, Name, Base)`宏来定义接口，接口必须继承IInterface，并以I大写开头，

```c++
class IList : public IInterface
{
    CORELIB_DEF_INTERFACE(AssemblyObject_jxcorlib, jxcorlib::IList, IInterface);

    virtual void Add(const sptr<Object>& value) = 0;
    virtual sptr<Object> At(int32_t index) = 0;
    virtual void Clear() = 0;
    virtual void RemoveAt(int32_t index) = 0;
    virtual int32_t IndexOf(const sptr<Object>& value) = 0;
    virtual bool Contains(const sptr<Object>& value) = 0;
    virtual int32_t GetCount() const = 0;
    virtual Type* GetIListElementType() const = 0;
};
```
这里直接拿出了IList接口的代码，在该宏后面的代码默认都是为public权限的，所以可以不用再次声明。

在接口实现时需要另外使用`CORELIB_IMPL_INTERFACES(...)`宏来将所有实现的接口类型写进，使用逗号分割。

```c++
template<typename T>
class List : public Object, public array_list<T>, public IList, public ICopy
{
    CORELIB_DEF_TEMPLATE_TYPE(AssemblyObject_jxcorlib, jxcorlib::List, Object, T);
    CORELIB_IMPL_INTERFACES(IList, ICopy);
    //implemented...
}
```

当需要将实例转换为接口实例时，使用`interface_cast<T>(Object*)`或`interface_shared_cast<T>(Object_rsp)`来转换，如：

```c++
sptr<List<int>> list = mksptr(new List<int>);

IList* ilist = interface_cast<IList>(list.get()); //ok

sptr<IList> silist = interface_shared_cast<IList>(list); //ok
```

如转换为裸指针则需要注意尽量不要保存等操作，以免指针悬垂。


## 反射系统
### 反射工厂动态创建对象
首先声明一个带构造函数的类型，并用`CORELIB_DEF_TYPE`和`CORELIB_DECL_DYNCINST`宏声明元数据和反射的工厂函数。
```c++
namespace space
{
    class DynCreateClass : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, space::DynCreateClass, Object);
        CORELIB_DECL_DYNCINST() {
            return new DynCreateClass(0);
        }
    private:
        int id;
    public:
        DynCreateClass(int id) : id(id) {}
    };
}
```
`CORELIB_DEF_TYPE`会根据以下顺序进行函数：  
- 自动查找反射工厂的函数，如果有则绑定
- 自动查找是否有无参构造函数，如果有则绑定
- 绑定失败，如果创建则会抛出。  

可以使用`CORELIB_DECL_DYNCINST`来自定义实现实现反射工厂函数体。  
或者直接使用`CORELIB_DECL_DYNCINST`原型
```c++
static Object* DynCreateInstance(const ParameterPackage& params)
```

可以使用类名来获取Type对象，使用`CreateInstance`创建
```c++
Type* dyn_type = Assembly::StaticFindAssembly(AssemblyObject_jxcorlib)->FindType("space::DynCreateClass");
Object* dyn = dyn_type->CreateInstance({});
```

### 参数包与变长验证模板函数
`ParameterPackage`是用一个any数组的封装类，可以从外部向ParameterPackage对象添加参数，在传入工厂函数内。
```c++
Type* dyn_type = Assembly::StaticFindAssembly(AssemblyObject_jxcorlib)->FindType("space::DynCreateClass");
Object* dyn = dyn_type->CreateInstance(ParameterPackage{ 20 });
```
之后`CreateInstance`将会调用对应类型的工厂函数。  
这里需要注意的是，即使外部并没有传入参数包，这里依然会得到一个空参数包的引用。  
在使用外部传入的参数包时，可以使用`IsEmpty()`或者`Count()`进行简单的验证，
也可以使用可变长参数模板来对参数类型进行验证：
```c++
if(!params.Check<int>()) {
    return /*...*/;
}
if(!params.Check<int, float>()) {
    return /*...*/;
}
if(!params.Check<int, float, String>()) {
    return /*...*/;
}
```
使用Get按索引获取指定类型的值：
```c++
int p1 = params.Get<int>(0);
```
如果索引值不在正确的范围内，std::vector将会抛出错误，所以总应该在函数最开始的地方对传入的数据进行验证。



### 字段反射
字段反射定义宏：实例对象成员字段的声明，现已不在支持静态字段的反射。
```c++
#define CORELIB_REFL_DECL_FIELD(NAME)
```

样例类：
```c++
class DataModel : public Object
{
    CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, DataModel, Object);
public:

    CORELIB_REFL_DECL_FIELD(id);
    const int id = 0;

    CORELIB_REFL_DECL_FIELD(is_human);
    bool is_human = true;

    COERLIB_REFL_DECL_FIELD(name);
    sptr<Object> name;
};
```


字段的反射信息存在类型`Type`中，使用`get_fieldinfo(string&)`来获取一个`FieldInfo*`。  
```c++
    //field reflection
    DataModel* model = new DataModel;

    Type* model_type = cltypeof<DataModel>();

    FieldInfo* id_field = model_type->get_fieldinfo("id");
    assert(id_field->is_public() == true);
    assert(id_field->is_pointer() == false);
    assert(id_field->get_name() == "id");

    id_field->SetValue(model, BoxUtil::Box(3)); //boxing

    Object_sp id_value = id_field->GetValue(model);
    assert(id_value->GetType() == cltypeof<int>());
    assert(UnboxUtil::Unbox<int>(id_value) == 3); //unboxing
```
使用GetValue和SetValue获取和设置值。如果字段为静态，实例指针传入nullptr即可。

### 方法反射
TODO

## 反射扩展
### Json序列化
json库来自于`nlohmann`，序列化使用`CoreLib.Extension`中的`JsonSerializer`。  
首先引入头文件`CoreLib.Extension`，在`JsonSerializer`中主要有两个静态方法：
```c++
static string Serialize(Object* obj);
static sptr<Object> Deserialize(const string& jstr, Type* type);
```
另外Deserialize还有一个模板版本
```c++
template<typename T>
static sptr<T> Deserialize(const string& str);
```
先声明两个可反射的类型
```c++
class PersonInfo : public Object
{
    CORELIB_DEF_TYPE(PersonInfo, Object);
public:
    CORELIB_REFL_DECL_FIELD(name);
    string name;
    CORELIB_REFL_DECL_FIELD(age);
    int age;
    virtual string ToString() const override
    {
        return std::format("name: {}, age: {}", name, age);
    }
};

class StudentInfo : public Object
{
    CORELIB_DEF_TYPE(StudentInfo, Object);
public:

    CORELIB_REFL_DECL_FIELD(id);
    int id;
    CORELIB_REFL_DECL_FIELD(president);
    bool president;
    CORELIB_REFL_DECL_FIELD(person_info);
    sptr<PersonInfo> person_info;
    CORELIB_REFL_DECL_FIELD(score);
    List_sp<int> score;

    virtual string ToString() const override
    {
        return std::format("id: {}, level: {}, person_info: {{{}}}, score: {}", id, level, person_info->ToString(), jxcvt::to_string(*score));
    }
};
```
定义`StudentInfo`对象并赋值：
```c++
StudentInfo* student = new StudentInfo;
student->id = 33;
student->level = true;
student->score = mksptr(new List<int>());
student->score->push_back(3);
student->score->push_back(4);

student->person_info = mksptr(new PersonInfo);
student->person_info->name = "jx";
student->person_info->age = 12;
```
随后调用序列化
```c++
string json_str = JsonSerializer::Serialize(student)
```
或者反序列化
```c++
sptr<StudentInfo> newstudent = JsonSerializer::Deserialize<StudentInfo>(json_str);
```

## 强类型枚举位运算的支持
通过导入`CoreLib/EnumUtil.h`即可访问，在枚举定义后使用`ENUM_CLASS_FLAGS(Enum)`宏进行运算符重载定义。

## 属性模板
属性是一种以类访问字段的方式来执行方法，主要使用括号重载operator()和类型转换operator T来实现。  
类型声明：  
```c++
#include "../CoreLib/Property.h"
class PropertyClass
{
private:
    int i_;
public:
    Property<int> i{
        PROP_GET(int) {
            return this->i_;
        },
        PROP_SET(int) {
            this->i_ = value;
        }
    };
};
```
直接使用
```c++
void TestProperty()
{
    PropertyClass c;
    
    c.i = 3;
    int num = c.i;
}
```

## 事件发送器与委托
### 事件类
- Events作为模板基类，提供回调函数的添加移除。  
  - Delegate是Events的派生类，提供更多的控制权，可以移除全部事件或者执行全部事件。  
- ActionEvents是Events的一个无返回值偏特化版本。  
  - Action是Delegate的一个无返回值的偏特化版本。  
- FunctionEvents是Events的一个别名。
  - Function是Delegate的派生类，除了Delegate的权限和执行能力之外还拥有返回所有回调执行返回的结果集的功能。 
  - Function<bool>是一个特化版本，还增加了返回结果集中是否存在false的功能，主要用于关闭询问等功能。

### 添加与移除
支持添加：
- 静态函数
- lambda静态函数
- 实例成员函数
- lambda捕获函数
拿Action来举例：
```c++
Action<> e;
```

### 静态函数
普通静态函数支持两种方法的添加：
```c++
e += static_func;
e.AddListener(static_func);
```
与之对应：
```c++
e -= static_func;
e.RemoveListener(static_func);
```

### Lambda
lambda也可以使用+=与AddListene进行添加，但由于lambda没有名字，没办法移除，所以需要使用返回的索引来进行移除。
```c++
int index = e += ([](){});
e.RemoveListenerByIndex(index);
```
另外，带捕获的lambda可以选择传入一个实例，这样就可以通过实例去移除。
```c++
c.AddListener(this, [this](){});
c.RemoveListenerByInstance(this);
```

### 成员函数
成员函数需要使用实例和成员函数地址。  
成员函数也可以使用按实例移除的方式来移除：
```c++
e.AddListener(this, &TestClass::MemFunc);
e.RemoveListener(this, &TestClass::MemFunc);
e.RemoveListenerByInstance(this);
```

### 执行
```c++
e.Invoke();
```

## 异常类
类库内内置了以下基本异常类，位置在`CommonException.h`
- ExceptionBase
  - RangeOutException
  - ArgumentException
    - ArgumentNullException
  - NotImplementException
  - NullPointerException

其中作为类库中异常类的基类`ExceptionBase`是一个多继承的类
```c++
class ExceptionBase : public std::exception, public Object
```
这是为了保证可以使用统一的`std::exception`来进行捕获，还可以使用Object的特性。

## 调试工具
引入DebugTool.h即可使用 (c++20)
```c++
#define DEBUG_INFO(info) std::format("info: {}; line: {}, file: {};", info, __LINE__, __FILE__);
```
