/*
 * STCameraManager
 * Original from: https://gist.github.com/stevetranby
 */

#ifndef __ST_CAMERA_MANAGER_H__
#define __ST_CAMERA_MANAGER_H__

#include "STLayerPanZoom.h"

typedef std::function<void()> popCallback;

class STCameraManager
{
    // Singleton Stuff
public:
    static STCameraManager *get();

private:
    STCameraManager() = default;
    ~STCameraManager() = default;
    STCameraManager(const STCameraManager &) = delete;
    STCameraManager &operator=(const STCameraManager &) = delete;
    STCameraManager(STCameraManager &&) = delete;
    STCameraManager &operator=(STCameraManager &&) = delete;

public:
    void update(float dt);
    void setupWithPanZoomLayer(STLayerPanZoom *layer);
    void resetManager();

    /// Get's world coordinate of the camera's center point
    ax::Vec2 worldToScreen(const ax::Vec2 &worldCoord, float atScale);
    ax::Vec2 screenToWorld(const ax::Vec2 &screenCoord, float atScale);
    ax::Vec2 currentWorldCoord();

    void pushState();
    bool popStateWithAction(const popCallback &callback);
    bool popState();

    bool isMoving();

    /// reposition camera's center to a given world coord
    /// defaults: newScale = 0, dur = 0, delay = 0
    void setViewpointCenter(
        const ax::Vec2 &newWorldCoord, bool animated, float newScale = 0, float dur = 0, float delay = 0, const popCallback &callback = []() {});

    /// reposition camera's center to a given world coord
    /// defaults: no animation (instant move), no change to zoom, dur = 0
    void moveTo(const ax::Vec2 &position, bool animated = false, float newScale = 0, float dur = 0);

    /// move camera's center by a given amount in world coord units
    /// defaults: no animation (instant move), no change to zoom, dur = 0
    void moveBy(const ax::Vec2 &position, bool animated = false, float newScale = 0, float dur = 0);

    void scaleBy(float scaleFactor);

    void setInputDisabled(bool disabled);

private:
    // ax::ValueVector
    struct CameraState
    {
        ax::Vec2 worldCoord;
        float scale;
    };
    std::vector<CameraState> stateStack;

    STLayerPanZoom *panZoomLayer;
};

#endif
