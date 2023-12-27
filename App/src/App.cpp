#include <Main.h>
#include <UserApplication.h>
#include <Utils/Layers/BaseLayer.h>
#include <glm/matrix.hpp>

using namespace TGE;

class Example : public Utils::BaseLayer
{
public:
	virtual void OnInit() {};
	virtual void OnUpdate() 
	{
		Raytracing2D::ChangeSampleRate(SampleRate);
		
		{
			Circle circle{};
			circle.Position = { 0.0f, -501.5f, 0.0f, 1.0f };
			circle.Radius = { 500.0f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 0.3f, 0.3f, 0.3f, 1.0f };
			circle.Emission = { 0.2f, 0.2f, 0.2f, 0.0f };
			circle.Reflectivity = { 1.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = {0.0f, 0.0f, 0.0f, 0.0f };
			Raytracing2D::PushCircle(circle);
		}

		{
			Circle circle{};
			circle.Position = glm::vec4(CirclePosition1, 1.0f);
			circle.Radius = { 0.5f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
			circle.Emission = { CircleEmmissionTest, 0.0f };
			circle.Reflectivity = { 0.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = { 0.0f, 0.0f, 0.0f, 0.0f };

			Raytracing2D::PushCircle(circle);
		}

		{
			Circle circle{};
			circle.Position = { 3.0f, 0.0f, 0.0f, 1.0f };
			circle.Radius = { 0.5f, 0.0f, 0.0f, 0.0f };
			circle.Albedo = { 1.0f, 0.0f, 0.0f, 1.0f };
			circle.Emission = { 0.2f, 0.2f, 0.2f, 0.0f };
			circle.Reflectivity = { 0.0f, 0.0f, 0.0f, 0.0f };
			circle.Roughness = { 0.0f, 0.0f, 0.0f, 0.0f };
			Raytracing2D::PushCircle(circle);
		}


		Raytracing2D::Trace();
	};
	virtual void OnGUIUpdate() 
	{
		ImGui::Begin("User Editing");
		ImGui::SliderInt("Sample Rate", &SampleRate, 1, 100);
		ImGui::SliderFloat3("Pos", glm::value_ptr(CirclePosition1), -10.0f, 10.0f);
		ImGui::SliderFloat3("Emmission Test", glm::value_ptr(CircleEmmissionTest), 0.0f, 10.0f);
		ImGui::End();
	};
	virtual void OnShutdown() {};
private:
	int SampleRate = 2;
	glm::vec3 CirclePosition1 = { 2.0f, 0.0f, 0.0f };
	glm::vec3 CircleEmmissionTest = { 0.0f, 0.0f, 0.0f };
};

int main()
{
	Details AppDetails;
	AppDetails.ApplicationName = "TooGoodEngine";
	AppDetails.Width = 1600;
	AppDetails.Height = 900;

	UserApplication NewApp(AppDetails);
	NewApp.PushLayer(std::make_shared<Example>());

	return TGE::Main(NewApp);
}