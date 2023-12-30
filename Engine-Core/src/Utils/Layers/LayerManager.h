#pragma once

#include "BaseLayer.h"
#include <vector>
#include <memory>

namespace Utils
{
	class LayerManager
	{
	public:
		LayerManager();
		~LayerManager();

		void PushLayer(std::shared_ptr<BaseLayer> layer);
		void PopLayer();

		void UpdateLayers();
		void UpdateGUI();
		

	private:
		std::vector<std::shared_ptr<BaseLayer>> m_LayerStack;
		size_t m_LayerIndex;
	};
}