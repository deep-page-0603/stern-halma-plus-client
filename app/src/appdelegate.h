
#pragma once

#include "global.h"

class AppDelegate : private Application {
public:
    AppDelegate();
    virtual ~AppDelegate();
    
	virtual void initGLContextAttrs();

    virtual bool applicationDidFinishLaunching();
    virtual void applicationDidEnterBackground();
    virtual void applicationWillEnterForeground();
};
