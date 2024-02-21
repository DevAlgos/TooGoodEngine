#pragma once

#include "BaseLayer.h"

#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>


#include <memory>
#include <Utils/Camera.h>
#include <filesystem>


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

		void DisplayHierarchyPanel();
		void DisplayPreview();
		void DisplayAudioPanel();
		void DisplayDebugStats();
		void DisplayLog();

		static void AddStatistsic(const std::string_view& name, int64_t time);

	private:
		static std::unordered_map<std::string_view, int64_t> s_StatsticsToRender;

		std::shared_ptr<TooGoodEngine::Framebuffer>  DisplayFramebuffer;
		std::shared_ptr<TooGoodEngine::Texture>		 DisplayColorAttachment;
		std::shared_ptr<TooGoodEngine::RenderBuffer> DisplayRenderBuffer;

		std::unique_ptr<TooGoodEngine::Texture> AudioIconImage;

		std::unordered_map<std::string, TooGoodEngine::AudioHandle> AudioFileMap;
		const std::filesystem::path AudioFiles{ "../Resources/Audio" };

	    std::vector<TooGoodEngine::Source> Sources;

		bool ScrollToBottom = false;
		bool ScrollToBottomCheckBox = true;

		ImVec4 CoreColor = { 1.0f, 0.0f, 1.0f, 1.0f};
		ImVec4 ErrorColor = { 1.0f, 0.0f, 0.0f, 1.0f};
		ImVec4 WarnColor = { 1.0f, 1.0f, 0.0f, 1.0f};
	};
}