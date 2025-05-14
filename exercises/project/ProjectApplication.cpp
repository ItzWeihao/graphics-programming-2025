#include "ProjectApplication.h"

#include <ituGL/asset/TextureCubemapLoader.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>

#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/lighting/PointLight.h>
#include <ituGL/lighting/SpotLight.h>
#include <ituGL/scene/SceneLight.h>

#include <ituGL/shader/ShaderUniformCollection.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/scene/SceneModel.h>

#include <ituGL/renderer/SkyboxRenderPass.h>
#include <ituGL/renderer/ForwardRenderPass.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>

#include <ituGL/scene/ImGuiSceneVisitor.h>
#include <imgui.h>
#include <iostream>
#include <glm/glm.hpp>

ProjectApplication::ProjectApplication()
    : Application(1024, 1024, "Lighthouse")
    , m_renderer(GetDevice())
    , m_exposure(1.0f)
    , m_contrast(1.0f)
    , m_hueShift(0.0f)
    , m_saturation(1.0f)
    , m_colorFilter(1.0f)
    , m_blurIterations(1)
    , m_bloomRange(1.0f, 2.0f)
    , m_bloomIntensity(1.0f)
{
}

void ProjectApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeLights();

    InitializeMaterials();
   
    InitializeModels();
    InitializeRenderer();
}

void ProjectApplication::Update()
{
    Application::Update();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Add the scene nodes to the renderer
    RendererSceneVisitor rendererSceneVisitor(m_renderer);
    m_scene.AcceptVisitor(rendererSceneVisitor);
}

void ProjectApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();
}

void ProjectApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void ProjectApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(11, 8, 12), glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0));
    camera->SetPerspectiveProjectionMatrix(1.0f, 1.0f, 0.1f, 500.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Add the camera node to the scene
    m_scene.AddSceneNode(sceneCamera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void ProjectApplication::InitializeLights()
{
    // Create a directional light and add it to the scene
    std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>();
    directionalLight->SetDirection(glm::vec3(0.0f, -1.0f, -0.3f)); // It will be normalized inside the function
    directionalLight->SetIntensity(1.0f);
    m_scene.AddSceneNode(std::make_shared<SceneLight>("directional light", directionalLight));

    // Create a point light and add it to the scene
    std::shared_ptr<PointLight> pointLight0 = CreatePointLight(glm::vec3(4.8f, -0.7f, -3.7f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 0", pointLight0));

    std::shared_ptr<PointLight> pointLight1 = CreatePointLight(glm::vec3(2.4f, -0.7f, -3.7f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 1", pointLight1));

    std::shared_ptr<PointLight> pointLight2 = CreatePointLight(glm::vec3(4.4f, -4.8f, -7.5f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 2", pointLight2));

    std::shared_ptr<PointLight> pointLight3 = CreatePointLight(glm::vec3(2.65f, 2.0f, 0.0f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 3", pointLight3));

    std::shared_ptr<PointLight> pointLight4 = CreatePointLight(glm::vec3(2.15f, 2.6f, 4.35f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 4", pointLight4));

    std::shared_ptr<PointLight> pointLight5 = CreatePointLight(glm::vec3(-1.00f, 1.3f, 0.1f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 5", pointLight5));

    std::shared_ptr<PointLight> pointLight6 = CreatePointLight(glm::vec3(0.70f, 0.4f, -1.75f), glm::vec3(1.0f, 0.98f, 0.5f), 8.0f, glm::vec2(0.5f, 1.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light 6", pointLight6));

    // Create a spot light and add it to the scene
    std::shared_ptr<SpotLight> spotLight = std::make_shared<SpotLight>();
    spotLight->SetPosition(glm::vec3(1.0f, 5.35f, 3.0f));
    spotLight->SetDistanceAttenuation(glm::vec2(1.0f, 10.0f));
    spotLight->SetIntensity(20.0f);
    spotLight->SetDirection(glm::vec3(0, 0, 1));
    spotLight->SetAngle(glm::cos(glm::radians(0.5f)));
    spotLight->SetColor(glm::vec3(1.0f, 0.98f, 0.5f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("spot light 0", spotLight));
}

void ProjectApplication::InitializeMaterials()
{
    // Load and build shader
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/default.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/lambert-ggx.glsl");
    fragmentShaderPaths.push_back("shaders/lighting.glsl");
    fragmentShaderPaths.push_back("shaders/default_pbr.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Get transform related uniform locations
    ShaderProgram::Location cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
    ShaderProgram::Location worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");

    // Register shader with renderer
    m_renderer.RegisterShaderProgram(shaderProgramPtr,
        [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
                shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
            }
            shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
        },
        m_renderer.GetDefaultUpdateLightsFunction(*shaderProgramPtr)
    );

    // Filter out uniforms that are not material properties
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("CameraPosition");
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("LightIndirect");
    filteredUniforms.insert("LightColor");
    filteredUniforms.insert("LightPosition");
    filteredUniforms.insert("LightDirection");
    filteredUniforms.insert("LightAttenuation");

    // Create reference material
    assert(shaderProgramPtr);
    m_defaultMaterial = std::make_shared<Material>(shaderProgramPtr, filteredUniforms);
}

void ProjectApplication::InitializeModels()
{
    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/skybox.png", TextureObject::FormatRGB, TextureObject::InternalFormatRGB16F);

    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    TextureCubemapObject::Unbind();

    // Set the environment texture on the default material
    m_defaultMaterial->SetUniformValue("AmbientColor", glm::vec3(0.25f));

    m_defaultMaterial->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
    m_defaultMaterial->SetUniformValue("EnvironmentMaxLod", maxLod);

    m_defaultMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_defaultMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);

    // Configure loader
    ModelLoader loader(m_defaultMaterial);

    // Create a new material copy for each submaterial
    loader.SetCreateMaterials(true);

    // Flip vertically textures loaded by the model loader
    loader.GetTexture2DLoader().SetFlipVertical(true);

    // Link vertex properties to attributes
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Tangent, "VertexTangent");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Bitangent, "VertexBitangent");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");

    // Link material properties to uniforms
    loader.SetMaterialProperty(ModelLoader::MaterialProperty::DiffuseColor, "Color");
    loader.SetMaterialProperty(ModelLoader::MaterialProperty::DiffuseTexture, "ColorTexture");
    loader.SetMaterialProperty(ModelLoader::MaterialProperty::NormalTexture, "NormalTexture");
    loader.SetMaterialProperty(ModelLoader::MaterialProperty::SpecularTexture, "SpecularTexture");

    // Load models
    std::shared_ptr<Model> lighthouseModel = loader.LoadShared("models/lighthouse/lighthouseobj.obj");
    m_scene.AddSceneNode(std::make_shared<SceneModel>("light house", lighthouseModel));
}

void ProjectApplication::InitializeRenderer()
{
    int width, height;
    GetMainWindow().GetDimensions(width, height);

    // Skybox pass
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));

    // Forward rendering
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
}

std::shared_ptr<Material> ProjectApplication::CreatePostFXMaterial(const char* fragmentShaderPath, std::shared_ptr<Texture2DObject> sourceTexture)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    material->SetUniformValue("SourceTexture", sourceTexture);

    return material;
}

Renderer::UpdateTransformsFunction ProjectApplication::GetFullscreenTransformFunction(std::shared_ptr<ShaderProgram> shaderProgramPtr) const
{
    // Get transform related uniform locations
    ShaderProgram::Location invViewMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewMatrix");
    ShaderProgram::Location invProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ProjMatrix");
    ShaderProgram::Location worldViewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldViewProjMatrix");

    // Return transform function
    return [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                shaderProgram.SetUniform(invViewMatrixLocation, (camera.GetViewMatrix()));
                shaderProgram.SetUniform(invProjMatrixLocation, (camera.GetProjectionMatrix()));
            }
            shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
        };
}

void ProjectApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for scene nodes, using the visitor pattern
    ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
    m_scene.AcceptVisitor(imGuiVisitor);

    // Draw GUI for camera controller
    m_cameraController.DrawGUI(m_imGui);

    m_imGui.EndFrame();
}

std::shared_ptr<PointLight> ProjectApplication::CreatePointLight(glm::vec3 position, glm::vec3 color, float intensity, glm::vec2 attenuation)
{
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(position);
    pointLight->SetColor(color);
    pointLight->SetIntensity(intensity);
    pointLight->SetDistanceAttenuation(attenuation);

    return pointLight;
}
