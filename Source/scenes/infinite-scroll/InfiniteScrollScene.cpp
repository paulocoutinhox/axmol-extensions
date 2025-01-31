#include "scenes/infinite-scroll/InfiniteScrollScene.h"
#include "scenes/main/MainScene.h"
#include "ui/UIButton.h"

bool InfiniteScrollScene::init() {
    // super init first
    if (!Scene::init()) {
        return false;
    }

    // custom
    setupCustom();

    // setup ui
    setupUI();

    // schedule update
    scheduleUpdate();

    return true;
}

void InfiniteScrollScene::setupCustom() {
    {
        Sprite *bg1 = Sprite::create("backgrounds/layers/sky.png");
        Sprite *bg2 = Sprite::create("backgrounds/layers/sky.png");
        Sprite *bg3 = Sprite::create("backgrounds/layers/sky.png");

        infiniteScroll1 = InfiniteScrollNode::create();
        infiniteScroll1->addInfiniteScrollXWithZ(0, Point(0.5, 0.5), Point(0, 0), bg1, bg2, bg3, nullptr);
        addChild(infiniteScroll1);
    }
    {
        Sprite *bg1 = Sprite::create("backgrounds/layers/rocks_1.png");
        Sprite *bg2 = Sprite::create("backgrounds/layers/rocks_1.png");
        Sprite *bg3 = Sprite::create("backgrounds/layers/rocks_1.png");

        infiniteScroll2 = InfiniteScrollNode::create();
        infiniteScroll2->addInfiniteScrollXWithZ(0, Point(0.5, 0.5), Point(0, 0), bg1, bg2, bg3, nullptr);
        addChild(infiniteScroll2);
    }
    {
        Sprite *bg1 = Sprite::create("backgrounds/layers/clouds_2.png");
        Sprite *bg2 = Sprite::create("backgrounds/layers/clouds_2.png");
        Sprite *bg3 = Sprite::create("backgrounds/layers/clouds_2.png");

        infiniteScroll3 = InfiniteScrollNode::create();
        infiniteScroll3->addInfiniteScrollXWithZ(0, Point(0.5, 0.5), Point(0, 0), bg1, bg2, bg3, nullptr);
        addChild(infiniteScroll3);
    }
    {
        Sprite *bg1 = Sprite::create("backgrounds/layers/rocks_2.png");
        Sprite *bg2 = Sprite::create("backgrounds/layers/rocks_2.png");
        Sprite *bg3 = Sprite::create("backgrounds/layers/rocks_2.png");

        infiniteScroll4 = InfiniteScrollNode::create();
        infiniteScroll4->addInfiniteScrollXWithZ(0, Point(0.5, 0.5), Point(0, 0), bg1, bg2, bg3, nullptr);
        addChild(infiniteScroll4);
    }
    {
        Sprite *bg1 = Sprite::create("backgrounds/layers/clouds_3.png");
        Sprite *bg2 = Sprite::create("backgrounds/layers/clouds_3.png");
        Sprite *bg3 = Sprite::create("backgrounds/layers/clouds_3.png");

        infiniteScroll5 = InfiniteScrollNode::create();
        infiniteScroll5->addInfiniteScrollXWithZ(0, Point(0.5, 0.5), Point(0, 0), bg1, bg2, bg3, nullptr);
        addChild(infiniteScroll5);
    }
}

void InfiniteScrollScene::update(float delta) {
    float scrollSpeedX1 = -1;
    float scrollSpeedX2 = -4;
    float scrollSpeedX3 = -6;
    float scrollSpeedX4 = -10;
    float scrollSpeedX5 = -3;
    float scrollSpeedY = 0;

    infiniteScroll1->updateWithVelocity(Point(scrollSpeedX1, scrollSpeedY), delta);
    infiniteScroll2->updateWithVelocity(Point(scrollSpeedX2, scrollSpeedY), delta);
    infiniteScroll3->updateWithVelocity(Point(scrollSpeedX3, scrollSpeedY), delta);
    infiniteScroll4->updateWithVelocity(Point(scrollSpeedX4, scrollSpeedY), delta);
    infiniteScroll5->updateWithVelocity(Point(scrollSpeedX5, scrollSpeedY), delta);
}

void InfiniteScrollScene::setupUI() {
    // layer
    uiLayer = Layer::create();
    addChild(uiLayer, 1000);

    // back button
    auto backButton = ui::Button::create("ButtonBack.png", "ButtonBack.png");
    backButton->setPosition(Vec2(
        _director->getWinSize().width - backButton->getContentSize().width / 2 - backButton->getContentSize().width / 2,
        backButton->getContentSize().height / 2 + backButton->getContentSize().height / 2));

    // clang-format off
    backButton->addTouchEventListener([=](Object *sender, ui::Widget::TouchEventType type) {
        if (type == ui::Widget::TouchEventType::ENDED)
        {
            auto scene = utils::createInstance<MainScene>();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene, Color3B(0, 0, 0)));
        }
    });
    // clang-format on

    uiLayer->addChild(backButton);
}
