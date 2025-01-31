#ifndef __MAIN_SCENE_H__
#define __MAIN_SCENE_H__

#include <string>

#include "axmol.h"
USING_NS_AX;

#include "ui/UIListView.h"

class MainScene : public Scene {
public:
    bool init() override;
    void update(float delta) override;

private:
    // main list view
    ax::ui::ListView *listView = nullptr;

    // total items count
    int totalCount = 10;

    // how many items we actually spawn, these items will be reused and should be > (list view size / template size)
    // + 2.
    int spawnCount = 0;

    // list item height
    float itemTemplateHeight = 0.f;

    void setupUI();
    void setupListData(ax::ui::Layout *defaultItem);
    void selectedItemEvent(Object *sender, ax::ui::ListView::EventType type);
    void addListItem(int tag, const std::string &text, ax::ui::Layout *defaultItem);
    void onItemSelected(int tag);
};

#endif // __MAIN_SCENE_H__
