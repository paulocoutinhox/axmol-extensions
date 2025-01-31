#include "scenes/layer-pan-zoom/LayerPanZoomScene.h"
#include "scenes/main/MainScene.h"
#include "ui/UIButton.h"

#include "STCameraManager.h"
#include "STLayerPanZoom.h"

bool LayerPanZoomScene::init() {
    // super init first
    if (!Scene::initWithPhysics()) {
        return false;
    }

    // custom
    setupCustom();

    // physics
    setupPhysics();

    // map
    setupMap();

    // player
    setupPlayer();

    // setup ui
    setupUI();

    // schedule update
    scheduleUpdate();

    return true;
}

void LayerPanZoomScene::setupCustom() {
    panZoomLayer = STLayerPanZoom::create();
    panZoomLayer->setPosition(Vec2::ZERO);
    panZoomLayer->setAnchorPoint(Vec2::ZERO);
    panZoomLayer->setupInput();

    panZoomLayer->setMinScale(.0625f);
    panZoomLayer->setMaxScale(10.f);
    panZoomLayer->setZoomDisabled(false);
    panZoomLayer->setZoomLevels({
        .0625f,
        .125f,
        .25f,
        .5f,
        .75f,
        1.f,
        1.5f,
        2.f,
        5.f,
        10.f,
    });

    addChild(panZoomLayer);

    STCameraManager::get()->setupWithPanZoomLayer(panZoomLayer);
}

void LayerPanZoomScene::setupPlayer() {
    playerLayer = Layer::create();
    panZoomLayer->addChild(playerLayer, 200);

    player = Sprite::create("HelloWorld.png");

    auto physicsBody = PhysicsBody::createBox(player->getContentSize(), PhysicsMaterial(1.0f, 0.1f, 1.0f), Vec2(0, 0));
    physicsBody->setDynamic(true);
    physicsBody->setMass(1.0f);
    physicsBody->setGravityEnable(false);
    physicsBody->setRotationEnable(false);
    player->addComponent(physicsBody);

    playerLayer->addChild(player);

    player->setPosition(Vec2(300, 300));
    player->getPhysicsBody()->setVelocity(Vec2(100.0f, 100.0f));

    // camera can follow the player
    // panZoomLayer->runAction(Follow::create(player));
}

void LayerPanZoomScene::setupMap() {
    // map
    mapLayer = Layer::create();
    panZoomLayer->addChild(mapLayer, 100);

    auto map = FastTMXTiledMap::create("maps/map1/map.tmx");
    map->setScale(1.0f);
    mapLayer->addChild(map);

    // collision layer
    auto collisionLayer = map->getLayer("Collision Layer");

    if (collisionLayer) {
        collisionLayer->setVisible(false);

        for (int y = 0; y < collisionLayer->getLayerSize().height; y++) {
            for (int x = 0; x < collisionLayer->getLayerSize().width; x++) {
                auto tileSprite = collisionLayer->getTileAt(Vec2(x, y));

                if (tileSprite) {
                    auto physicsBody =
                        PhysicsBody::createBox(tileSprite->getContentSize(), PhysicsMaterial(1.0f, 0.1f, 0.0f));
                    physicsBody->setDynamic(false);
                    physicsBody->setGravityEnable(false);
                    physicsBody->setRotationEnable(false);
                    tileSprite->setPhysicsBody(physicsBody);
                }
            }
        }
    }
}

void LayerPanZoomScene::setupPhysics() {
    getPhysicsWorld()->setGravity(Vec2(0, 0));
    getPhysicsWorld()->setSlopBias(0, 0);
    getPhysicsWorld()->setSubsteps(4);

#if _AX_DEBUG
    // enable to see physics debug draw
    // getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
#endif
}

void LayerPanZoomScene::update(float delta) {
    //
}

void LayerPanZoomScene::setupUI() {
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
