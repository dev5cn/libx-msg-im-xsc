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

#ifndef XMSGIMLOG_H_
#define XMSGIMLOG_H_

#include "XmsgImTrans.h"

class XmsgImLog
{
public:
	static shared_ptr<XmsgImLog> getLog(XscChannel* channel);
public:
	virtual void transInitStart(SptrXiti trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
	virtual void transInitFinished(SptrXiti trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
	virtual void transPassStart(SptrXitp trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
	virtual void transPassFinished(SptrXitp trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
	virtual void transInitUni(SptrXitui trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
	virtual void transPassUni(SptrXitup trans, shared_ptr<XscProtoPdu> pdu ) = 0; 
public:
	XmsgImLog();
	virtual ~XmsgImLog();
};

#endif 
