#include "stdafx.h"
#include "taskPoker.h"
#include "taskCube.h"

const int CARD_NUM = 32;

void dealLocal(std::vector<int>& cards);
void dealLocal5(std::vector<int>& cards);
void replaceLocal(std::vector<int>& cards, std::vector<int>& idxs);
enum PokerResult{
    ERROR,
    ROYAL_FLUSH,
    STRAIGHT_FLUSH,
    FOUR_OF_A_KIND,
    FULL_HOUSE,
    FLUSH,
    STRAIGHT,
    THREE_OF_A_KIND,
    TWO_PAIRS,
    ONE_PAIR,
    SINGLE,
};
int getType(int card);
int getNum(int card);
PokerResult checkResult(std::vector<int>& cards);

void dealLocal(std::vector<int>& cards){
    while (1) {
        int n = rand()%CARD_NUM;
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
    cards.clear();
    for ( int i = 0; i < 5; ++i ){
        dealLocal(cards);
    }
    std::sort(cards.begin(), cards.end());
}

void replaceLocal(std::vector<int>& cards, std::vector<int>& idxs){
    if ( cards.size() != 5 ){
        return;
    }
    for ( int i = 0; i < idxs.size(); ++i ){
        dealLocal(cards);
    }
    std::vector<int>::iterator it = idxs.begin();
    std::vector<int>::iterator itend = idxs.end();
    for ( ;it != itend; ++it ){
        cards[*it] = cards.back();
        cards.pop_back();
    }
    std::sort(cards.begin(), cards.end());
}

int getType(int card){
    return card % 4;
}

int getNum(int card){
    return card / 4;
}

PokerResult checkResult(std::vector<int>& cards){
    if ( cards.size() != 5 ){
        return ERROR;
    }
    std::vector<int> sameNum1, sameNum2;
    bool isStraight = true;
    bool isFlush = true;
    std::vector<int>::iterator it = cards.begin();
    std::vector<int>::iterator itend = cards.end();
    int lastType = getType(*it);
    int lastNum = getNum(*it);
    sameNum1.push_back(*it);
    std::vector<int> *currSameNum = &sameNum1;
    ++it;
    for ( ; it != itend; ++it ){
        int type = getType(*it);
        int num = getNum(*it);
        if ( type != lastType )
            isFlush = false;
        if ( num != lastNum+1 ){
            isStraight = false;
        }
        if ( num == lastNum ){
            currSameNum->push_back(*it);
        }else{
            if ( currSameNum->size() > 1 && currSameNum == &sameNum1 ){
                currSameNum = &sameNum2;
                currSameNum->push_back(*it);
            }
        }
        lastType = type;
        lastNum = num;
    }
    if ( isStraight && isFlush && cards[4]==CARD_NUM-1 ){
        return ROYAL_FLUSH;
    }
    if ( isStraight && isFlush ){
        return STRAIGHT_FLUSH;
    }
    if ( sameNum1.size() == 4 ){
        return FOUR_OF_A_KIND;
    }
    if ( (sameNum1.size() == 3 && sameNum2.size() == 2)
        || (sameNum1.size() == 2 && sameNum2.size() == 3)){
        return FULL_HOUSE;
    }
    if ( isFlush ){
        return FLUSH;
    }
    if ( isStraight ){
        return STRAIGHT;
    }
    if ( sameNum1.size() == 3 || sameNum2.size() == 3 ){
        return THREE_OF_A_KIND;
    }
    if ( sameNum2.size() == 2 ){
        return TWO_PAIRS;
    }
    if ( sameNum1.size() == 2 ){
        return ONE_PAIR;
    }
    return SINGLE;
}

//=====================================

void TaskPoker::vBegin(){
    lw::srand();
    
    lw::addAtlas("tt.xml");
    
    _royalFlush = 0;
    _straightFlush = 0;
    _fourOfAKind = 0;
    _fullHouse = 0;
    _flush = 0;
    _straight = 0;
    _threeOfAKind = 0;
    _twoPairs = 0;
    _onePair = 0;
    _single = 0;
    _playNum = 0;
    
    _pBtn = lw::Button9::create(this, NULL, _af("btn9_common_up.png"), _af("btn9_common_down.png"), _af("btn9_common_up.png"), _as(2), _as(1), _as(2), _as(2), _as(1), _as(2), "a.fnt");
    _pBtn->setPos(_as(160), _as(240));
    _pBtn->setSize(_as(100), _as(100));
    _pBtn->getLabel()->setText("abcABC\n我靠");
    _pBtn->getLabel()->setColor(lw::COLOR_BLACK);
//    
//    _pLabel = lw::LabelBM::create(_af("a.fnt"));
//    _pLabel->setPos(100, 100);
//    _pLabel->setText("abcABC\n我靠");
//    _pLabel->setColor(lw::COLOR_BLACK);
    //_pLabel->setAlign(lw::ALIGN_MID_MID);
    //_pLabel->setScale(0.5f, 0.5f);
    
//    _pSpt = lw::Sprite::createFromAtlas(_as("goalKeeper.png"));
    //_pSpt->setUV(_as(0), _as(160), _as(22), _as(22));
    
//    _pBtnPlay = lw::Button::create(this, NULL, _af("btn_play_up.png"), _af("btn_play_down.png"), _af("btn_play_up.png"));
//    _pBtnPlay->setPos(_as(160), _as(240));
}

void TaskPoker::vEnd(){
    delete _pBtn;
//    delete _pLabel;
//    delete _pSpt;
//    delete _pBtnPlay;
}

void TaskPoker::vMain(){
    for ( int i = 0; i < 1000; ++i ){
        dealLocal5(_cards);
        ++_playNum;
        PokerResult r = checkResult(_cards);
        if ( r == ROYAL_FLUSH ){
            ++_royalFlush;
        }else if ( r == STRAIGHT_FLUSH ){
            ++_straightFlush;
        }else if ( r == FOUR_OF_A_KIND ){
            ++_fourOfAKind;
        }else if ( r == FULL_HOUSE ){
            ++_fullHouse;
        }else if ( r == FLUSH ){
            ++_flush;
        }else if ( r == STRAIGHT ){
            ++_straight;
        }else if ( r == THREE_OF_A_KIND ){
            ++_threeOfAKind;
        }else if ( r == TWO_PAIRS ){
            ++_twoPairs;
        }else if ( r == ONE_PAIR ){
            ++_onePair;
        }else if ( r == SINGLE ){
            ++_single;
        }
    }
    if ( _playNum % 10000 == 0 ){
        lw::srand();
    }
}

void TaskPoker::vDraw(){
    glClearColor(1, 1, 1, 1.0f);
    //glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    static float f = 0.f;
    f += .06f;
    //_pBtn->setPos(f, f);
//    _pLabel->setPos(f, f);
//    //_pLabel->setScale(f+1.f, f+1.f);
//    _pLabel->draw();
//    _pSpt->draw();
}

bool TaskPoker::vEvent(const lw::TouchEvent& evt){
    //TaskCube::s().start();
    //stop();
    
    if ( evt.type == lw::TouchEvent::TOUCH ){
        {
            std::stringstream ss;
            ss << "PlayNum:" << _playNum << "\n";
            ss << "ROYAL_FLUSH:" << _royalFlush << ",  " << (float)_royalFlush/_playNum*100.f << "%\n";
            ss << "STRAIGHT_FLUSH:" << _straightFlush << ",  " << (float)_straightFlush/_playNum*100.f << "%\n";
            ss << "FOUR_OF_A_KIND:" << _fourOfAKind << ",  " << (float)_fourOfAKind/_playNum*100.f << "%\n";
            ss << "FULL_HOUSE:" << _fullHouse << ",  " << (float)_fullHouse/_playNum*100.f << "%\n";
            ss << "FLUSH:" << _flush << ",  " << (float)_flush/_playNum*100.f << "%\n";
            ss << "STRAIGHT:" << _straight << ",  " << (float)_straight/_playNum*100.f << "%\n";
            ss << "THREE_OF_A_KIND:" << _threeOfAKind << ",  " << (float)_threeOfAKind/_playNum*100.f << "%\n";
            ss << "TWO_PAIRS:" << _twoPairs << ",  " << (float)_twoPairs/_playNum*100.f << "%\n";
            ss << "ONE_PAIR:" << _onePair << ",  " << (float)_onePair/_playNum*100.f << "%\n";
            ss << "SINGLE:" << _single << ",  " << (float)_single/_playNum*100.f << "%\n";
            lwinfo(ss.str().c_str());
            return true;
        }
        if ( evt.x > 160 ){
            dealLocal5(_cards);
        }else{
            std::vector<int> idxs;
            idxs.push_back(1);
            idxs.push_back(2);
            replaceLocal(_cards, idxs);
        }
        
        std::vector<int>::iterator it = _cards.begin();
        std::vector<int>::iterator itend = _cards.end();
        std::stringstream ss;
        for ( ; it != itend; ++it ){
            ss << getNum(*it) << ",";
        }
        
        PokerResult r = checkResult(_cards);
        if ( r == ROYAL_FLUSH ){
            ss << "ROYAL_FLUSH";
        }else if ( r == STRAIGHT_FLUSH ){
            ss << "STRAIGHT_FLUSH";
        }else if ( r == FOUR_OF_A_KIND ){
            ss << "FOUR_OF_A_KIND";
        }else if ( r == FULL_HOUSE ){
            ss << "FULL_HOUSE";
        }else if ( r == FLUSH ){
            ss << "FLUSH";
        }else if ( r == STRAIGHT ){
            ss << "STRAIGHT";
        }else if ( r == THREE_OF_A_KIND ){
            ss << "THREE_OF_A_KIND";
        }else if ( r == TWO_PAIRS ){
            ss << "TWO_PAIRS";
        }else if ( r == ONE_PAIR ){
            ss << "ONE_PAIR";
        }else if ( r == SINGLE ){
            ss << "SINGLE";
        }
        lwinfo(ss.str().c_str());
    }

    return false;
}

void TaskPoker::vClick(lw::Button9* pButton){
    
}
