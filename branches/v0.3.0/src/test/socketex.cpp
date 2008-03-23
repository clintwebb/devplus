#include <stdio.h>
#include <string.h>

#include <DpSocketEx.h>
#include <DpMain.h>

#define _PORT	8888


class Client : public DpSocketEx
{
	private:
	protected:
	public:
	
	
	private:
	protected:
	
		virtual int OnReceive(char *pData, int nLength)
		{
			int nDone = 0;
			
			ASSERT(pData != NULL && nLength > 0);
			fwrite(pData, 1, nLength, stdout);
			nDone = nLength;
			
			return(nDone);
		}
		
		virtual void OnClosed(void)
		{
			printf("** Closed\n");
		}
		
		virtual void OnStalled(char *pData, int nLength)
		{
			printf("** Stalled\n");
		}


	
	public:
		Client()
		{
		}
		
		virtual ~Client()
		{
		}
};



class theApp : public DpMain
{
	private:
	
	
	public:
		theApp()
		{
			printf("theApp();\n");
		}

		virtual ~theApp()
		{
			printf("theApp();\n");
		}

	protected:
		virtual void OnStartup(void)
		{
			int code;
			Client client;
			
			printf("On Startup();\n");
			if (client.Connect("localhost", 8888) == true) {
				while(client.IsClosed() == false) {
					Sleep(1000);
				}
			}
			
			Shutdown();
		}

		
		virtual void OnShutdown(void)
		{
			printf("OnShutdown();\n");
		}
		
		virtual void OnCtrlBreak(void)
		{
			printf("OnCtrlBreak();\n");
		}
				

} myApp;



