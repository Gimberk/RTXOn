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
		camera.SetMouseSens(0.009f);
		camera.SetMoveSpeed(4.6f);

		Material& pink = scene.materials.emplace_back();
		pink.albedo = glm::vec3(1.0f, 0.0f, 1.0f);
		pink.roughness = 0.0f;
		pink.metallicness = 0.0f;

		Material& blue = scene.materials.emplace_back();
		blue.albedo = glm::vec3(0.2f, 0.3f, 1.0f);
		blue.roughness = 0.0f;
		blue.metallicness = 0.0f;

		Material& orange = scene.materials.emplace_back();
		orange.albedo = glm::vec3(0.8f, 0.5f, 0.2f);
		orange.roughness = 0.0f;
		orange.metallicness = 0.0f;

		{
			Sphere sphere;
			sphere.position = glm::vec3(0.0f);
			sphere.radius = 1.0f;
			sphere.matIndex = 0; // pink
			scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = glm::vec3(0.0f, -101.0f, 0.0f);
			sphere.radius = 100.0f;
			sphere.matIndex = 1; // bleu
			scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = glm::vec3(2.0f, 0.0f, 0.0f);
			sphere.radius = 1.0f;
			sphere.matIndex = 2; // bright stuff
			scene.spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override {
		if (camera.OnUpdate(ts)) renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Render took: %0.2fms", renderTime);
		if (ImGui::Button("Render")) renderFrame = 1;
		if (ImGui::Button("Pause")) renderFrame = 0;

		ImGui::Checkbox("Accumulate", &renderer.GetSettings().accumulate);
		if (ImGui::Button("Reset")) renderer.ResetFrameIndex();
		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < scene.spheres.size(); i++) {
			ImGui::PushID(i);

			Sphere& sphere = scene.spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
			ImGui::DragInt("Material", &sphere.matIndex, 1.0f, 0, 
							scene.materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}

		for (size_t i = 0; i < scene.materials.size(); i++) {
			ImGui::PushID(i);

			Material& material = scene.materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
			ImGui::DragFloat("Roughness", &material.roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallicness", &material.metallicness, 0.01f, 0.0f, 1.0f);
			ImGui::Checkbox("Light?", &material.light);
			if (material.light) {
				ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.emissionColor));
				ImGui::DragFloat("Emissiveness", &material.emissionPower, 0.05f, 0.0f, 
																			FLT_MAX);
			}

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