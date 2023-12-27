#include <pch.h>
#include "EditorLayer.h"
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>
#include <UI/UIManager.h>

#include <filesystem>

namespace 
{
	static bool opt_fullscreen = true;
	static bool p_open = true;
}

namespace Utils
{
	EditorLayer::EditorLayer()
	{
	}
	EditorLayer::~EditorLayer()
	{
	}
	void EditorLayer::OnInit()
	{
		TGE::TextureData AudioImageData;
		AudioImageData.Type = TGE::TextureType::Texture2D;

		AudioIconImage = TGE::Texture::Generate("../Resources/Images/Audio.png", AudioImageData, TGE::Format::RGBA);

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
		
		style.Colors[ImGuiCol_TitleBgActive]		= ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_TitleBg]				= ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_WindowBg]				= ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		style.Colors[ImGuiCol_Text]					= ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
		style.Colors[ImGuiCol_Button]			    = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive]			= ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
		style.FramePadding							= ImVec2(4, 4);
		style.WindowRounding						= 10.0f;
		style.WindowBorderSize						= 1.0f;

		ImGui_ImplGlfw_InitForOpenGL(TGE::Application::GetMainWindow().GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");

	}
	void EditorLayer::OnUpdate()
	{
	}
	void EditorLayer::OnGUIUpdate()
	{	
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("Editor", &p_open, ImGuiWindowFlags_MenuBar);
		ImGui::ShowDemoWindow();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::MenuItem("Show Rendering", nullptr, &opt_fullscreen);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

		}

		if (opt_fullscreen)
		{
			ImGui::Image((void*)(intptr_t)TGE::Raytracing2D::GetRenderImage()->Get(), {ImGui::GetContentRegionAvail().x,
			ImGui::GetContentRegionAvail().y },
				{ 0,1 }, { 1,0 });

		}

		ImGui::End();

		DisplayDebugStats();
		DisplayAudioPanel();
		DisplayLog();

	}
	void EditorLayer::OnShutdown()
	{
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
				TGE::AudioHandle NewSource = TGE::Audio::Load(Entry);
				AudioFileMap.insert({ Entry, NewSource });
			}

			ImGui::PushID(index);

			bool Pressed = ImGui::ImageButton((void*)(intptr_t)AudioIconImage->Get(), { 80.0f,
																						80.0f },
																						{ 0,1 }, { 1,0 });

			if (Pressed)
			{
				TGE::SourceData data;
				TGE::Source Src = TGE::Audio::GenerateSource(data, AudioFileMap[Entry]);

				Sources.push_back(Src);

				TGE::Audio::Submit(Src);
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
			ImGui::PushID(index);
			ImGui::SetColumnWidth(index, 200.0f);

			if (ImGui::Button("Play"))
				TGE::Audio::Play(Src);
			

			if (ImGui::Button("Pause"))
				TGE::Audio::PauseSource(Src);

			bool PressedSliderFloat		=	ImGui::SliderFloat("Pitch", &Src.Data.Pitch, 0.0f, 1.0f);
			bool PressedSliderFloat2	=	ImGui::SliderFloat("Gain", &Src.Data.Gain, 0.0f, 1.0f);
			bool CheckedBox				=	ImGui::Checkbox("Looping", &Src.Data.Looping);
			bool ChangedVelocity		=	ImGui::SliderFloat3("Velocity", glm::value_ptr(Src.Data.Velocity), -1.0f, 1.0f);
			bool ChangedPosition		=   ImGui::SliderFloat3("Position", glm::value_ptr(Src.Data.Position), -1.0f, 1.0f);


			if (PressedSliderFloat || PressedSliderFloat2 || CheckedBox || ChangedVelocity || ChangedPosition)
				TGE::Audio::EditSource(Src);

			if (ImGui::Button("Destroy Source"))
			{
				auto it = std::find(Sources.begin(), Sources.end(), Src);

				if (it != Sources.end())
				{
					TGE::Audio::RemoveSource(Src);
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
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("MS per frame: %.1f", TGE::Application::GetCurrentDelta());
		ImGui::End();
	}
	void EditorLayer::DisplayLog()
	{
		ImGui::Begin("Debug Output", &p_open, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

		while (!Logger::LogEmpty())
		{
			LogData Front = Logger::GetFront();
			MessagesToDraw.push_back(Front);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Log Options"))
			{
				ImGui::MenuItem("Scroll To Bottom", nullptr, &ScrollToBottomCheckBox);
				ImGui::ColorEdit3("Log Color",		&CoreColor.x);
				ImGui::ColorEdit3("Warn Color",		&WarnColor.x);
				ImGui::ColorEdit3("Error Color",	&ErrorColor.x);
				
				if (ImGui::Button("Clear Log"))
					MessagesToDraw.clear();
				

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 3));

		for (LogData& Message : MessagesToDraw)
		{
			switch (Message.MessageColor)
			{
			case LogColor::Core:	ImGui::PushStyleColor(ImGuiCol_Text, CoreColor);	break;
			case LogColor::Error:	ImGui::PushStyleColor(ImGuiCol_Text, ErrorColor);	break;
			case LogColor::Warn:	ImGui::PushStyleColor(ImGuiCol_Text, WarnColor);	break;
			default:				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
				break;
			}

			ImGui::TextUnformatted(Message.LogMessage.c_str());
			ImGui::PopStyleColor();
		}

		if ((ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) && ScrollToBottomCheckBox)
			ImGui::SetScrollHereY(1.0f);
		
		ScrollToBottom = false;

		ImGui::PopStyleVar();

		ImGui::End();
	}
}