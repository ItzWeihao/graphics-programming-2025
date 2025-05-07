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
#include <ituGL/scene/RendererSceneVisitor.h>

#include <ituGL/scene/ImGuiSceneVisitor.h>
#include <imgui.h>

ProjectApplication::ProjectApplication()
    : Application(1024, 1024, "Scene Viewer demo")
    , m_renderer(GetDevice())
{
}

void ProjectApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeLights();
    InitializeMaterial();
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
    camera->SetViewMatrix(glm::vec3(100, 100, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    camera->SetPerspectiveProjectionMatrix(1.0f, 1.0f, 0.1f, 2000.0f);

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
    directionalLight->SetDirection(glm::vec3(-0.3f, -1.0f, -0.3f)); // It will be normalized inside the function
    directionalLight->SetIntensity(1.0f);
    m_scene.AddSceneNode(std::make_shared<SceneLight>("directional light", directionalLight));

    // Create a point light and add it to the scene
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(29.800, -4.700, -23.100));
    pointLight->SetDistanceAttenuation(glm::vec2(5.0f, 10.0f));
    pointLight->SetIntensity(20.f);
    pointLight->SetColor(glm::vec3(1.0f, 0.86275f, 0.53333f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("point light", pointLight));

    // Create a point light and add it to the scene
    std::shared_ptr<SpotLight> spotLight = std::make_shared<SpotLight>();
    spotLight->SetPosition(glm::vec3(0, 0, 0));
    spotLight->SetDirection(glm::vec3(0.0f, -1.0f, 0.0f));
    spotLight->SetDistanceAttenuation(glm::vec2(5.0f, 10.0f));
    m_scene.AddSceneNode(std::make_shared<SceneLight>("spot light", spotLight));
}

void ProjectApplication::InitializeMaterial()
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

    // Loading unlit version of shader for Light Cone
    /*std::vector<const char*> unlitVertexShaderPaths;
    unlitVertexShaderPaths.push_back("shaders/version330.glsl");
    unlitVertexShaderPaths.push_back("shaders/unlit.vert");
    Shader unlitVertexShader = ShaderLoader(Shader::VertexShader).Load(unlitVertexShaderPaths);

    std::vector<const char*> unlitFragShaderPaths;
    unlitFragShaderPaths.push_back("shaders/version330.glsl");
    unlitFragShaderPaths.push_back("shaders/unlit.frag");
    Shader unlitFragmentShader = ShaderLoader(Shader::FragmentShader).Load(unlitFragShaderPaths);

    std::shared_ptr<ShaderProgram> unlitShaderProgramPtr = std::make_shared<ShaderProgram>();
    unlitShaderProgramPtr->Build(unlitVertexShader, unlitFragmentShader);*/

    // Get transform related uniform locations
    ShaderProgram::Location cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
    ShaderProgram::Location worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");

    // Get related uniforms for the unlit version of the shader
    // ShaderProgram::Location unlitWorldMatrixLocation = unlitShaderProgramPtr->GetUniformLocation("WorldMatrix");
    // ShaderProgram::Location unlitViewProjMatrixLocation = unlitShaderProgramPtr->GetUniformLocation("ViewProjMatrix");

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

    // Register unlit shader with renderer
    /*m_renderer.RegisterShaderProgram(unlitShaderProgramPtr,
        [=](const ShaderProgram& unlitShaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                unlitShaderProgram.SetUniform(unlitViewProjMatrixLocation, camera.GetProjectionMatrix());
            }
            unlitShaderProgram.SetUniform(unlitWorldMatrixLocation, worldMatrix);
        },
        [](const ShaderProgram&, std::span<const Light* const>, unsigned int&) -> bool
        {
            return true;
        }
    );*/

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

    // Create reference to Unlit material
    // assert(unlitShaderProgramPtr);
    // m_unlitMaterial = std::make_shared<Material>(unlitShaderProgramPtr, filteredUniforms);
}

void ProjectApplication::InitializeModels()
{
    // Testing Cubemap
    // m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/defaultCubemap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);
    
    // Project Cubemap
    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/skybox.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);


    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    TextureCubemapObject::Unbind();

    m_defaultMaterial->SetUniformValue("AmbientColor", glm::vec3(0.25f));

    m_defaultMaterial->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
    m_defaultMaterial->SetUniformValue("EnvironmentMaxLod", maxLod);
    m_defaultMaterial->SetUniformValue("Color", glm::vec3(1.0f));

    m_defaultMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_defaultMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);

    // m_unlitMaterial->SetUniformValue("Color", glm::vec4(1.0f));

    // Configure loader
    ModelLoader loader(m_defaultMaterial);
    // ModelLoader unlitLoader(m_unlitMaterial);

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
    //std::shared_ptr<Model> chestModel = loader.LoadShared("models/treasure_chest/treasure_chest.obj");
    //m_scene.AddSceneNode(std::make_shared<SceneModel>("treasure chest", chestModel));

    std::shared_ptr<Model> lighthouseModel = loader.LoadShared("models/lighthouse/lighthouseobj.obj");
    m_scene.AddSceneNode(std::make_shared<SceneModel>("lighthouse", lighthouseModel));

    // std::shared_ptr<Model> spotLightCone = unlitLoader.LoadShared("models/lightcone/spotlight_cone.obj");
    // m_scene.AddSceneNode(std::make_shared<SceneModel>("spotlight cone", spotLightCone));

    //std::shared_ptr<Model> millModel = loader.LoadShared("models/mill/Mill.obj");
    //m_scene.AddSceneNode(std::make_shared<SceneModel>("mill model", millModel));

    //std::shared_ptr<Model> cameraModel = loader.LoadShared("models/camera/camera.obj");
    //m_scene.AddSceneNode(std::make_shared<SceneModel>("camera model", cameraModel));

    //std::shared_ptr<Model> teaSetModel = loader.LoadShared("models/tea_set/tea_set.obj");
    //m_scene.AddSceneNode(std::make_shared<SceneModel>("tea set", teaSetModel));

    //std::shared_ptr<Model> clockModel = loader.LoadShared("models/alarm_clock/alarm_clock.obj");
    //m_scene.AddSceneNode(std::make_shared<SceneModel>("alarm clock", clockModel));
}

void ProjectApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
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
