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

#include "XmsgImTransInitiative.h"
#include "tcp/XmsgImTcpLog.h"

XmsgImTransInitiative::XmsgImTransInitiative(shared_ptr<XscChannel> channel, function<void(SptrXiti trans)> cb) :
		XmsgImTrans(channel, XmsgImTransType::TRANS_TYPE_INIT)
{
	this->cb = cb;
	this->raw = false;
}

void XmsgImTransInitiative::end(ushort ret, const string& desc)
{
	this->ret = ret;
	this->desc = desc;
	this->ets = DateMisc::nowGmt0();
	XmsgImLog::getLog(this->channel.get())->transInitFinished(static_pointer_cast<XmsgImTransInitiative>(this->shared_from_this()), nullptr);
	this->cb(static_pointer_cast<XmsgImTransInitiative>(this->shared_from_this()));
}

bool XmsgImTransInitiative::end(shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.header != NULL && pdu->transm.header->oob != NULL)
	{
		this->ioob = new xsc_transmission_header_oob();
		for (auto& it : pdu->transm.header->oob->kv)
			this->ioob->kv.push_back(it);
	}
	bool ret = true;
	this->ret = pdu->transm.trans->ret;
	this->desc = pdu->transm.trans->desc;
	if (!pdu->transm.trans->msg.empty())
	{
		if (!this->raw)
		{
			auto msg = XmsgImMsgStub::newPbMsg(pdu->transm.trans->msg, pdu->transm.trans->dat, pdu->transm.trans->dlen);
			if (msg == NULL)
			{
				ret = false;
				this->ret = RET_EXCEPTION;
				this->desc = pdu->transm.trans->desc + "(can not reflect to a pb object from data)";
				LOG_DEBUG("can not reflect to a pb object from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
				goto label;
			}
			this->endMsg.reset(msg);
			goto label;
		}
		this->rawMsg.reset(new x_msg_im_trans_raw_msg());
		this->rawMsg->msg = pdu->transm.trans->msg;
		this->rawMsg->dat.assign((char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
	}
	label:
	this->ets = DateMisc::nowGmt0();
	XmsgImLog::getLog(this->channel.get())->transInitFinished(static_pointer_cast<XmsgImTransInitiative>(this->shared_from_this()), pdu);
	this->cb(static_pointer_cast<XmsgImTransInitiative>(this->shared_from_this()));
	return ret;
}

XmsgImTransInitiative::~XmsgImTransInitiative()
{

}

