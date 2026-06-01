#include "Physics2D/PhysicsWorld2D.h"
#include <box2d/box2d.h>

namespace pulsar
{
    class _PhysicsWorld2DNative
    {
    public:
        b2WorldId m_b2world{};
        std::unordered_map<Physics2DObject*, b2BodyId> m_obj2Body;
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
            auto obj = (Physics2DObject*)event.userData;
            if (obj && obj->OnTransformChanged)
            {
                obj->OnTransformChanged(Vector2f(event.transform.p.x, event.transform.p.y), b2Rot_GetAngle(event.transform.q));
            }
        }

        b2ContactEvents contactEvents = b2World_GetContactEvents(m_world->m_b2world);

        if (contactEvents.beginCount > 0 || contactEvents.endCount > 0)
        {
            // 用 printf 因为 Logger 可能在这不可用或太重量级
            printf("[PhysicsWorld2D] contacts: begin=%d end=%d\n", contactEvents.beginCount, contactEvents.endCount);
        }

        for (int i = 0; i < contactEvents.beginCount; ++i)
        {
            auto& event = contactEvents.beginEvents[i];
            auto bodyA = b2Shape_GetBody(event.shapeIdA);
            auto bodyB = b2Shape_GetBody(event.shapeIdB);

            auto objA = (Physics2DObject*)b2Body_GetUserData(bodyA);
            auto objB = (Physics2DObject*)b2Body_GetUserData(bodyB);

            if (objA && objA->OnCollisionEnter)
                objA->OnCollisionEnter(objB ? objB->CallbackObject : ObjectHandle{});
            if (objB && objB->OnCollisionEnter)
                objB->OnCollisionEnter(objA ? objA->CallbackObject : ObjectHandle{});
        }

