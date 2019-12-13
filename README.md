# libx-msg-im-xsc

![img](http://www.dev5.cn/x_msg_im/start/xsc/img/libx-msg-im-xsc-arch.svg)

* 这是一个基于actor模型的单进程多线程并发通信服务器框架. 它的目标是为上层应用提供一个高性能, 可测量, 并行无锁, 网络透明, 全异步的开发环境.

* 在`X-MSG-IM`系统中, 它为所有核心网元提供网络事务控制, 应用层消息处理, 透明的分布式信令跟踪(调用链)能力. 

* 它建立在[libxsc-cpp](https://github.com/dev5cn/libxsc-cpp)和[libxsc-proto-cpp](https://github.com/dev5cn/libxsc-proto-cpp)之上. 

* 既然是到了应用层, 提供的api自然也比较友好. 因此, 你可以很快速地在这些api上构建起一个同时支持多种传输层协议(`tcp`, `http`, `websocket`, `udp`, `rudp`)的并发服务器. 

* 看下面的例子.


##### 服务器

```cpp
#include "XmsgTcpLog.h"
#include "XmsgHttpLog.h"
#include "XmsgWebSocketLog.h"
#include "net-x-msg-im-auth.pb.h"

#define X_MSG_N2H_PRPC_BEFOR_AUTH(__MSGMGR__, __BEGIN__, __END__, __CB__)					(__MSGMGR__->reg(__BEGIN__::descriptor(), __END__::descriptor(), NULL, (void*)(__CB__), false));

static void x_msg_im_auth_simple(shared_ptr<XscChannel> channel, SptrXitp trans, shared_ptr<XmsgImAuthSimpleReq> req);

int main(int argc, char **argv)
{
	Log::setRecord();
	Xsc::init(); /* libxsc-cpp初始化. */
	//
	shared_ptr<XscTcpServer> tcpServer(new XscTcpServer("tcp-server", shared_ptr<XmsgTcpLog>(new XmsgTcpLog())));
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = "0.0.0.0:1224";
	if (!tcpServer->startup(tcpCfg) || !tcpServer->publish()) /* tcp服务器启动. */
		return EXIT_FAILURE;
	//
	shared_ptr<XscHttpServer> httpServer(new XscHttpServer("http-server", shared_ptr<XmsgHttpLog>(new XmsgHttpLog())));
	shared_ptr<XscHttpCfg> httpCfg(new XscHttpCfg());
	httpCfg->addr = "0.0.0.0:1225";
	if (!httpServer->startup(httpCfg) || !httpServer->publish()) /* http服务器启动. */
		return EXIT_FAILURE;
	//
	shared_ptr<XscWebSocketServer> webSocketServer(new XscWebSocketServer("web-socket-server", shared_ptr<XmsgWebSocketLog>(new XmsgWebSocketLog())));
	shared_ptr<XscWebSocketCfg> webSocketCfg(new XscWebSocketCfg());
	webSocketCfg->addr = "0.0.0.0:1226";
	if (!webSocketServer->startup(webSocketCfg) || !webSocketServer->publish()) /* websocket服务器启动. */
		return EXIT_FAILURE;
	//
	shared_ptr<XmsgImN2HMsgMgr> msgMgrTcp(new XmsgImN2HMsgMgr(tcpServer)); /* 服务器上的消息管理器. */
	shared_ptr<XmsgImN2HMsgMgr> msgMgrHttp(new XmsgImN2HMsgMgr(httpServer));
	shared_ptr<XmsgImN2HMsgMgr> msgMgrWebSocket(new XmsgImN2HMsgMgr(webSocketServer));
	//
	X_MSG_N2H_PRPC_BEFOR_AUTH(msgMgrTcp, XmsgImAuthSimpleReq, XmsgImAuthSimpleRsp, x_msg_im_auth_simple /* 消息注册. */)
	X_MSG_N2H_PRPC_BEFOR_AUTH(msgMgrHttp, XmsgImAuthSimpleReq, XmsgImAuthSimpleRsp, x_msg_im_auth_simple)
	X_MSG_N2H_PRPC_BEFOR_AUTH(msgMgrWebSocket, XmsgImAuthSimpleReq, XmsgImAuthSimpleRsp, x_msg_im_auth_simple)
	//
	Misc::hold();
	return EXIT_FAILURE;
}

/* 在这里处理消息. */
void x_msg_im_auth_simple(shared_ptr<XscChannel> channel, SptrXitp trans, shared_ptr<XmsgImAuthSimpleReq> req)
{
	/**
	 *
	 * channel即网络信道, 这里是客户端连接.
	 *
	 * trans即network transaction, 一切消息都以事务开始, 以事务结束.
	 *
	 */
	thread t([trans, req]() /* 事务总是在channel归属的线程上开始, 却可以在任意线程上结束. */
	{
		shared_ptr<XmsgImAuthSimpleRsp> rsp(new XmsgImAuthSimpleRsp());
		rsp->set_token("token");
		trans->end(rsp); /* 结束事务. */
	});
	t.detach();
}
```

##### 客户端-tcp

```java
public static void main(String[] args) throws Exception
{
    XmsgImAuthSimpleReq.Builder req = XmsgImAuthSimpleReq.newBuilder();
    req.setUsr("usr");
    //
    XscProtoPdu pdu = new XscProtoPdu(); /* 基于xsc协议的pdu构造. */
    pdu.transm.indicator = 0x00;
    pdu.transm.trans = new XscProtoTransaction();
    pdu.transm.trans.trans = XscProto.XSC_TAG_TRANS_BEGIN;
    pdu.transm.trans.stid = 0x00112233;
    pdu.transm.trans.msg = XmsgImAuthSimpleReq.getDescriptor().getName();
    pdu.transm.trans.dat = req.build().toByteArray();
    //
    Socket sock = new Socket("127.0.0.1", 1224);
    sock.getOutputStream().write(pdu.bytes());
    byte by[] = new byte[0x200];
    int len = sock.getInputStream().read(by); /* 这里很不严谨, 仅用于演示. */
    pdu = XscProtoPdu.decode(by, 0, len);
    Log.info("rsp: %s", Misc.pb2str(XmsgImAuthSimpleRsp.parseFrom(pdu.transm.trans.dat)));
}
```

##### 客户端-http

```java
public static void main(String[] args) throws Exception
{
    XmsgImAuthSimpleReq.Builder req = XmsgImAuthSimpleReq.newBuilder();
    req.setUsr("usr");
    //
    HttpClient client = HttpClient.newBuilder().build();
    HttpRequest request = HttpRequest.newBuilder()//
            .uri(URI.create("http://127.0.0.1:1225/"))//
            .header("x-msg-name", XmsgImAuthSimpleReq.getDescriptor().getName())//
            .header("x-msg-dat", Crypto.base64enc(req.build().toByteArray()))//
            .build();
    HttpResponse<byte[]> rsp = client.send(request, HttpResponse.BodyHandlers.ofByteArray());
    Log.info("rsp: %s", Misc.pb2str(XmsgImAuthSimpleRsp.parseFrom(XmsgImHttpRsp.parseFrom(rsp.body()).getDat())));
}
```

##### 客户端-websocket

```java
public static void main(String[] args)
{
    Log.setRecord();
    var httpClient = HttpClient.newHttpClient();
    var wsCompletableFuture = httpClient.newWebSocketBuilder().buildAsync(URI.create("ws://127.0.0.1:1226"), new Listener()
    {
        public void onOpen(WebSocket ws)
        {
            XmsgImAuthSimpleReq.Builder req = XmsgImAuthSimpleReq.newBuilder();
            req.setUsr("usr");
            //
            XscProtoPdu pdu = new XscProtoPdu(); /* 基于xsc协议的pdu构造. */
            pdu.transm.indicator = 0x00;
            pdu.transm.trans = new XscProtoTransaction();
            pdu.transm.trans.trans = XscProto.XSC_TAG_TRANS_BEGIN;
            pdu.transm.trans.stid = 0x00112233;
            pdu.transm.trans.msg = XmsgImAuthSimpleReq.getDescriptor().getName();
            pdu.transm.trans.dat = req.build().toByteArray();
            //
            ws.sendBinary(ByteBuffer.wrap(pdu.bytes() /* 消息出栈. */), true);
            ws.request(1);
        }

        public CompletionStage<?> onBinary(WebSocket ws, ByteBuffer dat, boolean last)
        {
            byte by[] = new byte[dat.limit()];
            dat.get(by, 0, by.length);
            try
            {
                XscProtoPdu pdu = XscProtoPdu.decode(by, 0, by.length); /* 解析收到的响应字节流. */
                Log.info("rsp: %s", Misc.pb2str(XmsgImAuthSimpleRsp.parseFrom(pdu.transm.trans.dat)));
            } catch (Exception e)
            {
                Log.error(Log.trace(e));
            }
            ws.request(1);
            return null;
        }

        public CompletionStage<?> onClose(WebSocket webSocket, int statusCode, String reason)
        {
            Log.debug("web-socket channel closed");
            return null;
        }

        public void onError(WebSocket ws, Throwable error)
        {
            Log.debug("web-socket channel error occured: %s", Log.trace(error));
            ws.request(1);
        }
    });
    wsCompletableFuture.join();
    Misc.hold();
}
```

* 完整的例子在这里:

    * [x-msg-im-xsc-examples-cpp](https://github.com/dev5cn/x-msg-im-xsc-examples-cpp)
    * [x-msg-im-xsc-examples-java](https://github.com/dev5cn/x-msg-im-xsc-examples-java)