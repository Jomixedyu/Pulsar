# Pulsar Engine — Agent 指南

> 本文件面向 AI 编码助手编写。如果你正在阅读本文，应当假定自己对该项目的了解仅限于此处所写的内容。请勿基于其他引擎架构做任何假设。

---

## 项目概述

Pulsar Engine 是一款自定义 C++ 游戏引擎，目标平台为 Windows 64 位，使用 Vulkan 渲染后端。它采用基于包的模块系统（精神上类似于 Unreal Engine 的插件/模块）和实体-组件世界架构（类似于 Unity 的 GameObject/Component 模型）。

代码库最初由中文作者编写（`jxcorlib` 的 README 是中文的），但所有源代码标识符、注释和文档均使用英文。

关键特性：
- **语言**: C++26
- **平台**: 仅 Windows 64 位
- **图形 API**: Vulkan（通过 `gfx-vk` 模块）
- **构建系统**: CMake 3.23+，Ninja 生成器
- **编译器**: MSVC（Visual Studio 2022）

---

## 仓库结构

```
Pulsar/
├── CMakeLists.txt                 # 根目录 CMake：自动发现 Packages/ 下的包
├── Packages/
│   ├── Pulsar/                    # 核心引擎 STATIC 库
│   │   ├── Source/src/Pulsar/     # 引擎源代码
│   │   └── Source/third/          # 引擎第三方依赖
│   ├── PulsarEd/                  # 编辑器 OBJECT 库 + PulsarEd.exe
│   │   ├── Source/src/PulsarEd/   # 编辑器源代码
│   │   └── Source/third/          # 编辑器专用第三方依赖
│   ├── Project/                   # 用户游戏代码（OBJECT 库）
│   │   └── Source/Gameplay/       # 游戏专属 C++ 类
│   ├── ProjectEd/                 # 游戏专属编辑器可执行文件
│   │   └── Source/main.cpp        # 入口点，链接 Project + PulsarEd
│   └── UnrealSceneImporter/       # 独立导入器库
├── BuildNinja/                    # Debug 构建输出（生成）
├── BuildNinjaRelease/             # Release 构建输出（生成）
├── cmake-build-debug-visual-studio/ # Visual Studio CMake 生成文件
├── Tools/                         # Python/batch 工具
├── Temp/                          # 运行时临时文件（日志、着色器缓存、缩略图）
└── *.bat                          # 构建和运行辅助脚本
```

### 重要：`.gitignore` 行为
- `Packages/Project/` 被 git 忽略 —— 这是故意的。`Project` 包包含用户游戏代码，属于本地/工作区内容。
- `BuildNinja*/`、`cmake-build-*`、`out/`、`.vs/` 被忽略。

---

## 技术栈

### 核心引擎（`Pulsar` 包）
| 系统 | 实现 |
|------|----------------|
| 反射 / 类型系统 | `jxcorlib`（自定义 CoreLib） |
| 渲染器抽象 | `gfx`（静态库） |
| Vulkan 后端 | `gfx-vk`（静态库，链接 Vulkan SDK + SDL2） |
| 窗口 / 输入 | SDL2 (`SDL2lib`) |
| 2D 物理 | Box2D (`box2d`) |
| 3D 物理 | Jolt Physics (`Jolt`) |
| 音频 | `jaudio`（自定义封装） |
| 内存分配器 | `mimalloc`（静态，全局覆盖） |
| 纹理处理 | DirectXTex, ASTC encoder |
| 数学库 | CoreLib.Math + `EngineMath.h` |
| UI 运行时 | Dear ImGui (`imgui`, `imext`, `jximgui`) |
| 输入抽象 | `uinput`（自定义） |
| MikkTSpace | `mikktspace` |

### 编辑器专用（`PulsarEd` 包）
| 系统 | 实现 |
|------|----------------|
| 资源导入 (FBX) | Autodesk FBX SDK (`fbxsdk`) |
| 资源导入（通用） | Open Asset Import Library (`assimp`) |
| 着色器编译 | `psc`（包含 glslang + SPIRV-Cross） |

---

## 构建系统

### 前置条件
- Windows 10/11 x64
- Visual Studio 2022（含 C++ 桌面工作负载）
- Ninja（随 VS 或 CMake 捆绑）
- Vulkan SDK（用于 `gfx-vk`）

### 首次设置
运行 `Setup.py` 解压压缩的第三方库（例如 FBX SDK 库）：
```batch
python Setup.py
```

### 构建命令

