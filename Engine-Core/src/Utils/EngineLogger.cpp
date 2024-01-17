#include "pch.h"
#include "EngineLogger.h"

namespace TooGoodEngine {

	EngineLogger::EngineLogger()
	{
		std::cout.sync_with_stdio(false);
	}
	void EngineLogger::DisplayLogToImGui()
	{
		ImGui::Begin("Debug Output", &m_LogOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

		while (!m_CoreLogQueue.empty())
		{
			LogData Front = m_CoreLogQueue.front();
			m_CoreLogQueue.pop();
			m_MessagesToDraw.push_back(Front);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Log Options"))
			{
				ImGui::MenuItem("Scroll To Bottom", nullptr, &m_ScrollToBottomCheckBox);
				ImGui::ColorEdit3("Log Color", &CoreColor.x);
				ImGui::ColorEdit3("Warn Color", &WarnColor.x);
				ImGui::ColorEdit3("Error Color", &ErrorColor.x);

				if (ImGui::Button("Clear Log"))
					m_MessagesToDraw.clear();


				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 3));

		for (LogData& Message : m_MessagesToDraw)
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

		if ((m_ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) && m_ScrollToBottomCheckBox)
			ImGui::SetScrollHereY(1.0f);

		m_ScrollToBottom = false;

		ImGui::PopStyleVar();

		ImGui::End();
	}
}