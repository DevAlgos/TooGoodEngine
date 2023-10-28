#include "pch.h"
#include "Window.h"




static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		LOG(message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOGWARNING(message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		LOGERROR(message);
		break;
	default:
		LOG(message);
	}
}



namespace Graphics {

	Window::Window(const uint16_t& width, const uint16_t& height, const char* title)
	{
		Create(width, height, title);
		Init();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Window::Create(const uint16_t& width, const uint16_t& height, const char* title)
	{
		m_Width = width;
		m_Height = height;
		m_Title = title;
		m_Window = nullptr;
		m_WindowClosed = false;

	}

	void Window::Init()
	{

		LOG("Window is initalizing");

		if (!glfwInit())
			LOGERROR("glfw Failed To Initalize");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, nullptr, nullptr);

		if (!m_Window)
			LOGERROR("window Failed To Create");

		glfwSetWindowUserPointer(m_Window, this);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
				self->m_WindowClosed = true;
			});

		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
		
		glfwMakeContextCurrent(m_Window);
		gladLoadGL();

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(OpenGLDebugCallback, nullptr);
		
		const char* version = (const char*)glGetString(GL_VERSION);
		std::string msg = "OpenGL Version: " + std::string(version);
		LOG(msg);

		InputManager::BeginPolling(m_Window);
	}


	
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Application::GetMainWindow().SetWidth(width);
	Application::GetMainWindow().SetHeight(height);

	glViewport(0, 0, width, height);
}
