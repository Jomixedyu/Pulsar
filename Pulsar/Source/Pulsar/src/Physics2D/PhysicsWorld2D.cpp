#include "Physics2D/PhysicsWorld2D.h"
#include <box2d/box2d.h>

namespace pulsar
{
    class _PhysicsWorld2DNative
    {
    public:
        b2WorldId m_b2world;
    };



    void PhysicsWorld2D::Tick(float dt)
    {
        if (!m_isSimulating)
            return;

        float timeStep = 1.0f / 60.0f;
        int32_t velocityIterations = 6; // 1-10
        int32_t positionIterations = 2; // 1-10

        b2World_Step(m_world->m_b2world, dt, positionIterations);


        b2BodyEvents events = b2World_GetBodyEvents(m_world->m_b2world);
        for (int i = 0; i < events.moveCount; ++i)
        {
            auto event = events.moveEvents[i];
            auto eventCallback = (INotifyPhysics2DEvent*)event.userData;

            eventCallback->INotifyPhysics2DEvent_OnChangedTransform(event.transform.p, b2Rot_GetAngle(event.transform.q));
        }
    }

    static inline b2BodyType GetBodyType(RigidBody2DMode mode)
    {
        switch (mode)
        {
        case RigidBody2DMode::Static: return b2_staticBody;
        case RigidBody2DMode::Dynamic: return b2_dynamicBody;
        case RigidBody2DMode::Kinematic: return b2_kinematicBody;
        }
        return {};
    }

    void PhysicsWorld2D::BeginSimulate()
    {
        m_isSimulating = true;
        m_world = new _PhysicsWorld2DNative;

        auto worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2(0.0f, -9.81f);

        auto b2world = b2CreateWorld(&worldDef);;
        m_world->m_b2world = b2world;

        for (auto& obj : m_objects)
        {
            auto bodyDef = b2DefaultBodyDef();
            bodyDef.position = b2Vec2(obj->m_position.x, obj->m_position.y);
            bodyDef.type = GetBodyType(obj->m_rigidMode);
            bodyDef.userData = obj->m_event;
            bodyDef.rotation = b2MakeRot(obj->m_rotation);

            auto bodyId = b2CreateBody(b2world, &bodyDef);

            for (auto& inShape : obj->m_shapes)
            {
                auto shapeDef = b2DefaultShapeDef();
                shapeDef.density = inShape.m_density;
                shapeDef.friction = inShape.m_friction;
                shapeDef.isSensor = inShape.m_isSensor;

                auto size = inShape.size;
                auto radius = inShape.radius;

                b2ShapeId shapeId;
                switch (inShape.type)
                {
                case Physics2DObject::BOX: {
                    auto poly = b2MakeBox(size.x, size.y);
                    shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &poly);
                    break;
                }
                case Physics2DObject::CIRCLE: {
                    b2Circle circle = {{0,0}, radius};
                    shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                    break;
                }
                case Physics2DObject::CAPSULE: {
                    b2Capsule capsule {{0,0}, {size.x, size.y}, radius};
                    shapeId = b2CreateCapsuleShape(bodyId, &shapeDef, &capsule);
                    break;
                }
                }

            }

        }
    }
    void PhysicsWorld2D::EndSimulate()
    {
        b2DestroyWorld(m_world->m_b2world);

        delete m_world;
        m_world = nullptr;

        m_isSimulating = false;
    }

    PhysicsWorld2D::PhysicsWorld2D()
    {
        
    }

    void PhysicsWorld2D::AddObject(Physics2DObject* object)
    {
        m_objects.push_back(object);
    }
    void PhysicsWorld2D::RemoveObject(Physics2DObject* object)
    {
        std::erase(m_objects, object);
    }
} // namespace pulsar