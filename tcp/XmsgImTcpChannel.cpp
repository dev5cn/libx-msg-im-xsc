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

#include "XmsgImTcpChannel.h"
#include "XmsgImTcpLog.h"
#include "XmsgImTcpH2N.h"
#include "../XmsgImTransUnidirectionInit.h"

XmsgImTcpChannel::XmsgImTcpChannel(ActorType type, shared_ptr<XscTcpWorker> wk, int mtu, int cfd, const string &peer) :
		XmsgImChannel(type, XscProtocolType::XSC_PROTOCOL_TCP, wk), XscTcpChannel(type, wk, mtu, cfd, peer)
{
	this->lastCheckTransTs = Xsc::clock;
}

int XmsgImTcpChannel::evnRecv(XscWorker* wk, uchar* dat, int len)
{
	int used = 0;
	while (true)
	{
		bool exp = false; 
		shared_ptr<XscProtoPdu> pdu = XscProtoPdu::decode(dat + used, len - used, &exp);
		if (pdu != nullptr)
		{
			if (Log::isRecord() && !(pdu->transm.indicator & 0x80))
			{
				auto usr = this->usr.lock();
				LOG_RECORD("\n  <-- PEER: %s CFD: %d NE: %s\n%s", this->peer.c_str(), this->cfd, usr == nullptr ? "null" : usr->uid.c_str(), pdu->print(dat + used, pdu->transm.len).c_str())
			}
			used += pdu->transm.len;
			this->incMsg();
			if (!this->evnMsg(wk, pdu, this->at == ActorType::ACTOR_N2H ? static_pointer_cast<XmsgImMsgMgr>(this->worker->msgMgr) : ((XmsgImTcpH2N*) this)->msgMgr))
				return -1;
			if (used == len) 
				return used;
			if (used < len) 
				continue;
			LOG_FAULT("it`s a bug, used: %d, len: %d", used, len)
			return -1;
		}
		if (exp)
		{
			LOG_DEBUG("xsc pdu format error, channel: %s, dat: %s", this->toString().c_str(), Net::printHex2str(dat + used, len - used).c_str())
			return -1;
		}
		return used; 
	}
}

void XmsgImTcpChannel::clean()
{
	this->cleanTransInit();
}

XmsgImTcpChannel::~XmsgImTcpChannel()
{

}

