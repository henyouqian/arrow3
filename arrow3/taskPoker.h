#ifndef __TASK_POKER_H__
#define __TASK_POKER_H__

class TaskPoker : public lw::Task, public lw::Singleton<TaskPoker>, public lw::ButtonCallback{
public:
    virtual void vBegin();
    virtual void vEnd();
    virtual void vMain();
    virtual void vDraw();
    virtual bool vEvent(const lw::TouchEvent& evt);
    
private:
    virtual void vClick(lw::Button9* pButton);
    
    std::vector<int> _cards;
    
    int _royalFlush;
    int _straightFlush;
    int _fourOfAKind;
    int _fullHouse;
    int _flush;
    int _straight;
    int _threeOfAKind;
    int _twoPairs;
    int _onePair;
    int _single;
    int _playNum;
    
    lw::Button9 *_pBtn;
    lw::Button *_pBtnPlay;
    lw::LabelBM *_pLabel;
    lw::Sprite *_pSpt;
};

#endif //__TASK_POKER_H__