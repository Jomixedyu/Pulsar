#pragma once
#include <Pulsar/AssetObject.h>
#include "AnimationClip.h"

namespace pulsar
{
    // -----------------------------------------------------------------------
    // 参数类型
    // -----------------------------------------------------------------------
    enum class AnimatorParamType
    {
        Bool    = 0,
        Int     = 1,
        Float   = 2,
        Trigger = 3,   // 一次性触发，消费后自动重置
    };

    struct AnimatorParam
    {
        string            Name;
        AnimatorParamType Type        = AnimatorParamType::Bool;
        float             FloatValue  = 0.f;
        int               IntValue    = 0;
        bool              BoolValue   = false;
    };

    // -----------------------------------------------------------------------
    // 状态
    // -----------------------------------------------------------------------
    struct AnimatorState
    {
        string           Name;
        RCPtr<AnimationClip> Clip;
        float            Speed    = 1.f;
        bool             Loop     = true;
    };

    // -----------------------------------------------------------------------
    // 过渡条件
    // -----------------------------------------------------------------------
    enum class AnimatorConditionOp
    {
        Equals      = 0,   // bool/int/float == value
        NotEquals   = 1,
        Greater     = 2,   // float/int > value
        Less        = 3,
        Trigger     = 4,   // 参数为 Trigger 类型时直接触发
    };

    struct AnimatorCondition
    {
        string              ParamName;
        AnimatorConditionOp Op         = AnimatorConditionOp::Equals;
        float               Threshold  = 0.f;  // bool 用 1.f/0.f，int/float 用实际值
    };

    struct AnimatorTransition
    {
        string                       FromState;   // 空字符串 = AnyState
        string                       ToState;
        array_list<AnimatorCondition> Conditions;  // 所有条件 AND
        bool                         HasExitTime  = false;
        float                        ExitTime     = 1.f;   // 归一化时间 [0,1]，HasExitTime=true 时有效
        float                        Duration     = 0.1f;  // 过渡时长（秒），暂留，后续做 blend
    };

    // -----------------------------------------------------------------------
    // AnimatorController 资产
    // -----------------------------------------------------------------------
    class AnimatorController : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AnimatorController, AssetObject);
        CORELIB_CLASS_ATTR(
            new CreateAssetAttribute,
            new AssetIconAttribute("Editor/Icons/animatorcontroller.png")
            )
    public:

        static RCPtr<AnimatorController> StaticCreate(string_view name);

        // --- States ---
        void                          AddState(AnimatorState state);
        void                          RemoveState(const string& name);
        const AnimatorState*          FindState(const string& name) const;
        AnimatorState*                FindState(const string& name);
        const array_list<AnimatorState>& GetStates() const { return m_states; }

        // --- Transitions ---
        void                               AddTransition(AnimatorTransition transition);
        void                               RemoveTransition(int index);
        const array_list<AnimatorTransition>& GetTransitions() const { return m_transitions; }

        // --- Parameters ---
        void                            AddParam(AnimatorParam param);
        void                            RemoveParam(const string& name);
        const AnimatorParam*            FindParam(const string& name) const;
        AnimatorParam*                  FindParam(const string& name);
        const array_list<AnimatorParam>& GetParams() const { return m_params; }

        // --- Default State ---
        const string& GetDefaultState() const { return m_defaultState; }
        void          SetDefaultState(const string& name) { m_defaultState = name; }

        void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;
        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t>& deps) override;

    private:
        string                       m_defaultState;
        array_list<AnimatorState>    m_states;
        array_list<AnimatorTransition> m_transitions;
        array_list<AnimatorParam>    m_params;
    };
    DECL_PTR(AnimatorController);

} // namespace pulsar
