#ifndef _APP_DELEGATE_H_
#define _APP_DELEGATE_H_

#include "axmol.h"
USING_NS_AX;

/**
@brief    The axmol Application.

Private inheritance here hides part of interface from Director.
*/
class AppDelegate : private Application {
public:
    AppDelegate();
    virtual ~AppDelegate();

    void preload();
    void initGLContextAttrs() override;

    /**
    @brief    Implement Director and Scene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
    bool applicationDidFinishLaunching() override;

    /**
    @brief  Called when the application moves to the background
    @param  the pointer of the application
    */
    void applicationDidEnterBackground() override;

    /**
    @brief  Called when the application reenters the foreground
    @param  the pointer of the application
    */
    void applicationWillEnterForeground() override;
};

#endif // _APP_DELEGATE_H_
