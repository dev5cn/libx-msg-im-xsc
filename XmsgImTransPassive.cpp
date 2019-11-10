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
#include "XmsgImTransPassive.h"
#include "XmsgImTransInitiative.h"
#include "tcp/XmsgImTcpN2H.h"
#include "tcp/XmsgImTcpH2N.h"
#include "tcp/XmsgImTcpLog.h"

XmsgImTransPassive::XmsgImTransPassive(shared_ptr<XscChannel> channel, shared_ptr<XscProtoPdu> pdu ) :
		XmsgImTrans(channel, XmsgImTransType::TRANS_TYPE_PASS)
{
	this->lazyClose = false;
	if (pdu != nullptr && pdu->transm.header != NULL)
	{
		if (pdu->transm.header->oob != NULL) 
		{
			this->ioob = new xsc_transmission_header_oob();
			for (auto& it : pdu->transm.header->oob->kv)
				this->ioob->kv.push_back(it);
		}
	}
}

void XmsgImTransPassive::end(shared_ptr<Message> end)
{
	this->ret = RET_SUCCESS;
	this->endMsg = end;
	this->end();
}

void XmsgImTransPassive::end(ushort ret)
{
	this->ret = ret;
	this->end();
}

void XmsgImTransPassive::end(ushort ret, const string& desc, shared_ptr<Message> end)
{
	this->ret = ret;
	this->desc = desc;
	this->endMsg = end;
	this->end();
}

void XmsgImTransPassive::endDesc(ushort ret, const char* fmt, ...)
{
	this->ret = ret;
	if (fmt != NULL)
	{
		va_list va;
		::va_start(va, fmt);
		char* str = NULL;
		::vasprintf(&str, fmt, va);
		va_end(va);
		this->desc.assign(str);
		::free(str);
	}
	this->end();
}

void XmsgImTransPassive::endRaw(ushort ret, const string& desc, shared_ptr<x_msg_im_trans_raw_msg> rawMsg)
{
	this->ret = ret;
	this->desc = desc;
	this->rawMsg = rawMsg;
	this->end();
}

void XmsgImTransPassive::success()
{
	this->ret = RET_SUCCESS;
	this->end();
}

void XmsgImTransPassive::failure()
{
	this->ret = RET_FAILURE;
	this->end();
}

void XmsgImTransPassive::endAndLazyClose(shared_ptr<Message> end)
{
	this->lazyClose = true;
	this->end(end);
}

void XmsgImTransPassive::endAndLazyClose(ushort ret, const char* fmt, ...)
{
	this->lazyClose = true;
	this->ret = ret;
	if (fmt != NULL)
	{
		va_list va;
		::va_start(va, fmt);
		char* str;
		::vasprintf(&str, fmt, va);
		va_end(va);
		this->desc.assign(str);
		::free(str);
	}
	this->end();
}

void XmsgImTransPassive::end()
{
	SptrXitp trans = static_pointer_cast<XmsgImTransPassive>(this->shared_from_this());
	trans->ets = DateMisc::nowGmt0();
	trans->channel->future([trans]
	{
		trans->__end__(trans);
	});
}

void XmsgImTransPassive::__end__(SptrXitp trans)
{
	if (this->ret == RET_FAILURE) 
	{
		if (!this->channel->est) 
		{
			XmsgImLog::getLog(this->channel.get())->transPassFinished(trans, nullptr);
			return;
		}
		if (!this->lazyClose) 
			this->channel->close();
		else
			this->channel->lazyClose(); 
		XmsgImLog::getLog(this->channel.get())->transPassFinished(trans, nullptr);
		return;
	}
	if (!this->channel->est) 
	{
		XmsgImLog::getLog(this->channel.get())->transPassFinished(trans, nullptr);
		return;
	}
	XmsgImChannel::cast(this->channel)->sendEnd(trans); 
	if (this->lazyClose) 
		this->channel->lazyClose();
}

string XmsgImTransPassive::toString()
{
	string str = XmsgImTrans::toString();
	SPRINTF_STRING(&str, ", channel: %s, lazyClose: %s", this->channel->toString().c_str(), this->lazyClose ? "true" : "false")
	return str;
}

XmsgImTransPassive::~XmsgImTransPassive()
{

}

