#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QTimer>

#include <pxr/pxr.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include "pxr/imaging/hgi/tokens.h"

#include "pxr/usd/usd/primRange.h"
#include "pxr/imaging/hd/engine.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/imaging/hdSt/renderDelegate.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/imaging/hgi/tokens.h"

#include "pxr/imaging/glf/diagnostic.h"
#include "pxr/imaging/glf/contextCaps.h"
#include "pxr/imaging/glf/glContext.h"
#include "pxr/imaging/glf/info.h"

#include "pxr/imaging/hdx/pickTask.h"
#include "pxr/imaging/hdx/taskController.h"

#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include "pxr/imaging/hgi/tokens.h"

#include "pxr/imaging/garch/glApi.h"

#include <iostream>
#include <string>

class OpenGLWidget : public QOpenGLWidget , public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

protected:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
private:
    pxr::UsdStageRefPtr _stage;
    pxr::HgiUniquePtr _hgi;
    pxr::HdDriver _hdDriver;

    std::unique_ptr<pxr::HdStRenderDelegate> _renderDelegate;
    pxr::HdRenderIndex *_renderIndex;
    std::unique_ptr<pxr::UsdImagingDelegate> _sceneDelegate;
    std::unique_ptr<pxr::HdxTaskController> _taskController;
    std::unique_ptr<pxr::HdEngine> _engine;
    
public:
    bool loadUsdStage(const std::string &path);
    bool prepareTasks();
};
#endif // WIDGET_H