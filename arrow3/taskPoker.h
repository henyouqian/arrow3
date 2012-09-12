#ifndef __TASK_POKER_H__
#define __TASK_POKER_H__

class TaskPoker : public lw::Task, public lw::Singleton<TaskPoker>{
public:
    virtual void vBegin();
    virtual void vEnd();
    virtual void vMain();
    virtual void vDraw();
    virtual bool vEvent(const lw::TouchEvent& evt);
    
private:
    virtual void vClick(lw::Button* pButton);
    
    std::vector<int> _cards;
};

#endif //__TASK_POKER_H__