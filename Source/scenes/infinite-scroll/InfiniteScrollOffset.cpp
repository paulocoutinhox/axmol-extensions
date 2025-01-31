//
//  InfiniteScrollOffset.cpp
//  InfiniteScrollNodeTest
//
//  Created by Jason Marziani on 3/26/12
//  Copyright (c) 2012 Little Wins LLC. All rights reserved.
//
//  Made compatible with Cocos2d-x 2.x by Stefan Nguyen on 7/30/12 (as CCParallaxScrollNode).
//  Made compatible with Axmol Engine 2.x by Daniel Alias on 23/12/09 (yy/mm/dd)
//

#include "InfiniteScrollOffset.h"

using namespace axmol;

USING_NS_AX;

// on "init" you need to initialize your instance
bool InfiniteScrollOffset::init() {
    if (!Layer::init())
        return false;

    this->setRelVelocity(Point(0, 0));
    this->setScrollOffset(Point(0, 0));
    this->setPosition(Point(0, 0));
    this->setRatio(Point(0, 0));
    return true;
}

InfiniteScrollOffset *InfiniteScrollOffset::scrollWithNode(Node *node, Point r, Point p, Point s) {
    InfiniteScrollOffset *offset = (InfiniteScrollOffset *)InfiniteScrollOffset::create();
    return (InfiniteScrollOffset *)offset->initWithNode(node, r, p, s);
}

InfiniteScrollOffset *InfiniteScrollOffset::scrollWithNode(Node *node, Point r, Point p, Point s, Point v) {
    InfiniteScrollOffset *offset = (InfiniteScrollOffset *)InfiniteScrollOffset::create();
    return (InfiniteScrollOffset *)offset->initWithNode(node, r, p, s, v);
}

InfiniteScrollOffset *InfiniteScrollOffset::initWithNode(Node *node, Point r, Point p, Point s, Point v) {
    // if(!this) return false;
    this->setRatio(r);
    this->setScrollOffset(s);
    this->setRelVelocity(v);
    this->setOrigPosition(p);
    node->setPosition(p);
    node->setAnchorPoint(Point(0, 0));
    this->setTheChild(node);
    return this;
}

InfiniteScrollOffset *InfiniteScrollOffset::initWithNode(Node *node, Point r, Point p, Point s) {
    return this->initWithNode(node, r, p, p, Point(0, 0));
}
