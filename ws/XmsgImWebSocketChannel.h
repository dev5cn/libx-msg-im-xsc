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

#ifndef WS_XMSGIMWEBSOCKETCHANNEL_H_
#define WS_XMSGIMWEBSOCKETCHANNEL_H_

#include "../XmsgImChannel.h"

class XmsgImWebSocketChannel: public XmsgImChannel, public XscWebSocketChannel
{
public:
	bool hs; 
	vector<uchar> cachedXscPdu; 
public:
	int evnRecv(XscWorker* wk, uchar* dat, int len); 
public:
	void dida(ullong now); 
	void evnDisc(); 
	void clean(); 
public:
	string toString();
	XmsgImWebSocketChannel(shared_ptr<XscWebSocketWorker> wk, int mtu, int cfd, const string &peer);
	virtual ~XmsgImWebSocketChannel();
private:
	int checkHandShake(XscWorker* wk, uchar* dat, int len); 
	bool parseSwk(const char* dat, string& swk); 
	void sendHandShake(const string& swk); 
	int decode(XscWorker* wk, uchar* dat, int len); 
	bool decodeFrameBinary(XscWorker* wk, uchar* dat, int len); 
	int decodeXscPdu(XscWorker* wk, uchar* dat, int len); 
};

#endif 
