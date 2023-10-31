#include <pch.h>

#include "LayerManager.h"

namespace Utils
{
	LayerManager::LayerManager()
		: m_LayerIndex(0)
	{
	}
	LayerManager::~LayerManager()
	{
		for (size_t i = 0; i < m_LayerIndex; i++)
			m_LayerStack[i]->OnShutdown();
	}
	void LayerManager::PushLayer(std::unique_ptr<BaseLayer> layer)
	{
		m_LayerStack.push_back(std::move(layer));
		m_LayerIndex++;
	}
	void LayerManager::PopLayer()
	{
		m_LayerStack[m_LayerIndex].release();
		m_LayerIndex--;
	}
	void LayerManager::UpdateLayers()
	{
		for (size_t i = 0; i < m_LayerIndex; i++)
			m_LayerStack[i]->OnUpdate();

		for (size_t i = 0; i < m_LayerIndex; i++)
			m_LayerStack[i]->OnGUIUpdate();
	}
}