**生成 Debug 构建文件（Ninja）：**
```batch
regen-cmake-ninja.bat
```

**生成 Release 构建文件（Ninja）：**
```batch
regen-cmake-ninja-release.bat
```

**构建编辑器（`PulsarEd.exe`）：**
```batch
build-debug-pulsared.bat
build-release-pulsared.bat
```

**构建项目编辑器（`ProjectEd.exe`）：**
```batch
build-debug-projected.bat
build-release-projected.bat
```

**构建并运行编辑器：**
```batch
run-debug-editor.bat
run-release-editor.bat
```

### 构建输出
- 可执行文件：`<BuildDir>/bin/`
- 库文件：`<BuildDir>/lib/`
- 头文件：`<BuildDir>/include/`

### CMake 包自动发现
根目录的 `CMakeLists.txt` 会自动添加 `Packages/` 下任何包含 `CMakeLists.txt` 的子目录。添加新包时**无需**编辑根 CMake 文件。

---

## 架构

### 对象模型
所有引擎对象都派生自 `pulsar::ObjectBase`（它派生自 `jxcorlib::Object`）。引擎使用基于句柄的对象系统：
- `ObjectHandle` —— 每个引擎对象的 64 位整数句柄。
- `ObjectPtr<T>` —— 智能指针包装器，通过 `RuntimeObjectManager` 解析。
- `RuntimeObjectManager` —— 中央注册表，映射句柄到存活对象，提供垃圾回收和观察者/依赖通知。

关键宏：
- `DECL_PTR(Class)` —— 生成 `Class_sp` / `Class_rsp` 别名。
- `CORELIB_DEF_TYPE(Assembly, Namespace::Class, BaseClass)` —— 注册类型以支持反射。
- `CORELIB_REFL_DECL_FIELD(Name)` —— 声明一个反射字段。

完整的反射系统原理和使用方法，见下方**反射与类型系统（jxcorlib）**章节。

### World / Scene / Component
- `World` —— 拥有场景、渲染对象、物理世界、子系统和选择集。
- `NodeCollection` —— 场景资源（相当于 `.scene` 文件）。
- `Node` —— 世界中的一个实体。
- `Component` —— 所有附加到节点的组件的基类。
- 组件位于 `Packages/Pulsar/Source/src/Pulsar/Components/`（例如 `TransformComponent`、`CameraComponent`、`StaticMeshRendererComponent`、`InputComponent`）。

### Assets（资源）
- 基类：`AssetObject`
- 资源路径使用虚拟路径方案：`PackageName/Folder/AssetName`
- 编辑器维护一个 `AssetDatabase`，负责路径 ↔ GUID 的映射，并管理脏状态、保存和包搜索路径。
- 资源通过 `AssetManager`（运行时）或 `AssetDatabase`（编辑器）加载。

### Rendering（渲染）
- `gfx::GFXApplication` / `gfx::GFXRenderPipeline` —— 抽象层。
- `EngineRenderPipeline` —— 引擎专用管线，负责渲染一个或多个 `World`。
- 渲染对象实现 `rendering::RenderObject` 并在 `World` 中注册。
- 着色器通过 `IShaderCompileService` 编译；编辑器使用 `EditorShaderCompileService` 配合 `psc` 着色器编译栈实现该接口。
- 编辑器还配有 `ShaderHotReloadWatcher` 以实现着色器热重载。

### Editor Architecture（编辑器架构）
- `EditorAppInstance` —— 在编辑器模式下替换 `EngineAppInstance`；承载基于 ImGui 的 UI。
- 特定资源类型的编辑器位于 `PulsarEd/Source/src/PulsarEd/Editors/`（例如 `SceneEditor`、`MaterialEditor`、`TextureEditor`）。
- 导入器位于 `PulsarEd/Source/src/PulsarEd/Importers/`（`AssimpImporter`、`FBXImporter`、`ImageImporter`）。
- 编辑器使用模态对话框、独占任务队列和基于文件树的资源浏览器。

---

## 反射与类型系统（jxcorlib）

Pulsar 使用第三方库 `jxcorlib` 提供完整的 C++ 反射能力。该系统在编译期通过宏注入元数据，运行期通过全局 `Type` 对象查询。理解这一层对编写 Component、Asset 和任何引擎扩展类至关重要。

### 核心类与概念

