#include "stdafx.h"
#include "TaskDig.h"
#include "clod.h"

void TaskDig::vBegin(){
    _pClod = new Clod(15,16,15);
}

void TaskDig::vEnd(){
    delete _pClod;
}

void TaskDig::vMain(){
    
}

void TaskDig::vDraw(){
    glClearColor(1, 1, 1, 1.0f);
    //glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    _pClod->draw();
}

bool TaskDig::vEvent(const lw::TouchEvent& evt){
    return _pClod->event(evt);
}
