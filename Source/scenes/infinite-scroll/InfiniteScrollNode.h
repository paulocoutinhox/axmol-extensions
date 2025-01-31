/*
 * InfiniteScrollNode
 * Original from: https://github.com/danialias/InfiniteScrollNode/
 */

#ifndef InfiniteScrollNodeTest_InfiniteScrollNode_h
#define InfiniteScrollNodeTest_InfiniteScrollNode_h

#include "InfiniteScrollOffset.h"

#include "axmol.h"
USING_NS_AX;

#ifndef PTM_RATIO
#define PTM_RATIO 32
#endif

class InfiniteScrollNode : public ax::Layer {
public:
    virtual bool init();
    CREATE_FUNC(InfiniteScrollNode);

    SpriteBatchNode batch;
    Size _range;

    ax::Vector<InfiniteScrollOffset *> _scrollOffsets;

    void addChild(Sprite *node, int z, Point r, Point p, Point so);
    void addChild(Sprite *node, int z, Point r, Point p, Point so, Point v);

    void removeChild(Sprite *node, bool cleanup);
    void updateWithVelocity(Point vel, float dt);
    void updateWithYPosition(float y, float dt);

    void addInfiniteScrollWithZ(int z, Point ratio, Point pos, Point dir, Sprite *firstObject, ...);
    void addInfiniteScrollXWithZ(int z, Point ratio, Point pos, Sprite *firstObject, ...);
    void addInfiniteScrollYWithZ(int z, Point ratio, Point pos, Sprite *firstObject, ...);

    void addInfiniteScrollWithObjects(const ax::Vector<Sprite *> &objects, int z, Point ratio, Point pos, Point dir);
    void addInfiniteScrollWithObjects(const ax::Vector<Sprite *> &objects,
                                      int z,
                                      Point ratio,
                                      Point pos,
                                      Point dir,
                                      Point relVel);
    void addInfiniteScrollWithObjects(const ax::Vector<Sprite *> &objects,
                                      int z,
                                      Point ratio,
                                      Point pos,
                                      Point dir,
                                      Point relVel,
                                      Point padding);
};

#endif
