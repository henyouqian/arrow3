#include "stdafx.h"
#include "taskPoker.h"
#include "taskCube.h"

void TaskPoker::vBegin(){
    

}

void TaskPoker::vEnd(){
    
}

void TaskPoker::vMain(){
    
}

void TaskPoker::vDraw(){
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
}

bool TaskPoker::vEvent(const lw::TouchEvent& evt){
    TaskCube::s().start();
    stop();

    return false;
}

void TaskPoker::vClick(lw::Button* pButton){
    
}
