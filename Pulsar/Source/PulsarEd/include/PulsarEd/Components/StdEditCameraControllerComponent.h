#pragma once
#include "EditorComponent.h"

namespace pulsared
{
	class StdEditCameraControllerComponent : public EditorComponent
	{
		CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StdEditCameraControllerComponent, EditorComponent);

	public:
		virtual ~StdEditCameraControllerComponent() override
		{
			int a = 3;
		}
	};
}