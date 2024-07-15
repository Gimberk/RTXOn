#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "primitives/Sphere.h"
#include "primitives/Triangle.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : camera(45.0f, 0.1f, 100.0f) {
		camera.SetMouseSens(0.009f);
		camera.SetMoveSpeed(4.6f);

		Material& sphere = scene.materials.emplace_back();
		sphere.albedo = glm::vec3(0.0f);
		sphere.metallicness = 0.0f;

		Material& triangle = scene.materials.emplace_back();
		triangle.albedo = glm::vec3(1.0f);
		triangle.metallicness = 0.0f;

		Material& floor = scene.materials.emplace_back();
		floor.albedo = glm::vec3(1.0f, 0.0f, 1.0f);
		floor.metallicness = 0.0f;

		{
			Sphere sphere;
			sphere.position = glm::vec3(2.0f, 0.0f, 0.0f);
			sphere.radius = 0.5f;
			sphere.matIndex = 2;
			scene.objects.push_back(std::make_shared<Sphere>(sphere));
		}

		{
			Sphere sphere;
			sphere.position = glm::vec3(-2000.0f, -100.5f, 0.0f);
			sphere.radius = 100.0f;
			sphere.matIndex = 2;
			scene.objects.push_back(std::make_shared<Sphere>(sphere));
		}

		{
			Triangle triangle(
				glm::vec3(2.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, -5.0f),
				glm::vec3(-2.0f, 0.0f, 0.0f)
			);
			triangle.matIndex = 1;
			scene.objects.push_back(std::make_shared<Triangle>(triangle));
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
		ImGui::Checkbox("Correct Gamma", &renderer.GetSettings().correctGamma);
		if (renderer.GetSettings().correctGamma) {
			ImGui::DragFloat("Gamma Correction", &renderer.GetSettings().gammaCorrection,
				0.2f, 0.25f, 2);
		}

		if (ImGui::Button("Reset")) renderer.ResetFrameIndex();
		ImGui::End();

		ImGui::Begin("Scene");
		for (int i = 0; i < scene.objects.size(); i++) {
			ImGui::PushID(i);
			switch (scene.objects[i]->GetType()) {
			case PrimitiveType::SPHERE:
				std::shared_ptr<Sphere> sphere = 
					std::dynamic_pointer_cast<Sphere>(scene.objects[i]);
				ImGui::DragFloat3("Position", glm::value_ptr(sphere->position), 0.1f);
				ImGui::DragFloat("Radius", &sphere->radius, 0.1f);
				ImGui::DragInt("Material", &sphere->matIndex, 1.0f, 0,
					scene.materials.size() - 1);
			}

			ImGui::Separator();

			ImGui::PopID();
		}

		for (int i = 0; i < scene.materials.size(); i++) {
			ImGui::PushID(i);

			Material& material = scene.materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
			ImGui::DragFloat("Metallicness", &material.metallicness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Specular Probability", 
				&material.specularProbability, 0.01f, 0.0f, 1.0f);
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