#ifndef __LAYER_PAN_ZOOM_SCENE_H__
#define __LAYER_PAN_ZOOM_SCENE_H__

#include "axmol.h"
USING_NS_AX;

#include "STLayerPanZoom.h"
#include "scenes/layer-pan-zoom/STLayerPanZoom.h"

class LayerPanZoomScene : public Scene
{
public:
    bool init() override;
    void update(float delta) override;

private:
    Layer *mapLayer;
    Layer *playerLayer;
    Layer *uiLayer;

    STLayerPanZoom *panZoomLayer;

    Sprite *player;

    void setupUI();
    void setupMap();
    void setupPlayer();
    void setupPhysics();
    void setupCustom();
};

#endif // __LAYER_PAN_ZOOM_SCENE_H__