| 类 | 作用 |
|---|---|
| `jxcorlib::Object` | 所有反射类型的根基类；继承 `std::enable_shared_from_this<Object>`。 |
| `jxcorlib::Type` | 类型的运行时表示（元数据），包含基类、字段、方法、接口、属性。 |
| `jxcorlib::MemberInfo` | `FieldInfo` / `MethodInfo` 的基类；含名称、可见性、所属类型、排序。 |
| `jxcorlib::FieldInfo` | 反射字段描述；含类型、getter/setter、指针/const 标志。 |
| `jxcorlib::MethodInfo` | 反射方法描述；含参数列表、返回值、静态/抽象标志。 |
| `jxcorlib::Attribute` | 属性标记基类；可附加到类型或字段上（如 `HidePropertyAttribute`）。 |
| `jxcorlib::Assembly` | 程序集；每个包（如 `pulsar`）有自己的 Assembly，用于归类类型。 |

### 类型注册宏详解

#### `CORELIB_DEF_TYPE(ASSEMBLY, Namespace::Class, BaseClass)`
在类定义的 `public` 区域放置，完成以下工作：
1. 生成 `static Type* StaticType()`：首次调用时构造 `Type` 对象并注册到 Assembly。
2. 定义 `using base = BaseClass; using ThisClass = Class;`。
3. 生成 `virtual Type* GetType() const` 覆盖。
4. 添加一个静态内部结构体 `__corelib_type`，利用 C++ 静态初始化确保类型在程序启动时自动注册。
5. 提供 `self()` / `self_weak()` 快捷方法（返回 `SPtr<ThisClass>` / `WPtr<ThisClass>`）。
6. **静态断言**：`Class` 必须确实是 `BaseClass` 的派生类。

**使用位置**：必须放在类体的 `public:` 区域中，通常是第一条语句。

```cpp
class MyComponent : public Component
{
    CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MyComponent, Component);
    // ...
};
```

#### `CORELIB_DEF_ENUMTYPE(ASSEMBLY, EnumClass, Base)`
用于反射枚举类型。枚举类需要额外提供一个静态方法 `StaticGetDefinitions()` 返回键值对列表。

#### `CORELIB_DEF_INTERFACE(ASSEMBLY, Namespace::Interface, Base)` / `CORELIB_IMPL_INTERFACES(...)`
用于纯虚接口。接口类型不持有工厂函数（`CreateInstFunc` 为 `nullptr`），仅用于 `IsImplementedInterface()` 查询。`CORELIB_IMPL_INTERFACES(IStringify)` 在类的 `CORELIB_DEF_TYPE` 之后声明该类实现了哪些接口。

```cpp
class AssetObject : public ObjectBase
{
    CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AssetObject, ObjectBase);
    CORELIB_IMPL_INTERFACES(IStringify)
    // ...
};
```

### 反射字段声明

#### `CORELIB_REFL_DECL_FIELD(NAME, ...)`
在类体中（字段变量声明的上方）放置，为成员变量 `NAME` 生成反射元数据。

**原理**：该宏展开为一个匿名结构体的静态实例。结构体的构造函数在程序启动时执行，调用 `ReflectionBuilder::CreateFieldInfo`，通过 lambda 捕获字段地址，生成运行时的 `FieldInfo`。

**支持的字段类型**：
- 基本值类型（`int`, `float`, `bool`, `Vector3f`, `guid_t` 等）—— 会被自动装箱（Boxing）为对应的 `BoxingObject` 子类。
- 引擎对象指针（`SPtr<T>` / `ObjectPtr<T>` / `RCPtr<T>`）—— 直接透传。
- 不支持原生裸指针（`T*`）作为反射字段的常规用法；若需要，应使用智能指针或句柄包装。

**字段属性（Attribute）**：可在宏参数中附加若干 `Attribute*`，控制编辑器表现：

| Attribute | 作用 |
|---|---|
| `new HidePropertyAttribute` | 在编辑器 Inspector 中隐藏该字段。 |
| `new ReadOnlyPropertyAttribute` | 在 Inspector 中只读显示。 |
| `new NoSerializableAttribtue` | 序列化时跳过该字段。 |
| `new DebugPropertyAttribute` | 仅在 Debug 构建或特定调试模式下显示。 |
| `new RangePropertyAttribute(min, max)` | 对数值字段提供滑动条范围限制。 |
| `new PrecisionAttribute(n)` | 控制浮点字段在 Inspector 中显示的小数位数。 |

