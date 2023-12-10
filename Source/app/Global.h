#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "axmol.h"
USING_NS_AX;

class Global
{
public:
    Vec2 winSize;
    Vec2 visibleSize;
    Vec2 visibleOrigin;
    Vec2 origin;
    Rect safeArea;
    Vec2 safeOrigin;

    static Global &getInstance()
    {
        static Global instance;
        return instance;
    }

    float scaleFactor(const float &value)
    {
        return value * Director::getInstance()->getContentScaleFactor();
    }

    int scaleFactor(const int &value)
    {
        return value * Director::getInstance()->getContentScaleFactor();
    }

private:
    Global()
    {
        // size and origin
        winSize = Director::getInstance()->getWinSize();
        visibleSize = Director::getInstance()->getVisibleSize();
        visibleOrigin = Director::getInstance()->getVisibleOrigin();
        origin = Director::getInstance()->getVisibleOrigin();
        safeArea = Director::getInstance()->getSafeAreaRect();
        safeOrigin = safeArea.origin;

#if !defined(AX_TARGET_OS_TVOS)
        visibleSize = safeArea.size;
        origin = safeOrigin;
#endif
    }

    Global(Global const &) = delete;
    void operator=(Global const &) = delete;
};

#endif // __GLOBAL_H__
