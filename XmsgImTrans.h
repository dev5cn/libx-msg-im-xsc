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

#ifndef XMSGIMTRANS_H_
#define XMSGIMTRANS_H_

#include <libxsc.h>
#include <libxsc-proto.h>
#include "XmsgImMsgStub.h"

enum XmsgImTransType
{
	TRANS_TYPE_INIT, 
	TRANS_TYPE_PASS, 
	TRANS_TYPE_INIT_UNI, 
	TRANS_TYPE_PASS_UNI 
};

class XmsgImTransInitiative;
class XmsgImTransPassive;
class XmsgImTransUnidirectionInit;
class XmsgImTransUnidirectionPass;

typedef shared_ptr<XmsgImTransInitiative> SptrXiti;
typedef shared_ptr<XmsgImTransPassive> SptrXitp;
typedef shared_ptr<XmsgImTransUnidirectionInit> SptrXitui;
typedef shared_ptr<XmsgImTransUnidirectionPass> SptrXitup;
typedef shared_ptr<list<pair<uchar, string>>> SptrOob;

typedef struct
{
	string msg; 
	string dat; 
} x_msg_im_trans_raw_msg; 

class XmsgImTrans: public enable_shared_from_this<XmsgImTrans>
{
public:
	XmsgImTransType xitt; 
	shared_ptr<XscChannel> channel; 
	shared_ptr<Message> beginMsg; 
	shared_ptr<Message> endMsg; 
	shared_ptr<Message> uniMsg; 
	shared_ptr<x_msg_im_trans_raw_msg> rawMsg; 
	uint stid; 
	uint dtid; 
	ullong gts; 
	ullong ets; 
	ushort ret; 
	string desc; 
public:
	xsc_transmission_header_oob* ooob; 
	xsc_transmission_header_oob* ioob; 
public:
	void addOob(uchar tag, const string& val); 
	bool getOob(uchar tag, string& val); 
	bool haveOob(uchar tag); 
	string toString();
	XmsgImTrans(shared_ptr<XscChannel> channel, XmsgImTransType xitt);
	virtual ~XmsgImTrans();
public:
	static SptrOob newOob(vector<pair<uchar, string>>& oob); 
	static SptrOob newOob(); 
};

typedef shared_ptr<XmsgImTrans> SptrXit;

#endif 
