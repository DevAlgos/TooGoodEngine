#include <pch.h>
#include "UserApplication.h"

namespace tge
{
	UserApplication::UserApplication(const Details& details)
		: m_Details(details)
	{
	}
	UserApplication::~UserApplication()
	{
	}
	void UserApplication::PushLayer(std::shared_ptr<Utils::BaseLayer> Layer)
	{
		m_ApplicationLayers.push_back(Layer);
	}
	void UserApplication::PopLayer()
	{
		m_ApplicationLayers.back().reset();
		m_ApplicationLayers.pop_back();
	}

}