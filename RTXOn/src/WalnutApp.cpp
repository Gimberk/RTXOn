#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include "Walnut/Timer.h"

#include "../src/Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Render took: %0.2fms", renderTime);
		if (ImGui::Button("Render")) renderFrame = 1;
		if (ImGui::Button("Pause")) renderFrame = 0;
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		viewportWidth = ImGui::GetContentRegionAvail().x;
		viewportHeight = ImGui::GetContentRegionAvail().y;

		std::shared_ptr<Image> img = renderer.GetFinalImage();
		if (img) ImGui::Image(img->GetDescriptorSet(),
				   { (float)img->GetWidth(), 
					 (float)img->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		if (renderFrame) Render();
	}

	void Render() {
		Timer timer;

		renderer.OnResize(viewportWidth, viewportHeight);
		renderer.Render();
		renderTime = timer.ElapsedMillis();
	}
private:
	Renderer renderer;
	
	uint32_t viewportWidth = 0, viewportHeight = 0;

	float renderTime = 0.0f;
	bool renderFrame = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RTXOn!";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}