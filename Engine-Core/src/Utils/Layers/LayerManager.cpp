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
	void LayerManager::PushLayer(std::shared_ptr<BaseLayer> layer)
	{
		m_LayerStack.push_back(layer);
		m_LayerStack[m_LayerIndex]->OnInit();
		m_LayerIndex++;
	}
	void LayerManager::PopLayer()
	{
		m_LayerStack[m_LayerIndex]->OnShutdown();
		m_LayerStack[m_LayerIndex].reset();
		m_LayerStack.pop_back();
		m_LayerIndex--;
	}
	void LayerManager::UpdateLayers()
	{
		for (size_t i = 0; i < m_LayerIndex; i++)
			m_LayerStack[i]->OnUpdate();
	}
	void LayerManager::UpdateGUI()
	{
		for (size_t i = 0; i < m_LayerIndex; i++)
			m_LayerStack[i]->OnGUIUpdate();
	}
}
