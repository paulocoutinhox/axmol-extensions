#ifndef __LAYER_PAN_ZOOM_SCENE_H__
#define __LAYER_PAN_ZOOM_SCENE_H__

#include "axmol.h"
USING_NS_AX;

class LayerPanZoomScene : public Scene
{
public:
    bool init() override;
    void update(float delta) override;

private:
    Layer *mapLayer;
    Layer *playerLayer;
    Layer *uiLayer;

    Sprite *player;

    void setupUI();
    void setupMap();
    void setupPlayer();
    void setupPhysics();
};

#endif // __LAYER_PAN_ZOOM_SCENE_H__