**示例**：
```cpp
class TransformComponent : public Component
{
    // ...
    CORELIB_REFL_DECL_FIELD(m_position);
    Vector3f m_position{};

    CORELIB_REFL_DECL_FIELD(m_localToWorldMatrix, new NoSerializableAttribtue, new DebugPropertyAttribute, new ReadOnlyPropertyAttribute);
    Matrix4f m_localToWorldMatrix;

    CORELIB_REFL_DECL_FIELD(m_children, new HidePropertyAttribute);
    List_sp<SceneObjectPtr<TransformComponent>> m_children{};
    // ...
};
```

### 类级别 Attribute

#### `CORELIB_CLASS_ATTR(...)`
在 `CORELIB_DEF_TYPE` 之后放置，为整个类附加 Attribute。常见用法：

| Attribute | 作用 |
|---|---|
| `new AssetIconAttribute("PulsarEd/Icons/xxx.png")` | 在编辑器资源浏览器中显示该 Asset 的图标。 |
| `new CreateAssetAttribute` / `new CreateAssetAttribute("menu/path")` | 标记该 Asset 可在编辑器中通过右键菜单新建。 |
| `new CategoryAttribute("Renderer")` | 为 Component 指定编辑器菜单分类。 |
| `new ComponentIconAttribute(ICON_FK_CUBE)` | 为 Component 指定编辑器中的图标。 |
| `new AbstractComponentAttribute` | 标记该 Component 为抽象类，不可直接附加到 Node。 |

**示例**：
```cpp
class StaticMeshRendererComponent : public RendererComponent
{
    CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::StaticMeshRendererComponent, RendererComponent);
    CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"), new ComponentIconAttribute(ICON_FK_CUBE));
    // ...
};
```

### 引擎对象句柄系统

#### 句柄与 `RuntimeObjectManager`
所有派生自 `ObjectBase` 的引擎对象在构造时都会由 `RuntimeObjectManager` 分配一个唯一的 `ObjectHandle`（64 位整数）。该句柄在对象生命周期内保持不变，即使对象被移动或序列化也能稳定标识。

```cpp
ObjectHandle h = obj->GetObjectHandle();
ObjectBase* raw = RuntimeObjectManager::GetObject(h);        // 不安全裸指针
SPtr<ObjectBase> sp = RuntimeObjectManager::GetSharedObject(h); // 安全 shared_ptr
```

`RuntimeObjectManager` 还提供：
- `TickGCollect()` —— 清理已销毁对象的句柄映射。
- `DestroyObject(handle, isForce)` —— 请求销毁对象。
- `ObjectHook` —— 全局事件，在对象创建/销毁时触发（调试用）。

### 智能指针体系

引擎中有三套智能指针/句柄包装器，分别服务于不同场景：

#### 1. `SPtr<T>` / `WPtr<T>`（`jxcorlib` 层）
标准 `std::shared_ptr<T>` / `std::weak_ptr<T>` 的别名。用于纯 C++ 生命周期管理，不经过 `RuntimeObjectManager`。

#### 2. `ObjectPtr<T>`（通用引擎对象句柄指针）
- 内部持有 `SPtr<ManagedPointer>`（句柄的弱引用包装）。
- 通过 `RuntimeObjectManager::GetPointer(handle)` 解析为实际对象指针。
- **特点**：允许在对象已销毁后安全地持有"空引用"，不会悬空崩溃；支持跨序列化稳定保存。
- 所有 `Component`、`Node` 之间的交叉引用优先使用 `ObjectPtr<T>`。

```cpp
ObjectPtr<Component> comp = ObjectPtr<Component>::UnsafeCreate(handle);
if (comp) { comp->BeginComponent(); }
```

#### 3. `SceneObjectPtr<T>`（场景对象专用指针）
- 继承自 `ObjectPtrBase` 的独立体系，但语义与 `ObjectPtr<T>` 一致。
- 可从 `ObjectPtr<T>` 隐式构造，也可转换回 `ObjectPtr<T>`。
- `TransformComponent` 的父子链使用 `SceneObjectPtr<TransformComponent>`。

#### 4. `RCPtr<T>`（资源引用计数指针）
- 专用于 `AssetObject` 及其派生类。
- 内部除了 `ManagedPointer` 外，还维护一个**引用计数器（Counter）**。
- 当引用计数归零时，自动调用 `RuntimeObjectManager::DestroyObject()` 销毁资产。
- 这是运行时资产加载/卸载的核心机制。

```cpp
RCPtr<Texture2D> tex = RuntimeAssetManager::GetLoadedAssetByGuid<Texture2D>(guid);
// tex 超出作用域且计数归零时，资产自动卸载
```

