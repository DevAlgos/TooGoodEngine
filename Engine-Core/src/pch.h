#pragma once

#define AL_LIBTYPE_STATIC 
#include <AL/al.h>
#include <AL/alext.h>

#include <glad/glad.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <imgui.h>
#include <impl_imgui/imgui_impl_glfw.h>
#include <impl_imgui/imgui_impl_opengl3.h>


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <thread>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <exception>
#include <map>
#include <random>
#include <execution>

#include <ECS/Component.h>
#include <ECS/Entity.h>

#include <Physics/PhysicsScene.h>
#include <Particles/ParticleScene.h>

#include <Audio/Audio.h>

#include <Utils/RandomNumbers.h>
#include <Utils/Clock.h>
#include <Utils/Layers/BaseLayer.h>
#include <Utils/Layers/LayerManager.h>
#include <Utils/Layers/EditorLayer.h>

#include <Utils/Threading/PriorityQueue.h>
#include <Utils/Threading/CommandQueue.h>

#include <Utils/Utils.h>
#include <Utils/InputManager.h>
#include <Utils/Camera.h>
#include <Utils/Logger.h>
#include <Window.h>

#include <Graphics/Shader.h>
#include <Graphics/VertexArray.h>
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>
#include <Graphics/Renderer2D.h>

#include <Application.h>


