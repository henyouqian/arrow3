#include "stdafx.h"
#include "taskPoker.h"
#include "taskCube.h"

void dealLocal(std::vector<int>& cards);
void dealLocal5(std::vector<int>& cards);
void replaceLocal(std::vector<int>& cards, std::vector<int>& idxs);
bool check();

void dealLocal(std::vector<int>& cards){
    while (1) {
        int n = rand()%32;
        std::vector<int>::iterator it = cards.begin();
        std::vector<int>::iterator itend = cards.end();
        bool find = false;
        for ( ; it != itend; ++it ){
            if ( n == *it ){
                find = true;
                break;
            }
        }
        if ( !find ){
            cards.push_back(n);
            return;
        }
    }
}

void dealLocal5(std::vector<int>& cards){
    for ( int i = 0; i < 5; ++i ){
        dealLocal(cards);
    }
}

void replaceLocal(std::vector<int>& cards, std::vector<int>& idxs){
    for ( int i = 0; i < idxs.size(); ++i ){
        dealLocal(cards);
    }
    std::vector<int>::iterator it = idxs.begin();
    std::vector<int>::iterator itend = idxs.end();
    for ( ;it != itend; ++it ){
        cards[*it] = cards.back();
        cards.pop_back();
    }
}

//=====================================

void TaskPoker::vBegin(){
    lw::srand();
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
    //TaskCube::s().start();
    //stop();
    
    if ( evt.type == lw::TouchEvent::TOUCH ){
        if ( evt.x > 160 ){
            _cards.clear();
            dealLocal5(_cards);
        }else{
            std::vector<int> idxs;
            idxs.push_back(1);
            idxs.push_back(2);
            replaceLocal(_cards, idxs);
        }
        std::sort(_cards.begin(), _cards.end());
        std::vector<int>::iterator it = _cards.begin();
        std::vector<int>::iterator itend = _cards.end();
        std::stringstream ss;
        for ( ; it != itend; ++it ){
            ss << *it << ",";
        }
        lwinfo(ss.str().c_str());
    }

    return false;
}

void TaskPoker::vClick(lw::Button* pButton){
    
}
