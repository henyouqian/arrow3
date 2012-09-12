#include "stdafx.h"
#include "httpMsg.h"

#define HOST_AUTH "https://localhost/auth"

MsgLogin::MsgLogin(const char *gcid, const char *gcname){
    lwassert(gcid && gcname);
    std::stringstream ss;
    ss << HOST_AUTH << "/gclogin_e0252334753c435db65e9726544b6596?gcid=" << gcid << "&gcname=" << gcname;
    setURL(ss.str().c_str());
}

void MsgLogin::onRespond(int error){
    lwinfo(_buff.c_str());
}
