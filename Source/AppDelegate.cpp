#include "AppDelegate.h"
#include "scenes/main/MainScene.h"

#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#    include "audio/AudioEngine.h"
#endif

static ax::Size designResolutionSize = ax::Size(1280, 720);

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    auto director = Director::getInstance();
    auto glView   = director->getGLView();
    if (!glView)
    {
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
    (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
        glView = GLViewImpl::createWithRect("Axmol Extensions",
                                            ax::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        glView = GLViewImpl::create("Axmol Extensions");
#endif
        director->setGLView(glView);
    }

    // turn on display FPS
    // director->setStatsDisplay(true);

    // set fps
    director->setAnimationInterval(1.0f / 60);

    // set the design resolution
    glView->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height,
                                    ResolutionPolicy::SHOW_ALL);

    // preload
    preload();

    // create main scene
    auto scene = utils::createInstance<MainScene>();

    // run
    director->runWithScene(scene);

    return true;
}

void AppDelegate::preload()
{
    //
}

// this function will be called when the app is inactive and when receiving a phone call
void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#endif
}

AppDelegate::AppDelegate()
{
    // ignore
}

AppDelegate::~AppDelegate()
{
    // ignore
}

void AppDelegate::initGLContextAttrs()
{
    // set GL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(glContextAttrs);
}
