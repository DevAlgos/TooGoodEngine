#include "pch.h"

#include <filesystem>

#include "EditorLayer.h"
#include "Graphics/Texture.h"
#include "Graphics/Buffers.h"
#include "UI/UIManager.h"
#include "SceneSystem/Scene.h"
#include "Utils/Log.h"
#include "Graphics/Renderer.h"

namespace 
{
	static bool opt_fullscreen = true;
	static bool p_open = true;
	static int CurrentScale = 2;
}

namespace Utils
{
	std::unordered_map<std::string_view, int64_t> EditorLayer::s_StatsticsToRender;

	EditorLayer::EditorLayer()
	{
	}
	EditorLayer::~EditorLayer()
	{
	}
	void EditorLayer::OnInit()
	{
		int Width = TooGoodEngine::Application::GetMainWindow().GetWidth();
		int Height = TooGoodEngine::Application::GetMainWindow().GetHeight();
		

		TooGoodEngine::TextureData TextureData;
		TextureData.InternalFormat = TooGoodEngine::TextureFormat::RGBA32F;
		TextureData.Width = Width;
		TextureData.Height = Height;

		float* TempData = new float[Width * Height * 4];
		memset(TempData, 0, Width * Height * 4);

		DisplayColorAttachment = TooGoodEngine::Texture::GenerateShared(TempData, TextureData);

		delete[] TempData;

		TooGoodEngine::RenderBufferData RenderBufferData;
		RenderBufferData.width = Width;
		RenderBufferData.height = Height;
		RenderBufferData.InternalFormat = TooGoodEngine::TextureFormat::RGBA;

		DisplayRenderBuffer = TooGoodEngine::RenderBuffer::GenerateShared(RenderBufferData);

		TooGoodEngine::FramebufferData FramebufferData;
		FramebufferData.AttachmentList = 
		{
			{TooGoodEngine::Attachment(TooGoodEngine::AttachmentType::Color, Width, Height), DisplayColorAttachment},
		};


		DisplayFramebuffer = TooGoodEngine::Framebuffer::GenerateShared(FramebufferData);

		TooGoodEngine::CurrentScene = std::make_shared<TooGoodEngine::Scene>("Testing Scene");

		TooGoodEngine::TextureData AudioImageData;
		AudioImageData.Type = TooGoodEngine::TextureType::Texture2D;

		AudioIconImage = TooGoodEngine::Texture::Generate("../Resources/Images/Audio.png", AudioImageData, TooGoodEngine::Format::RGBA);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImFont* font = io.Fonts->AddFontFromFileTTF("../Resources/fonts/metropolis.otf", 16);

		io.FontDefault = font;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		
		ImGuiStyle& style = ImGui::GetStyle();
		//1, 0.761, 0.8

		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4f, 0.4f, 0.45f, 1.0f); 
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.6f, 0.3f, 0.1f, 1.0f); 

		style.GrabRounding = 3.0f;
		style.GrabMinSize = 20.0f;

		style.Colors[ImGuiCol_TitleBgActive]		= ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_TitleBg]				= ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_WindowBg]				= ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
		style.Colors[ImGuiCol_Text]					= ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
		style.Colors[ImGuiCol_Button]			    = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive]			= ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		style.FramePadding							= ImVec2(4, 4);
		style.WindowRounding						= 10.0f;
		style.WindowBorderSize						= 1.0f;

		ImGui_ImplGlfw_InitForOpenGL(TooGoodEngine::Application::GetMainWindow().GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");

	}
	void EditorLayer::OnUpdate()
	{
		/*TooGoodEngine::CurrentScene->SceneUpdate();

		DisplayFramebuffer->Bind();

		TooGoodEngine::CurrentScene->SceneDisplay();

		DisplayFramebuffer->UnBind();*/
		Utils::TimedScope a("Scene update and render");
		TooGoodEngine::CurrentScene->SceneUpdate();
		TooGoodEngine::CurrentScene->SceneDisplay();
		
	}
	void EditorLayer::OnGUIUpdate()
	{	
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("Editor", &p_open, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::MenuItem("Show Rendering", nullptr, &opt_fullscreen);

				ImGui::Text("Renderer down scaling factor");
				ImGui::SameLine();
				int Previous = CurrentScale;
				ImGui::SliderInt("##ScaleSlider", &CurrentScale, 1, 10);

				if (Previous != CurrentScale)
					TooGoodEngine::Renderer::ChangeScaledResolution((float)CurrentScale);

				if (ImGui::Button("Median Split"))
					TooGoodEngine::Renderer::ChangeBVHBuildType(TooGoodEngine::BuildType::MedianSplit);

				if (ImGui::Button("SAH Split"))
					TooGoodEngine::Renderer::ChangeBVHBuildType(TooGoodEngine::BuildType::SAHSplit);

				if (ImGui::Button("Morton Split"))
					TooGoodEngine::Renderer::ChangeBVHBuildType(TooGoodEngine::BuildType::HLSplit);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

		}

		if (opt_fullscreen)
		{
			/*ImGui::Image((void*)(intptr_t)TooGoodEngine::Raytracing2D::GetRenderImage()->Get(), {ImGui::GetContentRegionAvail().x,
			ImGui::GetContentRegionAvail().y },
				{ 0,1 }, { 1,0 });*/
		
			/*ImGui::Image((void*)(intptr_t)DisplayColorAttachment->Get(), {ImGui::GetContentRegionAvail().x,
			ImGui::GetContentRegionAvail().y },
				{ 0,1 }, { 1,0 });*/

			ImGui::Image((void*)(intptr_t)TooGoodEngine::Renderer::GetColorBuffer()->Get(), { ImGui::GetContentRegionAvail().x,
			ImGui::GetContentRegionAvail().y },
				{ 0,1 }, { 1,0 });

		}

		ImGui::End();

		TooGoodEngine::CurrentScene->DisplayEntites();

		DisplayDebugStats();
		DisplayAudioPanel();
		DisplayLog();

	}
	void EditorLayer::OnShutdown()
	{
		TooGoodEngine::CurrentScene = nullptr;
	}
	void EditorLayer::DisplayHierarchyPanel()
	{
		
	}
	void EditorLayer::DisplayPreview()
	{
	}
	void EditorLayer::DisplayAudioPanel()
	{
		ImGui::Begin("Audio", &p_open);
		
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });

		int numColumns = static_cast<int>(ImGui::GetContentRegionAvail().x / 100); 
		numColumns = std::max(numColumns, 1);

		ImGui::SetCursorPosY(50);
		ImGui::Columns(numColumns, 0, false);
		
		size_t index = 0;
		for (const auto& dir_entry : std::filesystem::directory_iterator{ AudioFiles })
		{
			std::string Entry = dir_entry.path().string();

			if (!AudioFileMap.contains(Entry))
			{
				TooGoodEngine::AudioHandle NewSource = TooGoodEngine::Audio::Load(Entry);
				AudioFileMap.insert({ Entry, NewSource });
			}

			ImGui::PushID((int)index);

			bool Pressed = ImGui::ImageButton((void*)(intptr_t)AudioIconImage->Get(), { 80.0f,
																						80.0f },
																						{ 0,1 }, { 1,0 });

			if (Pressed)
			{
				TooGoodEngine::SourceData data;
				TooGoodEngine::Source Src = TooGoodEngine::Audio::GenerateSource(data, AudioFileMap[Entry]);

				Sources.push_back(Src);

				TooGoodEngine::Audio::Submit(Src);
			}

			ImGui::TextWrapped("%s", dir_entry.path().filename().string().c_str());
			ImGui::NextColumn();

			index++;
			ImGui::PopID();
		}


		ImGui::PopStyleColor();
		ImGui::End();


		ImGui::Begin("Sources");

		numColumns = static_cast<int>(ImGui::GetContentRegionAvail().x / 100);
		numColumns = std::max(numColumns, 1);

		ImGui::Columns(numColumns, 0, false);
		

		index = 0;
		for (auto& Src : Sources)
		{
			ImGui::PushID((int)index);
			ImGui::SetColumnWidth((int)index, 200.0f);

			if (ImGui::Button("Play"))
				TooGoodEngine::Audio::Play(Src);
			

			if (ImGui::Button("Pause"))
				TooGoodEngine::Audio::PauseSource(Src);

			bool PressedSliderFloat		=	ImGui::SliderFloat("Pitch", &Src.Data.Pitch, 0.0f, 1.0f);
			bool PressedSliderFloat2	=	ImGui::SliderFloat("Gain", &Src.Data.Gain, 0.0f, 1.0f);
			bool CheckedBox				=	ImGui::Checkbox("Looping", &Src.Data.Looping);
			bool ChangedVelocity		=	ImGui::SliderFloat3("Velocity", glm::value_ptr(Src.Data.Velocity), -1.0f, 1.0f);
			bool ChangedPosition		=   ImGui::SliderFloat3("Position", glm::value_ptr(Src.Data.Position), -1.0f, 1.0f);


			if (PressedSliderFloat || PressedSliderFloat2 || CheckedBox || ChangedVelocity || ChangedPosition)
				TooGoodEngine::Audio::EditSource(Src);

			if (ImGui::Button("Destroy Source"))
			{
				auto it = std::find(Sources.begin(), Sources.end(), Src);

				if (it != Sources.end())
				{
					TooGoodEngine::Audio::RemoveSource(Src);
					Sources.erase(it);
				}
			}

			ImGui::NextColumn();
			index++;
			ImGui::PopID();
		}

		ImGui::End();

	}
	void EditorLayer::DisplayDebugStats()
	{
		ImGui::Begin("Debug Window");
		ImGui::Text("time per frame: %.1f ms", TooGoodEngine::Application::GetCurrentDelta());

		for (auto& [name, time] : s_StatsticsToRender)
			ImGui::Text("%s took: %lld milliseconds", name.data(), time);

		ImGui::End();
	}
	void EditorLayer::DisplayLog()
	{
		TooGoodEngine::Log::GetEngineLogger()->DisplayLogToImGui();
	}

	void EditorLayer::AddStatistsic(const std::string_view& name, int64_t time)
	{
		s_StatsticsToRender[name] = time;
	}

}