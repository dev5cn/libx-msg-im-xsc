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

#include "XmsgImMsgMgr.h"

XmsgImMsgMgr::XmsgImMsgMgr() :
		XscMsgMgr()
{
	this->itcpCb = nullptr;
}

bool XmsgImMsgMgr::reg(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign)
{
	shared_ptr<XmsgImMsgStub> stub(new XmsgImMsgStub(begin, end, uni, cb, auth, foreign));
	if (this->msgs.find(stub->msg) != this->msgs.end())
	{
		LOG_ERROR("duplicate message: %s", stub->msg.c_str())
		return false;
	}
	this->msgs[stub->msg] = stub;
	return true;
}

void XmsgImMsgMgr::setItcp(function<XscMsgItcpRetType(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)> cb)
{
	this->itcpCb.reset(new function<XscMsgItcpRetType(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)>());
	*(this->itcpCb) = cb;
}

XscMsgItcpRetType XmsgImMsgMgr::itcp(XscWorker* wk, XscChannel* channel, shared_ptr<void> pdu)
{
	return this->itcpCb == nullptr ? XscMsgItcpRetType::DISABLE : ((*(this->itcpCb))(wk, channel, static_pointer_cast<XscProtoPdu>(pdu)));
}

shared_ptr<XmsgImMsgStub> XmsgImMsgMgr::getMsgStub(const string& msg)
{
	auto it = this->msgs.find(msg);
	return it == this->msgs.end() ? nullptr : it->second;
}

XmsgImMsgMgr::~XmsgImMsgMgr()
{

}

