#include "Components/StaticMeshRendererComponent.h"
#include "Components/MeshContainerComponent.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <gfx/GFXBuffer.h>

namespace pulsar
{



    class StaticMeshRenderObject : public rendering::RenderObject
    {
    public:
        void OnCreateResource()
        {
            m_meshBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, 0);
        }
        void OnDestroyResource()
        {
            delete m_meshBuffer;
        }
        StaticMeshSection GetSection()
        {

        }
        gfx::GFXBuffer* m_meshBuffer;
        gfx::GFXShaderPass* m_shaderPass;
    };

    sptr<rendering::RenderObject> StaticMeshRendererComponent::CreateRenderObject()
    {
        auto obj = new StaticMeshRenderObject;

        auto allocSize = 0;
        obj->m_meshBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, sizeof(allocSize));

        return mksptr(obj);
    }

    void StaticMeshRendererComponent::OnInitialize()
    {

    }

    //void StaticMeshRendererComponent::OnDraw()
    //{
    //    auto filter = this->get_node()->GetComponent<MeshContainerComponent>();
    //    if (!filter)
    //    {
    //        Logger::Log("empty mesh container", LogLevel::Warning);
    //        return;
    //    } 

    //    auto mat = this->materials_->at(0);

    //    {
    //        //ShaderPassScope pass(mat->get_shader()->GetPass(0));
    //        //pass->SetUniformMatrix4fv("MODEL", Matrix4f::StaticScalar());
    //        //pass->SetUniformMatrix4fv("VIEW", RenderContext::GetCurrentCamera()->GetViewMat());
    //        //pass->SetUniformMatrix4fv("PROJECTION", RenderContext::GetCurrentCamera()->GetProjectionMat());

    //        //auto mesh = filter->get_mesh();
    //        //assert(IsValid(mesh));
    //        //assert(mesh->GetIsBindGPU());

    //        //auto a = mesh->GetRenderHandle();

    //        //glBindVertexArray(mesh->GetRenderHandle());
 
    //        //glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);
    //    }
    //}

}