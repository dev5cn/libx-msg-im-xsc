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

#include "XmsgImHttpChannel.h"
#include "net-x-msg-xsc-http.pb.h"
#include "../XmsgImLog.h"

XmsgImHttpChannel::XmsgImHttpChannel(XscHttpWorker* wk, int mtu, int cfd, const string &peer) :
		XmsgImChannel(ActorType::ACTOR_N2H, XscProtocolType::XSC_PROTOCOL_HTTP, wk), XscHttpChannel(wk, mtu, cfd, peer)
{

}

bool XmsgImHttpChannel::evnHeader(XscHttpWorker* wk, map<string, string>& header)
{
	auto it = header.find("x-msg-name"); 
	if (it == header.end())
	{
		LOG_DEBUG("header 'x-msg-name' is required, channel: %s", this->toXscChannel()->toString().c_str())
		return false;
	}
	this->msgName = it->second;
	it = header.find("x-msg-client-token");
	if (it != header.end())
		this->clientToken = it->second;
	it = header.find("x-msg-oob");
	if (it != header.end())
		this->oob = it->second;
	if (this->method != "GET")
		return true;
	it = header.find("x-msg-dat");
	if (it == header.end()) 
	{
		LOG_DEBUG("header 'x-msg-dat' is required, channel: %s", this->toXscChannel()->toString().c_str())
		return false;
	}
	this->dat = Crypto::base64dec(it->second);
	shared_ptr<XscProtoPdu> pdu(new XscProtoPdu());
	if (!this->oob.empty())
	{
		pdu->transm.header = new xsc_transmission_header();
		pdu->transm.header->oob = new xsc_transmission_header_oob();
		pdu->transm.header->oob->kv.push_back(make_pair<>(XSC_TAG_CLIENT_OOB, this->oob));
	}
	pdu->transm.trans = new XscProtoTransaction();
	pdu->transm.trans->trans = XSC_TAG_TRANS_BEGIN; 
	pdu->transm.trans->stid = this->genTid();
	pdu->transm.trans->dlen = this->dat.length();
	pdu->transm.trans->dat = (uchar*) this->dat.data();
	pdu->transm.trans->msg = this->msgName;
	if (Log::isRecord())
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		shared_ptr<XscChannel> channel = this->toXscChannel();
		auto usr = channel->usr.lock();
		bool exp;
		auto p = XscProtoPdu::decode(dat, len, &exp);
		LOG_RECORD("\n  --> PEER: %s CFD: %d NE: %s\n%s", channel->peer.c_str(), channel->cfd, usr == nullptr ? "" : usr->uid.c_str(), p == nullptr ? "exception" : p->print(dat, len).c_str())
	}
	if (this->worker->msgMgr == nullptr)
	{
		LOG_ERROR("current xsc worker not support receive any message, channel: %s", this->toXscChannel()->toString().c_str())
		return false;
	}
	shared_ptr<XscChannel> channel = this->toXscChannel();
	shared_ptr<XmsgImMsgMgr> msgMgr = static_pointer_cast<XmsgImMsgMgr>(this->worker->msgMgr);
	XscMsgItcpRetType rt = msgMgr->itcp(wk, channel.get(), pdu);
	if (rt == XscMsgItcpRetType::SUCCESS) 
		return true;
	else if (rt == XscMsgItcpRetType::FORBIDDEN) 
		return true;
	else if (rt == XscMsgItcpRetType::EXCEPTION) 
		return false;
	shared_ptr<XmsgImMsgStub> stub = msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im-msg-stub for msg: %s, channel: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	shared_ptr<Message> req = stub->newBegin(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (req == nullptr)
	{
		LOG_DEBUG("can not reflect a pb message from dat, msg: %s, channel: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	SptrXitp trans(new XmsgImTransPassive(channel, pdu));
	trans->beginMsg = req;
	trans->dtid = pdu->transm.trans->stid;
	XmsgImLog::getLog(channel.get())->transPassStart(trans, pdu); 
	if (!stub->auth)
	{
		((void (*)(shared_ptr<XscChannel> channel, SptrXitp trans, shared_ptr<Message> req)) (stub->cb))(channel, trans, req);
		return true;
	}
	auto usr = this->usr.lock();
	if (usr == nullptr) 
	{
		LOG_DEBUG("no permission, msg: %s, channel: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	((void (*)(shared_ptr<XscUsr> usr, SptrXitp trans, shared_ptr<Message> req)) (stub->cb))(usr, trans, req);
	return true;
}

bool XmsgImHttpChannel::evnBody(XscHttpWorker* wk, uchar* dat, int len, bool more)
{
	if (this->msgName.empty()) 
	{
		LOG_DEBUG("no 'x-msg-name' no body, channel: %s", this->toXscChannel()->toString().c_str())
		return false;
	}
	if (this->method != "POST") 
		return false;
	return true;
}

void XmsgImHttpChannel::evnDisc()
{
	LOG_DEBUG("x-msg-im http channel lost: %s", this->toXscChannel()->toString().c_str())
}

void XmsgImHttpChannel::dida(ullong now)
{

}

void XmsgImHttpChannel::clean()
{

}

void XmsgImHttpChannel::sendXscPdu(shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.trans == NULL)
	{
		LOG_FAULT("it`s a bug, no xsc transaction layer, channel: %s", this->toXscChannel()->toString().c_str())
		return;
	}
	if (pdu->transm.trans->trans != XSC_TAG_TRANS_END)
	{
		LOG_FAULT("it`s a bug, must be XSC_TAG_TRANS_END, channel: %s", this->toXscChannel()->toString().c_str())
		return;
	}
	XmsgImHttpRsp rsp;
	rsp.set_ret(pdu->transm.trans->ret);
	if (!pdu->transm.trans->desc.empty())
		rsp.set_desc(pdu->transm.trans->desc);
	if (!pdu->transm.trans->msg.empty())
		rsp.set_msg(pdu->transm.trans->msg);
	if (pdu->transm.trans->dlen > 0)
		rsp.set_dat((const char*) pdu->transm.trans->dat, pdu->transm.trans->dlen);
	string dat = rsp.SerializeAsString();
	this->sendBin((uchar*) dat.data(), dat.length(), dat.length());
}

string XmsgImHttpChannel::toString()
{
	return this->peer;
}

XmsgImHttpChannel::~XmsgImHttpChannel()
{

}

