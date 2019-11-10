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

#include "XmsgImN2HMsgMgr.h"

XmsgImN2HMsgMgr::XmsgImN2HMsgMgr(shared_ptr<XscServer> xscServer)
{
	this->xscServer = xscServer;
}

bool XmsgImN2HMsgMgr::reg(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign)
{
	shared_ptr<XmsgImMsgStub> stub(new XmsgImMsgStub(begin, end, uni, cb, auth, foreign));
	if (this->msgs.find(stub->msg) != this->msgs.end())
	{
		LOG_ERROR("duplicate message: %s", stub->msg.c_str())
		return false;
	}
	this->msgs[stub->msg] = stub;
	this->copyMsg2allXscWorker(stub->msg, stub);
	return true;
}

void XmsgImN2HMsgMgr::setItcp(function<XscMsgItcpRetType(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)> cb)
{
	XmsgImMsgMgr::setItcp(cb);
	shared_ptr<XscServer> xscServer = this->xscServer.lock();
	shared_ptr<XscJoinCounter> jc(new XscJoinCounter(xscServer->xscWorker.size()));
	for (uint i = 0; i < xscServer->xscWorker.size(); ++i)
	{
		shared_ptr<XscWorker> wk = xscServer->xscWorker[i];
		wk->future([xscServer, wk, jc, cb]
		{
			if( wk->msgMgr == nullptr)
			{
				wk->msgMgr.reset(new XmsgImN2HMsgMgr(xscServer));
			}
			auto msgMgr = static_pointer_cast<XmsgImN2HMsgMgr>(wk->msgMgr);
			msgMgr->itcpCb.reset(new function<XscMsgItcpRetType(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)>());
			*(msgMgr->itcpCb) = cb;
			jc->set();
		});
	}
	jc->wait();
}

void XmsgImN2HMsgMgr::copyMsg2allXscWorker(const string& msg, shared_ptr<XmsgImMsgStub> stub)
{
	shared_ptr<XscServer> xscServer = this->xscServer.lock();
	shared_ptr<XscJoinCounter> jc(new XscJoinCounter(xscServer->xscWorker.size()));
	for (uint i = 0; i < xscServer->xscWorker.size(); ++i)
	{
		shared_ptr<XscWorker> wk = xscServer->xscWorker[i];
		wk->future([xscServer, wk, msg, stub, jc]
		{
			if( wk->msgMgr == nullptr)
			{
				wk->msgMgr.reset(new XmsgImN2HMsgMgr(xscServer));
			}
			auto msgMgr = static_pointer_cast<XmsgImN2HMsgMgr>(wk->msgMgr);
			msgMgr->msgs[msg] = stub;
			jc->set();
		});
	}
	jc->wait();
}

XmsgImN2HMsgMgr::~XmsgImN2HMsgMgr()
{

}

