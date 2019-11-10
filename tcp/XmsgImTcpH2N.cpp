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

#include "XmsgImTcpH2N.h"
#include "XmsgImTcpLog.h"
#include "../XmsgImTransPassive.h"

XmsgImTcpH2N::XmsgImTcpH2N(shared_ptr<XscTcpServer> tcpServer, const string &peer) :
		XmsgImTcpChannel(ActorType::ACTOR_H2N, static_pointer_cast<XscTcpWorker>(tcpServer->rr()), tcpServer->cfg->peerRcvBuf, 0, peer)
{
	this->msgMgr.reset(new XmsgImH2NMsgMgr());
	this->needWait = false;
	this->trying = false;
	this->h2nReConn = tcpServer->cfg->h2nReConn;
	this->lastHbTs = Xsc::clock;
	this->msgMgr->setItcp([](XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
	{
		return ((XmsgImTcpH2N*)channel)->itcp(wk, channel, pdu);
	});
}

void XmsgImTcpH2N::connect()
{
	if (this->est)
	{
		LOG_ERROR("connection already established, this: %s", this->toString().c_str())
		return;
	}
	if (this->trying.exchange(true)) 
	{
		LOG_WARN("we are trying connect to remote server, this: %s", this->toString().c_str())
		return;
	}
	auto h2n = static_pointer_cast<XmsgImTcpH2N>(this->shared_from_this());
	thread t([h2n]()
	{
		h2n->svc(h2n);
	});
	t.detach();
}

void XmsgImTcpH2N::svc(shared_ptr<XmsgImTcpH2N> h2n)
{
	if (!h2n->needWait) 
	{
		h2n->future([h2n]
		{
			Xsc::getXscTcpWorker()->h2ns.push_back(h2n); 
		});
	}
	while (true)
	{
		if (h2n->needWait)
			Misc::sleep(h2n->h2nReConn * 1000L); 
		else
			h2n->needWait = true; 
		string ip;
		int port;
		Net::str2ipAndPort(h2n->peer.c_str(), &ip, &port);
		int cfd = Net::tcpConnect(ip.c_str(), port);
		if (cfd < 1)
		{
			LOG_WARN("can not connect to remote-addr, this: %s", h2n->toString().c_str())
			continue;
		}
		h2n->future([h2n, cfd]
		{
			LOG_INFO("connect to remote server successful, this: %s, cfd: %d", h2n->toString().c_str(), cfd)
			h2n->est = true;
			h2n->cfd = cfd;
			h2n->dlen = 0;
			h2n->cleanWbuf();
			h2n->wbuf = new queue<xsc_tcp_channel_wbuf*>();
			h2n->gts = Xsc::clock;
			h2n->lts = 0ULL;
			h2n->stat.clear();
			Xsc::getXscTcpWorker()->setFdAtt((XscTcpServer*)(h2n->worker->server), cfd);
			Xsc::getXscTcpWorker()->addTcpChannel(h2n);
			Xsc::getXscTcpWorker()->addCfd4Read(cfd);
			h2n->estab();
		});
		break;
	}
	this->trying.exchange(false);
}

void XmsgImTcpH2N::dida(ullong now)
{
	if (!this->est)
		return;
	this->checkTransInit(now);
	this->heartbeat(now);
}

void XmsgImTcpH2N::heartbeat(ullong now)
{
	shared_ptr<XscTcpServer> tcpSever = Xsc::getXscTcpServer();
	auto cfg = static_pointer_cast<XscTcpCfg>(tcpSever->cfg);
	if (now < this->lastHbTs + cfg->heartbeat * DateMisc::sec)
		return;
	this->lastHbTs = Xsc::clock;
	static uchar ping[] { XSC_TAG_TRANSM_PING };
	XscTcpChannel::send(ping, 1);
}

bool XmsgImTcpH2N::regMsg(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign)
{
	return this->msgMgr->reg(begin, end, uni, cb, auth, foreign);
}

XmsgImTcpH2N::~XmsgImTcpH2N()
{

}

