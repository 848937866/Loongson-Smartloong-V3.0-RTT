/****************************2012-2013, NJUT, Edu.****************************** 
FileName: clienttest.c 
Author:  孙冬梅       Version :  1.0        Date: 2014.07.30
Description:    使用最小限度API实现的一个简单的HTTP/1.0服务器  

Version:         1.0 
History:         
      <author>  <time>   <version >   <desc> 
      Sundm    14/07/30     1.0     文件创建   
********************************************************************************/ 
#include <lwip/api.h>
#include <finsh.h> 

/*  实际的web页面数据。大部分的编译器会将这些数据放在ROM里 */
ALIGN(4)
const static char indexdata[] = "<html> \
	<head><title>A test page</title></head> \
	<body> \
	<h1>This is a small test page. </h1> \
        <h2>     Made by sundm75.  </h2>  \
        <h3>   Loongson SmartLoongV3.0 </h3> \
        <h3>                 2018.06.11 </h3> \
	</body> \
	</html>";
ALIGN(4)
const static char http_html_hdr[] = "Content-type: text/html\r\n\r\n";

/*  处理进入的连接 */
static void process_connection(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *rq;
	rt_uint16_t len;
        rt_err_t net_rev_result;
        
	/*  从这个连接读取数据到inbuf，我们假定在这个netbuf中包含完整的请求 */
    net_rev_result = netconn_recv(conn, &inbuf);
	 
	if(net_rev_result == ERR_OK){
    /*  获取指向netbuf中第一个数据片断的指针，在这个数据片段里我们希望包含这个请求 */
	netbuf_data(inbuf, (void**)&rq, &len);
	
	rt_kprintf("net_rev_data : %s   \n", (char*)rq);
          /*  检查这个请求是不是HTTP "GET /\r\n"  */
          if( rq[0] == 'G' &&
                  rq[1] == 'E' &&
                  rq[2] == 'T' &&
                  rq[3] == ' ')
          {
				 /*  发送头部数据 */
                  netconn_write(conn, http_html_hdr, sizeof(http_html_hdr), NETCONN_COPY);
                  /*  发送实际的web页面 */
                  netconn_write(conn, indexdata, sizeof(indexdata), NETCONN_COPY);
          }
        }
        /*  关闭连接 */
        netconn_close(conn);
	netbuf_delete(inbuf);
}

/* 线程入口 */
static void lw_thread(void* paramter)
{
	struct netconn *conn, *newconn;
        rt_err_t net_acp_result;

	/*  建立一个新的TCP连接句柄 */
	conn = netconn_new(NETCONN_TCP);

	/*  将连接绑定在任意的本地IP地址的80端口上 */
	netconn_bind(conn, NULL, 80);

	/*  连接进入监听状态 */
	netconn_listen(conn);
    rt_kprintf("TCP/IP listening ......\n");
	/*  循环处理 */
	while(1)
	{
		/*  接受新的连接请求 */
		net_acp_result = netconn_accept(conn,&newconn);//线程阻塞
		/*  处理进入的连接 */
		process_connection(newconn);
		/*  删除连接句柄 */
		netconn_delete(newconn);
        rt_kprintf("TCP/IP closed!\n");
	}
}

void test_websrv(void)
{
	rt_thread_t tid;

	tid = rt_thread_create("websrv", lw_thread, RT_NULL,
		4096, 25, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);
}

 #include  <finsh.h> 
FINSH_FUNCTION_EXPORT(test_websrv, startup a simple web server e.g.test_websrv()); 
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(test_websrv, startup a simple web server test);