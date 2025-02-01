#ifndef __SPLASH_SCENE_H__
#define __SPLASH_SCENE_H__

#include "axmol.h"
USING_NS_AX;

#include "scenes/main/MainScene.h"

class SplashScene : public Scene
{
public:
    virtual bool init() override
    {
        if (!Scene::init())
        {
            return false;
        }

        // general
        Size winSize = Director::getInstance()->getWinSize();

        // background image
        std::string backgroundImage;

        if (winSize.width > winSize.height)
        {
            backgroundImage = "SplashScreenLandscape.png";
        }
        else
        {
            backgroundImage = "SplashScreenPortrait.png";
        }

        auto background = Sprite::create(backgroundImage);
        background->setPosition(Vec2(winSize.width / 2, winSize.height / 2));

        // resize
        float scaleX   = winSize.width / background->getContentSize().width;
        float scaleY   = winSize.height / background->getContentSize().height;
        float maxScale = std::max(scaleX, scaleY);
        background->setScale(maxScale);
        this->addChild(background);

        // schedule scene change
        // clang-format off
        this->scheduleOnce([=](float delta) { load(); }, 3.0f, "SplashSceneSwitchScheduler");
        // clang-format on

        return true;
    }

    void load()
    {
        // preload your assets
        // AudioEngine::preload("sounds/music.mp3");

        // load your music
        // AudioEngine::play2d("sounds/music.mp3", true);

        // switch scene
        auto scene = utils::createInstance<MainScene>();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene, Color3B(0, 0, 0)));
    }
};

#endif  // __SPLASH_SCENE_H__
