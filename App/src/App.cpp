#include <Main.h>
#include <ECS/Registry.h>
#include <UserApplication.h>
#include <Utils/Layers/BaseLayer.h>
#include <glm/matrix.hpp>
#include <filesystem>

#include <Graphics/ModeImporting/AssimpImporter.h>

using namespace TooGoodEngine;

class Example : public Utils::BaseLayer
{
public:
	virtual void OnInit() override
	{
		
	};
	virtual void OnUpdate() override
	{
	};
	virtual void OnGUIUpdate() override
	{
	};
	virtual void OnShutdown() override {};
private:
	int SampleRate = 2;
	glm::vec3 CirclePosition1 = { 2.0f, 0.0f, 0.0f };
	glm::vec3 CircleEmmissionTest = { 0.0f, 0.0f, 0.0f };
	float Roughness = 0.0f;
};


struct ExampleType
{
	float a, b;

	ExampleType(float Oa, float Ob) : a(Oa), b(Ob) {}
};

int main()
{
	Details AppDetails;
	AppDetails.ApplicationName = "TooGoodEngine";
	AppDetails.Width = 1600;
	AppDetails.Height = 800;
	AppDetails.PythonEnviromentPath = "../Resources/Scripts/";

	UserApplication NewApp(AppDetails);
	NewApp.PushLayer(std::make_shared<Example>());

	return TooGoodEngine::Main(NewApp);
	

}