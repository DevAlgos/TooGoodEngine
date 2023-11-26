#pragma once

namespace Utils
{
	struct MemoryData
	{
		size_t AllocatedMemory;
		size_t FreedMemory;

		size_t CurrentlyUsingMemory() { return AllocatedMemory - FreedMemory; }
	};


	class DebuggingLayer : public BaseLayer
	{
	public:
		DebuggingLayer() = default;
		virtual ~DebuggingLayer() = default;

		virtual void OnInit() override;
		virtual void OnUpdate() override;
		virtual void OnGUIUpdate() override;
		virtual void OnShutdown() override;

	private:
		long long m_Framerate;
		int m_FrameCount;
		long long m_Start, m_Finish;
		long long m_Delta, m_ElapsedTime;

	};
}