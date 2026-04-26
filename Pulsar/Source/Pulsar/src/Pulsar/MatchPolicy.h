#pragma once
#include "Assembly.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        MatchPolicy,
        None,           // 不做任何匹配，忽略
        Match,          // 仅匹配已有资产，匹配不到则跳过
        MatchOrCreate,  // 匹配已有资产，匹配不到则新建
        AlwaysCreate    // 始终新建，不做匹配
    );
}
CORELIB_DECL_BOXING(pulsar::MatchPolicy, pulsar::BoxingMatchPolicy);
