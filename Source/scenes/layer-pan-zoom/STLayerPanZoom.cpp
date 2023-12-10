/*
 * STLayerPanZoom
 * Original from: https://gist.github.com/stevetranby
 */

#include "STLayerPanZoom.h"
#include "STCameraManager.h"

USING_NS_AX;

////////////////////////////////////////////////////

STLayerPanZoomDelegate::~STLayerPanZoomDelegate() {}

STLayerPanZoom::STLayerPanZoom()
    : _tapCount(0)
    , _tapEnabled(false)
    , _tapBeganPosition(Vec2::ZERO)
    , _tapPosition(Vec2::ZERO)
    , _worldBounds(ax::Rect::ZERO)

    , _delegate(nullptr)
    , _isDoubleTapAllowed(false)
    , _isDisabled(false)
    , _inputDisabled(false)
    , _panDisabled(false)
    , _zoomDisabled(false)

    , _maxTouchDistanceToClick(0.f)

    , _touchDistance(0.f)
    , _minSpeed(0.f)
    , _maxSpeed(0.f)
    , _topFrameMargin(0.f)
    , _bottomFrameMargin(0.f)
    , _leftFrameMargin(0.f)
    , _rightFrameMargin(0.f)
    , _rubberEffectRecoveryTime(0.f)
    , _panBoundsRect(ax::Rect::ZERO)
    , _maxScale(1.f)
    , _minScale(0.2f)
    , _mode(kSTLayerPanZoomModeSheet)
    , _prevSingleTouchPositionInLayer(Vec2::ZERO)
    , _singleTouchTimestamp(0)
    , _touchMoveBegan(false)
    , _rubberEffectRatio(0.f)
    , _rubberEffectRecovering(false)
    , _rubberEffectZooming(false)
    , _zoomIndex(3)

    //, maxZoom(0)
    //, minZoom(0)

    , _mouseBeganPosition(Vec2::ZERO)
    , _lastMouseScrollVelocity(0)
    , _isMouseDown(false)
    , _isMouseScrolling(false)
    , _zoomWaitTimer(0)
    , _zoomWaitTimerDampen(0)
{
    setZoomLevels({
        .0625f,
        .125f,
        .25f,
        .5f,
        .75f,
        1.f,
        1.5f,
        2.f,
        //        5.f,
        //        10.f, TODO: too close at 736x320
    });

    _stage = -1;
    _targetDelay = 0.f;
    _targetScale = 1.f;
    _targetPosition = Vec2::ZERO;
    _startPosition = Vec2::ZERO;
    _startScaleX = 0;
    _startScaleY = 0;
    _startScaleZ = 0;
    _deltaScaleX = 0;
    _deltaScaleY = 0;
    _deltaScaleZ = 0;
    _easeRate = 0.f;
    _elapsed = 0.f;
}

STLayerPanZoom::~STLayerPanZoom()
{
    //    AX_SAFE_RELEASE(_touches);
}

//////////

bool STLayerPanZoom::isDisabled(void) const { return _isDisabled; }
void STLayerPanZoom::setDisabled(bool var)
{
    _isDisabled = var;
    AXLOG("setting panzoom disabled = %d", var);
}

bool STLayerPanZoom::isInputDisabled(void) const { return _inputDisabled; }
void STLayerPanZoom::setInputDisabled(bool var)
{
    if (var)
        AXLOG("stupid!");
    _inputDisabled = var;
    AXLOG("setting input disabled = %d", var);
}

bool STLayerPanZoom::isPanDisabled(void) const { return _panDisabled; }
void STLayerPanZoom::setPanDisabled(bool var)
{
    _panDisabled = var;
    AXLOG("setting pan disabled = %d", var);
}

bool STLayerPanZoom::isZoomDisabled(void) const { return _zoomDisabled; }
void STLayerPanZoom::setZoomDisabled(bool var)
{
    _zoomDisabled = var;
    AXLOG("setting zoom disabled = %d", var);
}

///////////

void STLayerPanZoom::setMaxScale(float maxScale)
{
    _maxScale = maxScale;
    setScale(MIN(getScaleX(), _maxScale));
}

float STLayerPanZoom::maxScale()
{
    return _maxScale;
}

void STLayerPanZoom::setMinScale(float minScale)
{
    _minScale = minScale;
    setScale(MAX(getScaleX(), minScale));
}

float STLayerPanZoom::minScale()
{
    return _minScale;
}

void STLayerPanZoom::setRubberEffectRatio(float rubberEffectRatio)
{
    _rubberEffectRatio = rubberEffectRatio;

    // Avoid turning rubber effect On in frame mode.
    if (_mode == kSTLayerPanZoomModeFrame)
    {
        AXLOGERROR("STLayerPanZoom#setRubberEffectRatio: rubber effect is not supported in frame mode.");
        _rubberEffectRatio = 0.f;
    }
}

float STLayerPanZoom::rubberEffectRatio()
{
    return _rubberEffectRatio;
}

STLayerPanZoom *STLayerPanZoom::layer()
{
    STLayerPanZoom *layer = STLayerPanZoom::create();
    return layer;
}

// MARK: Init -
// on "init" you need to initialize your instance
bool STLayerPanZoom::init()
{
    if (!Layer::init())
    {
        return false;
    }

    setIgnoreAnchorPointForPosition(true);

    _maxScale = 2.f * SCALE_LARGE;
    _minScale = 0.1f * SCALE_LARGE;

    _worldBounds = ax::Rect::ZERO;
    _panBoundsRect = ax::Rect::ZERO;
    _touchDistance = 0.f;
    _maxTouchDistanceToClick = 15.f;

    _mode = kSTLayerPanZoomModeSheet;
    _minSpeed = 100.f;
    _maxSpeed = 1000.f;
    _topFrameMargin = 100.f;
    _bottomFrameMargin = 100.f;
    _leftFrameMargin = 100.f;
    _rightFrameMargin = 100.f;

    _rubberEffectRatio = 0.5f;
    _rubberEffectRecoveryTime = 0.2f;
    _rubberEffectRecovering = false;
    _rubberEffectZooming = false;

    return true;
}

// MARK: Steve Additions -

void STLayerPanZoom::setWorldBounds(ax::Rect rect)
{
    _worldBounds = rect;
}

void STLayerPanZoom::resetTouches()
{
    _touches.clear();
    _tapCount = 0;
}

// MARK: Update -

