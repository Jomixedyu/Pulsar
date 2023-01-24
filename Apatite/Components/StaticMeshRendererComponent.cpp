#include "StaticMeshRendererComponent.h"
#include "MeshContainerComponent.h"
#include <Apatite/Logger.h>
#include <ThirdParty/glad/glad.h>

namespace apatite
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
        //auto mat = this->materials_->at(0);
        //mat->UseMaterial();

        //auto mesh = filter->get_mesh();
        //assert(IsValid(mesh));
        //assert(mesh->GetIsBindGPU());
        //auto a = mesh->GetRenderHandle();
        //assert(glGetError() == GL_NO_ERROR);
        //glBindVertexArray(mesh->GetRenderHandle());
        //assert(glGetError() == GL_NO_ERROR);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);
    }
}