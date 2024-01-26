#include "pch.h"
#include "Window.h"




static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		TGE_LOG_INFO(message);
		TGE_CLIENT_LOG(message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		TGE_LOG_WARN(message);
		TGE_CLIENT_WARN(message);
		//TGE_HALT();
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		TGE_LOG_ERROR(message);
		TGE_CLIENT_ERROR(message);
		TGE_HALT();
		break;
	default:
		break;
	}
}

static void GLFWErrorCallBack(int error_code, const char* description)
{
	TGE_LOG_ERROR(error_code, " ", description);
	TGE_CLIENT_ERROR(error_code, " ", description);
}

namespace TooGoodEngine {

	Window::Window(const uint32_t& width, const uint32_t& height, const char* title)
	{
		Create(width, height, title);
		Init();
	}

	Window::~Window()
	{
		GLFWwindow* curr = glfwGetCurrentContext();

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
		TGE_LOG_INFO("Window is initalizing");
		TGE_CLIENT_LOG("Window is initalizing");

		TGE_FORCE_ASSERT(glfwInit(), "glfw failed to initalize");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, nullptr, nullptr);

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
			TGE_CLIENT_ERROR("window Failed To Create");

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

		TGE_LOG_INFO("OpenGL Version ",   version);
		TGE_CLIENT_LOG("OpenGL Version ", version);

		Input::BeginPolling(m_Window);
	}


	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	TooGoodEngine::Application::GetMainWindow().SetWidth(width);
	TooGoodEngine::Application::GetMainWindow().SetHeight(height);

	glViewport(0, 0, width, height);
}