// Updates position in frame mode.
void STLayerPanZoom::update(float dt)
{
    if (isInputDisabled())
    {
        updateCameraAction(dt);
        AXASSERT(getScheduler()->getTimeScale() > 0, "timescale IS ZERO!");
        return;
    }

#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))

    // Mouse Pan Support (Desktop)

    if (_delegate && _delegate->layerPanZoomCanMouseEdgePanOrScrollZoom(this))
    {
        // TODO: ONLY EXECUTE
        if (_zoomWaitTimer > 0)
        {
            _zoomWaitTimer -= dt;
            // AXLOG("[update] _zoomWaitTimer = %f", _zoomWaitTimer);
            //         if(_zoomWaitTimer <= 0) {
            //             _zoomWaitTimer = 0;
            //         }
        }

        // handle near edge movement
        // TODO: don't apply camera scroll if camera keyboard input is active
        bool edgeScrollEnabled = false;
        if (edgeScrollEnabled)
        {
            auto ws = Director::getInstance()->getWinSize();
            auto screenRect = ax::Rect(0, 0, ws.width, ws.height);
            auto p = _mouseBeganPosition;
            const float scrollEdgeMargin = 35.f * SCALE_LARGE;

            if (screenRect.containsPoint(p))
            {
                Vec2 moveBy = Vec2::ZERO;
                float left = scrollEdgeMargin;
                float bottom = scrollEdgeMargin;
                float top = ws.height - scrollEdgeMargin;
                float right = ws.width - scrollEdgeMargin;

                // make sure to not include CORNERS (or rather hopefully HUD buttons are checked for first)
                if (p.x < left && p.y > bottom && p.y < top)
                {
                    moveBy.x = -1;
                }
                if (p.x > right && p.y > bottom && p.y < top)
                {
                    moveBy.x = 1;
                }
                if (p.y < bottom && p.x > left && p.x < right)
                {
                    moveBy.y = -1;
                }
                if (p.y > top && p.x > left && p.x < right)
                {
                    moveBy.y = 1;
                }

                auto unitPerSec = 1 * SCALE_LARGE;
                auto speed = unitPerSec * (60 * dt);
                STCameraManager::get()->moveBy(moveBy * speed);
            }
        }
    }

#endif // ifdef desktop

    updateCameraAction(dt);

    AXASSERT(getScheduler()->getTimeScale() > 0, "timescale IS ZERO!");

#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))

    // Keyboard Pan Support (Desktop)

    // TODO: may want to set flag that any input is available or none
    if (!isPanDisabled())
    {
        float timeFactor = dt * (.5f / getScale()) * (1.f / getScheduler()->getTimeScale());
        for (auto pair : _cameraInputs)
        {
            auto keycode = pair.first;
            auto it = _keyState.find(keycode);
            if (it != _keyState.end())
            {
                bool isPressed = it->second;
                if (isPressed)
                {
                    auto velocity = pair.second * timeFactor;
                    STCameraManager::get()->moveBy(velocity, false);
                }
            }
        }
    }

    // don't allow pan with arrow keys when menu is displayed
    // if(GameManager::get()->getCurrentGameState() != GameStateType::CommandMenu) // TODO: what the hell?
    if (true)
    {
        if (!isPanDisabled())
        {
            float timeFactor = dt * (.5f / getScale()) * (1.f / getScheduler()->getTimeScale());
            for (auto pair : _cameraInputsArrows)
            {
                auto keycode = pair.first;
                auto it = _keyState.find(keycode);
                if (it != _keyState.end())
                {
                    bool isPressed = it->second;
                    if (isPressed)
                    {
                        auto velocity = pair.second * timeFactor;
                        STCameraManager::get()->moveBy(velocity, false);
                    }
                }
            }
        }
    }

#endif

    // Only for frame mode with one touch.
    if (_mode == kSTLayerPanZoomModeFrame && _touches.size() == 1)
    {
        AXASSERT(false, "DIDNT THINK THIS CODE WAS REACHABLE");
        // Do not update position if click is still possible.
        if (_touchDistance <= _maxTouchDistanceToClick)
        {
            return;
        }

        // Do not update position if pinch is still possible.
        time_t seconds;
        seconds = time(nullptr);
        time_t delta = seconds - _singleTouchTimestamp;
        if (delta > 0 && delta < kSTLayerPanZoomMultitouchGesturesDetectionDelay)
        {
            return;
        }

        // Otherwise - update touch position. Get current position of touch.
        Touch *touch = (Touch *)_touches.at(0);
        Vec2 curPos = Director::getInstance()->convertToGL(touch->getLocationInView());

        // Scroll if finger in the scroll area near edge.
        if (frameEdgeWithPoint(curPos) != kCCLayerPanZoomFrameEdgeNone)
        {
            float x = getPosition().x + dt * horSpeedWithPosition(curPos);
            float y = getPosition().y + dt * vertSpeedWithPosition(curPos);
            setPosition(Vec2(x, y));
        }

        // Inform delegate if touch position in layer was changed due to finger or layer movement.
        Vec2 touchPositionInLayer = convertToNodeSpace(curPos);
        if (!_prevSingleTouchPositionInLayer.equals(touchPositionInLayer))
        {
            _prevSingleTouchPositionInLayer = touchPositionInLayer;
            _delegate && _delegate->layerPanZoomTouchMoved(this, touchPositionInLayer);
        }
    }
}

void STLayerPanZoom::onEnter()
{
    Layer::onEnter();
    scheduleUpdate();
    setupInput();
}

void STLayerPanZoom::onExit()
{
    getEventDispatcher()->removeEventListenersForTarget(this);
    unscheduleUpdate();
    Layer::onExit();
}

// MARK: Scale and Position related -

void STLayerPanZoom::setPanBoundsRect(ax::Rect rect)
{
    _panBoundsRect = rect;
    setScale(minPossibleScale());
    setPosition(getPosition());
}

Vec2 nearestPointInPerimiter(ax::Rect r, Vec2 p)
{
    // local function getNearestPointInPerimeter(l,t,w,h, x,y)
    float left = r.origin.x;
    float right = left + r.size.width;
    float bottom = r.origin.y;
    float top = bottom + r.size.height;

    float x = clampf(p.x, left, right);
    float y = clampf(p.y, bottom, top);

    float dl = fabsf(x - left);
    float dr = fabsf(x - right);
    float db = fabsf(y - bottom);
    float dt = fabsf(y - top);

    auto min4 = [](float a, float b, float c, float d)
    {
        return MIN(a, MIN(b, MIN(c, d)));
    };

    float m = min4(dl, dr, dt, db);

    if (m == db)
        return Vec2(x, bottom);
    if (m == dt)
        return Vec2(x, top);
    if (m == dl)
        return Vec2(left, y);
    if (m == dr)
        return Vec2(right, y);
    return Vec2(x, y);
}

