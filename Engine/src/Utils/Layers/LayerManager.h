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

		void PushLayer(std::unique_ptr<BaseLayer> layer);
		void PopLayer();

		void UpdateLayers();

	private:
		std::vector<std::unique_ptr<BaseLayer>> m_LayerStack;
		size_t m_LayerIndex;
	};
}