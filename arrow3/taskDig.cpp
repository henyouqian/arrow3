#include "stdafx.h"
#include "TaskDig.h"
#include "clod.h"

void TaskDig::vBegin(){
    //_pClod = new Clod(3,3,3);
    _pClod = new Clod(16,16,16);
}

void TaskDig::vEnd(){
    delete _pClod;
}

void TaskDig::vMain(){
    
}

void TaskDig::vDraw(){
    glClearColor(.6f, .6f, .6f, 1.0f);
    //glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    _pClod->draw();
}

bool TaskDig::vEvent(const lw::TouchEvent& evt){
    return _pClod->event(evt);
}