void STLayerPanZoom::setPosition(const ax::Vec2 &position)
{
    if (isPanDisabled())
        return;

    // TODO: allow disabling bounds check (without resetting) for cutscenes

    // this->position is negative the world coord (w/scale factored in)
    Vec2 prevPosition = getPosition();
    Layer::setPosition(position);
    Vec2 newPos = position;

    // TODO: make this a game options setting
    // auto a = SCGlobals::DebugMode && SCGlobals::get("Enable Camera Boundary");
    if (!_worldBounds.equals(ax::Rect::ZERO))
    {
        // TODO: affect by scale

        Vec2 worldCoord = STCameraManager::get()->currentWorldCoord();
        if (!_worldBounds.containsPoint(worldCoord))
        {
            Vec2 newWorld = nearestPointInPerimiter(_worldBounds, worldCoord);
            float scale = getScaleX();
            newPos = STCameraManager::get()->worldToScreen(newWorld, scale);
        }
        // AXLOG("scale = %f, old = %s, new = %s, world = %s", getScale(), CStrFromPoint(prevPosition), CStrFromPoint(newPos), CStrFromPoint(worldCoord));
        Node::setPosition(newPos);
    }
    else
    {
        if (!_panBoundsRect.equals(ax::Rect::ZERO) && !_rubberEffectZooming)
        {
            if (_rubberEffectRatio != 0.f && _mode == kSTLayerPanZoomModeSheet)
            {
                if (!_rubberEffectRecovering)
                {
                    float topEdgeDist = topEdgeDistance();
                    float bottomEdgeDist = bottomEdgeDistance();
                    float leftEdgeDist = leftEdgeDistance();
                    float rightEdgeDist = rightEdgeDistance();
                    float dx = getPosition().x - prevPosition.x;
                    float dy = getPosition().y - prevPosition.y;
                    AXLOG("%f,%f,%f,%f => dx = %f, dy = %f", topEdgeDist, bottomEdgeDist, leftEdgeDist, rightEdgeDist, dx, dy);
                    if (bottomEdgeDist != 0.f || topEdgeDist != 0.f)
                    {
                        Node::setPosition(Vec2(getPosition().x,
                                               prevPosition.y + dy * _rubberEffectRatio));
                    }
                    if (leftEdgeDist != 0.f || rightEdgeDist != 0.f)
                    {
                        Node::setPosition(Vec2(prevPosition.x + dx * _rubberEffectRatio,
                                               getPosition().y));
                    }
                }
            }
            else
            {
                ax::Rect boundBox = getBoundingBox();
                // AXLOG("bbox = %s", CStrFromRect(boundBox));

                if (getPosition().x - boundBox.size.width * getAnchorPoint().x > _panBoundsRect.origin.x)
                {
                    Node::setPosition(Vec2(boundBox.size.width * getAnchorPoint().x + _panBoundsRect.origin.x,
                                           getPosition().y));
                }
                if (getPosition().y - boundBox.size.height * getAnchorPoint().y > _panBoundsRect.origin.y)
                {
                    Node::setPosition(Vec2(getPosition().x, boundBox.size.height * getAnchorPoint().y +
                                                                _panBoundsRect.origin.y));
                }
                if (getPosition().x + boundBox.size.width * (1 - getAnchorPoint().x) < _panBoundsRect.size.width +
                                                                                           _panBoundsRect.origin.x)
                {
                    Node::setPosition(Vec2(_panBoundsRect.size.width + _panBoundsRect.origin.x -
                                               boundBox.size.width * (1 - getAnchorPoint().x),
                                           getPosition().y));
                }
                if (getPosition().y + boundBox.size.height * (1 - getAnchorPoint().y) < _panBoundsRect.size.height +
                                                                                            _panBoundsRect.origin.y)
                {
                    Node::setPosition(Vec2(getPosition().x, _panBoundsRect.size.height + _panBoundsRect.origin.y -
                                                                boundBox.size.height * (1 - getAnchorPoint().y)));
                }
            }
        }

        Node::setPosition(Vec2(ceilf(getPosition().x),
                               ceilf(getPosition().y)));
    }
}

void STLayerPanZoom::setScale(float scale)
{
    if (isZoomDisabled() || isPanDisabled())
        return;

    scale = MAX(scale, _minScale);
    scale = MIN(scale, _maxScale);
    //    CCLOG("scale = %f [%f,%f]", scale, _minScale, _maxScale);
    Layer::setScale(scale);
}

// MARK: Ruber Edges related -

void STLayerPanZoom::recoverPositionAndScale()
{
    if (!_panBoundsRect.equals(ax::Rect::ZERO))
    {
        ax::Size winSize = Director::getInstance()->getWinSize();
        float rightEdgeDist = rightEdgeDistance();
        float leftEdgeDist = leftEdgeDistance();
        float topEdgeDist = topEdgeDistance();
        float bottomEdgeDist = bottomEdgeDistance();
        float scale = minPossibleScale();

        if (rightEdgeDist == 0.f && leftEdgeDist == 0.f && topEdgeDist == 0.f && bottomEdgeDist == 0.f)
        {
            return;
        }

        if (getScaleX() < scale)
        {
            _rubberEffectRecovering = true;
            Vec2 newPosition = Vec2::ZERO;
            if (rightEdgeDist != 0.f && leftEdgeDist != 0.f && topEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);
                newPosition = Vec2(winSize.width * 0.5f + dx, winSize.height * 0.5f + dy);
            }
            else if (rightEdgeDist != 0.f && leftEdgeDist != 0.f && topEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * (1.f - getAnchorPoint().y);
                newPosition = Vec2(winSize.width * 0.5f + dx, winSize.height - dy);
            }
            else if (rightEdgeDist != 0.f && leftEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                float dy = scale * getContentSize().height * getAnchorPoint().y;
                newPosition = Vec2(winSize.width * 0.5f + dx, dy);
            }
            else if (rightEdgeDist != 0.f && topEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (1.f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);
                newPosition = Vec2(winSize.width - dx, winSize.height * 0.5f + dy);
            }
            else if (leftEdgeDist != 0.f && topEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);
                newPosition = Vec2(dx, winSize.height * 0.5f + dy);
            }
            else if (leftEdgeDist != 0.f && topEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * (1.f - getAnchorPoint().y);
                newPosition = Vec2(dx, winSize.height - dy);
            }
            else if (leftEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * getAnchorPoint().x;
                float dy = scale * getContentSize().height * getAnchorPoint().y;
                newPosition = Vec2(dx, dy);
            }
            else if (rightEdgeDist != 0.f && topEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (1.f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * (1.f - getAnchorPoint().y);
                newPosition = Vec2(winSize.width - dx, winSize.height - dy);
            }
            else if (rightEdgeDist != 0.f && bottomEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (1.f - getAnchorPoint().x);
                float dy = scale * getContentSize().height * getAnchorPoint().y;
                newPosition = Vec2(winSize.width - dx, dy);
            }
            else if (topEdgeDist != 0.f || bottomEdgeDist != 0.f)
            {
                float dy = scale * getContentSize().height * (getAnchorPoint().y - 0.5f);
                newPosition = Vec2(getPosition().x, winSize.height * 0.5f + dy);
            }
            else if (leftEdgeDist != 0.f || rightEdgeDist != 0.f)
            {
                float dx = scale * getContentSize().width * (getAnchorPoint().x - 0.5f);
                newPosition = Vec2(winSize.width * 0.5f + dx, getPosition().y);
            }

            auto moveToPosition = MoveTo::create(_rubberEffectRecoveryTime, newPosition);
            auto scaleToPosition = ScaleTo::create(_rubberEffectRecoveryTime, scale, scale);
            auto callback = CallFunc::create([this]()
                                             { recoverEnded(); });
            auto spawn = Spawn::create(scaleToPosition, moveToPosition, callback, nullptr);
            runAction(spawn);
        }
        else
        {
            _rubberEffectRecovering = false;
            auto delta = Vec2(getPosition().x + rightEdgeDist - leftEdgeDist,
                              getPosition().y + topEdgeDist - bottomEdgeDist);
            auto callback = CallFunc::create([this]()
                                             { recoverEnded(); });
            auto moveToPosition = MoveTo::create(_rubberEffectRecoveryTime, delta);
            auto seq = Spawn::create(moveToPosition, callback, nullptr);
            runAction(seq);
        }
    }
}

void STLayerPanZoom::recoverEnded()
{
    _rubberEffectRecovering = false;
}

// MARK: Helpers -

float STLayerPanZoom::topEdgeDistance()
{
    ax::Rect boundBox = getBoundingBox();
    return round(MAX(_panBoundsRect.size.height + _panBoundsRect.origin.y - getPosition().y - boundBox.size.height * (1 - getAnchorPoint().y), 0.f));
}

float STLayerPanZoom::leftEdgeDistance()
{
    ax::Rect boundBox = getBoundingBox();
    return round(MAX(getPosition().x - boundBox.size.width * getAnchorPoint().x - _panBoundsRect.origin.x, 0.f));
}

float STLayerPanZoom::bottomEdgeDistance()
{
    ax::Rect boundBox = getBoundingBox();
    return round(MAX(getPosition().y - boundBox.size.height * getAnchorPoint().y - _panBoundsRect.origin.y, 0.f));
}

