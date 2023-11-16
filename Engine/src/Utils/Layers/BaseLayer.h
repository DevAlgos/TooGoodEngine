#pragma once


namespace Utils
{
	class BaseLayer //Base class for every single type of layer
	{
	public:
		virtual void OnInit() = 0;			//required
		virtual void OnUpdate() = 0;		//required
		virtual void OnGUIUpdate() = 0;		//required
		virtual void OnShutdown() = 0;		//required
	};	//The required may be unused but should be defined somewhere
}