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

#include "XmsgImMsgStub.h"

XmsgImMsgStub::XmsgImMsgStub(const Descriptor* begin, const Descriptor* end, const Descriptor* unidirection, void* cb, bool auth, ForeignAccessPermission foreign)
{
	this->auth = auth;
	this->foreign = foreign;
	this->cb = cb;
	this->msg = begin != NULL ? begin->name() : (unidirection != NULL ? unidirection->name() : "");
	this->begin = begin;
	this->end = end;
	this->unidirection = unidirection;
}

shared_ptr<Message> XmsgImMsgStub::newBegin(uchar* dat, int len)
{
	if (this->begin == NULL)
		return nullptr;
	Message* msg = XmsgImMsgStub::newPbMsg(this->begin, dat, len);
	return msg == NULL ? nullptr : shared_ptr<Message>(msg);
}

shared_ptr<Message> XmsgImMsgStub::newUnidirection(uchar* dat, int len)
{
	if (this->unidirection == NULL)
		return nullptr;
	Message* msg = XmsgImMsgStub::newPbMsg(this->unidirection, dat, len);
	return msg == NULL ? nullptr : shared_ptr<Message>(msg);
}

Message* XmsgImMsgStub::newPbMsg(const Descriptor* desc)
{
	const Message* pt = MessageFactory::generated_factory()->GetPrototype(desc);
	if (pt == NULL)
	{
		LOG_DEBUG("can`t not found message type stub for this desc, pb name: %s", desc->name().c_str())
		return NULL;
	}
	return pt->New();
}

Message* XmsgImMsgStub::newPbMsg(const Descriptor* desc, uchar* dat, uint len)
{
	Message* msg = XmsgImMsgStub::newPbMsg(desc);
	if (msg == NULL)
		return NULL;
	if (len < 1 || dat == NULL)
		return msg;
	if (msg->ParseFromArray(dat, len))
		return msg;
	LOG_DEBUG("reflect message failed, name: %s", desc->name().c_str())
	delete msg;
	return NULL;
}

Message* XmsgImMsgStub::newPbMsg(const string& name, uchar* dat, uint len)
{
	const Descriptor* desc = DescriptorPool::generated_pool()->FindMessageTypeByName(name);
	if (desc == NULL)
	{
		LOG_DEBUG("can not found descriptor for name: %s", name.c_str())
		return NULL;
	}
	return XmsgImMsgStub::newPbMsg(desc, dat, len);
}

string XmsgImMsgStub::toString()
{
	string str;
	if (this->begin != NULL)
	{
		SPRINTF_STRING(&str, "auth: %s, begin: %s, end: %s", this->auth ? "true " : "false", this->begin->name().c_str(), this->end->name().c_str())
		return str;
	}
	SPRINTF_STRING(&str, "auth: %s, unidirection: %s", this->auth ? "true " : "false", this->unidirection->name().c_str())
	return str;
}

XmsgImMsgStub::~XmsgImMsgStub()
{

}