        for (int i = 0; i < contactEvents.endCount; ++i)
        {
            auto& event = contactEvents.endEvents[i];
            auto bodyA = b2Shape_GetBody(event.shapeIdA);
            auto bodyB = b2Shape_GetBody(event.shapeIdB);

            auto objA = (Physics2DObject*)b2Body_GetUserData(bodyA);
            auto objB = (Physics2DObject*)b2Body_GetUserData(bodyB);

            if (objA && objA->OnCollisionExit)
                objA->OnCollisionExit(objB ? objB->CallbackObject : ObjectHandle{});
            if (objB && objB->OnCollisionExit)
                objB->OnCollisionExit(objA ? objA->CallbackObject : ObjectHandle{});
        }
    }

    static inline b2BodyType GetBodyType(RigidBody2DMode mode)
    {
        switch (mode)
        {
        case RigidBody2DMode::Static:
            return b2_staticBody;
        case RigidBody2DMode::Dynamic:
            return b2_dynamicBody;
        case RigidBody2DMode::Kinematic:
            return b2_kinematicBody;
        }
        return {};
    }

    void PhysicsWorld2D::BeginSimulate()
    {
        m_isSimulating = true;
        m_world = new _PhysicsWorld2DNative;

        auto worldDef = b2DefaultWorldDef();
        worldDef.gravity = b2Vec2(0.0f, -9.81f);

        auto b2world = b2CreateWorld(&worldDef);

        m_world->m_b2world = b2world;

        for (auto& object : m_objects)
        {
            AddObjectToSystem(object);
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
        if (std::ranges::contains(m_objects, object))
        {
            return;
        }
        m_objects.push_back(object);
        AddObjectToSystem(object);
    }

    void PhysicsWorld2D::RemoveObject(Physics2DObject* object)
    {
        if (!std::ranges::contains(m_objects, object))
        {
            return;
        }
        std::erase(m_objects, object);
        RemoveObjectFromSystem(object);
    }

    Vector2f PhysicsWorld2D::GetLinearVelocity(Physics2DObject* object) const
    {
        if (!m_world || !m_world->m_obj2Body.contains(object))
            return {};
        auto bodyId = m_world->m_obj2Body.at(object);
        auto v = b2Body_GetLinearVelocity(bodyId);
        return Vector2f(v.x, v.y);
    }

    void PhysicsWorld2D::SetLinearVelocity(Physics2DObject* object, Vector2f velocity)
    {
        if (!m_world || !m_world->m_obj2Body.contains(object))
            return;
        auto bodyId = m_world->m_obj2Body.at(object);
        b2Body_SetLinearVelocity(bodyId, b2Vec2{velocity.x, velocity.y});
    }

    void PhysicsWorld2D::ApplyLinearImpulse(Physics2DObject* object, Vector2f impulse, Vector2f point)
    {
        if (!m_world || !m_world->m_obj2Body.contains(object))
            return;
        auto bodyId = m_world->m_obj2Body.at(object);
        b2Body_ApplyLinearImpulse(bodyId, b2Vec2{impulse.x, impulse.y}, b2Vec2{point.x, point.y}, true);
    }

    void PhysicsWorld2D::SetTransform(Physics2DObject* object, Vector2f position, float rotation)
    {
        if (!m_world || !m_world->m_obj2Body.contains(object))
            return;
        auto bodyId = m_world->m_obj2Body.at(object);
        b2Body_SetTransform(bodyId, b2Vec2{position.x, position.y}, b2MakeRot(rotation));
    }

    bool PhysicsWorld2D::RayCast(Vector2f start, Vector2f end, RayCastResult& outResult)
    {
        if (!m_world)
            return false;

        b2QueryFilter filter = b2DefaultQueryFilter();
        b2Vec2 origin(start.x, start.y);
        b2Vec2 translation(end.x - start.x, end.y - start.y);

        b2RayResult result = b2World_CastRayClosest(m_world->m_b2world, origin, translation, filter);

        if (result.hit)
        {
            outResult.hit = true;
            outResult.point = Vector2f(result.point.x, result.point.y);
            outResult.normal = Vector2f(result.normal.x, result.normal.y);

            auto body = b2Shape_GetBody(result.shapeId);
            auto obj = (Physics2DObject*)b2Body_GetUserData(body);
            outResult.hitObject = obj ? obj->CallbackObject : ObjectHandle{};
            return true;
        }

        return false;
    }

    struct CapsuleCastContext
    {
        PhysicsWorld2D::CapsuleCastResult* result;
    };

    static float CapsuleCastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
    {
        auto* ctx = static_cast<CapsuleCastContext*>(context);
        auto* result = ctx->result;

        result->hit = true;
        result->point = Vector2f(point.x, point.y);
        result->normal = Vector2f(normal.x, normal.y);
        result->fraction = fraction;

        auto body = b2Shape_GetBody(shapeId);
        auto obj = (Physics2DObject*)b2Body_GetUserData(body);
        result->hitObject = obj ? obj->CallbackObject : ObjectHandle{};

        // 返回 fraction 进行裁剪，继续搜索更近的命中
        return fraction;
    }

    bool PhysicsWorld2D::CastCapsule(Vector2f position, float radius, float height, Vector2f translation, CapsuleCastResult& outResult)
    {
        if (!m_world)
            return false;

        outResult = CapsuleCastResult{};

        float halfLine = height * 0.5f - radius;
        if (halfLine < 0.0f) halfLine = 0.0f;

        b2Capsule capsule{{0.0f, -halfLine}, {0.0f, halfLine}, radius};

        b2Transform transform;
        transform.p = {position.x, position.y};
        transform.q = b2MakeRot(0.0f);

        b2QueryFilter filter = b2DefaultQueryFilter();

        CapsuleCastContext context{&outResult};
        b2World_CastCapsule(m_world->m_b2world, &capsule, transform, {translation.x, translation.y}, filter, CapsuleCastCallback, &context);

        return outResult.hit;
    }

    void PhysicsWorld2D::AddObjectToSystem(Physics2DObject* object)
    {
        if (!m_world)
        {
            return;
        }
        if (m_world->m_obj2Body.contains(object))
        {
            return;
        }
        auto b2world = m_world->m_b2world;

        auto bodyDef = b2DefaultBodyDef();
        bodyDef.position = b2Vec2(object->m_position.x, object->m_position.y);
        bodyDef.type = GetBodyType(object->m_rigidMode);
        bodyDef.userData = object;
        bodyDef.rotation = b2MakeRot(object->m_rotation);

        auto bodyId = b2CreateBody(b2world, &bodyDef);

        for (auto& inShape : object->m_shapes)
        {
            auto shapeDef = b2DefaultShapeDef();
            shapeDef.density = inShape.m_density;
            shapeDef.friction = inShape.m_friction;
            shapeDef.isSensor = inShape.m_isSensor;
            shapeDef.enableContactEvents = true;

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
                b2Circle circle = {{0, 0}, radius};
                shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                break;
            }
            case Physics2DObject::CAPSULE: {
                b2Capsule capsule{{0, 0}, {size.x, size.y}, radius};
                shapeId = b2CreateCapsuleShape(bodyId, &shapeDef, &capsule);
                break;
            }
            }
        }

        m_world->m_obj2Body.emplace(object, bodyId);
    }

    void PhysicsWorld2D::RemoveObjectFromSystem(Physics2DObject* object)
    {
        if (!m_world)
        {
            return;
        }
        if (m_world->m_obj2Body.contains(object))
        {
            auto bodyId = m_world->m_obj2Body[object];
            b2DestroyBody(bodyId);
            m_world->m_obj2Body.erase(object);
        }
    }

} // namespace pulsar
