/*
 * STCameraManager
 * Original from: https://gist.github.com/stevetranby
 */

#include "STCameraManager.h"

USING_NS_AX;

STCameraManager *STCameraManager::get() {
    static STCameraManager instance;
    return &instance;
}

void STCameraManager::setInputDisabled(bool disabled) {
    panZoomLayer->setInputDisabled(disabled);
}

void STCameraManager::setupWithPanZoomLayer(STLayerPanZoom *layer) {
    panZoomLayer = layer;
}

void STCameraManager::resetManager() {
    AXLOG("Resetting CM Manager!!");
    // setPanZoomLayer(nullptr);
    panZoomLayer = nullptr;
    stateStack.clear();
}

///////////////////////////////////////////
// MARK: -

void STCameraManager::pushState() {
    if (!panZoomLayer) {
        return;
    }

    panZoomLayer->setInputDisabled(true);

    CameraState state;
    state.worldCoord = currentWorldCoord();
    state.scale = panZoomLayer->getScaleX();
    stateStack.push_back(state);

    // dlog("pushstate: %s, scaleTo=%f", CStrFromPoint(state.worldCoord), state.scale);
}

bool STCameraManager::popStateWithAction(const popCallback &callback) {
    if (!panZoomLayer) {
        return false;
    }
    if (stateStack.empty()) {
        return false;
    }

    panZoomLayer->stopAllActions();

    // return camera to top state (pos,zoom)
    auto state = stateStack.back();

    float duration = 0.4f;

    // dlog("popstate: %s, scaleTo=%f, callback = %p", CStrFromPoint(state.worldCoord), state.scale, &callback);

    setViewpointCenter(state.worldCoord, true, state.scale, duration, 0.f, callback);

    stateStack.pop_back();

    panZoomLayer->setInputDisabled(false);
    panZoomLayer->resetTouches();

    return true;
}

bool STCameraManager::popState() {
    return popStateWithAction(nullptr);
}

// MARK: -

Vec2 STCameraManager::worldToScreen(const ax::Vec2 &worldCoord, float atScale) {
    auto ws = Director::getInstance()->getWinSize();
    Vec2 screenCenter{ws.width * .5f, ws.height * .5f};
    Vec2 newOrigin = worldCoord * atScale;

    Vec2 screenCoord = screenCenter - newOrigin;
    // dinfo2("screenCoored: %s", CStrFromPoint(screenCoord));
    // return { truncf(screenCoord.x), truncf(screenCoord.y) };
    return {screenCoord.x, screenCoord.y};
}

Vec2 STCameraManager::screenToWorld(const ax::Vec2 &screenCoord, float atScale) {
    auto wsHalf = Director::getInstance()->getWinSize() * .5f;
    Vec2 screenCenter{wsHalf.width, wsHalf.height};
    Vec2 worldOrigin = screenCenter - screenCoord;
    Vec2 worldCoord = worldOrigin * 1.f / atScale;
    return worldCoord;
}

Vec2 STCameraManager::currentWorldCoord() {
    if (!panZoomLayer) {
        return Vec2::ZERO;
    }
    Vec2 p = panZoomLayer->getPosition();
    float s = panZoomLayer->getScaleX();
    // TODO: add to dlog macros header
    //    #define dlogVec2(x)  dlog(#x ": %s", CStrFromPoint(x))
    //    dlogVec2(p);
    //    dlogFloat(s);
    return screenToWorld(p, s);
}

// MARK: -

bool STCameraManager::isMoving() {
    if (!panZoomLayer) {
        return false;
    }

    // TODO: What need be here?
    //    auto action = panZoomLayer->getActionByTag(kTagActionCameraMoving);
    //    if(action && ! action->isDone()) {
    //        return true;
    //    }
    return false;
}

// MARK: -

// void CameraManager::setViewpointCenter(const Vec2& newWorldCoord, bool animated, float newScale /* = 0 */, float dur
// /* = 0 */, float delay /* = 0 */, FiniteTimeAction* callback /* = nullptr */)
void STCameraManager::setViewpointCenter(const Vec2 &newWorldCoord,
                                         bool animated,
                                         float newScale /* = 0 */,
                                         float dur /* = 0 */,
                                         float delay /* = 0 */,
                                         const std::function<void()> &callback) {
    if (!panZoomLayer) {
        return;
    }

    if (newScale == 0) {
        newScale = panZoomLayer->getScaleX();
    }

    // TODO: @profile @performance
    float minScale = panZoomLayer->minScale();
    float maxScale = panZoomLayer->maxScale();
    newScale = clampf(newScale, minScale, maxScale); // .25f * SCALE_LARGE, 2.f * SCALE_LARGE);

    // just find where panZoom should be if centering at new scale

    Vec2 newPos = worldToScreen(newWorldCoord, newScale);

    if (animated) {
        float easeRate = 1.5f;
        // dinfo2("panZoomLayer position by action => %s, scale: %f, easerate: %f\n\n", CStrFromPoint(newPos), newScale,
        // easeRate);
        panZoomLayer->runCameraAction(delay, dur, newPos, newScale, easeRate, callback);
    } else {
        panZoomLayer->setPosition(newPos);
        panZoomLayer->setScale(newScale);
        //        dinfo2("newWorldCoord: %s, newPos: %s, newScale: %f\n\n",
        //              CStrFromPoint(newWorldCoord),
        //              CStrFromPoint(newPos),
        //              newScale);
    }
}

// MARK: Move To -

void STCameraManager::moveTo(const Vec2 &position,
                             bool animated /* = false */,
                             float newScale /* = 0 */,
                             float dur /* = 0 */) {
    if (!panZoomLayer) {
        return;
    }
    if (newScale == 0) {
        newScale = panZoomLayer->getScaleX();
    }
    setViewpointCenter(position, animated, newScale, dur);
}

// MARK: Move By -

// position - ?? world coord ?? offset from current camera "center of screen" world coord
void STCameraManager::moveBy(const Vec2 &position,
                             bool animated /* = false */,
                             float newScale /* = 0 */,
                             float dur /* = 0 */) {
    // dinfo("enter: moveby p = %s", CStrFromPoint(position));
    if (!panZoomLayer) {
        return;
    }

    if (position.isZero() && newScale == 0) {
        return;
    }

    Vec2 oldWorldCoord = currentWorldCoord();
    Vec2 newWorldCoord = oldWorldCoord + position;
    if (animated && dur == 0) {
        dur = 0.5f;
    }
    if (newScale == 0) {
        newScale = panZoomLayer->getScaleX();
    }
    setViewpointCenter(newWorldCoord, animated, newScale, dur);
}

void STCameraManager::scaleBy(float scaleFactor) {
    if (!panZoomLayer) {
        return;
    }

    float newScale = panZoomLayer->getScaleX();
    newScale *= scaleFactor;
    moveBy(Vec2::ZERO, false, newScale, 0);
}