float STLayerPanZoom::rightEdgeDistance()
{
    ax::Rect boundBox = getBoundingBox();
    return round(MAX(_panBoundsRect.size.width + _panBoundsRect.origin.x - getPosition().x - boundBox.size.width * (1 - getAnchorPoint().x), 0));
}

float STLayerPanZoom::minPossibleScale()
{
    if (!_panBoundsRect.equals(ax::Rect::ZERO))
    {
        return MAX(_panBoundsRect.size.width / getContentSize().width,
                   _panBoundsRect.size.height / getContentSize().height);
    }
    else
    {
        return _minScale;
    }
}

CCLayerPanZoomFrameEdge STLayerPanZoom::frameEdgeWithPoint(const Vec2 &point)
{
    bool isLeft = point.x <= _panBoundsRect.origin.x + _leftFrameMargin;
    bool isRight = point.x >= _panBoundsRect.origin.x + _panBoundsRect.size.width - _rightFrameMargin;
    bool isBottom = point.y <= _panBoundsRect.origin.y + _bottomFrameMargin;
    bool isTop = point.y >= _panBoundsRect.origin.y + _panBoundsRect.size.height - _topFrameMargin;

    if (isLeft && isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottomLeft;
    }
    if (isLeft && isTop)
    {
        return kCCLayerPanZoomFrameEdgeTopLeft;
    }
    if (isRight && isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottomRight;
    }
    if (isRight && isTop)
    {
        return kCCLayerPanZoomFrameEdgeTopRight;
    }

    if (isLeft)
    {
        return kCCLayerPanZoomFrameEdgeLeft;
    }
    if (isTop)
    {
        return kCCLayerPanZoomFrameEdgeTop;
    }
    if (isRight)
    {
        return kCCLayerPanZoomFrameEdgeRight;
    }
    if (isBottom)
    {
        return kCCLayerPanZoomFrameEdgeBottom;
    }

    return kCCLayerPanZoomFrameEdgeNone;
}

float STLayerPanZoom::horSpeedWithPosition(const Vec2 &pos)
{
    CCLayerPanZoomFrameEdge edge = frameEdgeWithPoint(pos);
    float speed = 0.f;
    if (edge == kCCLayerPanZoomFrameEdgeLeft)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) *
                                (_panBoundsRect.origin.x + _leftFrameMargin - pos.x) / _leftFrameMargin;
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomLeft || edge == kCCLayerPanZoomFrameEdgeTopLeft)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) *
                                (_panBoundsRect.origin.x + _leftFrameMargin - pos.x) / (_leftFrameMargin * sqrtf(2));
    }
    if (edge == kCCLayerPanZoomFrameEdgeRight)
    {
        speed = -(_minSpeed + (_maxSpeed - _minSpeed) *
                                  (pos.x - _panBoundsRect.origin.x - _panBoundsRect.size.width +
                                   _rightFrameMargin) /
                                  _rightFrameMargin);
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomRight || edge == kCCLayerPanZoomFrameEdgeTopRight)
    {
        speed = -(_minSpeed + (_maxSpeed - _minSpeed) *
                                  (pos.x - _panBoundsRect.origin.x - _panBoundsRect.size.width +
                                   _rightFrameMargin) /
                                  (_rightFrameMargin * sqrtf(2)));
    }
    return speed;
}

float STLayerPanZoom::vertSpeedWithPosition(const Vec2 &pos)
{
    CCLayerPanZoomFrameEdge edge = frameEdgeWithPoint(pos);
    float speed = 0.f;
    if (edge == kCCLayerPanZoomFrameEdgeBottom)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) *
                                (_panBoundsRect.origin.y + _bottomFrameMargin - pos.y) / _bottomFrameMargin;
    }
    if (edge == kCCLayerPanZoomFrameEdgeBottomLeft || edge == kCCLayerPanZoomFrameEdgeBottomRight)
    {
        speed = _minSpeed + (_maxSpeed - _minSpeed) *
                                (_panBoundsRect.origin.y + _bottomFrameMargin - pos.y) / (_bottomFrameMargin * sqrtf(2));
    }
    if (edge == kCCLayerPanZoomFrameEdgeTop)
    {
        speed = -(_minSpeed + (_maxSpeed - _minSpeed) *
                                  (pos.y - _panBoundsRect.origin.y - _panBoundsRect.size.height +
                                   _topFrameMargin) /
                                  _topFrameMargin);
    }
    if (edge == kCCLayerPanZoomFrameEdgeTopLeft || edge == kCCLayerPanZoomFrameEdgeTopRight)
    {
        speed = -(_minSpeed + (_maxSpeed - _minSpeed) *
                                  (pos.y - _panBoundsRect.origin.y - _panBoundsRect.size.height +
                                   _topFrameMargin) /
                                  (_topFrameMargin * sqrtf(2)));
    }
    return speed;
}

// MARK: Input -

void STLayerPanZoom::tapWaitFinished()
{
    AXLOG("tap enabled");
    _tapEnabled = true;
    _tapCount = 0;
    _touches.clear();
}

void STLayerPanZoom::tapHandler()
{
    if (_tapCount == 2)
    {
        _delegate && _delegate->layerPanZoomClickedAtPoint(_tapPosition, 2, false);
    }
    else if (_tapCount == 1)
    {
        // TODO: check that hasn't moved since
        _delegate && _delegate->layerPanZoomClickedAtPoint(_tapPosition, 1, false);
    }

    // Resetting double tap count..
    AXLOG("resetting tap count");
    _tapCount = 0;
}

void STLayerPanZoom::setupInput()
{
    getEventDispatcher()->removeEventListenersForTarget(this);

#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))
    setupMouseInput();
    setupKeyboardInput();
#endif

    setupTouchInputInternal();
}

