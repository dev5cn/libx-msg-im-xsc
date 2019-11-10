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

#ifndef XMSGIMMSGSTUB_H_
#define XMSGIMMSGSTUB_H_

#include <libmisc.h>
#include <google/protobuf/message.h>
using namespace google::protobuf;

typedef enum ForeignAccessPermission
{
	FOREIGN_FORBIDDEN = 0x00, 
	FOREIGN_ALLOW = 0x01, 
	FOREIGN_ALLOW_EXISTED = 0x02 
} ForeignAccessPermission;

class XmsgImMsgStub
{
public:
	bool auth; 
	ForeignAccessPermission foreign; 
	void* cb; 
	string msg; 
	const Descriptor* begin; 
	const Descriptor* end; 
	const Descriptor* unidirection; 
public:
	XmsgImMsgStub(const Descriptor* begin, const Descriptor* end, const Descriptor* unidirection, void* cb, bool auth, ForeignAccessPermission foreign = FOREIGN_FORBIDDEN);
	shared_ptr<Message> newBegin(uchar* dat, int len); 
	shared_ptr<Message> newUnidirection(uchar* dat, int len); 
	string toString();
	virtual ~XmsgImMsgStub();
public:
	static Message* newPbMsg(const Descriptor* desc); 
	static Message* newPbMsg(const Descriptor* desc, uchar* dat, uint len); 
	static Message* newPbMsg(const string& name, uchar* dat, uint len); 
};

#endif 
