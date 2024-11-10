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
    using namespace JPH;
    using namespace JPH::literals;
    
    namespace BroadPhaseLayers
    {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    }
    namespace Layers
    {
        static constexpr ObjectLayer NON_MOVING = 0;
        static constexpr ObjectLayer MOVING = 1;
        static constexpr ObjectLayer NUM_LAYERS = 2;
    };

    // BroadPhaseLayerInterface implementation
    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint					GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

        #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
        {
            switch ((BroadPhaseLayer::Type)inLayer)
            {
            case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
            case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
            default:													JPH_ASSERT(false); return "INVALID";
            }
        }
        #endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
    };
    class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };
    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
    {
    public:
        virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };


    class _PhysicsWorld3DNative
    {
    public:
        PhysicsSystem m_physicsSystem;
        BPLayerInterfaceImpl broad_phase_layer_interface;

        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;

        TempAllocatorImpl temp_allocator{10 * 1024 * 1024};
        JobSystemThreadPool job_system{cMaxPhysicsJobs, cMaxPhysicsBarriers, int(std::thread::hardware_concurrency() - 1)};
    };
    class MyBodyActivationListener : public BodyActivationListener
    {
    public:
        virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            //cout << "A body got activated" << endl;
        }

        virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
        {
            //cout << "A body went to sleep" << endl;
        }
    };
    // An example contact listener
    class MyContactListener : public ContactListener
    {
    public:
        // See: ContactListener
        virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
        {
            //cout << "Contact validate callback" << endl;

            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            //cout << "A contact was added" << endl;
        }

        virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
        {
            //cout << "A contact was persisted" << endl;
        }

        virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
        {
            //cout << "A contact was removed" << endl;
        }
    };

    void PhysicsWorld3D::BeginSimulate()
    {
        m_world = new _PhysicsWorld3DNative;


        RegisterDefaultAllocator();

        Factory::sInstance = new Factory();

        RegisterTypes();



        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        const uint32_t cMaxBodies = 65535;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxBodyPairs = 65535;
        const uint32_t cMaxContactConstraints = 10240;


        m_world->m_physicsSystem.Init(
            cMaxBodies,
            cNumBodyMutexes,
            cMaxBodyPairs,
            cMaxContactConstraints,
            m_world->broad_phase_layer_interface,
            m_world->object_vs_broadphase_layer_filter,
            m_world->object_vs_object_layer_filter);

        // A body activation listener gets notified when bodies activate and go to sleep
	    // Note that this is called from a job so whatever you do here needs to be thread safe.
	    // Registering one is entirely optional.
	    MyBodyActivationListener body_activation_listener;
	    m_world->m_physicsSystem.SetBodyActivationListener(&body_activation_listener);

	    // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	    // Note that this is called from a job so whatever you do here needs to be thread safe.
	    // Registering one is entirely optional.
	    MyContactListener contact_listener;
	    m_world->m_physicsSystem.SetContactListener(&contact_listener);

	    // // The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	    // // variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	    // BodyInterface &body_interface = m_world->m_physicsSystem.GetBodyInterface();
	    //
	    // // Next we can create a rigid body to serve as the floor, we make a large box
	    // // Create the settings for the collision volume (the shape).
	    // // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	    // BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
	    // floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
	    //
	    // // Create the shape
	    // ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	    // ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
	    //
	    // // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	    // BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
	    //
	    // // Create the actual rigid body
	    // Body *floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
	    //
	    // // Add it to the world
	    // body_interface.AddBody(floor->GetID(), EActivation::DontActivate);
	    //
	    // // Now create a dynamic body to bounce on the floor
	    // // Note that this uses the shorthand version of creating and adding a body to the world
	    // BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	    // BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);
	    //
	    // // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
	    // // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
	    // body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));
	    //
	    // // We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
	    // const float cDeltaTime = 1.0f / 60.0f;
	    //
	    // // Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	    // // You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	    // // Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	    // m_world->m_physicsSystem.OptimizeBroadPhase();
     //
    }
    
    void PhysicsWorld3D::EndSimulate()
    {
        UnregisterTypes();

        // Destroy the factory
        delete Factory::sInstance;
        Factory::sInstance = nullptr;

        delete m_world;
        m_world = nullptr;

    }

    void PhysicsWorld3D::StepSimulate(float dt)
    {
        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
        const int cCollisionSteps = 1;

        // Step the world
        m_world->m_physicsSystem.Update(dt, cCollisionSteps, &m_world->temp_allocator, &m_world->job_system);
    }
} // namespace pulsar