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

#include "XmsgImChannel.h"
#include "XmsgImLog.h"
#include "XmsgImN2HMsgMgr.h"
#include "XmsgImH2NMsgMgr.h"
#include "http/XmsgImHttpChannel.h"
#include "tcp/XmsgImTcpChannel.h"
#include "ws/XmsgImWebSocketChannel.h"

XmsgImChannel::XmsgImChannel(ActorType type, XscProtocolType proType, shared_ptr<XscWorker> wk)
{
	this->__worker__ = wk.get();
	this->at = type;
	this->pro = proType;
	this->tidSeq = Crypto::randomInt();
}

void XmsgImChannel::begin(shared_ptr<Message> req, function<void(SptrXiti trans)> cb, SptrOob oob, SptrXit upstreamTrans)
{
	auto xscChannel = this->toXscChannel();
	SptrXiti trans(new XmsgImTransInitiative(xscChannel, cb));
	trans->beginMsg = req;
	if (oob != nullptr)
	{
		for (auto& it : *oob)
			trans->addOob(it.first, it.second);
	}
	auto channel = this->cast();
	xscChannel->future([channel, trans, upstreamTrans]
	{
		channel->futureBegin(trans, upstreamTrans);
	});
}

void XmsgImChannel::begin(const string& msg, const string& dat, function<void(SptrXiti trans)> cb, SptrOob oob, bool raw, SptrXit upstreamTrans)
{
	auto xscChannel = this->toXscChannel();
	SptrXiti trans(new XmsgImTransInitiative(xscChannel, cb));
	trans->raw = raw;
	trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
	trans->rawMsg->msg = msg;
	trans->rawMsg->dat = dat;
	if (oob != nullptr)
	{
		for (auto& it : *oob)
			trans->addOob(it.first, it.second);
	}
	auto channel = this->cast();
	xscChannel->future([channel, trans, upstreamTrans]
	{
		channel->futureBegin(trans, upstreamTrans);
	});
}

void XmsgImChannel::futureBegin(SptrXiti trans, SptrXit upstreamTrans)
{
	trans->stid = this->genTid();
	this->transInit[trans->stid] = trans; 
	this->sendBegin(trans);
}

void XmsgImChannel::unidirection(shared_ptr<Message> uni, SptrOob oob, SptrXit upstreamTrans)
{
	auto xscChannel = this->toXscChannel();
	shared_ptr<XmsgImTransUnidirectionInit> trans(new XmsgImTransUnidirectionInit(xscChannel));
	trans->uniMsg = uni;
	if (oob != nullptr)
	{
		for (auto& it : *oob)
			trans->addOob(it.first, it.second);
	}
	auto channel = this->cast();
	xscChannel->future([channel, trans, upstreamTrans]
	{
		channel->futureUnidirection(trans, upstreamTrans);
	});
}

void XmsgImChannel::unidirection(const string& msg, const string& dat, SptrOob oob, SptrXit upstreamTrans)
{
	auto xscChannel = this->toXscChannel();
	shared_ptr<XmsgImTransUnidirectionInit> trans(new XmsgImTransUnidirectionInit(xscChannel));
	trans->rawMsg.reset(new x_msg_im_trans_raw_msg());
	trans->rawMsg->msg = msg;
	trans->rawMsg->dat = dat;
	if (oob != nullptr)
	{
		for (auto& it : *oob)
			trans->addOob(it.first, it.second);
	}
	auto channel = this->cast();
	xscChannel->future([channel, trans, upstreamTrans]
	{
		channel->futureUnidirection(trans, upstreamTrans);
	});
}

void XmsgImChannel::futureUnidirection(shared_ptr<XmsgImTransUnidirectionInit> trans, SptrXit upstreamTrans)
{
	this->sendUnidirection(trans);
}

