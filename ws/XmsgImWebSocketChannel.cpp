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

#include "XmsgImWebSocketChannel.h"

XmsgImWebSocketChannel::XmsgImWebSocketChannel(shared_ptr<XscWebSocketWorker> wk, int mtu, int cfd, const string &peer) :
		XmsgImChannel(ActorType::ACTOR_N2H, XscProtocolType::XSC_PROTOCOL_WEBSOCKET, wk), XscWebSocketChannel(wk, mtu, cfd, peer)
{
	this->est = true;
	this->hs = false;
}

int XmsgImWebSocketChannel::evnRecv(XscWorker* wk, uchar* dat, int len)
{
	if (!this->hs)
	{
		LOG_RECORD("\n  <-- PEER: %s CFD: %d, NE: null\n%s", this->peer.c_str(), this->cfd, Net::printHex2str(dat, len).c_str())
		return this->checkHandShake(wk, dat, len);
	}
	return this->decode(wk, dat, len);
}

void XmsgImWebSocketChannel::dida(ullong now)
{

}

void XmsgImWebSocketChannel::evnDisc()
{
	shared_ptr<XscUsr> usr = this->usr.lock();
	if (usr != nullptr)
		usr->evnDisc();
	LOG_DEBUG("have a x-msg-im web-socket channel lost: %s", this->toString().c_str())
}

void XmsgImWebSocketChannel::clean()
{

}

int XmsgImWebSocketChannel::checkHandShake(XscWorker* wk, uchar* dat, int len)
{
	bool r = false;
	int ofst = 0;
	for (int i = len - 1; i >= 4; i -= 4)
	{
		if (dat[i] == 0x0A && dat[i - 1] == 0x0D && dat[i - 2] == 0x0A && dat[i - 3] == 0x0D)
		{
			dat[i - 1] = 0; 
			ofst = i;
			r = true;
			break;
		}
	}
	if (!r) 
		return 0;
	string swk;
	if (!this->parseSwk((char*) dat, swk))
		return -1;
	this->hs = true;
	this->sendHandShake(swk);
	return ofst + 1;
}

bool XmsgImWebSocketChannel::parseSwk(const char* dat, string& swk)
{
	static const char* key = "Sec-WebSocket-Key:";
	static const int keylen = ::strlen(key);
	vector<string> arr;
	Misc::split(dat, "\r\n", arr);
	for (auto& it : arr)
	{
		const char* p = ::strstr(it.c_str(), key);
		if (p == NULL)
			continue;
		int plen = (int) ::strlen(p);
		if (keylen >= plen) 
		{
			LOG_DEBUG("parse web-socket hand shake failed, channel: %s, dat: %s", this->toString().c_str(), dat)
			return false;
		}
		for (int i = 0; i < plen; ++i)
		{
			if (p[keylen + i] == ' ') 
				continue;
			swk.assign(p + keylen + i, plen - keylen - i);
			return true;
		}
	}
	LOG_DEBUG("parse web-socket hand shake failed, channel: %s, dat: %s", this->toString().c_str(), dat)
	return false;
}