#### 5. `DECL_PTR(Class)` 宏
为类生成以下别名（放置在类定义的同一命名空间中）：
```cpp
Class_sp   // SPtr<Class>
Class_rsp  // const SPtr<Class>&
Class_wp   // WPtr<Class>
Class_ref  // ObjectPtr<Class>   (仅引擎层，由 DECL_PTR 生成)
```

```cpp
// 在类定义后使用
DECL_PTR(TransformComponent);
// 之后可直接写：
TransformComponent_sp obj = mksptr(new TransformComponent);
TransformComponent_ref ref = obj->self_ptr();
```

#### 6. `DECL_OBJECTPTR_SELF`
在类体中放置，生成一个 `self_ptr()` 方法，返回 `ObjectPtr<ThisClass>`：
```cpp
class Component : public SceneObject
{
    DECL_OBJECTPTR_SELF
    // 生成：ObjectPtr<ThisClass> self_ptr() const;
};
```

### 动态创建对象

#### `CORELIB_DECL_DYNCINST()` / `DynCreateInstance`
若一个类需要支持**通过类型信息运行时创建实例**（例如编辑器新建 Asset、反序列化），需在类中声明：
```cpp
class MyAsset : public AssetObject
{
    CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MyAsset, AssetObject);
    CORELIB_DECL_DYNCINST();  // 声明
public:
    static Object* DynCreateInstance(const ParameterPackage& params); // 定义
};
```
`ParameterPackage` 是一个轻量的 `std::any` 容器，可传递构造参数。`Type::CreateInstance()` / `CreateSharedInstance()` 会调用此函数。

### 使用示例：完整 Component 定义

```cpp
#pragma once
#include <Pulsar/Components/Component.h>

namespace pulsar
{
    class MyComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MyComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Gameplay"), new ComponentIconAttribute(ICON_FK_COG));
        DECL_OBJECTPTR_SELF

    public:
        void BeginComponent() override;
        void Tick(float dt) override;

    protected:
        // 反射字段：编辑器可见、可序列化
        CORELIB_REFL_DECL_FIELD(m_speed, new RangePropertyAttribute(0.0f, 100.0f));
        float m_speed = 10.0f;

        // 反射字段：编辑器可见但运行时由代码维护，不序列化
        CORELIB_REFL_DECL_FIELD(m_currentHealth, new NoSerializableAttribtue);
        float m_currentHealth = 100.0f;

        // 反射字段：对编辑器隐藏
        CORELIB_REFL_DECL_FIELD(m_internalState, new HidePropertyAttribute);
        int m_internalState = 0;

        // 反射字段：引用另一个场景对象
        CORELIB_REFL_DECL_FIELD(m_targetNode);
        ObjectPtr<Node> m_targetNode;
    };
    DECL_PTR(MyComponent);
}
```

### 运行时反射操作参考

```cpp
// 获取类型
Type* t = MyComponent::StaticType();
Type* t2 = obj->GetType();

// 检查继承关系
bool isComp = t->IsSubclassOf(Component::StaticType());
bool isInst = t->IsInstanceOfType(obj);

// 获取字段并读写
FieldInfo* fi = t->GetFieldInfo("m_speed");
SPtr<Object> boxedValue = fi->GetValue(obj);
fi->SetValue(obj, mksptr(new BoxingFloat(50.0f)));

// 遍历所有字段
for (FieldInfo* f : t->GetFieldInfos())
{
    string name = f->GetName();
    Type* fieldType = f->GetFieldType();
    bool isPtr = f->IsPointer();
}

// 获取 Attribute
if (auto attr = fi->GetAttribute<RangePropertyAttribute>())
{
    float min = attr->m_min;
    float max = attr->m_max;
}

// 类型安全转换
if (auto casted = ptr_cast<MyComponent>(obj)) { /* ... */ }
if (auto sp = sptr_cast<MyComponent>(obj_sp)) { /* ... */ }
```

### 基础类型与 Boxing 系统

`jxcorlib` 中所有反射字段的原始值类型都会被自动**装箱（Boxing）**为 `BoxingObject` 的派生类。反射字段的 `GetValue()` / `SetValue()` 操作的是装箱后的对象，需要通过 `UnboxUtil` 还原，或使用 `BoxUtil` / `mkbox` 构造。

#### 基础值类型的 Boxing 映射

