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

#include <libxsc-proto.h>
#include "XmsgImTrans.h"

XmsgImTrans::XmsgImTrans(shared_ptr<XscChannel> channel, XmsgImTransType xitt)
{
	this->xitt = xitt;
	this->channel = channel;
	this->stid = 0;
	this->dtid = 0;
	this->ret = 0;
	this->gts = DateMisc::nowGmt0();
	this->ets = 0ULL;
	this->ooob = NULL;
	this->ioob = NULL;
}

void XmsgImTrans::addOob(uchar tag, const string& val)
{
	if (this->ooob == NULL)
		this->ooob = new xsc_transmission_header_oob();
	this->ooob->kv.push_back(make_pair<>(tag, val));
}

bool XmsgImTrans::getOob(uchar tag, string& val)
{
	if (this->ioob == nullptr)
		return false;
	for (auto& it : this->ioob->kv)
	{
		if (it.first != tag)
			continue;
		val = it.second;
		return true;
	}
	return false;
}

bool XmsgImTrans::haveOob(uchar tag)
{
	if (this->ioob == nullptr)
		return false;
	for (auto& it : this->ioob->kv)
	{
		if (it.first != tag)
			continue;
		return true;
	}
	return false;
}

SptrOob XmsgImTrans::newOob(vector<pair<uchar, string>>& oob)
{
	SptrOob o = XmsgImTrans::newOob();
	for (auto& it : oob)
		o->push_back(it);
	return o;
}

SptrOob XmsgImTrans::newOob()
{
	SptrOob oob(new list<pair<uchar, string>>());
	return oob;
}

string XmsgImTrans::toString()
{
	string str;
	SPRINTF_STRING(&str, "begin(%s): %s, end(%s): %s, uni(%s): %s, stid: %08X, dtid: %08X, ret: %04X, desc: %s, gts: %s, ets: %s", 
			this->beginMsg == nullptr ? "null" : this->beginMsg->GetDescriptor()->name().c_str(),
			this->beginMsg == nullptr ? "null" : this->beginMsg->ShortDebugString().c_str(), 
			this->endMsg == nullptr ? "null" : this->endMsg->GetDescriptor()->name().c_str(),
			this->endMsg == nullptr ? "null" : this->endMsg->ShortDebugString().c_str(), 
			this->uniMsg == nullptr ? "null" : this->uniMsg->GetDescriptor()->name().c_str(),
			this->uniMsg == nullptr ? "null" : this->uniMsg->ShortDebugString().c_str(), 
			this->stid,
			this->dtid,
			this->ret,
			this->desc.c_str(),
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->gts).c_str(),
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->ets).c_str())
	return str;
}

XmsgImTrans::~XmsgImTrans()
{
	if (this->ooob != NULL)
	{
		delete this->ooob;
		this->ooob = NULL;
	}
	if (this->ioob != NULL)
	{
		delete this->ioob;
		this->ioob = NULL;
	}
}

