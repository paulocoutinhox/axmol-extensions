#ifndef __UI_SIMPLE_CHECK_BOX_SCENE_H__
#define __UI_SIMPLE_CHECK_BOX_SCENE_H__

#include "axmol.h"
#include "ui/SimpleCheckBox.h"
USING_NS_AX;

class SimpleCheckBoxScene : public Scene {
public:
    virtual bool init() override {
        if (!Scene::init()) {
            return false;
        }

        // general
        Size visibleSize = Director::getInstance()->getVisibleSize();

        // checkbox
        auto checkbox = SimpleCheckBox::create("ui/CheckBoxNormal.png", "ui/CheckBoxActive.png");
        checkbox->setSelected(false);
        checkbox->setPosition(visibleSize / 2);
        checkbox->setScale(2.0f);
        checkbox->addEventListener(
            [checkbox](Object *, ax::ui::CheckBox::EventType type) {
                if (type == ui::CheckBox::EventType::SELECTED) {
                    AXLOGD("CheckBox is checked!");
                } else {
                    AXLOGD("CheckBox is not checked!");
                }
            });

        addChild(checkbox);

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

        addChild(backButton);

        return true;
    }
};

#endif // __UI_SIMPLE_CHECK_BOX_SCENE_H__