| 原始类型 | Boxing 类型 | 头文件 |
|---|---|---|
| `bool` | `Boolean` | `<CoreLib/BasicTypes.h>` |
| `char` | `CharObject` | `<CoreLib/BasicTypes.h>` |
| `int8_t` / `uint8_t` | `Integer8` / `UInteger8` | `<CoreLib/BasicTypes.h>` |
| `int16_t` / `uint16_t` | `Integer16` / `UInteger16` | `<CoreLib/BasicTypes.h>` |
| `int32_t` / `uint32_t` | `Integer32` / `UInteger32` | `<CoreLib/BasicTypes.h>` |
| `int64_t` / `uint64_t` | `Integer64` / `UInteger64` | `<CoreLib/BasicTypes.h>` |
| `float` | `Single32` | `<CoreLib/BasicTypes.h>` |
| `double` | `Double64` | `<CoreLib/BasicTypes.h>` |
| `string` / `string_view` | `String` (`jxcorlib::String`) | `<CoreLib/BasicTypes.h>` |
| `guid_t` | `Guid` (`jxcorlib::Guid`) | `<CoreLib/Guid.h>` |

#### 数学类型的 Boxing 映射

| 原始类型 | Boxing 类型 | 头文件 |
|---|---|---|
| `Vector2f` | `BoxingVector2f` | `<CoreLib.Math/Math.h>` |
| `Vector3f` | `BoxingVector3f` | `<CoreLib.Math/Math.h>` |
| `Vector4f` | `BoxingVector4f` | `<CoreLib.Math/Math.h>` |
| `Quat4f` | `BoxingQuat4f` | `<CoreLib.Math/Math.h>` |
| `Matrix4f` | `BoxingMatrix4f` | `<CoreLib.Math/Math.h>` |
| `Color4f` | `BoxingColor4f` | `<CoreLib.Math/Math.h>` |

#### 引擎对象指针的 Boxing 映射

| 原始类型 | Boxing 类型 | 头文件 |
|---|---|---|
| `ObjectHandle` | `BoxingObjectHandle` | `<Pulsar/ObjectBase.h>` |
| `ObjectPtrBase` | `BoxingObjectPtrBase` | `<Pulsar/ObjectBase.h>` |
| `SceneObjectPtrBase` | `BoxingSceneObjectPtrBase` | `<Pulsar/SceneObject.h>` |
| `RCPtrBase` | `BoxingRCPtrBase` | `<Pulsar/AssetObject.h>` |

#### Boxing / Unboxing API

```cpp
// 将原始值装箱为 Object_sp
auto boxed = BoxUtil::Box(3.14f);              // -> SPtr<Object> (实际为 Single32)
auto boxed2 = mkbox(Vector3f(1, 2, 3));       // -> SPtr<BoxingVector3f>
auto boxed3 = mksptr(new Single32(42.0f));    // 直接构造

// 从 Object_sp 还原原始值
float f = UnboxUtil::Unbox<float>(boxed);
Vector3f v = UnboxUtil::Unbox<Vector3f>(boxed2);

// 字段反射读写时自动 boxing/unboxing
FieldInfo* fi = type->GetFieldInfo("m_speed");
fi->SetValue(obj, mkbox(50.0f));              // float -> Single32 -> 字段
auto value = fi->GetValue(obj);
float current = UnboxUtil::Unbox<float>(value);
```

#### 自定义类型的 Boxing

如果需要在反射字段中使用自定义值类型，需要显式声明 Boxing 映射：

```cpp
// 假设自定义枚举
enum class MyEnum { A, B, C };
class BoxingMyEnum : public BoxingObject, public IStringify
{
    CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingMyEnum, BoxingObject);
    CORELIB_IMPL_INTERFACES(IStringify)
public:
    using unboxing_type = MyEnum;
    MyEnum value;
    MyEnum get_unboxing_value() { return value; }
    BoxingMyEnum(MyEnum v) : value(v) {}
    void IStringify_Parse(const string& s) override { /* ... */ }
    string IStringify_Stringify() override { /* ... */ }
};
CORELIB_DECL_BOXING(pulsar::MyEnum, pulsar::BoxingMyEnum);
```

### 序列化与 `IStringify`

`IStringify` 是 `jxcorlib` 提供的纯虚接口，用于自定义类型的字符串序列化/反序列化：

```cpp
class IStringify : public IInterface
{
    virtual void IStringify_Parse(const string& value) = 0;   // 字符串 -> 对象
    virtual string IStringify_Stringify() = 0;                // 对象 -> 字符串
};
```

