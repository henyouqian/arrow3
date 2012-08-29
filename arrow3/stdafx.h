//
// Prefix header for all source files of the 'lwtfTest' target in the 'lwtfTest' project
//

#import <Availability.h>

#ifndef __IPHONE_5_0
#warning "This project uses features only available in iOS SDK 5.0 and later."
#endif

#ifdef __OBJC__
    #import <UIKit/UIKit.h>
    #import <Foundation/Foundation.h>
#endif

#ifdef __cplusplus
    #include <OpenGLES/ES2/gl.h>
    #include "lwtf/lwPreInclude.h"
    #include "lwtf/lwLog.h"
    #include "lwtf/lwTask.h"
    #include "lwtf/lwSingleton.h"
    #include "lwtf/lwSprite.h"
    #include "lwtf/lwTouchEvent.h"
    #include "lwtf/lwSound.h"
    #include "lwtf/lwFileSys.h"
    #include "lwtf/lwTexture.h"

    #include "Box2D/Box2D.h"
#endif

extern const float B2_SCALE; 