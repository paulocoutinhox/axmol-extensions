#include "scenes/main/MainScene.h"
#include "app/Global.h"
#include "ui/UIButton.h"

#include "scenes/infinite-scroll/InfiniteScrollScene.h"
#include "scenes/layer-pan-zoom/LayerPanZoomScene.h"
#include "scenes/splash/SplashScene.h"
#include "scenes/ui/SimpleCheckBoxScene.h"

enum ListItem {
    Splash = 0,
    LayerPanZoom = 1,
    InfiniteScroll = 2,
    SimpleCheckBox = 3,
};

bool MainScene::init() {
    // super init first
    if (!Scene::init()) {
        return false;
    }

    // ui
    setupUI();

    // schedule update
    scheduleUpdate();

    return true;
}

void MainScene::setupUI() {
    // create list view
    listView = ax::ui::ListView::create();
    listView->setDirection(ui::ScrollView::Direction::VERTICAL);
    listView->setBounceEnabled(true);
    listView->setContentSize(Global::getInstance().visibleSize);
    listView->setAnchorPoint(Vec2(0.5, 0.5));
    listView->setPosition(Vec2{
        Global::getInstance().origin.x + Global::getInstance().visibleSize.width / 2,
        Global::getInstance().origin.y + Global::getInstance().visibleSize.height / 2,
    });
    listView->addEventListener((ui::ListView::ccListViewCallback)AX_CALLBACK_2(MainScene::selectedItemEvent, this));
    addChild(listView);

    // create model
    ax::ui::Button *defaultButton = ax::ui::Button::create();
    defaultButton->setName("TitleButton");
    defaultButton->setTitleFontSize(30);

    ax::ui::Layout *defaultItem = ax::ui::Layout::create();
    defaultItem->setTouchEnabled(true);
    defaultItem->setContentSize(Size(Global::getInstance().winSize.width, Global::getInstance().winSize.height * 0.1f));
    defaultButton->setPosition(defaultItem->getContentSize() / 2);
    defaultItem->addChild(defaultButton);

    // set model
    listView->setItemModel(defaultItem);

    // set all items layout gravity
    listView->setGravity(ax::ui::ListView::Gravity::CENTER_VERTICAL);

    // set item margin
    float spacing = 4;
    listView->setItemsMargin(spacing);
    itemTemplateHeight = defaultItem->getContentSize().height;

    // spawn count
    spawnCount = (Global::getInstance().visibleSize.height / itemTemplateHeight) + 2;

    // initial data
    setupListData(defaultItem);

    // force layout
    listView->forceDoLayout();

    float totalHeight = itemTemplateHeight * totalCount + (totalCount - 1) * spacing;
    listView->setInnerContainerSize(Size(listView->getInnerContainerSize().width, totalHeight));
    listView->jumpToTop();
}

void MainScene::addListItem(int tag, const std::string &text, ax::ui::Layout *defaultItem) {
    ax::ui::Widget *item = defaultItem->clone();
    item->setTag(tag);
    ax::ui::Button *btn = (ax::ui::Button *)item->getChildByName("TitleButton");
    btn->setTitleText(text);
    listView->pushBackCustomItem(item);
}

void MainScene::setupListData(ax::ui::Layout *defaultItem) {
    // splash
    addListItem(ListItem::Splash, "Splash", defaultItem);

    // layer pan zoom
    addListItem(ListItem::LayerPanZoom, "Layer Pan Zoom", defaultItem);

    // infinite scroll
    addListItem(ListItem::InfiniteScroll, "Infinite Scroll", defaultItem);

    // simple check box
    addListItem(ListItem::SimpleCheckBox, "Simple Check Box", defaultItem);
}

void MainScene::onItemSelected(int tag) {
    Scene *scene;

    switch (tag) {
    case ListItem::Splash:
        // splash
        scene = utils::createInstance<SplashScene>();
        break;
    case ListItem::LayerPanZoom:
        // layer pan zoom
        scene = utils::createInstance<LayerPanZoomScene>();
        break;
    case ListItem::InfiniteScroll:
        // infinite scroll
        scene = utils::createInstance<InfiniteScrollScene>();
        break;
    case ListItem::SimpleCheckBox:
        // simple check box
        scene = utils::createInstance<SimpleCheckBoxScene>();
        break;
    }

    if (scene) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene, Color3B(0, 0, 0)));
    }
}

void MainScene::selectedItemEvent(Object *sender, ax::ui::ListView::EventType type) {
    switch (type) {
    case ax::ui::ListView::EventType::ON_SELECTED_ITEM_END: {
        ax::ui::ListView *listView = static_cast<ax::ui::ListView *>(sender);
        auto tag = listView->getItem(listView->getCurSelectedIndex())->getTag();
        onItemSelected(tag);
        break;
    }
    default:
        break;
    }
}

void MainScene::update(float delta) {
    //
}
