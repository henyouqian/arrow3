#include "stdafx.h"
#include "TaskDig.h"
#include "clod.h"

void TaskDig::vBegin(){
    _pClod = new Clod(4, 4, 4);
}

void TaskDig::vEnd(){
    delete _pClod;
}

void TaskDig::vMain(){
    
}

void TaskDig::vDraw(){
    _pClod->draw();
}

bool TaskDig::vEvent(const lw::TouchEvent& evt){
    return false;
}
