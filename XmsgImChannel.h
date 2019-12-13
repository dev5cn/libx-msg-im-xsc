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

#ifndef XMSGIMCHANNEL_H_
#define XMSGIMCHANNEL_H_

#include "XmsgImMsgMgr.h"
#include "XmsgImTransInitiative.h"
#include "XmsgImTransPassive.h"
#include "XmsgImTransUnidirectionInit.h"
#include "XmsgImTransUnidirectionPass.h"

class XmsgImChannel
{
public:
	unordered_map<uint , SptrXiti> transInit; 
	ullong lastCheckTransTs; 
	uint tidSeq; 
	XscWorker* __worker__; 
	ActorType at; 
	XscProtocolType pro; 
public:
	void begin(shared_ptr<Message> req, function<void(SptrXiti trans)> cb, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void begin(const string& msg, const string& dat, function<void(SptrXiti trans)> cb, SptrOob oob = nullptr, bool raw = false, SptrXit upstreamTrans = nullptr); 
	void unidirection(shared_ptr<Message> uni, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void unidirection(const string& msg, const string& dat, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void forwardBegin(const string& sne, const string& dne, shared_ptr<Message> req, function<void(SptrXiti trans)> cb, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void forwardBegin2ne(const string& sne, const string& dne, shared_ptr<Message> req, function<void(SptrXiti trans)> cb, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void forwardBegin(const string& sne, const string& dne, const string& msg, const string& dat, function<void(SptrXiti trans)> cb, SptrOob oob = nullptr, bool raw = false, SptrXit upstreamTrans = nullptr); 
	void forwardUnidirection(const string& sne, const string& dne, shared_ptr<Message> uni, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
	void forwardUnidirection2ne(const string& sne, const string& dne, shared_ptr<Message> uni, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
public:
	void sendEnd(SptrXitp trans); 
	void sendEnd(uint dtid, ushort ret, const string& desc, shared_ptr<Message> endMsg, shared_ptr<XscProtoPdu> upstreamPdu , function<void(shared_ptr<XscProtoPdu> pdu)> cb); 
	uint genTid(); 
	XmsgImChannel(ActorType type, XscProtocolType proType, XscWorker* wk);
	virtual ~XmsgImChannel();
private:
	void sendBegin(SptrXiti trans); 
	void futureBegin(SptrXiti trans, SptrXit upstreamTrans); 
	void futureUnidirection(shared_ptr<XmsgImTransUnidirectionInit> trans, SptrXit upstreamTrans); 
	void sendUnidirection(shared_ptr<XmsgImTransUnidirectionInit> trans); 
private:
	void sendPdu(shared_ptr<XscProtoPdu> pdu); 
	bool isEnableTracingOnOutStack(); 
public:
	bool evnMsg(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XmsgImMsgMgr> msgMgr); 
	bool evnBegin(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel, shared_ptr<XmsgImMsgMgr> msgMgr); 
	bool evnEnd(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	bool evnContinue(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	bool evnDialog(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	bool evnCancel(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	bool evnAbort(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	bool evnUnidirection(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel, shared_ptr<XmsgImMsgMgr> msgMgr); 
	bool evnPartial(XscWorker* wk, shared_ptr<XscProtoPdu> pdu, shared_ptr<XscChannel> channel); 
	void checkTransInit(ullong now); 
	void cleanTransInit(); 
public:
	static shared_ptr<XmsgImChannel> cast(shared_ptr<XscChannel> channel); 
	shared_ptr<XmsgImChannel> cast(); 
	shared_ptr<XscChannel> toXscChannel(); 
	int n2hTransTimeout();
};

#endif 
