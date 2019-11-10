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

#ifndef XMSGIMTCPH2N_H_
#define XMSGIMTCPH2N_H_

#include "XmsgImTcpChannel.h"
#include "../XmsgImH2NMsgMgr.h"

class XmsgImTcpH2N: public XmsgImTcpChannel
{
public:
	shared_ptr<XmsgImH2NMsgMgr> msgMgr; 
public:
	void connect(); 
	void dida(ullong now); 
	bool regMsg(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign = FOREIGN_FORBIDDEN); 
	XmsgImTcpH2N(shared_ptr<XscTcpServer> tcpServer, const string &peer);
	virtual ~XmsgImTcpH2N();
public:
	virtual void estab() = 0; 
	virtual XscMsgItcpRetType itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu) = 0; 
private:
	bool needWait; 
	atomic_bool trying; 
	uint h2nReConn; 
	ullong lastHbTs; 
private:
	void svc(shared_ptr<XmsgImTcpH2N> h2n); 
	void heartbeat(ullong now); 
};

#endif 
