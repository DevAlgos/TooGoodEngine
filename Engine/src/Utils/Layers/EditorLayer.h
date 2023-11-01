#pragma once

#include "BaseLayer.h"

#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>

#include <memory>


namespace Utils
{
	class EditorLayer : public BaseLayer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer();

		void OnInit()       override;
		void OnUpdate()     override;
		void OnGUIUpdate()  override;
		void OnShutdown()	override;
	private:
		std::unique_ptr<Graphics::Texture> Ben1;
		std::unique_ptr<Graphics::Framebuffer> ViewFrame;
	};
}