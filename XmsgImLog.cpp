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

#include "XmsgImLog.h"
#include "http/XmsgImHttpLog.h"
#include "rudp/XmsgImRudpLog.h"
#include "tcp/XmsgImTcpLog.h"
#include "udp/XmsgImUdpLog.h"
#include "ws/XmsgImWebSocketLog.h"

XmsgImLog::XmsgImLog()
{

}

shared_ptr<XmsgImLog> XmsgImLog::getLog(XscChannel* channel)
{
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_TCP)
		return static_pointer_cast<XmsgImLog>(static_pointer_cast<XmsgImTcpLog>(static_pointer_cast<XscTcpLog>(channel->worker->server->log)));
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_HTTP)
		return static_pointer_cast<XmsgImLog>(static_pointer_cast<XmsgImHttpLog>(static_pointer_cast<XscHttpLog>(channel->worker->server->log)));
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_WEBSOCKET)
		return static_pointer_cast<XmsgImLog>(static_pointer_cast<XmsgImWebSocketLog>(static_pointer_cast<XscWebSocketLog>(channel->worker->server->log)));
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_UDP)
		return static_pointer_cast<XmsgImLog>(static_pointer_cast<XmsgImUdpLog>(static_pointer_cast<XscUdpLog>(channel->worker->server->log)));
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_RUDP)
		return static_pointer_cast<XmsgImLog>(static_pointer_cast<XmsgImRudpLog>(static_pointer_cast<XmsgImRudpLog>(channel->worker->server->log)));
	LOG_FAULT("it`s a bug, unexpected xsc protocol type, channel: %02X", channel->proType)
	return nullptr;
}

XmsgImLog::~XmsgImLog()
{

}

