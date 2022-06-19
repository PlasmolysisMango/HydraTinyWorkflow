#include "widget.h"
#include "pxr/imaging/hd/engine.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    
}

OpenGLWidget::~OpenGLWidget()
{
    
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // glClearColor(0.0f,0.5f,0.9f,1.0f);
    glEnable(GL_DEPTH_TEST | GL_ALPHA_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    using namespace pxr;

    // Initialize GL context caps based on shared context
    GlfContextCaps::InitInstance();

    // Initalize the Hgi and HdDriver
    
    // Create a hgi connected to existed OpenGL context. 
    _hgi = Hgi::CreatePlatformDefaultHgi();
    // hgi must be guaranteed alive during the application. 
    _hdDriver.name = HgiTokens->renderDriver;
    _hdDriver.driver = VtValue(_hgi.get());
    HdDriverVector driverVec;
    driverVec.push_back(&_hdDriver);

    // set render delegate
    _renderDelegate = std::make_unique<HdStRenderDelegate>();

    // set render index
    _renderIndex = HdRenderIndex::New(_renderDelegate.get(), driverVec); // q1

    // scenedelegate
    _sceneDelegate = std::make_unique<UsdImagingDelegate>(_renderIndex, SdfPath::AbsoluteRootPath());

    // task controller
    _taskController = std::make_unique<HdxTaskController>(_renderIndex, SdfPath::AbsoluteRootPath());

    // engine
    _engine = std::make_unique<HdEngine>();

    // load stage 
    loadUsdStage("E:\\github\\Usd_atfx\\qt\\tiny_useqt\\stage.usda");

    // set camera, material, light and aov. 
    prepareTasks();
}

void OpenGLWidget::paintGL()
{
    using namespace pxr;
    // get UsdPrim 
    const auto &root = _stage->GetPseudoRoot();

    // set params
    UsdImagingGLRenderParams renderParams;
    renderParams.frame = 60.0f;

    // scene delegate
    _sceneDelegate->SetUsdDrawModesEnabled(renderParams.enableUsdDrawModes);
    _sceneDelegate->Populate(root);
    _sceneDelegate->SetTime(renderParams.frame);

    // set collection
    // what's collection? obtain the address and contain the Rprim. 

    // HdReprSelector: allow one or more different display result of a model. 
    // repr: representation.
    // choose basic repr
    HdReprSelector reprSelector = HdReprSelector(HdReprTokens->smoothHull);
    // By default our main collection will be called geometry
    const TfToken colName = HdTokens->geometry;
    // Recreate the collection.
    HdRprimCollection renderCollection(colName, reprSelector);
    _taskController->SetCollection(renderCollection);

    // set RenderTaskParams
    HdxRenderTaskParams params;
    params.enableSceneMaterials = renderParams.enableSceneMaterials;
    params.enableSceneLights = renderParams.enableSceneLights;

    _taskController->SetRenderParams(params);

    // Forward scene materials enable option to delegate
    _sceneDelegate->SetSceneMaterialsEnabled(renderParams.enableSceneMaterials);
    _sceneDelegate->SetSceneLightsEnabled(renderParams.enableSceneLights);

    // set color
    HdxColorCorrectionTaskParams hdParams;
    _taskController->SetColorCorrectionParams(hdParams);

    // enable selection: ? target a highlight and present it.
    _taskController->SetEnableSelection(renderParams.highlight);

    // set selection satae to the engine. 
    auto selTracker = std::make_shared<HdxSelectionTracker>();
    VtValue selectionValue(selTracker);
    _engine->SetTaskContextData(HdxTokens->selectionState, selectionValue);

    // obtain render tasks. 
    auto tasks = _taskController->GetRenderingTasks();

    _engine->Execute(_renderIndex, &tasks);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    _taskController->SetRenderViewport({0,0,(double)w,(double)h});
}

bool OpenGLWidget::loadUsdStage(const std::string &path)
{
    _stage = pxr::UsdStage::Open(path);
    for (const auto& prim : _stage->Traverse()) 
    {
        std::cout << prim.GetPath() << std::endl;
    }
    return _stage != nullptr;
}

bool OpenGLWidget::prepareTasks()
{
    using namespace pxr;

    const UsdPrim* cameraPrim = nullptr;
    for (const auto& prim : _stage->Traverse()) 
    {
        if (prim.GetTypeName() == "Camera")
        {
            cameraPrim = &prim;
            std::cout << "Camera found: " << cameraPrim->GetPath() << "\n";
            break;
        }
    }
    if (!cameraPrim)
    {
        std::cout << "No camera found on stage" << '\n';
        exit(1);
    }
    // m_engine->SetCameraPath(cameraPrim->GetPath());
    _taskController->SetCameraPath(cameraPrim->GetPath());

    GfCamera cam = UsdGeomCamera(*cameraPrim).GetCamera(1);
    const GfFrustum frustum = cam.GetFrustum();
    const GfVec3d cameraPos = frustum.GetPosition();

    const GfVec4f SCENE_AMBIENT(0.01f, 0.01f, 0.01f, 1.0f);
    const GfVec4f SPECULAR_DEFAULT(0.1f, 0.1f, 0.1f, 1.0f);
    const GfVec4f AMBIENT_DEFAULT(0.2f, 0.2f, 0.2f, 1.0f);
    const float   SHININESS_DEFAULT(32.0);

    GlfSimpleLight cameraLight(
        GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.0f));
    cameraLight.SetAmbient(SCENE_AMBIENT);

    const GlfSimpleLightVector lights({cameraLight});

    // Make default material and lighting match usdview's defaults... we expect 
    // GlfSimpleMaterial to go away soon, so not worth refactoring for sharing
    GlfSimpleMaterial material;
    material.SetAmbient(AMBIENT_DEFAULT);
    material.SetSpecular(SPECULAR_DEFAULT);
    material.SetShininess(SHININESS_DEFAULT);

    // m_engine->SetLightingState(lights, material, SCENE_AMBIENT);
    auto _lightingContextForOpenGLState = GlfSimpleLightingContext::New();
    // }
    _lightingContextForOpenGLState->SetLights(lights);
    _lightingContextForOpenGLState->SetMaterial(material);
    _lightingContextForOpenGLState->SetSceneAmbient(SCENE_AMBIENT);
    _lightingContextForOpenGLState->SetUseLighting(lights.size() > 0);

    _taskController->SetLightingState(_lightingContextForOpenGLState);
    

    // m_engine->SetRendererAov(HdAovTokens->color);
    _taskController->SetRenderOutputs({HdAovTokens->color});

    _taskController->SetFreeCameraMatrices(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

    _taskController->SetRenderViewport(GfVec4d(0, 0, 800, 800));

    _taskController->SetEnablePresentation(true);

    return true;
}