实现了 `IStringify` 的 Boxing 类型（如 `BoxingVector3f`、`BoxingObjectHandle` 等）可以被 `JsonSerializer` 自动序列化为文本资产。引擎中所有纯文本资产（场景、材质、Prefab）的序列化都依赖这一层。

自定义值类型若需要支持纯文本序列化，需要在 Boxing 类上实现 `IStringify` 并调用 `CORELIB_IMPL_INTERFACES(IStringify)`。

---

## Code Style Guidelines（代码风格指南）

### Formatting（通过 `.clang-format` 强制执行）
- **基础风格**: Microsoft
- **缩进**: 4 个空格
- **命名空间缩进**: 所有命名空间都缩进
- **指针对齐**: 左对齐 (`Type* ptr`)
- **列限制**: 0（不强制换行）
- **短代码块**: 允许单行 (`if (x) return;`)
- **预处理指令**: 在 `#` 前缩进，4 个空格
- **访问修饰符**: 不缩进，相对于类缩进偏移 `-4`

### Naming conventions（命名规范，来自 jxcorlib / 项目约定）
- **Classes / structs**: PascalCase (`class World`, `struct InputContext`)
- **Methods / functions**: PascalCase (`void BeginPlay()`, `void Tick(float dt)`)
  - 历史原因，`jxcorlib` 中的成员方法有时会使用尾部下划线 snake_case（`list_`、`get_field_`），但引擎代码通常使用无尾部下划线的 PascalCase。
- **Properties**: `get_field()` / `set_field()` 模式
- **Interfaces**: `I` 前缀 (`IStringify`, `IShaderCompileService`)
- **局部变量**: snake_case (`all_item`, `dt`)
- **成员变量**: `m_camelCase` 或 `m_snake_case`，视文件而定；遵循现有文件约定并保持统一。
- **宏 / 常量**: `UPPER_SNAKE_CASE` (`DECL_PTR`, `OF_Transient`)

### Code conventions（代码约定）
- 反射类型仅支持单继承；仅对纯虚接口允许多继承。
- 继承始终是 `public`。
- 所有字符串使用 Unicode/UTF-8（强制使用 MSVC `/utf-8` 标志）。
- `struct` 实例应表现为值类型；`class` 实例应进行堆分配。
- 引擎对象生命周期优先使用 `ObjectPtr<T>` / `SPtr<T>`。
- 容器使用 `array_list<T>`（项目别名）或 `std::vector<T>`。

### Linting
- `.clang-tidy` 禁用了多个 `modernize-*` 检查和 `bugprone-reserved-identifier`。
- Tidy 仅供参考；项目不会因 tidy 警告而导致构建失败。

---

## Module Dependency Graph（模块依赖图）

```
ProjectEd.exe
    ├── Project (OBJECT)
    └── PulsarEdLib (OBJECT)
            └── PulsarLib (STATIC)
                    ├── PUBLIC: jxcorlib, gfx, gfx-vk, imgui, imext, jaudio, uinput, mikktspace
                    └── PRIVATE: DirectXTex, astc, box2d, SDL2lib, Jolt, mimalloc-static

PulsarEd.exe
    └── PulsarEdLib (same as above)
```

`PulsarLib` 在 MSVC 上作为静态库构建，并带有 `/WHOLEARCHIVE`，以防止链接时丢弃目标文件。

---

## Adding New Code（添加新代码）

### 添加新 Component（组件）
1. 在 `Packages/Pulsar/Source/src/Pulsar/Components/` 中创建 `.h` + `.cpp`。
2. 继承自 `Component`（或其更具体的子类）。
3. 在类体 `public:` 区域放置 `CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MyComp, Component);`。
4. 使用 `CORELIB_CLASS_ATTR` 附加编辑器分类/图标（可选但推荐）。
5. 在 `protected:` 区域为需要暴露的字段上方放置 `CORELIB_REFL_DECL_FIELD(m_field, ...)`。
6. 在类定义外部同一命名空间中使用 `DECL_PTR(MyComp);`。
7. 通过现有的 `GLOB_RECURSE` 将新文件纳入 CMake 构建（无需编辑 CMake）。

完整示例参考上方**反射与类型系统**章节的"使用示例：完整 Component 定义"。

