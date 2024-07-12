#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include "Walnut/Timer.h"

#include "../src/Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : camera(45.0f, 0.1f, 100.0f) {
		camera.SetMouseSens(0.004f);
		camera.SetMoveSpeed(4.6f);

		{
			Sphere sphere;
			sphere.albedo = glm::vec3(1.0f, 0.0f, 1.0f);
			sphere.position = glm::vec3(0.0f);
			sphere.radius = 0.5f;
			scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.albedo = glm::vec3(0.2f, 0.3f, 1.0f);
			sphere.position = glm::vec3(1.0f, 0.0f, -5.0f);
			sphere.radius = 1.5f;
			scene.spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override {
		camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Render took: %0.2fms", renderTime);
		if (ImGui::Button("Render")) renderFrame = 1;
		if (ImGui::Button("Pause")) renderFrame = 0;
		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < scene.spheres.size(); i++) {
			ImGui::PushID(i);

			Sphere& sphere = scene.spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.albedo));

			ImGui::Separator();

			ImGui::PopID();
		}
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
		camera.OnResize(viewportWidth, viewportHeight);
		renderer.Render(scene, camera);
		renderTime = timer.ElapsedMillis();
	}
private:
	Renderer renderer;
	Camera camera;
	Scene scene;

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