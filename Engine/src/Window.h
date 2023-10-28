#pragma once
#include <pch.h>

namespace Graphics {

	class Window
	{
	public:
		Window(const uint16_t& width, const uint16_t& height, const char* title);
		Window() = default;
		~Window();

		void Create(const uint16_t& width, const uint16_t& height, const char* title);
		void Init();

		inline void SwapBuffers() const { glfwSwapBuffers(m_Window); }
		inline void PollEvents() { glfwPollEvents(); }


		inline const bool WindowClosed() const { return m_WindowClosed; }

		inline const uint16_t GetWidth() const {return m_Width; }
		inline const uint16_t GetHeight() const { return m_Height; }

		inline void SetWidth(int NewWidth) { m_Width = NewWidth; }
		inline void SetHeight(int NewHeight) { m_Height = NewHeight; }


		inline GLFWwindow* GetWindow() { return m_Window; }

	private:
		uint32_t m_Width, m_Height;
		bool m_WindowClosed;
		const char* m_Title;
		GLFWwindow* m_Window;
	};

}