void XmsgImWebSocketChannel::sendHandShake(const string& swk)
{
	static const string k = string("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	string str = swk + k;
	uchar sha[0x14];
	Crypto::sha1((uchar*) str.data(), str.length(), sha);
	string dat = Crypto::base64enc(sha, 0x14);
	str.clear();
	SPRINTF_STRING(&str, "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: WebSocket\r\nSec-WebSocket-Accept: %s\r\n\r\n", dat.c_str())
	LOG_RECORD("\n  --> PEER: %s, CFD: %d, NE: null\n%s", this->peer.c_str(), this->cfd, Net::printHex2str((uchar* ) str.data(), str.length()).c_str())
	XscTcpChannel::send((uchar*) str.data(), str.length()); 
}

int XmsgImWebSocketChannel::decode(XscWorker* wk, uchar* dat, int len)
{
	int used = 0;
	while (true)
	{
		if (len < 2) 
			return used;
		if (!((dat[0] & 0x80) ? true : false)) 
		{
			LOG_WARN("unsupported multi-frame on this channel: %s", this->toString().c_str())
			return -1;
		}
		char opcode = (dat[0] & 0x0F); 
		if (!((dat[0 + 1] & 0x80) ? true : false)) 
		{
			LOG_DEBUG("client must use mask code on this channel: %s", this->toString().c_str())
			return -1;
		}
		switch (opcode)
		{
		case WS_FRAME_CLOSE: 
			LOG_DEBUG("got a web socket close request, we will close this socket immediate: %s", this->toString().c_str())
			return -1;
		case WS_FRAME_CONTINUATION: 
		case WS_FRAME_PING:
		case WS_FRAME_PONG:
		{
			LOG_DEBUG("unsupported operation code: %02X, channel: %s", opcode, this->toString().c_str())
			used += 2;
			dat += 2;
			len -= 2;
			continue;
		}
		case WS_FRAME_TEXT:
		case WS_FRAME_BINARY:
			break; 
		default:
			LOG_WARN("unexpected operation code: %02X, channel: %s", opcode, this->toString().c_str())
			return -1;
		}
		if (dat[1] == 0x80)
		{
			LOG_DEBUG("need 1 byte at least, channel: %s", this->toString().c_str())
			return -1;
		}
		if (dat[1] == 0xFF) 
		{
			LOG_DEBUG("over 64K pdu unsupported, channel: %s", this->toString().c_str())
			return -1;
		}
		int size;
		if (dat[1] < 0xFE)
			size = 1 + 1 + 4 + (dat[1] & 0x7F) ;
		else 
		{
			ushort s;
			::memcpy(&s, dat + 2, 2);
			s = ::ntohs(s); 
			size = 1 + 3 + 4 + s ;
		}
		if (size > wk->mtu)
		{
			LOG_DEBUG("packet format error, over the mtu, we will close this channel, size: %08X, channel: %s", size, this->toString().c_str())
			return -1;
		}
		if (len < size) 
			break;
		uchar mask[4];
		memcpy(&mask, (dat[1] < 0xFE) ? dat + 2 : dat + 4, 4);
		int ofst = (dat[1] < 0xFE) ? 6 : 8;
		for (int i = ofst; i < size; i++) 
			dat[i] = (dat[i] ^ mask[(i - ofst) % 4]);
		if (!this->decodeFrameBinary(wk, dat + ofst, size - ofst))
			return -1;
		used += size;
		dat += size;
		len -= size;
	}
	return used;
}

bool XmsgImWebSocketChannel::decodeFrameBinary(XscWorker* wk, uchar* __dat__, int __len__)
{
	uchar* dat;
	int len;
	if (this->cachedXscPdu.empty())
	{
		dat = __dat__;
		len = __len__;
	} else
	{
		for (int i = 0; i < __len__; ++i)
			this->cachedXscPdu.push_back(__dat__[i]);
		dat = this->cachedXscPdu.data();
		len = this->cachedXscPdu.size();
	}
	int used = this->decodeXscPdu(wk, dat, len);
	if (used < 0)
		return false;
	int remain = len - used;
	if (remain < 0)
	{
		LOG_FAULT("it`s a bug, used: %d, len: %d, channel: %s", used, len, this->toString().c_str())
		return false;
	}
	if (remain == 0) 
	{
		this->cachedXscPdu.clear();
		return true;
	}
	if (this->cachedXscPdu.empty()) 
	{
		for (int i = used; i < len; ++i)
			this->cachedXscPdu.push_back(dat[i]);
		return true;
	}
	this->cachedXscPdu.erase(std::begin(this->cachedXscPdu) + 0, std::end(this->cachedXscPdu) + used); 
	return true;
}

int XmsgImWebSocketChannel::decodeXscPdu(XscWorker* wk, uchar* dat, int len)
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
			if (!this->evnMsg(wk, pdu, static_pointer_cast<XmsgImMsgMgr>(this->worker->msgMgr))) 
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

string XmsgImWebSocketChannel::toString()
{
	shared_ptr<XscUsr> usr = this->usr.lock();
	string str;
	SPRINTF_STRING(&str, "cfd: %d, peer: %s, usr: %s", this->cfd, this->peer.c_str(), usr == nullptr ? "null" : usr->uid.c_str())
	return str;
}

XmsgImWebSocketChannel::~XmsgImWebSocketChannel()
{

}