void XmsgImChannel::sendBegin(SptrXiti trans)
{
	shared_ptr<XscProtoPdu> pdu(new XscProtoPdu());
	if (trans->ooob != NULL)
	{
		if (pdu->transm.header == NULL)
			pdu->transm.header = new xsc_transmission_header();
		pdu->transm.header->oob = new xsc_transmission_header_oob();
		for (auto& it : trans->ooob->kv)
			pdu->transm.header->oob->kv.push_back(it);
	}
	pdu->transm.trans = new XscProtoTransaction();
	pdu->transm.trans->trans = XSC_TAG_TRANS_BEGIN;
	pdu->transm.trans->partSeq = 0x00;
	pdu->transm.trans->haveNextPart = false;
	pdu->transm.trans->refDat = true;
	pdu->transm.trans->stid = trans->stid;
	pdu->transm.trans->dtid = 0x00;
	if (trans->rawMsg != nullptr)
	{
		pdu->transm.trans->msg = trans->rawMsg->msg;
		pdu->transm.trans->dlen = trans->rawMsg->dat.length();
		pdu->transm.trans->dat = (uchar*) trans->rawMsg->dat.data(); 
		XmsgImLog::getLog(trans->channel.get())->transInitStart(trans, pdu);
		this->sendPdu(pdu);
		return;
	}
	string pb = trans->beginMsg->SerializeAsString();
	pdu->transm.trans->msg = trans->beginMsg->GetDescriptor()->name();
	pdu->transm.trans->dlen = pb.length();
	pdu->transm.trans->dat = pb.length() < 1 ? NULL : (uchar*) pb.data(); 
	XmsgImLog::getLog(trans->channel.get())->transInitStart(trans, pdu);
	this->sendPdu(pdu);
}

void XmsgImChannel::sendEnd(SptrXitp trans)
{
	shared_ptr<XscProtoPdu> pdu(new XscProtoPdu());
	if (trans->ooob != NULL)
	{
		if (pdu->transm.header == NULL)
			pdu->transm.header = new xsc_transmission_header();
		pdu->transm.header->oob = new xsc_transmission_header_oob();
		for (auto& it : trans->ooob->kv)
			pdu->transm.header->oob->kv.push_back(it);
	}
	pdu->transm.trans = new XscProtoTransaction();
	pdu->transm.trans->trans = XSC_TAG_TRANS_END;
	pdu->transm.trans->partSeq = 0x00;
	pdu->transm.trans->haveNextPart = false;
	pdu->transm.trans->refDat = true;
	pdu->transm.trans->stid = 0x00;
	pdu->transm.trans->dtid = trans->dtid;
	pdu->transm.trans->ret = trans->ret;
	pdu->transm.trans->desc = trans->desc;
	if (trans->rawMsg != nullptr)
	{
		pdu->transm.trans->msg = trans->rawMsg->msg;
		pdu->transm.trans->dlen = trans->rawMsg->dat.length();
		pdu->transm.trans->dat = (uchar*) trans->rawMsg->dat.data();
		XmsgImLog::getLog(trans->channel.get())->transPassFinished(trans, pdu);
		this->sendPdu(pdu);
		return;
	}
	if (trans->endMsg == nullptr)
	{
		XmsgImLog::getLog(trans->channel.get())->transPassFinished(trans, pdu);
		this->sendPdu(pdu);
		return;
	}
	string pb = trans->endMsg->SerializeAsString();
	pdu->transm.trans->msg = trans->endMsg->GetDescriptor()->name();
	pdu->transm.trans->dlen = pb.length();
	pdu->transm.trans->dat = (uchar*) pb.data();
	XmsgImLog::getLog(trans->channel.get())->transPassFinished(trans, pdu);
	this->sendPdu(pdu);
}

void XmsgImChannel::sendEnd(uint dtid, ushort ret, const string& desc, shared_ptr<Message> endMsg, shared_ptr<XscProtoPdu> upstreamPdu , function<void(shared_ptr<XscProtoPdu> pdu)> cb)
{
	shared_ptr<XscProtoPdu> pdu(new XscProtoPdu());
	pdu->transm.trans = new XscProtoTransaction();
	pdu->transm.trans->trans = XSC_TAG_TRANS_END;
	pdu->transm.trans->partSeq = 0x00;
	pdu->transm.trans->haveNextPart = false;
	pdu->transm.trans->refDat = true;
	pdu->transm.trans->stid = 0x00;
	pdu->transm.trans->dtid = dtid;
	pdu->transm.trans->ret = ret;
	pdu->transm.trans->desc = desc;
	if (endMsg == nullptr)
	{
		cb(pdu); 
		this->sendPdu(pdu);
		return;
	}
	string pb = endMsg->SerializeAsString();
	pdu->transm.trans->dlen = pb.length();
	pdu->transm.trans->dat = (uchar*) pb.data();
	pdu->transm.trans->msg = endMsg->GetDescriptor()->name();
	cb(pdu); 
	this->sendPdu(pdu);
}

