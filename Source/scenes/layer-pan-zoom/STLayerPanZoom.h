#pragma once

#define kSTLayerPanZoomMultitouchGesturesDetectionDelay (0.1f)
#define kZoomScaleAnimationDuration (0.2f)

typedef enum
{
    /** Standard mode: swipe to scroll */
    kSTLayerPanZoomModeSheet,
    /** Frame mode (i.e. drag inside objects): hold finger at edge of the screen to the sroll in this direction */
    kSTLayerPanZoomModeFrame
} STLayerPanZoomMode;

// https://raw.github.com/gameslovin/cocos2dx_extension/master/Extensions/CCLayerPanZoom/CCLayerPanZoom.h
typedef enum
{
    kCCLayerPanZoomFrameEdgeNone,
    kCCLayerPanZoomFrameEdgeTop,
    kCCLayerPanZoomFrameEdgeBottom,
    kCCLayerPanZoomFrameEdgeLeft,
    kCCLayerPanZoomFrameEdgeRight,
    kCCLayerPanZoomFrameEdgeTopLeft,
    kCCLayerPanZoomFrameEdgeBottomLeft,
    kCCLayerPanZoomFrameEdgeTopRight,
    kCCLayerPanZoomFrameEdgeBottomRight
} CCLayerPanZoomFrameEdge;

// forward decl
class STLayerPanZoom;
class GameLayer;

class STLayerPanZoomDelegate
{
public:
    virtual ~STLayerPanZoomDelegate();

    virtual bool layerPanZoomClickedAtPoint(const cocos2d::Vec2 &touchPosition, int touchCount, bool shortCircuited = false) = 0;
    virtual bool layerPanZoomTouchBegan(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &touchPositionInLayer) = 0;
    virtual bool layerPanZoomTouchMoved(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &touchPositionInLayer) = 0;
    virtual bool layerPanZoomTouchEnded(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &touchPositionInLayer) = 0;

    virtual bool layerPanZoomMouseBegan(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &screenPositionInLayer) = 0;
    virtual bool layerPanZoomMouseMoved(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &screenPositionInLayer) = 0;
    virtual bool layerPanZoomMouseEnded(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &screenPositionInLayer) = 0;

    virtual bool layerPanZoomMouseMovedOver(STLayerPanZoom *layerPanZoom, const cocos2d::Vec2 &screenPosition) = 0;

    /// return false to prevent mouse edge scrolling or wheel zooming
    virtual bool layerPanZoomCanMouseEdgePanOrScrollZoom(STLayerPanZoom * /*layerPanZoom*/) { return true; }
};

/** class STLayerPanZoom Class that represents the layer that can be scrolled
 * and zoomed with one or two fingers. */
class STLayerPanZoom : public cocos2d::Layer
{
public:
    STLayerPanZoom();
    virtual ~STLayerPanZoom();
    virtual bool init();
    static STLayerPanZoom *layer();

    CREATE_FUNC(STLayerPanZoom);

    /////////////////////////////////////////////////////////////
    // STEVE
    /** Delegate for callbacks. */
    int _tapCount;
    bool _tapEnabled;
    cocos2d::Vec2 _tapBeganPosition;
    cocos2d::Vec2 _tapPosition;
    cocos2d::Rect _worldBounds;
    CC_SYNTHESIZE(STLayerPanZoomDelegate *, _delegate, Delegate);
    CC_SYNTHESIZE_BOOL(_isDoubleTapAllowed, DoubleTapAllowed);

    /////////////////////////////////////////////////////////////
    // TODO: remove getters
    // TODO: maybe remove setters??

protected:
    bool _isDisabled;
    bool _inputDisabled;
    bool _panDisabled;
    bool _zoomDisabled;

public:
    virtual bool isDisabled(void) const;
    virtual void setDisabled(bool var);
    virtual bool isInputDisabled(void) const;
    virtual void setInputDisabled(bool var);
    virtual bool isPanDisabled(void) const;
    virtual void setPanDisabled(bool var);
    virtual bool isZoomDisabled(void) const;
    virtual void setZoomDisabled(bool var);

    /////////////////////////////////////////////////////////////

    void resetTouches();
    void tapWaitFinished();
    void tapHandler();
    void setWorldBounds(cocos2d::Rect rect);

    /////////////////////////////////////////////////////////////

    void setMaxScale(float maxScale);
    float maxScale();
    void setMinScale(float minScale);
    float minScale();
    void setRubberEffectRatio(float rubberEffectRatio);
    float rubberEffectRatio();

    // TODO: add delegate
    CC_SYNTHESIZE(float, _maxTouchDistanceToClick, MaxTouchDistanceToClick);

    CC_SYNTHESIZE_PASS_BY_REF(cocos2d::Vector<cocos2d::Touch *>, _touches, Touches);

