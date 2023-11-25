#pragma once

#include "BaseLayer.h"

#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>

#include <memory>
#include <Utils/Camera.h>


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
		std::unique_ptr<TGE::Texture> Ben1;
		std::unique_ptr<TGE::Texture> ComputeTexture;
		std::unique_ptr<TGE::Framebuffer> ViewFrame;
		std::unique_ptr<TGE::Texture> BackGround;
		std::unique_ptr<TGE::Texture> TestTexture;

		OrthoGraphicCamera m_OrthoCam;

	};
}