void XmsgImChannel::sendUnidirection(shared_ptr<XmsgImTransUnidirectionInit> trans)
{
	shared_ptr<XscProtoPdu> pdu(new XscProtoPdu());
	if (trans->ooob != NULL)
	{
		if (pdu->transm.header == NULL)
			pdu->transm.header = new xsc_transmission_header();
		pdu->transm.header->oob = new xsc_transmission_header_oob();
		for (auto& it : trans->ooob->kv)
			pdu->transm.header->oob->kv.push_back(it);
	}
	pdu->transm.trans = new XscProtoTransaction();
	pdu->transm.trans->trans = XSC_TAG_TRANS_UNIDIRECTION;
	pdu->transm.trans->partSeq = 0x00;
	pdu->transm.trans->haveNextPart = false;
	pdu->transm.trans->refDat = true;
	pdu->transm.trans->stid = 0x00;
	pdu->transm.trans->dtid = 0x00;
	if (trans->rawMsg != nullptr)
	{
		pdu->transm.trans->msg = trans->rawMsg->msg;
		pdu->transm.trans->dlen = trans->rawMsg->dat.length();
		pdu->transm.trans->dat = (uchar*) trans->rawMsg->dat.data();
		XmsgImLog::getLog(trans->channel.get())->transInitUni(trans, pdu);
		this->sendPdu(pdu);
		return;
	}
	string pb = trans->uniMsg->SerializeAsString();
	pdu->transm.trans->dlen = pb.length();
	pdu->transm.trans->dat = pb.length() < 1 ? NULL : (uchar*) pb.data();
	pdu->transm.trans->msg = trans->uniMsg->GetDescriptor()->name();
	XmsgImLog::getLog(trans->channel.get())->transInitUni(trans, pdu);
	this->sendPdu(pdu);
}

void XmsgImChannel::sendPdu(shared_ptr<XscProtoPdu> pdu)
{
	shared_ptr<XscChannel> channel = this->toXscChannel();
	if (this->pro != XscProtocolType::XSC_PROTOCOL_HTTP)
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		if (Log::isRecord())
		{
			auto usr = channel->usr.lock();
			bool exp;
			auto p = XscProtoPdu::decode(dat, len, &exp);
			LOG_RECORD("\n  --> PEER: %s CFD: %d NE: %s\n%s", 
					channel->peer.c_str(),
					channel->cfd,
					usr == nullptr ? "" : usr->uid.c_str(),
					p == nullptr ? "exception" : p->print(dat, len).c_str())
		}
		channel->send(dat, len);
		return;
	}
	if (Log::isRecord())
	{
		int len;
		uchar* dat = pdu->bytes(&len);
		auto usr = channel->usr.lock();
		bool exp;
		auto p = XscProtoPdu::decode(dat, len, &exp);
		LOG_RECORD("\n  --> PEER: %s CFD: %d NE: %s\n%s", 
				channel->peer.c_str(),
				channel->cfd,
				usr == nullptr ? "" : usr->uid.c_str(),
				p == nullptr ? "exception" : p->print(dat, len).c_str())
	}
	static_pointer_cast<XmsgImHttpChannel>(channel)->sendXscPdu(pdu);
}

uint XmsgImChannel::genTid()
{
	return ++(this->tidSeq);
}

bool XmsgImChannel::evnMsg(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XmsgImMsgMgr> msgMgr)
{
	shared_ptr<XscChannel> channel = this->toXscChannel();
	channel->lts = Xsc::clock;
	if (pdu->transm.indicator & XSC_TAG_TRANSM_PING) 
	{
		if (pdu->transm.indicator == XSC_TAG_TRANSM_PONG) 
			return true;
		static uchar pong = XSC_TAG_TRANSM_PONG;
		channel->send(&pong, 1);
		return true;
	}
	if (pdu->transm.trans == NULL) 
	{
		LOG_DEBUG("must be have transaction layer, this: %s", channel->toString().c_str())
		return false;
	}
	if (msgMgr == nullptr)
	{
		LOG_ERROR("current xsc worker not support receive any message, this: %s", channel->toString().c_str())
		return false;
	}
	XscMsgItcpRetType rt = msgMgr->itcp(wk, channel.get(), pdu);
	if (rt == XscMsgItcpRetType::SUCCESS) 
		return true;
	else if (rt == XscMsgItcpRetType::FORBIDDEN) 
		return true;
	else if (rt == XscMsgItcpRetType::EXCEPTION) 
		return false;
	bool ret = false;
	switch (pdu->transm.trans->trans)
	{
	case XSC_TAG_TRANS_BEGIN:
		ret = this->evnBegin(wk, pdu, channel, msgMgr);
		break;
	case XSC_TAG_TRANS_END:
		ret = this->evnEnd(wk, pdu, channel);
		break;
	case XSC_TAG_TRANS_UNIDIRECTION:
		ret = this->evnUnidirection(wk, pdu, channel, msgMgr);
		break;
	case XSC_TAG_TRANS_PARTIAL:
		ret = this->evnPartial(wk, pdu, channel);
		break;
	default:
		LOG_DEBUG("it`s a bug, unexpected XSC_TAG_TRANS: %04X", pdu->transm.trans->trans)
		break;
	}
	return ret;
}

