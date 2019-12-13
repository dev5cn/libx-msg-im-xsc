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

#include "XmsgImUdpChannel.h"

XmsgImUdpChannel::XmsgImUdpChannel(XscUdpWorker* wk, int mtu, int cfd, const string &peer) :
		XmsgImChannel(ActorType::ACTOR_N2H, XscProtocolType::XSC_PROTOCOL_UDP, wk), XscUdpChannel(wk, mtu, cfd, peer)
{
	this->est = true;
}

int XmsgImUdpChannel::evnRecv(XscWorker* wk, uchar* dat, int len)
{
	bool exp = false; 
	shared_ptr<XscProtoPdu> pdu = XscProtoPdu::decode(dat, len, &exp);
	if (pdu == nullptr || exp)
	{
		LOG_DEBUG("xsc pdu format error, channel: %s, dat: %s", this->toString().c_str(), Net::printHex2str(dat, len).c_str())
		return -1;
	}
	if (Log::isRecord() && !(pdu->transm.indicator & 0x80))
	{
		auto usr = this->usr.lock();
		LOG_RECORD("\n  <-- PEER: %s CFD: %d NE: %s\n%s", this->peer.c_str(), this->cfd, usr == nullptr ? "null" : usr->uid.c_str(), pdu->print(dat, len).c_str())
	}
	this->incMsg();
	return this->evnMsg(wk, pdu, static_pointer_cast<XmsgImMsgMgr>(this->worker->msgMgr)) ? len : -1;
}

void XmsgImUdpChannel::dida(ullong now)
{

}

void XmsgImUdpChannel::evnDisc()
{

}

void XmsgImUdpChannel::clean()
{

}

string XmsgImUdpChannel::toString()
{
	return this->peer;
}

XmsgImUdpChannel::~XmsgImUdpChannel()
{

}

