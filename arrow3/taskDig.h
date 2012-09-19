#ifndef __TASK_DIG_H__
#define __TASK_DIG_H__

class Clod;

class TaskDig : public lw::Task, public lw::Singleton<TaskDig>{
public:
    virtual void vBegin();
    virtual void vEnd();
    virtual void vMain();
    virtual void vDraw();
    virtual bool vEvent(const lw::TouchEvent& evt);
    
private:
    Clod* _pClod;
};

#endif //__TASK_DIG_H__