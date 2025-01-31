/*
 * InfiniteScrollOffset
 * Original from: https://github.com/danialias/InfiniteScrollNode/
 */

#ifndef InfiniteScrollNodeTest_InfiniteScrollOffset_h
#define InfiniteScrollNodeTest_InfiniteScrollOffset_h

#include "axmol.h"
USING_NS_AX;

class InfiniteScrollOffset : public ax::Layer {
public:
    virtual bool init();
    CREATE_FUNC(InfiniteScrollOffset);
    //
    AX_SYNTHESIZE(Point, scrollOffset, ScrollOffset);
    AX_SYNTHESIZE(Point, origPosition, OrigPosition);
    AX_SYNTHESIZE(Point, relVelocity, RelVelocity);
    AX_SYNTHESIZE(Point, ratio, Ratio);
    AX_SYNTHESIZE(Point, buffer, Buffer);
    AX_SYNTHESIZE(Node *, theChild, TheChild);

    static InfiniteScrollOffset *scrollWithNode(Node *node, Point r, Point p, Point s);
    static InfiniteScrollOffset *scrollWithNode(Node *node, Point r, Point p, Point s, Point v);
    InfiniteScrollOffset *initWithNode(Node *node, Point r, Point p, Point s, Point v);
    InfiniteScrollOffset *initWithNode(Node *node, Point r, Point p, Point s);
};

#endif
