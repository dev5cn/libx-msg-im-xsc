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

#ifndef XMSGIMTRANSPASSIVE_H_
#define XMSGIMTRANSPASSIVE_H_

#include "XmsgImTrans.h"
#include "XmsgImChannel.h"

class XmsgImTransPassive: public XmsgImTrans
{
public:
	bool lazyClose; 
public:
	void end(shared_ptr<Message> end); 
	void end(ushort ret, const string& desc, shared_ptr<Message> end); 
	void end(ushort ret); 
	void endDesc(ushort ret, const char* fmt, ...); 
	void endRaw(ushort ret, const string& desc, shared_ptr<x_msg_im_trans_raw_msg> rawMsg); 
	void success(); 
	void failure(); 
	void endAndLazyClose(ushort ret, const char* fmt, ...); 
	void endAndLazyClose(shared_ptr<Message> end); 
public:
	string toString();
	XmsgImTransPassive(shared_ptr<XscChannel> channel, shared_ptr<XscProtoPdu> pdu );
	virtual ~XmsgImTransPassive();
private:
	void end(); 
	void __end__(SptrXitp trans); 
};

#endif 
