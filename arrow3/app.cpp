#include "app.h"
#include "taskCube.h"
#include "taskPoker.h"
#include "taskDig.h"

App::App(){
    new TaskCube();
    new TaskPoker();
    new TaskDig();
    
//    TaskCube::s().start();
//    TaskPoker::s().start();   
    TaskDig::s().start();
}

App::~App(){
    delete TaskCube::ps();
    delete TaskPoker::ps();
    delete TaskDig::ps();
}