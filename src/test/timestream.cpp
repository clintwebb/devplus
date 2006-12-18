#include <stdio.h>
#include <string.h>

#include <DpServerInterface.h>
#include <DpMain.h>

#define _PORT	8888


class Client : public DpThreadObject
{
	public:
		Client() {
			Lock();
			_pSocket = NULL;
			_nCount = 0;
			Unlock();
		}
		
		
		~Client() {
		
			WaitForThread();
			
			Lock();
			if (_pSocket != NULL) {
				delete _pSocket;
				_pSocket = NULL;
			}
			Unlock();
			printf("Client connection deleted.\n");
		}
		
		void Accept(SOCKET nSocket) {
			ASSERT(nSocket > 0);
			
			Lock();
			ASSERT(_pSocket == NULL);
			_pSocket = new DpSocket;
			ASSERT(_pSocket != NULL);
			_pSocket->Accept(nSocket);
			Unlock();
			
			SetCycleTime(1000);
			Start();
		}
		
		void OnThreadRun(void)
		{
			char buffer[128];
			int res;
			
			Lock();
			_nCount ++;
			sprintf(buffer, "%06d\n", _nCount);
			ASSERT(_pSocket != NULL);
			res = _pSocket->Send(buffer, strlen(buffer));
			if (res <= 0) {
				printf("RES: %d\n", res);
				delete _pSocket; _pSocket = NULL;
				Stop();
			}
			
			Unlock();
		}
		
	protected:
		
		
	private:
		DpSocket *_pSocket;
		int _nCount;
};

class Server : public DpServerInterface 
{
	public:
		Server() 
		{	
			printf("Server()\n");
		}
		
		virtual ~Server() 
		{
			printf("~Server()\n");
		}
		
		
	protected:
		
		virtual void OnAccept(SOCKET nSocket)
		{
			Client *pClient;
			
			ASSERT(nSocket > 0);
			
			// create a handler object, pass this socket to it.
			printf("New connection received\n");
			pClient = new Client;
			ASSERT(pClient != NULL);
			pClient->Accept(nSocket);
			AddObject(pClient);
		}
		
	private:
};



class theApp : public DpMain
{
	private:
		Server *_pServer;
	
	
	public:
		theApp()
		{
			printf("theApp();\n");
			_pServer = NULL;
		}

		virtual ~theApp()
		{
			if (_pServer != NULL) {
				delete _pServer;
				_pServer = NULL;
			}
			printf("theApp();\n");
		}

	protected:
		virtual void OnStartup(void)
		{
			ASSERT(_pServer == NULL);
			printf("On Startup();\n");
			
			printf(".. Creating server object.\n");
			_pServer = new Server;
			if (_pServer->Listen(_PORT) == false) {
				printf("Unable to listen on socket %d.\n", _PORT);
				Shutdown();
			}
			else {
				printf("Listening on socket %d.\n", _PORT);
			}
		}

		
		virtual void OnShutdown(void)
		{
			if (_pServer != NULL) {
				delete _pServer;
				_pServer = NULL;
			}
			
			printf("OnShutdown();\n");
		}
		
		virtual void OnCtrlBreak(void)
		{
			printf("OnCtrlBreak();\n");
		}
				

} myApp;



