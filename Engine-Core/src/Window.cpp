#include "pch.h"
#include "Window.h"




static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		LOG(message);
		LOG_CORE(message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOGWARNING(message);
		LOG_CORE_WARNING(message);

		break;
	case GL_DEBUG_SEVERITY_HIGH:
		LOGERROR(message);
		LOG_CORE_ERROR(message);
		break;
	default:
		break;
	}
}

static void GLFWErrorCallBack(int error_code, const char* description)
{
	std::string msg = std::to_string(error_code) + " " + description;
	LOGERROR(msg);
	LOG_CORE_ERROR(msg);
}

namespace TGE {

	Window::Window(const uint32_t& width, const uint32_t& height, const char* title)
	{
		Create(width, height, title);
		Init();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void Window::Create(const uint32_t& width, const uint32_t& height, const char* title)
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
		LOG_CORE("Window is initalizing");

		if (!glfwInit())
		{
			LOGERROR("glfw Failed To Initalize");
			LOG_CORE_ERROR("glfw Failed To Initalize");
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, nullptr, nullptr);

		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		int width, height, channels;
		uint8_t* pixels = stbi_load("../Resources/Images/Icon.png", &width, &height, &channels, 4); 

		if (pixels) 
		{
			GLFWimage icon;
			icon.width = width;
			icon.height = height;
			icon.pixels = pixels;

			glfwSetWindowIcon(m_Window, 1, &icon);
			stbi_image_free(pixels);
		}


		if (!m_Window)
			LOG_CORE_ERROR("window Failed To Create");

		glfwSetWindowUserPointer(m_Window, this);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
				self->m_WindowClosed = true;
			});

		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

		glfwSetErrorCallback(GLFWErrorCallBack);
				
		glfwMakeContextCurrent(m_Window);
		gladLoadGL();

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLDebugCallback, nullptr);
		
		const char* version = (const char*)glGetString(GL_VERSION);
		std::string msg = "OpenGL Version: " + std::string(version);
		LOG_CORE(msg);
		LOG(msg);

		InputManager::BeginPolling(m_Window);
	}


	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	TGE::Application::GetMainWindow().SetWidth(width);
	TGE::Application::GetMainWindow().SetHeight(height);

	glViewport(0, 0, width, height);
}