void STLayerPanZoom::setupMouseInput()
{
    auto director = Director::getInstance();
    auto ws = director->getWinSize();
    auto screenRect = ax::Rect(0, 0, ws.width, ws.height);

    auto listener0 = EventListenerMouse::create();

    listener0->onMouseDown = [this, screenRect](Event *event)
    {
        if (isInputDisabled())
        {
            return;
        }

        auto mouseEvent = static_cast<EventMouse *>(event);

        // bail if outside window
        Vec2 mouseLocation = mouseEvent->getLocationInView();
        if (!screenRect.containsPoint(mouseLocation))
        {
            return;
        }

        if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
        {
            _isMouseDown = true;
            Vec2 worldLocation = convertToNodeSpace(mouseLocation);
            _delegate && _delegate->layerPanZoomTouchBegan(this, worldLocation);
            _mouseBeganPositionRightDrag = mouseLocation;
            _mouseBeganPositionRightMouseLocation = mouseLocation;
        }
        else
        {
            _isMouseDown = true;
        }
    };

    listener0->onMouseMove = [this, screenRect](Event *event)
    {
        if (isInputDisabled())
        {
            return;
        }

        auto mouseEvent = static_cast<EventMouse *>(event);

        // bail if outside window
        Vec2 mouseLocation = mouseEvent->getLocationInView();

        if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
        {
            // handled by touch events
        }
        else if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
        {
            // let's allow panning with right mouse

            if (!screenRect.containsPoint(mouseLocation))
            {
                return;
            }

            if (_isMouseDown) // TODO: && ! isPanDisabled())
            {
                Vec2 worldLocation = convertToNodeSpace(mouseLocation);

                // TODO: may want to allow delegate to claim first?
                //_delegate && _delegate->layerPanZoomTouchMoved(this, worldLocation);

                // Always scroll in sheet mode.
                if (_mode == kSTLayerPanZoomModeSheet)
                {
                    // move "game camera" by moving map
                    auto delta = mouseLocation - _mouseBeganPositionRightDrag;
                    // AXLOG("delta = %s", CStrFromPoint(delta));
                    auto newPos = getPosition() + delta;
                    // AXLOG("newPos = %s", CStrFromPoint(newPos));
                    Layer::setPosition(newPos);
                    _mouseBeganPositionRightDrag = mouseLocation;
                }
            }
        }
        else
        {
            Vec2 worldLocation = convertToNodeSpace(mouseLocation);
            if (!_isMouseDown)
            {
                _delegate && _delegate->layerPanZoomMouseMovedOver(this, worldLocation);
            }
        }
    };

    listener0->onMouseUp = [this, screenRect](Event *event)
    {
        if (isInputDisabled())
        {
            return;
        }

        auto mouseEvent = static_cast<EventMouse *>(event);

        _isMouseDown = false;

        // bail if outside window
        Vec2 mouseLocation = mouseEvent->getLocationInView();
        if (!screenRect.containsPoint(mouseLocation))
        {
            return;
        }

        if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
        {
            // handled by touch events
        }
        else if (mouseEvent->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
        {
            // TODO: may want to invalidate the click if moves too far while in mousemove instead

            // check if close to began position
            auto dist = _mouseBeganPositionRightDrag.distance(_mouseBeganPositionRightMouseLocation);
            if (dist < getMaxTouchDistanceToClick())
            {
                // Didn't move, register as click
                Vec2 worldLocation = convertToNodeSpace(mouseLocation);
                _delegate && _delegate->layerPanZoomTouchEnded(this, worldLocation);
            }
        }
    };

    listener0->onMouseScroll = [this](Event *event)
    {
        if (isInputDisabled())
        {
            return;
        }
        if (isZoomDisabled())
        {
            return;
        }

        if (_delegate && !_delegate->layerPanZoomCanMouseEdgePanOrScrollZoom(this))
        {
            return;
        }

        if (STCameraManager::get()->isMoving())
        {
            return;
        }

        if (_zoomWaitTimer > 0)
        {
            return;
        }

        // TODO: check and make sure only works in window
        auto mouseEvent = static_cast<EventMouse *>(event);
        float scrollVelocity = mouseEvent->getScrollY();

        // TODO: could reset zoom wait timer
        if (_zoomWaitTimer > 0)
        {
            // wait
        }
        else
        {
            // TODO: is SCALE_LARGE necessary?
            // don't process unless large enough scroll
            const float scrollVelocityRequired = 0.4f;
            if (fabsf(scrollVelocity) < scrollVelocityRequired)
            {
                return;
            }

            // TODO: should allow inverting, and changing rate of scroll

            // zoom only on single press
            auto curZoom = getScaleX();

            // find cur zoom?? TODO: WHY??
            auto n = _zoomLevelsNew.size();
            auto nextZoomIndex = 0;
            for (auto i = 0; i < n; ++i)
            {
                auto zoom = _zoomLevelsNew[i] * SCALE_LARGE;
                if (fabsf(zoom - curZoom) < FLT_EPSILON)
                {
                    nextZoomIndex = i;
                }
            }

            // get new zoom
            // int nextZoomIndex = _zoomIndex;

            if (scrollVelocity < 0)
            {
                nextZoomIndex++;
            }
            else if (scrollVelocity > 0)
            {
                nextZoomIndex--;
            }

            nextZoomIndex = clampf(nextZoomIndex, 0, int(_zoomLevelsNew.size() - 1));

            if (nextZoomIndex != _zoomIndex)
            {
                AXLOG("_zoomIndex = %d, nextzoomindex = %d", _zoomIndex, nextZoomIndex);
                auto newScale = _zoomLevelsNew[nextZoomIndex] * SCALE_LARGE;
                AXLOG("scaling to %f", newScale);

                float dur = 0.3f; // kZoomScaleAnimationDuration;

                STCameraManager::get()->moveBy(Vec2::ZERO, true, newScale, dur);

                _zoomIndex = nextZoomIndex;
                _isMouseScrolling = true;
                _zoomWaitTimer = dur * 2.f; // TODO: make into const
                _zoomWaitTimerDampen = _zoomWaitTimer;
                _lastMouseScrollVelocity = scrollVelocity;
            }
        }
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener0, this);
}

//// TODO:
// bool STLayerPanZoom::onTouchesBegan(const std::vector<Touch*>& pTouches, Event *)
//{
// }
// void STLayerPanZoom::onTouchesMoved(const std::vector<Touch*>& pTouches, Event *)
//{
// }
// void STLayerPanZoom::onTouchesEnded(const std::vector<Touch*>& pTouches, Event *)
//{
// }
// void STLayerPanZoom::onTouchesCancelled(const std::vector<Touch*>& pTouches, Event *)
//{
// }

void STLayerPanZoom::setupTouchInputInternal()
{
    AXLOG("enter: %p", this);
    // Create a "one by one" touch event listener (processes one touch at a time)
    auto listener1 = EventListenerTouchAllAtOnce::create();

    // Example of using a lambda expression to implement onTouchBegan event callback function
#warning TODO: listener1->onTouchesBegan = AX_CALLBACK_1(&STLayerPanZoom::onTouchesBegan, this);
    listener1->onTouchesBegan = [this](const std::vector<Touch *> &pTouches, Event *)
    {
        AXLOG("STLayerPanZoom[onTouchesBegan] isInputDisabled = %d", isInputDisabled());
        if (isInputDisabled())
        {
            return;
        }
        if (!isVisitableByVisitingCamera())
        {
            return;
        }

        for (auto &pTouch : pTouches)
        {
            Vec2 viewLocation = pTouch->getLocationInView();
            // AXLOG("touch view location: %s", CStrFromPoint(viewLocation));
            Vec2 screenLocation = Director::getInstance()->convertToGL(viewLocation);
            // AXLOG("touch screen location: %s", CStrFromPoint(screenLocation));
            Vec2 worldLocation = convertToNodeSpace(screenLocation);
            // AXLOG("touch world location: %s", CStrFromPoint(worldLocation));

            _touches.pushBack(pTouch);
        }

        if (_touches.size() == 1)
        {
            _touchMoveBegan = false;
            _singleTouchTimestamp = time(nullptr);

            Touch *touch = (Touch *)_touches.at(0);
            Vec2 curPos = Director::getInstance()->convertToGL(touch->getLocationInView());
            _tapBeganPosition = convertToNodeSpace(curPos);

            // REMOVE: this seems to be useless, method prob already restricted by platform
#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))
            auto mouseLocation = touch->getLocationInView();
            // touch is not (or already) reversed Y
            auto ws = Director::getInstance()->getWinSize();
            mouseLocation.y = ws.height - mouseLocation.y;

            _isMouseDown = true;
            _mouseBeganPosition = mouseLocation;
            _delegate && _delegate->layerPanZoomMouseBegan(this, mouseLocation);
#endif
        }
        else
        {
            //            AXLOG("[began] multitouch");
            _singleTouchTimestamp = 999999;
        }

        // AXLOG("touch count = " PRINTF_ZD "", _touches->count());
    };

    listener1->onTouchesMoved = [this](const std::vector<Touch *> &, Event *)
    {
        if (isInputDisabled())
        {
            return;
        }

        if (_touches.size() >= 2)
        {
            // Get the two first touches
            Touch *touch1 = (Touch *)_touches.at(0);
            Touch *touch2 = (Touch *)_touches.at(1);

            // Get current and previous positions of the touches
            Vec2 curPosTouch1 = touch1->getLocation();
            Vec2 curPosTouch2 = touch2->getLocation();
            Vec2 prevPosTouch1 = touch1->getPreviousLocation();
            Vec2 prevPosTouch2 = touch2->getPreviousLocation();

            if (curPosTouch1 == curPosTouch2)
            {
                _touches.clear();
                return;
            }

            float curTouchDist = curPosTouch1.getDistance(curPosTouch2);
            float prevTouchDist = prevPosTouch1.getDistance(prevPosTouch2);

            // Calculate new scale
            float prevScale = getScaleX();
            float newScale = prevScale * curTouchDist / prevTouchDist;
            newScale = clampf(newScale, _minScale, _maxScale);

            // If current and previous position of the multitouch's center aren't equal -> change position of the layer
            Vec2 midpointDelta = Vec2::ZERO;

            // Calculate current and previous positions of the layer relative the anchor point
            Vec2 curMidpointTouch = curPosTouch1.getMidpoint(curPosTouch2);
            Vec2 prevMidpointTouch = prevPosTouch1.getMidpoint(prevPosTouch2);

            //            AXLOG("pos: %s", CStrFromPoint(getPosition()));
            //            AXLOG("cur1: %s", CStrFromPoint(curPosTouch1));
            //            AXLOG("cur2: %s", CStrFromPoint(curPosTouch2));
            //            AXLOG("midPrev: %s", CStrFromPoint(prevMidpointTouch));
            //            AXLOG("midCur: %s", CStrFromPoint(curMidpointTouch));
            //            AXLOGFloat(prevScale);
            //            AXLOGFloat(newScale);

            ////////////////////////////////////////////////////////////////////////
            // pan if the midpoint has moved
            // TODO: should probably check with an epsilon
            if (!prevMidpointTouch.equals(curMidpointTouch))
            {
                // In Screen Coords (opposite direction of world coords)
                Vec2 midpointPanDelta = (prevMidpointTouch - curMidpointTouch);
                // AXLOG("midpointPanDelta [before]: %s", CStrFromPoint(midpointPanDelta));

                // stretch/shrink to match whether zooming in/out
                // midpointPanDelta *= (newScale / prevScale);
                midpointPanDelta *= (prevScale / newScale);
                // AXLOG("midpointPanDelta [after]: %s", CStrFromPoint(midpointPanDelta));

                // convert into world coords
                // this mostly corrects for too much/little camera move if scale is <> 1
                // paranoia: FLT_EPSILON
                midpointPanDelta *= (1.f / (newScale + FLT_EPSILON));
                midpointDelta += midpointPanDelta;
                // AXLOG("midpointDelta: %s", CStrFromPoint(midpointDelta));
            }

            ////////////////////////////////////////////////////////////////////////
            // Pan/Move of camera required to center on midpoint while scaling
            {
                auto wsHalf = Director::getInstance()->getWinSize() * .5f;
                Vec2 screenCenter{wsHalf.width, wsHalf.height};
                Vec2 offsetFromCenter = curMidpointTouch - screenCenter;

                // ??? why 1/newscale ??? ... maybe because needs to also be in world coord???
                float scaleDelta = (prevScale - newScale);

                offsetFromCenter *= scaleDelta;
                offsetFromCenter *= (-1.f / prevScale);
                offsetFromCenter *= (1.f / newScale);

                midpointDelta += offsetFromCenter; // negative since camera screen is opposite camera world coord system
                                                   //                AXLOG("[test2]");
                                                   //                AXLOG("screenCenter: %s", CStrFromPoint(screenCenter));
                                                   //                AXLOG("offsetFromCenter: %s", CStrFromPoint(offsetFromCenter));
                                                   //                AXLOG("midpointDelta: %s", CStrFromPoint(midpointDelta));
                                                   //                AXLOG(scaleDelta);
            }

            // NOTE: this is move by in "world coords"
            STCameraManager::get()->moveBy(midpointDelta, false, newScale, 0.f);

            // Don't click with multitouch
            _touchDistance = 999999.9f;
        }
        else if (_touches.size() == 1)
        {
            Touch *touch = (Touch *)_touches.at(0);

            // Get the single touch and it's previous & current position.
            Vec2 curTouchPosition = touch->getLocation();
            Vec2 prevTouchPosition = touch->getPreviousLocation();

            // Accumulate touch distance for all modes.
            setTouchDistance(getTouchDistance() + curTouchPosition.getDistance(prevTouchPosition));

            // Inform delegate about starting updating touch position, if click isn't possible.
            if (getTouchDistance() > getMaxTouchDistanceToClick())
            {
                // MOVING

#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))
                Vec2 mouseLocation = touch->getLocationInView();
                // touch is not (or already) reversed Y
                auto ws = Director::getInstance()->getWinSize();
                mouseLocation.y = ws.height - mouseLocation.y;

                //            if(! screenRect.containsPoint(mouseLocation)) { return; }
                if (_isMouseDown)
                {
                    _delegate && _delegate->layerPanZoomMouseMoved(this, mouseLocation);
                }
#else
                bool claimed = false;
                if (!_touchMoveBegan)
                {
                    // ToDo add delegate here
                    //[self.delegate layerPanZoom: self
                    //   touchMoveBeganAtPosition: [self convertToNodeSpace: prevTouchPosition]];

                    //                        claimed = _delegate->layerPanZoomTouchBegan(this, convertToNodeSpace(prevTouchPosition));

                    _touchMoveBegan = true;
                }
                //                    else {
                //                        claimed = _delegate->layerPanZoomTouchMoved(this, convertToNodeSpace(prevTouchPosition));
                //                    }

                if (!claimed)
                {
                    // Always scroll in sheet mode.
                    if (_mode == kSTLayerPanZoomModeSheet)
                    {
                        // Set new position of the layer.
                        float x = getPosition().x + curTouchPosition.x - prevTouchPosition.x;
                        float y = getPosition().y + curTouchPosition.y - prevTouchPosition.y;
                        setPosition(Vec2(x, y));
                    }
                }
#endif
            }
        }
    };

    // Process the touch end event
    listener1->onTouchesEnded = [this](const std::vector<Touch *> &pTouches, Event *)
    {
        // AXLOG("[STLayerPanZoom::ccTouchesEnded] touch end");
        if (isInputDisabled())
        {
            for (auto &pTouch : pTouches)
            {
                _touches.eraseObject(pTouch);
            }
            return;
        }

        _singleTouchTimestamp = 9999999;

        // Process click event in single touch.
        // ToDo add delegate
        if ((getTouchDistance() < getMaxTouchDistanceToClick()) && getDelegate())
        {
            if (_touches.size() == 1)
            {
                Touch *touch = (Touch *)_touches.at(0);
                _tapPosition = touch->getLocation();

                // check for short circuited touch
                bool shortCircuited = _delegate->layerPanZoomClickedAtPoint(_tapPosition, 1, true);
                AXLOG("touched, and shortCircuited = %d", shortCircuited);
                if (shortCircuited)
                {
                    // delegate swalled the short circuit

                    //                    // (Maximum allowed delay between taps)
                    //                    DelayTime* delayAction = DelayTime::create(kDoubleTapDelay);
                    //                    CallFunc* callSelectorAction = CallFunc::create([this]() {
                    //                        tapWaitFinished();
                    //                    });
                    //                    runAction(Sequence::create(delayAction,callSelectorAction,nullptr));

                    //                    _tapEnabled = false;
                    _tapCount = 0;
                    _touches.clear();
                    stopActionByTag(5);
                    return;
                }
                else if (_tapCount == 0)
                {
                    //                    auto delayAction = DelayTime::create(kDoubleTapDelay);
                    //                    auto callSelectorAction = CallFunc::create([this]() {
                    //                        tapHandler();
                    //                    });
                    //                    auto seq = Sequence::createWithTwoActions(delayAction, callSelectorAction);
                    //                    seq->setTag(5);
                    //                    runAction(seq);
                }
                _tapCount++;
                AXLOG("tapcount = %d", _tapCount);
            }

            //                if(_tapCount == 0)
            //                {
            //                    _tapPosition = convertToNodeSpace(curPos);
            //
            //                    // TODO:
            //                    // - setup delayed handler
            //                    // (Maximum allowed delay between taps)
            //
            //                    // - if claimed cancel handler
            //
            //                    // - if delayed handler fires, call click
            //                    // - if tapcount 2 call click w/(2)
            //
            //                    else if(_tapEnabled)
            //                    {
            //                        float dist = _tapPosition.getDistanceSq(_tapBeganPosition);
            //                        const float maxDist = 10*10;
            //                        bool canDoublTap = _isDoubleTapAllowed && dist < maxDist;
            //                        if(canDoublTap)
            //                        {
            //                            // (Maximum allowed delay between taps)
            //                            auto action = getActionByTag(5);
            //                            if(! action || action->isDone())
            //                            {
            //                                auto delayAction = DelayTime::create(kDoubleTapDelay);
            //                                auto callSelectorAction = CallFunc::create([this]() {
            //                                    tapHandler();
            //                                });
            //                                auto seq = Sequence::createWithTwoActions(delayAction, callSelectorAction);
            //                                seq->setTag(5);
            //                                runAction(seq);
            //                            }
            //                            else {
            //                                dinfo("touch while taphandler action still running");
            //                            }
            //                        }
            //                        else
            //                        {
            //                            _delegate->layerPanZoomClickedAtPoint(_tapPosition, 1, false);
            //                            _tapCount = 0;
            //                            _touches.clear();
            //                            return;
            //                        }
            //                    }
            //                }
            //                else if(_tapCount > 2)
            //                {
            //                    CCLOG("_tapcount > 3");
            //                    _tapEnabled = false;
            //                    auto delayAction = DelayTime::create(kDoubleTapDelay);
            //                    auto callSelectorAction = CallFunc::create([this]() {
            //                        tapWaitFinished();
            //                    });
            //                    runAction(Sequence::createWithTwoActions(delayAction, callSelectorAction));
            //                    _tapCount = 0;
            //                    _touches.clear();
            //                    return;
            //                }
            //                else if(_tapCount > 0)
            //                {
            //                    dinfo("_tapcount was %d", _tapCount);
            //                }
            //
            //                ++_tapCount;
            //                CCLOG("tapcount = %d", _tapCount);
            //            }
            //            _touches.clear();
        }
        else if (!isPanDisabled())
        {
            bool multitouch = _touches.size() > 1;
            if (multitouch)
            {
                // Get the two first touches
                Touch *touch1 = (Touch *)_touches.at(0);
                Touch *touch2 = (Touch *)_touches.at(1);

                // Get current and previous positions of the touches
                auto loc1 = touch1->getLocationInView();
                auto loc2 = touch2->getLocationInView();
                auto prevLoc1 = touch1->getPreviousLocationInView();
                auto prevLoc2 = touch2->getPreviousLocationInView();
                Vec2 curPosTouch1 = Director::getInstance()->convertToGL(loc1);
                Vec2 curPosTouch2 = Director::getInstance()->convertToGL(loc2);
                Vec2 prevPosTouch1 = Director::getInstance()->convertToGL(prevLoc1);
                Vec2 prevPosTouch2 = Director::getInstance()->convertToGL(prevLoc2);

                if (curPosTouch1 == curPosTouch2)
                {
                    _touches.clear();
                    return;
                }

                float curTouchDist = curPosTouch1.getDistance(curPosTouch2);
                float prevTouchDist = prevPosTouch1.getDistance(prevPosTouch2);

                // Calculate new scale
                float prevScale = getScaleX();
                float newScale = prevScale;

                if (prevTouchDist > 0.1f)
                {
                    newScale = newScale * curTouchDist / prevTouchDist;

                    // ipad air (4x): 8,4,2,1.2
                    // ipad non-retina 3rd (2x): 8,4,2,1.2
                    // iphone 6 plus (3x):
                    // iphone 6 (3x):
                    // iphone 5s (3x):

                    float retinaScale = SCALE_LARGE;
                    if (newScale < 0.15f * retinaScale)
                    {
                        newScale = 0.1f * retinaScale;
                    }
                    else if (newScale < 0.25f * retinaScale)
                    {
                        newScale = 0.2f * retinaScale;
                    }
                    else if (newScale < 0.4f * retinaScale)
                    {
                        newScale = 0.3f * retinaScale;
                    }
                    else if (newScale < 0.75f * retinaScale)
                    {
                        newScale = 0.5f * retinaScale;
                    }
                    else if (newScale < 1.5f * retinaScale)
                    {
                        newScale = 1.f * retinaScale;
                    }
                    else
                    { // if(newScale < 2.5f * retinaScale) {
                        newScale = 2 * retinaScale;
                    }
                    //                    else { //if(newScale < 3.5f * retinaScale) {
                    //                        newScale = 3.f * retinaScale;
                    //                    }

                    if (newScale != prevScale)
                    {
                        // TODO: use same calcs as in onMove
                        Vec2 midpointDelta{Vec2::ZERO};
                        Vec2 curMidpointTouch = curPosTouch1.getMidpoint(curPosTouch2);

                        // test #2 move adjust to allow pinch to zoom in non-centered screen locations
                        {
                            auto wsHalf = Director::getInstance()->getWinSize() * .5f;
                            Vec2 screenCenter{wsHalf.width, wsHalf.height};
                            Vec2 offsetFromCenter = curMidpointTouch - screenCenter;
                            float scaleDelta = (prevScale - newScale) / newScale; // ??? why ??? ... maybe because needs to also be in world coord???
                            offsetFromCenter *= scaleDelta;
                            midpointDelta = -1.f / prevScale * offsetFromCenter * 1.f; // negative since camera screen is opposite camera world coord system
                                                                                       //                            AXLOG("[test2]");
                                                                                       //                            AXLOG("screenCenter: %s", CStrFromPoint(screenCenter));
                                                                                       //                            AXLOG("offsetFromCenter: %s", CStrFromPoint(offsetFromCenter));
                                                                                       //                            AXLOG("midpointDelta: %s", CStrFromPoint(midpointDelta));
                                                                                       //                            AXLOGFloat(scaleDelta);
                        }

                        bool animated = true;
                        float dur = kZoomScaleAnimationDuration;
                        // AXLOG("newScale = %f, retinaScale = %f", newScale);
                        STCameraManager::get()->moveBy(midpointDelta, animated, newScale, dur);
                    }
                }

                // Don't click with multitouch
                setTouchDistance(999999.9f);
            }
            else if (_touches.size() == 1)
            {
#if ((AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX))
                // Get the two first touches
                Touch *touch = (Touch *)_touches.at(0);

                // handle end for moved
                Vec2 mouseLocation = touch->getLocationInView();
                // touch is not (or already) reversed Y
                auto ws = Director::getInstance()->getWinSize();
                mouseLocation.y = ws.height - mouseLocation.y;

                // TODO: remove or use for making sure this event is valid
                if (_isMouseDown)
                {
                }

                auto dist = _mouseBeganPosition.distance(mouseLocation);
                if (dist < getMaxTouchDistanceToClick())
                {
                    // mouse didn't move, register as CLICK event
                    Vec2 worldLocation = convertToNodeSpace(mouseLocation);
                    // dinfo2("calling TOUCH for single click @ %s", CStrFromPoint(worldLocation));
                    _delegate && _delegate->layerPanZoomClickedAtPoint(worldLocation, 1, false);
                }
                else
                {
                    // dinfo2("calling MOUSE END @ %s", CStrFromPoint(mouseLocation));
                    _delegate && _delegate->layerPanZoomMouseEnded(this, mouseLocation);
                }
#else
                // nothing
#endif
            }
        }

        // remove touches
        for (auto &pTouch : pTouches)
        {
            _touches.eraseObject(pTouch);
        }

        if (_touches.size() == 0)
        {
            setTouchDistance(0.f);
        }
    };

#warning TODO: listener1->onTouchesCancelled = std::bind();
    listener1->onTouchesCancelled = [this](const std::vector<Touch *> &pTouches, Event *)
    {
        for (auto &pTouch : pTouches)
        {
            _touches.eraseObject(pTouch);
        }

        if (_touches.size() == 0)
        {
            setTouchDistance(0.f);
        }
    };

    // Add listener
    AXLOG("adding stlayerpanzoom event listener1");
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener1, this);
}

