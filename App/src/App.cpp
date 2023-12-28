#include <Main.h>
#include <UserApplication.h>
#include <Utils/Layers/BaseLayer.h>
#include <glm/matrix.hpp>

using namespace tge;

class Example : public Utils::BaseLayer
{
public:
	virtual void OnInit() 
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
	virtual void OnUpdate() 
	{
		Raytracing2D::ChangeSampleRate(SampleRate);
		
		for (const Circle& circle : Copy)
			Raytracing2D::PushCircle(circle);
		
		Raytracing2D::Trace();
	};
	virtual void OnGUIUpdate() 
	{
		ImGui::Begin("User Editing");

		ImGui::Columns(Copy.size() - 1);

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

		ImGui::End();
	};
	virtual void OnShutdown() {};
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
	/*Details AppDetails;
	AppDetails.ApplicationName = "TooGoodEngine";
	AppDetails.Width = 1600;
	AppDetails.Height = 900;

	UserApplication NewApp(AppDetails);
	NewApp.PushLayer(std::make_shared<Example>());

	return tge::Main(NewApp);*/

	Ecs::Entity entity;

	Ecs::Registry registry;
	registry.Emplace<ExampleType>(entity, 10.0f, 50.0f);

	registry.View<ExampleType>([](auto component) 
		{
			std::cout << component->a << " " << component->b << std::endl;
		});

	registry.Delete<ExampleType>(entity);

	registry.View<ExampleType>([](auto component)
		{
			std::cout << component->a << " " << component->b << std::endl;
		});


	/*Py_Initialize();

	FILE* file = nullptr;
	fopen_s(&file, "../Resources/TestScripts/Test.py", "r");

	if (file)
	{
		PyRun_AnyFileEx(file, "../Resources/TestScripts/Test.py", 1);
		fclose(file);
	}
	else
	{
		std::cout << "incorrect path" << std::endl;
	}

	Py_Finalize();*/
}