#include "app.h"
#include "taskCube.h"

App::App(){
    new TaskCube();
    TaskCube::s().start();
}

App::~App(){
    delete TaskCube::ps();
}