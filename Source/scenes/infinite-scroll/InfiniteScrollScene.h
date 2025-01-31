#ifndef __INFINITE_SCROLL_SCENE_H__
#define __INFINITE_SCROLL_SCENE_H__

#include "axmol.h"
USING_NS_AX;

#include "InfiniteScrollNode.h"

class InfiniteScrollScene : public Scene
{
public:
    bool init() override;
    void update(float delta) override;

private:
    Layer* uiLayer;
    InfiniteScrollNode* infiniteScroll1;
    InfiniteScrollNode* infiniteScroll2;
    InfiniteScrollNode* infiniteScroll3;
    InfiniteScrollNode* infiniteScroll4;
    InfiniteScrollNode* infiniteScroll5;

    void setupUI();
    void setupCustom();
};

#endif  // __INFINITE_SCROLL_SCENE_H__
