#pragma once

#include <Application.h>
#include <Utils/Layers/LayerManager.h>
#include <Utils/Layers/BaseLayer.h>
#include <memory>
#include <filesystem>

namespace TooGoodEngine
{
	struct Details
	{
		std::string_view ApplicationName = "Game";
		std::filesystem::path PythonEnviromentPath;
		int Width = 1280;
		int Height = 720;
	};

	class UserApplication
	{
	public:
		UserApplication(const Details& details);
		virtual ~UserApplication();

		void PushLayer(std::shared_ptr<Utils::BaseLayer> Layer);
		void PopLayer();

		const Details GetDetails() const { return m_Details; }

		const std::vector<std::shared_ptr<Utils::BaseLayer>> GetLayers() const { return m_ApplicationLayers; }
	private:
		std::vector<std::shared_ptr<Utils::BaseLayer>> m_ApplicationLayers;
		Details m_Details;
	};
}