    CC_SYNTHESIZE(float, _touchDistance, TouchDistance);
    CC_SYNTHESIZE(float, _minSpeed, MinSpeed);
    CC_SYNTHESIZE(float, _maxSpeed, MaxSpeed);
    CC_SYNTHESIZE(float, _topFrameMargin, TopFrameMargin);
    CC_SYNTHESIZE(float, _bottomFrameMargin, BottomFrameMargin);
    CC_SYNTHESIZE(float, _leftFrameMargin, LeftFrameMargin);
    CC_SYNTHESIZE(float, _rightFrameMargin, RightFrameMargin);

    // CC_SYNTHESIZE(CCScheduler*, _scheduler, Scheduler);
    CC_SYNTHESIZE(float, _rubberEffectRecoveryTime, RubberEffectRecoveryTime);

    cocos2d::Rect _panBoundsRect;
    float _maxScale;
    float _minScale;

    STLayerPanZoomMode _mode;

    // previous position in layer if single touch was moved.
    cocos2d::Vec2 _prevSingleTouchPositionInLayer;

    // Time when single touch has began, used to wait for possible multitouch
    // gestures before reacting to single touch.
    time_t _singleTouchTimestamp;

    // Flag used to call touchMoveBeganAtPosition: only once for each single touch event.
    bool _touchMoveBegan;

    float _rubberEffectRatio;
    bool _rubberEffectRecovering;
    bool _rubberEffectZooming;

    // Updates position in frame mode.
    virtual void update(float dt);
    void onEnter();
    void onExit();

    // Scale and Position related
    void setPanBoundsRect(cocos2d::Rect rect);

    virtual void setPosition(const cocos2d::Vec2 &position);
    virtual void setScale(float scale);

    // Ruber Edges related
    void recoverPositionAndScale();
    void recoverEnded();

    // Helpers
    float topEdgeDistance();
    float leftEdgeDistance();
    float bottomEdgeDistance();
    float rightEdgeDistance();
    float minPossibleScale();
    CCLayerPanZoomFrameEdge frameEdgeWithPoint(const cocos2d::Vec2 &point);
    float horSpeedWithPosition(const cocos2d::Vec2 &pos);
    float vertSpeedWithPosition(const cocos2d::Vec2 &pos);

    // NEW STUFF (Mouse & Desktop)
public:
    void setupInput();
    void setupMouseInput();
    void setupTouchInputInternal();
    void setupKeyboardInput();
    void setZoomLevels(std::vector<float> zoomLevels)
    {
        dlog("enter");
        _zoomLevelsNew = zoomLevels;

        setMinScale(_zoomLevelsNew.front());
        setMaxScale(_zoomLevelsNew.back());
        dlog("before sorting [%f - %f]", _minScale, _maxScale);
        for (auto zoom : _zoomLevelsNew)
        {
            dlog("zoom: %f", zoom);
        }

        std::sort(_zoomLevelsNew.begin(), _zoomLevelsNew.end());

        setMinScale(_zoomLevelsNew.front());
        setMaxScale(_zoomLevelsNew.back());
        dlog("after sorting [%f - %f]", _minScale, _maxScale);
        for (auto zoom : _zoomLevelsNew)
        {
            dlog("zoom: %f", zoom);
        }
    }

private:
    int _zoomIndex;
    std::vector<float> _zoomLevelsNew;

    cocos2d::Vec2 _mouseBeganPosition;

    // unsure if necessary as 2nd pos tracking
    cocos2d::Vec2 _mouseBeganPositionRightDrag;
    cocos2d::Vec2 _mouseBeganPositionRightMouseLocation;

    float _lastMouseScrollVelocity;
    bool _isMouseDown;
    bool _isMouseScrolling;
    float _zoomWaitTimer;
    float _zoomWaitTimerDampen;

    std::map<cocos2d::EventKeyboard::KeyCode, bool> _keyState;
    std::map<cocos2d::EventKeyboard::KeyCode, cocos2d::Vec2> _cameraInputsArrows;
    std::map<cocos2d::EventKeyboard::KeyCode, cocos2d::Vec2> _cameraInputs;

public:
    ////////////////////////////////////////////////////////////////////////////
    // camera pan/zoom action (without Simulation Time, _timeScale affecting)

    void updateCameraAction(float dt);
    void runCameraAction(float delay, float duration, const cocos2d::Vec2 &newPos, float newScale, float easingRate, const std::function<void()> &callback = nullptr);

    int _stage;
    float _targetDelay;
    float _easeRate;
    float _elapsed;
    float _targetTime;
    std::function<void()> _targetCallback;

    cocos2d::Vec2 _startPosition; // do we need?
    cocos2d::Vec2 _previousPosition;
    cocos2d::Vec2 _targetPosition;
    cocos2d::Vec2 _deltaPosition;

    float _targetScale;
    float _previousScale;
    float _startScaleX;
    float _startScaleY;
    float _startScaleZ;
    float _endScaleX;
    float _endScaleY;
    float _endScaleZ;
    float _deltaScaleX;
    float _deltaScaleY;
    float _deltaScaleZ;

    iRect _tileCameraBounds;
};
