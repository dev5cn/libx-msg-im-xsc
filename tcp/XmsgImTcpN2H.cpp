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

#include "XmsgImTcpN2H.h"
#include "XmsgImTcpLog.h"
#include "../XmsgImN2HMsgMgr.h"
#include "../XmsgImTransPassive.h"

XmsgImTcpN2H::XmsgImTcpN2H(XscTcpWorker* wk, int mtu, int cfd, const string& peer) :
		XmsgImTcpChannel(ActorType::ACTOR_N2H, wk, mtu, cfd, peer)
{
	this->est = true;
}

void XmsgImTcpN2H::evnDisc()
{
	shared_ptr<XscUsr> usr = this->usr.lock();
	if (usr != nullptr)
		usr->evnDisc();
	LOG_DEBUG("have a x-msg-im tcp n2h channel lost: %s", this->toString().c_str())
}

void XmsgImTcpN2H::dida(ullong now)
{
	if (!this->est)
		return;
	this->checkTransInit(now);
}

string XmsgImTcpN2H::toString()
{
	shared_ptr<XscUsr> usr = this->usr.lock();
	string str;
	SPRINTF_STRING(&str, "cfd: %d, peer: %s, usr: %s", this->cfd, this->peer.c_str(), usr == nullptr ? "null" : usr->uid.c_str())
	return str;
}

XmsgImTcpN2H::~XmsgImTcpN2H()
{

}

