#include "app.h"
#include "taskCube.h"
#include "taskPoker.h"

App::App(){
    new TaskCube();
//    TaskCube::s().start();
    new TaskPoker();
    TaskPoker::s().start();
}

App::~App(){
    delete TaskCube::ps();
    delete TaskPoker::ps();
}