//
//  lwGLKView.m
//  arrow2
//
//  Created by Li Wei on 12-8-23.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "lwGLKView.h"
#include "lwtf/lwTouchEvent.h"

@implementation lwGLKView

//The EAGL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:.
- (id)initWithCoder:(NSCoder*)coder
{
    self = [super initWithCoder:coder];
	if (self) {
        self.multipleTouchEnabled = TRUE;
        inTouchEnd = false;
        
        CGRect rect_screen = [[UIScreen mainScreen]bounds];
        CGSize size_screen = rect_screen.size;
        
        CGFloat scale_screen = [UIScreen mainScreen].scale;
        NSLog(@"%f, %f, %f", size_screen.width, size_screen.height, scale_screen);
    }
    
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event{
	for ( UITouch* touch in touches ){
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
        lw::onTouchEvent(lw::TouchEvent::TOUCH, pt.x, pt.y, prevPt.x, prevPt.y);
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event{
	for ( UITouch* touch in touches ){
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
        lw::onTouchEvent(lw::TouchEvent::MOVE, pt.x, pt.y, prevPt.x, prevPt.y);
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event{
    inTouchEnd = true;
	for ( UITouch* touch in touches ){
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
        lw::onTouchEvent(lw::TouchEvent::UNTOUCH, pt.x, pt.y, prevPt.x, prevPt.y);
	}
    inTouchEnd = false;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event{
    if ( inTouchEnd ){
        return;
    }
	for ( UITouch* touch in touches ){
		CGPoint pt = [touch locationInView:self];
		CGPoint prevPt = [touch previousLocationInView:self];
        lw::onTouchEvent(lw::TouchEvent::UNTOUCH, pt.x, pt.y, prevPt.x, prevPt.y);
	}
}

@end
