#pragma once


namespace Utils
{
	class BaseLayer //Base class for every single type of layer
	{
	public:
		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnGUIUpdate() = 0;
		virtual void OnShutdown() = 0;
	};
}