bool XmsgImChannel::evnBegin(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel, shared_ptr<XmsgImMsgMgr> msgMgr)
{
	shared_ptr<XmsgImMsgStub> stub = msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im-msg-stub for msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	shared_ptr<Message> req = stub->newBegin(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (req == nullptr)
	{
		LOG_DEBUG("can not reflect a pb message from dat, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
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
	auto usr = channel->usr.lock();
	if (usr == nullptr) 
	{
		LOG_DEBUG("no permission, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	((void (*)(shared_ptr<XscUsr> usr, SptrXitp trans, shared_ptr<Message> req)) (stub->cb))(usr, trans, req);
	return true;
}

bool XmsgImChannel::evnEnd(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel)
{
	auto it = this->transInit.find(pdu->transm.trans->dtid); 
	if (it == this->transInit.end()) 
	{
		LOG_DEBUG("can not found transaction for tid: %08X, may be it was timeout, this: %s", pdu->transm.trans->dtid, this->toXscChannel()->toString().c_str())
		return true;
	}
	auto trans = it->second;
	this->transInit.erase(it); 
	return trans->end(pdu);
}

bool XmsgImChannel::evnUnidirection(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel, shared_ptr<XmsgImMsgMgr> msgMgr)
{
	shared_ptr<XmsgImMsgStub> stub = msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im-msg-stub for msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	shared_ptr<Message> unidirection = stub->newUnidirection(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (unidirection == nullptr)
	{
		LOG_DEBUG("can not reflect a unidirection message from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	if (!stub->auth)
	{
		SptrXitup trans(new XmsgImTransUnidirectionPass(channel, pdu));
		trans->uniMsg = unidirection;
		((void (*)(shared_ptr<XscChannel> channel, SptrXitup xitup, shared_ptr<Message> unidirection)) (stub->cb))(trans->channel, trans, trans->uniMsg);
		return true;
	}
	shared_ptr<XscUsr> usr = channel->usr.lock();
	if (usr == nullptr) 
	{
		LOG_DEBUG("no permission, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), channel->toString().c_str())
		return false;
	}
	SptrXitup trans(new XmsgImTransUnidirectionPass(channel, pdu));
	trans->uniMsg = unidirection;
	((void (*)(shared_ptr<XscUsr> usr, SptrXitup xitup, shared_ptr<Message> unidirection)) (stub->cb))(usr, trans, trans->uniMsg);
	return true;
}

bool XmsgImChannel::evnPartial(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel)
{
	return true;
}

void XmsgImChannel::checkTransInit(ullong now)
{
	if (now < this->lastCheckTransTs + 10 * DateMisc::sec)
		return;
	this->lastCheckTransTs = now;
	int tm = this->n2hTransTimeout();
	list<SptrXiti> tmp;
	for (auto it = this->transInit.begin(); it != this->transInit.end();)
	{
		SptrXiti trans = it->second;
		if (now < trans->gts + tm)
		{
			++it;
			continue;
		}
		tmp.push_back(trans);
		this->transInit.erase(it++);
	}
	if (tmp.empty())
		return;
	for (auto& it : tmp)
	{
		LOG_DEBUG("x-msg-im initiative transaction time out, elap: %dms, trans: %s", DateMisc::elap(now, it->gts), it->toString().c_str())
		it->end(RET_TIME_OUT, "local timeout");
	}
}

void XmsgImChannel::cleanTransInit()
{
	if (this->transInit.empty())
		return;
	list<SptrXiti> tmp;
	for (auto it = this->transInit.begin(); it != this->transInit.end();) 
	{
		tmp.push_back(it->second);
		this->transInit.erase(it++);
	}
	for (auto& it : tmp)
	{
		LOG_DEBUG("xsc tcp channel lost, we will end x-msg-im initiative transaction, this: %s, trans: %s", this->toXscChannel()->toString().c_str(), it->toString().c_str())
		it->end(RET_TIME_OUT, "local clean");
	}
}

shared_ptr<XmsgImChannel> XmsgImChannel::cast(shared_ptr<XscChannel> channel)
{
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_TCP)
		return static_pointer_cast<XmsgImTcpChannel>(channel);
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_HTTP)
		return static_pointer_cast<XmsgImHttpChannel>(channel);
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_WEBSOCKET)
		return static_pointer_cast<XmsgImWebSocketChannel>(channel);
	LOG_FAULT("it`s a bug, unexpected xsc protocol type, channel: %02X", channel->proType)
	return nullptr;
}

shared_ptr<XmsgImChannel> XmsgImChannel::cast()
{
	if (this->pro == XscProtocolType::XSC_PROTOCOL_TCP)
		return static_pointer_cast<XmsgImChannel>(static_pointer_cast<XmsgImTcpChannel>(((XmsgImTcpChannel*) this)->shared_from_this()));
	if (this->pro == XscProtocolType::XSC_PROTOCOL_HTTP)
		return static_pointer_cast<XmsgImChannel>(static_pointer_cast<XmsgImHttpChannel>(((XmsgImHttpChannel*) this)->shared_from_this()));
	if (this->pro == XscProtocolType::XSC_PROTOCOL_WEBSOCKET)
		return static_pointer_cast<XmsgImChannel>(static_pointer_cast<XmsgImWebSocketChannel>(((XmsgImWebSocketChannel*) this)->shared_from_this()));
	LOG_FAULT("it`s a bug, unexpected xsc protocol type, channel: %02X", this->pro)
	return nullptr;
}

shared_ptr<XscChannel> XmsgImChannel::toXscChannel()
{
	if (this->pro == XscProtocolType::XSC_PROTOCOL_TCP)
		return static_pointer_cast<XmsgImTcpChannel>(((XmsgImTcpChannel*) this)->shared_from_this());
	if (this->pro == XscProtocolType::XSC_PROTOCOL_HTTP)
		return static_pointer_cast<XmsgImHttpChannel>(((XmsgImHttpChannel*) this)->shared_from_this());
	if (this->pro == XscProtocolType::XSC_PROTOCOL_WEBSOCKET)
		return static_pointer_cast<XmsgImWebSocketChannel>(((XmsgImWebSocketChannel*) this)->shared_from_this());
	LOG_FAULT("it`s a bug, unexpected xsc protocol type, channel: %02X", this->pro)
	return nullptr;
}

int XmsgImChannel::n2hTransTimeout()
{
	int tm = 10;
	if (this->pro == XscProtocolType::XSC_PROTOCOL_TCP)
	{
		shared_ptr<XscTcpServer> tcpServer = Xsc::getXscTcpServer();
		auto cfg = static_pointer_cast<XscTcpCfg>(tcpServer->cfg);
		tm = this->at == ActorType::ACTOR_H2N ? cfg->h2nTransTimeout : cfg->n2hTransTimeout;
		tm *= DateMisc::sec;
		return tm;
	}
	if (this->pro == XscProtocolType::XSC_PROTOCOL_HTTP)
	{
		shared_ptr<XscHttpServer> httpServer = static_pointer_cast<XscHttpServer>(Xsc::getXscTcpServer());
		auto cfg = static_pointer_cast<XscHttpCfg>(httpServer->cfg);
		tm = this->at == ActorType::ACTOR_H2N ? cfg->h2nTransTimeout : cfg->n2hTransTimeout;
		tm *= DateMisc::sec;
		return tm;
	}
	if (this->pro == XscProtocolType::XSC_PROTOCOL_WEBSOCKET)
	{
		shared_ptr<XscWebSocketServer> webSocketServer = static_pointer_cast<XscWebSocketServer>(Xsc::getXscTcpServer());
		auto cfg = static_pointer_cast<XscWebSocketCfg>(webSocketServer->cfg);
		tm = this->at == ActorType::ACTOR_H2N ? cfg->h2nTransTimeout : cfg->n2hTransTimeout;
		tm *= DateMisc::sec;
		return tm;
	}
	LOG_FAULT("it`s a bug, unexpected xsc protocol type, channel: %02X", this->pro)
	return tm;
}

XmsgImChannel::~XmsgImChannel()
{

}

