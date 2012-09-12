#ifndef __HTTPMSG_H__
#define __HTTPMSG_H__

class MsgLogin : public lw::HTTPMsg{
public:
    MsgLogin(const char *gcid, const char *gcname);
    virtual void onRespond(int error);
    
};

#endif //__HTTPMSG_H__