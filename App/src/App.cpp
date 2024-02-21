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
		{
			Circle circle{};
			circle.Position = { 0.0f, -500.5f, 0.0f, 1.0f };
			circle.Radius = { 500.0f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 0.3f, 0.3f, 0.3f, 1.0f };
			circle.Emission = { 1.0f, 1.0f, 1.0f, 0.0f };
			circle.Reflectivity = { 1.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = { 0.0f, 0.0f, 0.0f, 0.0f };
			Copy.push_back(circle);
		}

		{
			Circle circle{};
			circle.Position = glm::vec4(CirclePosition1, 1.0f);
			circle.Radius = { 0.5f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
			circle.Emission = { 0.0f, 0.0f, 0.0f, 0.0f };
			circle.Reflectivity = { 0.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = { 0.0f, 0.0f, 0.0f, 0.0f };

			Copy.push_back(circle);

		}

		{
			Circle circle{};
			circle.Position = { 3.0f, 0.0f, 0.0f, 1.0f };
			circle.Radius = { 0.5f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 1.0f, 0.0f, 0.0f, 1.0f };
			circle.Emission = { 1.0f, 1.0f, 1.0f, 0.0f };
			circle.Reflectivity = { 0.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = { Roughness, 0.0f, 0.0f, 0.0f };
			Copy.push_back(circle);

		}
	};
	virtual void OnUpdate() override
	{
		/*Raytracing2D::ChangeSampleRate(SampleRate);
		
		for (const Circle& circle : Copy)
			Raytracing2D::PushCircle(circle);
		
		Raytracing2D::Trace();*/
	};
	virtual void OnGUIUpdate() override
	{
		/*ImGui::Begin("User Editing");
		ImGui::Columns(Copy.size() - 1);
		ImGui::SliderInt("Samples: ",&SampleRate,  1, 100);

		int i = 0;
		for (Circle& circle : Copy)
		{
			ImGui::PushID(i);
			ImGui::SliderFloat("Roughness", &circle.Roughness.x, 0.0f, 1.0f);
			ImGui::SliderFloat3("Position", glm::value_ptr(circle.Position), -10.0f, 10.0f);
			ImGui::SliderFloat3("Emmission", glm::value_ptr(circle.Emission), 0.0f, 10.0f);
			ImGui::SliderFloat3("Albedo", glm::value_ptr(circle.Albedo), 0.0f, 1.0f);
			ImGui::SliderFloat3("Reflectivity", glm::value_ptr(circle.Reflectivity), 0.0f, 1.0f);
			ImGui::PopID();
			ImGui::NextColumn();
			i++;
		}

		ImGui::End();*/
	};
	virtual void OnShutdown() override {};
private:
	int SampleRate = 2;
	glm::vec3 CirclePosition1 = { 2.0f, 0.0f, 0.0f };
	glm::vec3 CircleEmmissionTest = { 0.0f, 0.0f, 0.0f };
	float Roughness = 0.0f;
	std::vector<Circle> Copy;
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

	/*Ecs::TransformComponent TestInstanceTransform({ 0.0f, 30.0f, 5.0f },
		{ 10.0f, 10.0f, 10.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f);

	TooGoodEngine::AssimpImporter importer;

	TooGoodEngine::Model TestModel = importer.ImportModel("mrmadman.obj");

	test::TestBVHBuilder builder;

	builder.AddModel(TestModel, TestInstanceTransform.Transform);

	TestInstanceTransform = Ecs::TransformComponent({ 0.0f, 50.0f, 5.0f },
		{ 10.0f, 10.0f, 10.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f);

	builder.AddModel(TestModel, TestInstanceTransform.Transform);

	builder.BuildAndDispatch();*/

	

}