### 添加新 Asset type（资源类型）
1. 在 `Packages/Pulsar/Source/src/Pulsar/Assets/` 中创建继承自 `AssetObject` 的类。
2. 放置 `CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MyAsset, AssetObject);`。
3. 添加 `CORELIB_CLASS_ATTR(new AssetIconAttribute("PulsarEd/Icons/xxx.png"))` 以支持编辑器图标。
4. 若需要在编辑器右键菜单中新建，添加 `new CreateAssetAttribute`。
5. 若支持反序列化/运行时创建，声明 `CORELIB_DECL_DYNCINST()` 并实现 `DynCreateInstance`。
6. 为需要序列化的字段添加 `CORELIB_REFL_DECL_FIELD`。
7. 在类外使用 `DECL_PTR(MyAsset);`。
8. 如果需要自定义编辑器，在 `Packages/PulsarEd/Source/src/PulsarEd/Editors/` 中创建并注册。

### 添加新 Editor（编辑器）
1. 在 `Packages/PulsarEd/Source/src/PulsarEd/Editors/` 中继承编辑器基类。
2. 在编辑器注册表中注册编辑器类型。

---

## Runtime and Data Directories（运行时和数据目录）

- **运行时工作目录**: 构建目录（`BuildNinja` 或 `BuildNinjaRelease`）。编辑器期望相对于此目录加载资源。
- **临时目录**: 项目根目录下的 `Temp/`（日志、着色器缓存、缩略图）。
- **资源包**: 运行时解析到 `Packages/` 下；`AssetDatabase::FileTreeRootPath` 为 `"Packages"`。
- **UI 配置**: `uiconfig.json` 存储编辑器窗口大小（默认 1280×720）。

---

## Testing（测试）

本仓库**没有专用的引擎测试套件**。第三方依赖有自己的测试，但在 CMake 中被禁用：
- `MI_BUILD_TESTS OFF`
- `ASSIMP_BUILD_TESTS OFF`
- `ASSIMP_BUILD_ASSIMP_TOOLS OFF`

如需添加测试，请将其放入新的 `Tests/` 包或子目录中，并通过 `enable_testing()` / `add_test()` 接入 CMake。

---

## Security and Safety Notes（安全与稳定性说明）

- 引擎使用 `mimalloc` 作为默认分配器，并已全局覆盖。
- 文件日志通过 `Logger::InitializeFileLogging(path)` 提供；它会立即刷新以保留崩溃日志。
- `assert_err` 和 `assert_warn` 宏已定义，但当前为空操作；请勿依赖它们进行安全检查。
- 项目在 Debug 模式下使用 `/MDd` 编译，Release 模式下推测使用 `/MD`（标准 MSVC 运行时）。

---

## Quick Reference（快速参考）

| 任务 | 命令 / 位置 |
|------|-------------------|
| 重新生成 Debug CMake | `regen-cmake-ninja.bat` |
| 构建编辑器 | `build-debug-pulsared.bat` |
| 构建并运行编辑器 | `run-debug-editor.bat` |
| 核心引擎源码 | `Packages/Pulsar/Source/src/Pulsar/` |
| 编辑器源码 | `Packages/PulsarEd/Source/src/PulsarEd/` |
| 用户游戏代码 | `Packages/Project/Source/` |
| 第三方依赖 | `Packages/Pulsar/Source/third/`、`Packages/PulsarEd/Source/third/` |
| 构建产物 | `BuildNinja/bin/`、`BuildNinjaRelease/bin/` |
| 资源路径根目录 | `Packages/` |
| 反射宏 | `CORELIB_DEF_TYPE`、`CORELIB_REFL_DECL_FIELD`、`DECL_PTR` |
| 引擎对象基类 | `pulsar::ObjectBase` |
| World / Scene | `pulsar::World`、`pulsar::NodeCollection` |
| 渲染后端 | `gfx-vk` (Vulkan) |
| 运行时类型获取 | `Type::StaticType()` / `obj->GetType()` |
| 类型安全转换 | `ptr_cast<T>(obj)` / `sptr_cast<T>(sp)` |
| 字段反射读写 | `FieldInfo::GetValue()` / `SetValue()` |
| 句柄解析对象 | `RuntimeObjectManager::GetObject(handle)` |
| 资产引用指针 | `RCPtr<T>` / `cast<T>(rcp)` |
| 场景对象指针 | `SceneObjectPtr<T>` / `ObjectPtr<T>` |
| 反射头文件 | `<CoreLib/Reflection.h>`、`<CoreLib/Type.h>`、`<CoreLib/Attribute.h>` |
| 引擎对象头文件 | `<Pulsar/ObjectBase.h>`、`<Pulsar/AssetObject.h>`、`<Pulsar/SceneObject.h>` |
