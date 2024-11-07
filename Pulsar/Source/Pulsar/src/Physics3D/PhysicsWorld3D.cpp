#include "Physics3D/PhysicsWorld3D.h"


#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace pulsar
{

    class _PhysicsWorld3DNative
    {
    public:
        JPH::PhysicsSystem m_physicsSystem;

    };

    void PhysicsWorld3D::BeginSimulate()
    {
        JPH::RegisterDefaultAllocator();
    }
    void PhysicsWorld3D::EndSimulate()
    {
    }
    void PhysicsWorld3D::StepSimulate(float dt)
    {
    }
} // namespace pulsar