static const float keyInputCameraSpeed = 1000.f;
void STLayerPanZoom::setupKeyboardInput()
{
    // TODO: keystates don't get removed
    if (_cameraInputs.empty())
    {
        _cameraInputs[EventKeyboard::KeyCode::KEY_A] = Vec2(-keyInputCameraSpeed, 0);
        _cameraInputs[EventKeyboard::KeyCode::KEY_D] = Vec2(keyInputCameraSpeed, 0);
        _cameraInputs[EventKeyboard::KeyCode::KEY_W] = Vec2(0, keyInputCameraSpeed);
        _cameraInputs[EventKeyboard::KeyCode::KEY_S] = Vec2(0, -keyInputCameraSpeed);
    }

    if (_cameraInputsArrows.empty())
    {
        _cameraInputsArrows[EventKeyboard::KeyCode::KEY_LEFT_ARROW] = Vec2(-keyInputCameraSpeed, 0);
        _cameraInputsArrows[EventKeyboard::KeyCode::KEY_RIGHT_ARROW] = Vec2(keyInputCameraSpeed, 0);
        _cameraInputsArrows[EventKeyboard::KeyCode::KEY_UP_ARROW] = Vec2(0, keyInputCameraSpeed);
        _cameraInputsArrows[EventKeyboard::KeyCode::KEY_DOWN_ARROW] = Vec2(0, -keyInputCameraSpeed);
    }

    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [this](ax::EventKeyboard::KeyCode keyCode, Event *)
    {
        // dinfo2("PANZOOM :: key pressed: %d", keyCode);
        _keyState[keyCode] = true;
    };

    listener->onKeyReleased = [this](ax::EventKeyboard::KeyCode keyCode, Event *)
    {
        // dinfo2("PANZOOM :: key released: %d", keyCode);
        _keyState[keyCode] = false;

        if (STCameraManager::get()->isMoving())
        {
            return;
        }
        else if (!isZoomDisabled())
        {
            // zoom only on single press
            auto curZoom = getScaleX();

            // TODO: find cur zoom ... WHY?
            auto n = _zoomLevelsNew.size();
            size_t nextZoomIndex = 0;
            for (auto i = 0; i < n; ++i)
            {
                auto zoom = _zoomLevelsNew[i] * SCALE_LARGE;
                if (fabsf(zoom - curZoom) < FLT_EPSILON)
                {
                    nextZoomIndex = i;
                }
            }
            // nextZoomIndex = getZoomIndex(zoomLevels, curZoom);

            bool dirty = false;
            if (keyCode == EventKeyboard::KeyCode::KEY_Q)
            {
                if (nextZoomIndex > 0)
                {
                    nextZoomIndex--;
                    dirty = true;
                }
            }
            else if (keyCode == EventKeyboard::KeyCode::KEY_E)
            {
                if (nextZoomIndex < _zoomLevelsNew.size() - 1)
                {
                    nextZoomIndex++;
                    dirty = true;
                }
            }

            if (dirty)
            {
                nextZoomIndex = clampf(nextZoomIndex, 0, (_zoomLevelsNew.size() - 1));
                auto newScale = _zoomLevelsNew[nextZoomIndex] * SCALE_LARGE;
                STCameraManager::get()->moveBy(Vec2::ZERO, true, newScale);
            }
        }
    };

    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    //    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, kFixedPriorityPanZoomKeyboard);
}

