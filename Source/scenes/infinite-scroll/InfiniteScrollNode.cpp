//
//  InfiniteScrollNode.cpp
//  InfiniteScrollNodeTest
//
//  Created by Jason Marziani on 3/26/12
//  Copyright (c) 2012 Little Wins LLC. All rights reserved.
//
//  Made compatible with Cocos2d-x 2.x by Stefan Nguyen on 7/30/12 (as CCParallaxScrollNode).
//  Made compatible with Axmol Engine 2.x by Daniel Alias on 23/12/09 (yy/mm/dd)
//

#include "InfiniteScrollNode.h"

#ifndef SIGN
#    define SIGN(x) ((x < 0) ? -1 : (x > 0))
#endif

USING_NS_AX;

bool InfiniteScrollNode::init()
{
    if (!Layer::init())
        return false;

    Size screen = Director::getInstance()->getWinSize();
    _range      = Size(screen.width, screen.height);
    this->setAnchorPoint(Point(0, 0));

    return true;
}

// Used with box2d style velocity (m/s where m = 32 pixels), but box2d is not required
void InfiniteScrollNode::updateWithVelocity(Point vel, float dt)
{
    vel = vel * PTM_RATIO;

    for (int i = 0; i < _scrollOffsets.size(); i++)
    {
        InfiniteScrollOffset* scrollOffset = (InfiniteScrollOffset*)_scrollOffsets.at(i);
        Point relVel                       = scrollOffset->getRelVelocity() * PTM_RATIO;
        Point totalVel                     = vel + relVel;
        Point offset                       = ((totalVel * dt) * scrollOffset->getRatio());

        Node* child = scrollOffset->getTheChild();
        child->setPosition(child->getPosition() + offset);

        if ((offset.x < 0 && child->getPosition().x + child->getContentSize().width < 0) ||
            (offset.x > 0 && child->getPosition().x > _range.width))
        {
            child->setPosition(
                (child->getPosition() + Point(-SIGN(offset.x) * fabs(scrollOffset->getScrollOffset().x), 0)));
        }

        // Positive y indicates upward movement in cocos2d
        if ((offset.y < 0 && child->getPosition().y + child->getContentSize().height < 0) ||
            (offset.y > 0 && child->getPosition().y > _range.height))
        {
            child->setPosition(
                (child->getPosition() + Point(0, -SIGN(offset.y) * fabs(scrollOffset->getScrollOffset().y))));
        }
    }
}

void InfiniteScrollNode::updateWithYPosition(float y, float dt)
{
    for (int i = 0; i < _scrollOffsets.size(); i++)
    {
        InfiniteScrollOffset* scrollOffset = (InfiniteScrollOffset*)_scrollOffsets.at(i);

        Node* child  = scrollOffset->getTheChild();
        float offset = y * scrollOffset->getRatio().y;
        child->setPosition(Point(child->getPosition().x, scrollOffset->getOrigPosition().y + offset));
    }
}

void InfiniteScrollNode::addChild(Sprite* node, int z, Point r, Point p, Point so, Point v)
{
    node->setAnchorPoint(Point(0, 0));

    InfiniteScrollOffset* obj = InfiniteScrollOffset::scrollWithNode(node, r, p, so, v);
    _scrollOffsets.pushBack(obj);
    this->ax::Node::addChild(node, z);
}

void InfiniteScrollNode::addChild(Sprite* node, int z, Point r, Point p, Point so)
{
    this->addChild(node, z, r, p, so, Point(0, 0));
}

void InfiniteScrollNode::removeChild(Sprite* node, bool cleanup)
{
    ax::Vector<InfiniteScrollOffset*> tobeDeleted;

    for (int i = 0; i < _scrollOffsets.size(); i++)
    {
        InfiniteScrollOffset* scrollOffset = (InfiniteScrollOffset*)_scrollOffsets.at(i);

        if (scrollOffset->getTheChild() == node)
        {
            tobeDeleted.pushBack(scrollOffset);
            break;
        }
    }

    for (int i = 0; i < tobeDeleted.size(); i++)
    {
        _scrollOffsets.eraseObject(tobeDeleted.at(i));
    }
}

void InfiniteScrollNode::addInfiniteScrollWithObjects(const ax::Vector<Sprite*>& objects,
                                                      int z,
                                                      Point ratio,
                                                      Point pos,
                                                      Point dir,
                                                      Point relVel,
                                                      Point padding)
{
    // NOTE: corrects for 1 pixel at end of each sprite to avoid thin lines appearing

    // Calculate total width and height
    float totalWidth  = 0;
    float totalHeight = 0;

    for (int i = 0; i < objects.size(); i++)
    {
        Sprite* object = (Sprite*)objects.at(i);
        totalWidth += object->getContentSize().width * object->getScaleX() + dir.x * padding.x;
        totalHeight += object->getContentSize().height * object->getScaleY() + dir.y * padding.y;
    }

    // Position objects, add to parallax
    Point currPos = pos;
    for (int i = 0; i < objects.size(); i++)
    {
        Sprite* object = (Sprite*)objects.at(i);
        this->addChild(object, z, ratio, currPos, Point(totalWidth, totalHeight), relVel);
        Point nextPosOffset = Point(dir.x * (object->getContentSize().width * object->getScaleX() + padding.x),
                                    dir.y * (object->getContentSize().height * object->getScaleY() + padding.y));
        currPos             = currPos + nextPosOffset;
    }
}

void InfiniteScrollNode::addInfiniteScrollWithObjects(const ax::Vector<Sprite*>& objects,
                                                      int z,
                                                      Point ratio,
                                                      Point pos,
                                                      Point dir,
                                                      Point relVel)
{
    this->addInfiniteScrollWithObjects(objects, z, ratio, pos, dir, relVel, Point(-1, -1));
}

void InfiniteScrollNode::addInfiniteScrollWithObjects(const ax::Vector<Sprite*>& objects,
                                                      int z,
                                                      Point ratio,
                                                      Point pos,
                                                      Point dir)
{
    this->addInfiniteScrollWithObjects(objects, z, ratio, pos, dir, Point(0, 0));
}

void InfiniteScrollNode::addInfiniteScrollXWithZ(int z, Point ratio, Point pos, Sprite* firstObject, ...)
{
    ax::Vector<Sprite*> objects;

    va_list args;
    va_start(args, firstObject);

    for (Sprite* arg = firstObject; arg != NULL; arg = va_arg(args, Sprite*))
    {
        objects.pushBack(arg);
    }
    va_end(args);

    this->addInfiniteScrollWithObjects(objects, z, ratio, pos, Point(1, 0));
}

void InfiniteScrollNode::addInfiniteScrollYWithZ(int z, Point ratio, Point pos, Sprite* firstObject, ...)
{
    ax::Vector<Sprite*> objects;

    va_list args;
    va_start(args, firstObject);

    for (Sprite* arg = firstObject; arg != NULL; arg = va_arg(args, Sprite*))
    {
        objects.pushBack(arg);
    }
    va_end(args);

    this->addInfiniteScrollWithObjects(objects, z, ratio, pos, Point(0, 1));
}
