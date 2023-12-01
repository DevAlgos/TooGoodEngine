#include <Main.h>
#include <UserApplication.h>
#include <Utils/Layers/BaseLayer.h>
#include <glm/matrix.hpp>


class Example : public Utils::BaseLayer
{
public:
	virtual void OnInit() {};
	virtual void OnUpdate() {};
	virtual void OnGUIUpdate() {};
	virtual void OnShutdown() {};
private:
};

int main()
{
	TGE::Details AppDetails;
	AppDetails.ApplicationName = "TooGoodEngine";
	AppDetails.Width = 1600;
	AppDetails.Height = 900;

	TGE::UserApplication NewApp(AppDetails);
	NewApp.PushLayer(std::make_shared<Example>());

	return TGE::Main(NewApp);
}