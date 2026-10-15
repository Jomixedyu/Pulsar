
namespace pulsar
{


    class ObjectBase
    {
        private uint64 m_objectHandle;

        uint64 GetObjectHandle()
        {
            return m_objectHandle;
        }
    };

    class SceneObject : ObjectBase
    {
        
    };

    class AssetObject : ObjectBase
    {

    };


    class Component : SceneObject
    {
        void BeginPlay() {}
        void EndPlay() {}
        void Tick(float dt) {}
    };


}