/////////////////////////////////////////////////////
// MARK: -

// FIXME - is this needed? yes for the action based camera movement it is
void STLayerPanZoom::updateCameraAction(float dt)
{
    // TODO: counter-effect the simulation speed
    //    float simulationTimeScale = Director::getInstance()->getScheduler()->getTimeScale();
    //    AXASSERT(simulationTimeScale, "simulationTimeScale SHOULD NOT BE ZERO!");
    //    _elapsed += (dt * 1.f/simulationTimeScale);
    _elapsed += dt;

    switch (_stage)
    {
    case 0:
    {
        // delay
        if (_elapsed >= _targetDelay)
        {
            ++_stage;
            _elapsed = 0;
        }
    }
    break;
    case 1:
    {
        // done?
        if (_elapsed >= _targetTime)
        {
            ++_stage;
            _elapsed = _targetTime;
            // break;
        }
        // movement

#if AX_ENABLE_STACKABLE_ACTIONS
        // easeing - ease in
        float updateDt = MAX(0, // needed for rewind. elapsed could be negative
                             MIN(1, _elapsed /
                                        MAX(_targetTime, FLT_EPSILON) // division by 0
                                 ));
        auto time = tweenfunc::easeIn(updateDt, _easeRate);

        //            //_targetPos
        Vec2 currentPos = this->getPosition();

        Vec2 diff = currentPos - _previousPosition;
        _startPosition = _startPosition + diff;

        Vec2 newPos = _startPosition + (_deltaPosition * time);
        this->setPosition(newPos);
        _previousPosition = newPos;

        // scaling
        this->setScaleX(_startScaleX + _deltaScaleX * time);
        this->setScaleY(_startScaleY + _deltaScaleY * time);
        this->setScaleZ(1.f);
#else
#endif
    }
    break;
    case 2:
    {
        _stage = -1;
        _elapsed = 0;
        _targetTime = 0;
        if (_targetCallback)
        {
            _targetCallback();
        }
    }
    break;
    default:
    {
        // waiting
    }
    }
}

void STLayerPanZoom::runCameraAction(float delay, float duration, const Vec2 &newPos, float newScale, float easingRate, const std::function<void()> &callback)
{
    _targetCallback = callback;

    _stage = 0;
    _elapsed = 0;
    _targetTime = duration;

    _easeRate = easingRate;

    // moveto
    _startPosition = this->getPosition();
    _previousPosition = _startPosition;
    _previousScale = this->getScaleX();

    _targetDelay = delay;
    _targetScale = newScale;
    _targetPosition = newPos;

    _deltaPosition = _targetPosition - _previousPosition;

    // scaleto
    _startScaleX = this->getScaleX();
    _startScaleY = this->getScaleY();

    _endScaleX = newScale;
    _endScaleY = newScale;

    _deltaScaleX = _endScaleX - _startScaleX;
    _deltaScaleY = _endScaleY - _startScaleY;
}
