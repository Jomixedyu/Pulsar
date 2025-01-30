#pragma once
#include "Pulsar/Assets/Graph.h"
namespace pulsar
{
    class RDGNode : public GraphNode
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RDGNode, GraphNode);
    public:

    };
    class RDGPassNode : public RDGNode
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RDGPassNode, RDGNode);
    public:

    };
    class RDGResourceNode : public RDGNode
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RDGResourceNode, RDGNode);
    public:

    };

    class RDGAttachmentNode : public RDGResourceNode
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RDGAttachmentNode, RDGResourceNode);
    public:
        RDGAttachmentNode()
        {
            CreatePin("in", GraphPinType::Input);
            CreatePin("out", GraphPinType::Output);
        }

    };

    class RDGGraph : public Graph
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RDGGraph, Graph);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute)
    public:

    };
}