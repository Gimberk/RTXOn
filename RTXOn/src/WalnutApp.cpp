#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Util/ModelLoader.h"

#include "Renderer.h"
#include "primitives/Quad.h"
#include "primitives/Sphere.h"
#include "primitives/Triangle.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

VkDevice device;
VkPhysicalDevice physicalDevice;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : camera(40.0f, 0.1f, 100.0f) {
		//camera.SetMouseSens(0.009f);
		//camera.SetMoveSpeed(4.6f);
		// Materials

		//Material& sphere = scene.materials.emplace_back();
		//sphere.albedo = glm::vec3(0.0f);
		//sphere.metallicness = 0.7843f;

		//Material& purple = scene.materials.emplace_back();
		//purple.albedo = glm::vec3(0.6f, 0.0f, 0.7f);
		//purple.metallicness = 0.0f;

		//// Objects
		//{
		//	Sphere sphere(glm::vec3(0.0f), 1.234f);
		//	sphere.matIndex = 0;
		//	scene.objects.push_back(std::make_shared<Sphere>(sphere));
		//}

		//{
		//	Sphere sphere(glm::vec3(2.75f, 0.0f, 0.0f), 1.234f);
		//	sphere.matIndex = 1;
		//	scene.objects.push_back(std::make_shared<Sphere>(sphere));
		//}

		Mesh pawn = ModelLoader::LoadModel("low-poly-pawn\\pawn.obj");
		if (pawn.GetType() != PrimitiveType::TRIANGLE) { // ensure we successfully built the mesh
			pawn.matIndex = 4;
			pawn.SetObjectIndex(0);
			scene.objects.push_back(std::make_shared<Mesh>(pawn));
		}

		Mesh king = ModelLoader::LoadModel("low-poly-king\\king.obj");
		king.SetPosition(glm::vec3(3, 0, 0), true);
		if (king.GetType() != PrimitiveType::TRIANGLE) { // ensure we successfully built the mesh
			king.matIndex = 5;
			king.SetObjectIndex(0);
			scene.objects.push_back(std::make_shared<Mesh>(king));
		}

		camera.SetMouseSens(0.01);
		camera.SetMoveSpeed(40.0f);
		Material& b = scene.materials.emplace_back();
		b.albedo = glm::vec3(0.12f, 0.45f, 0.15f);
		b.metallicness = 0.0f;
		Material& d = scene.materials.emplace_back();
		d.albedo = glm::vec3(0.65f, 0.05f, 0.05f);
		d.metallicness = 0.0f;
		Material& f = scene.materials.emplace_back();
		f.albedo = glm::vec3(1.0f);
		f.emissionPower = 1000.0f;
		f.light = true;
		f.emissionColor = glm::vec3(1.0f);
		Material& c = scene.materials.emplace_back();
		c.albedo = glm::vec3(0.73f);
		c.metallicness = 0.0f;

		Material& sphere = scene.materials.emplace_back();
		sphere.albedo = glm::vec3(0.73f);
		sphere.metallicness = 0.0f;

		Material& a = scene.materials.emplace_back();
		a.albedo = glm::vec3(0, 0, 0.73f);
		a.metallicness = 0.0f;

		//{
		//	Triangle tri(
		//		glm::vec3(-2.0f, 0.0f, 0.0f),
		//		glm::vec3(0.0f, 5.0f, 0.0f),
		//		glm::vec3(2.0f, 0.0f, 0.0f)
		//	);
		//	tri.matIndex = 1;
		//	scene.objects.push_back(std::make_shared<Triangle>(tri));
		//}

		double factor = 22.0f;
		{
			Quad quad(glm::vec3(555 / factor, 0, 0), glm::vec3(0, 555 / factor, 0), glm::vec3(0, 0, 555 / factor));
			quad.matIndex = 0;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}
		{
			Quad quad(glm::vec3(0), glm::vec3(0, 555 / factor, 0), glm::vec3(0, 0, 555 / factor));
			quad.matIndex = 1;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}
		{
			Quad quad(glm::vec3(343 / factor, 554 / factor, 332 / factor), glm::vec3(-130 / factor, 0, 0), glm::vec3(0, 0, -105 / factor));
			quad.matIndex = 2;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}
		{
			Quad quad(glm::vec3(0), glm::vec3(555 / factor, 0, 0), glm::vec3(0, 0, 555 / factor));
			quad.matIndex = 3;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}
		{
			Quad quad(glm::vec3(555 / factor), glm::vec3(-555 / factor, 0, 0), glm::vec3(0, 0, -555 / factor));
			quad.matIndex = 3;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}
		{
			Quad quad(glm::vec3(0, 0, 555 / factor), glm::vec3(555 / factor, 0, 0), glm::vec3(0, 555 / factor, 0));
			quad.matIndex = 3;
			scene.objects.push_back(std::make_shared<Quad>(quad));
		}

	}

	virtual void OnUpdate(float ts) override {
		if (camera.OnUpdate(ts)) renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Render took: %0.2fms", renderTime);
		ImGui::DragFloat("Move Speed", &camera.speed);
		if (ImGui::Button("Render")) renderFrame = 1;
		if (ImGui::Button("Pause")) renderFrame = 0;

		ImGui::DragInt("Bounce Limit", 
			&renderer.GetSettings().bounceLimit, 1, 1, 1000);

		ImGui::Checkbox("Accumulate", &renderer.GetSettings().accumulate);
		ImGui::Checkbox("Correct Gamma", &renderer.GetSettings().correctGamma);
		ImGui::Checkbox("Render Bounding Boxes", &renderer.GetSettings().renderBoundingBoxes);
		if (renderer.GetSettings().correctGamma) {
			ImGui::DragFloat("Gamma Correction", &renderer.GetSettings().gammaCorrection,
				0.2f, 0.25f, 2);
		}

		if (ImGui::Button("Reset")) renderer.ResetFrameIndex();
		ImGui::End();

		ImGui::Begin("Scene");
		for (int i = 0; i < scene.objects.size(); i++) {
			ImGui::PushID(i);
			if (scene.objects[i]->GetType() == PrimitiveType::SPHERE) {
				std::shared_ptr<Sphere> sphere =
					std::dynamic_pointer_cast<Sphere>(scene.objects[i]);
				ImGui::DragFloat3("Position", glm::value_ptr(sphere->position), 0.1f);
				ImGui::DragFloat("Radius", &sphere->radius, 0.1f);
				ImGui::DragInt("Material", &sphere->matIndex, 1.0f, 0,
					scene.materials.size() - 1);
			}
			else if (scene.objects[i]->GetType() == PrimitiveType::MESH){
				std::shared_ptr<Mesh> mesh =
					std::dynamic_pointer_cast<Mesh>(scene.objects[i]);
				ImGui::DragFloat3("Position", glm::value_ptr(mesh->offset), 0.1f);
				ImGui::DragInt("Material", &mesh->matIndex, 1.0f, 0,
					scene.materials.size() - 1);

				if (ImGui::Button("~Reset Position~")) mesh->SetPosition(mesh->offset, true);
				if (ImGui::Button("~Reset Position -~")) mesh->SetPosition(mesh->offset, false);
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

		for (int i = 0; i < scene.objects.size(); i++) {
			scene.objects[i]->OnRender();
		}
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

	device = app->GetDevice();
	physicalDevice = app->GetPhysicalDevice();

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