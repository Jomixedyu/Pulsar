#include "Components/StaticMeshRendererComponent.h"
#include "Components/MeshContainerComponent.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderContext.h>

namespace pulsar
{
    void StaticMeshRendererComponent::OnInitialize()
    {
        auto filter = this->get_node()->GetComponent<MeshContainerComponent>();
        if (!filter)
        {
            Logger::Log("empty mesh container", LogLevel::Warning);
            return;
        }
        assert(this->materials_->size() > 0);
        auto mesh = filter->get_mesh();

    }

    void StaticMeshRendererComponent::OnDraw()
    {
        auto filter = this->get_node()->GetComponent<MeshContainerComponent>();
        if (!filter)
        {
            Logger::Log("empty mesh container", LogLevel::Warning);
            return;
        } 

        auto mat = this->materials_->at(0);

        {
            //ShaderPassScope pass(mat->get_shader()->GetPass(0));
            //pass->SetUniformMatrix4fv("MODEL", Matrix4f::StaticScalar());
            //pass->SetUniformMatrix4fv("VIEW", RenderContext::GetCurrentCamera()->GetViewMat());
            //pass->SetUniformMatrix4fv("PROJECTION", RenderContext::GetCurrentCamera()->GetProjectionMat());

            //auto mesh = filter->get_mesh();
            //assert(IsValid(mesh));
            //assert(mesh->GetIsBindGPU());

            //auto a = mesh->GetRenderHandle();

            //glBindVertexArray(mesh->GetRenderHandle());
 
            //glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);


        }
        


    }
}