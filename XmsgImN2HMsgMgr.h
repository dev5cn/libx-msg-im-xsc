/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef XMSGIMN2HMSGMGR_H_
#define XMSGIMN2HMSGMGR_H_

#include "XmsgImMsgMgr.h"

#define X_MSG_N2H_PRPC_BEFOR_AUTH(__MSGMGR__, __BEGIN__, __END__, __CB__)					(__MSGMGR__->reg(__BEGIN__::descriptor(), __END__::descriptor(), NULL, (void*)(__CB__), false));
#define X_MSG_N2H_PRPC_AFTER_AUTH(__MSGMGR__, __BEGIN__, __END__, __CB__)					(__MSGMGR__->reg(__BEGIN__::descriptor(), __END__::descriptor(), NULL, (void*)(__CB__), true));
#define X_MSG_N2H_PRPC_BEFOR_AUTH_UNI(__MSGMGR__, __UNI__, __CB__)							(__MSGMGR__->reg(NULL, NULL, __UNI__::descriptor(), (void*)(__CB__), false));
#define X_MSG_N2H_PRPC_AFTER_AUTH_UNI(__MSGMGR__, __UNI__, __CB__)							(__MSGMGR__->reg(NULL, NULL, __UNI__::descriptor(), (void*)(__CB__), true));

class XmsgImN2HMsgMgr: public XmsgImMsgMgr
{
public:
	weak_ptr<XscServer> xscServer;
public:
	bool reg(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign = FOREIGN_FORBIDDEN); 
	void setItcp(function<XscMsgItcpRetType(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)> cb); 
	XmsgImN2HMsgMgr(shared_ptr<XscServer> xscServer);
	virtual ~XmsgImN2HMsgMgr();
private:
	void copyMsg2allXscWorker(const string& msg, shared_ptr<XmsgImMsgStub> stub); 
};

#endif 
