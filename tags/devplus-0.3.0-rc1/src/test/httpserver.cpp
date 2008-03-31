#include <stdio.h>
#include <string.h>

#include <DpHttpServer.h>
#include <DpMain.h>

#define _PORT	8888


class WebServer : public DpHttpServer 
{
	public:
		WebServer() 
		{	
			printf("WebServer()\n");
		}
		
		virtual ~WebServer() 
		{
			printf("~WebServer()\n");
		}
		
		
	protected:
		
		// This function will be called when a connection is established and all of the header information has been retreived.
		virtual int OnPage(char *szUrl, char *szHeaders, char *szData, DpCgiFormData *pForm, DpDataQueue *pQueue)
		{
			char s[2048];
			char *ptr;
			
			ASSERT(szUrl != NULL);
			ASSERT(szHeaders != NULL); 
			ASSERT(szData != NULL);
			ASSERT(pForm != NULL);
			ASSERT(pQueue != NULL);
			
			printf("WebServer::OnPage()\n");
			printf(" -- URL: %s\n", szUrl);
			printf(" -- Data: %s\n", szData);
			
			ptr = pForm->Get("val");
			printf(" -- 'val': %s\n", ptr);
			
			sprintf(s, "Content-type: %s\n\n", "text/html");
			pQueue->Add(s, strlen(s));
			
			sprintf(s, "<html><body>Test page<br><br><form method=post><input type=hidden name=val value=123><input type=submit></form></body></html>\n\n");
			pQueue->Add(s, strlen(s));
			
			return(200);
		}
		
	private:
};



class theApp : public DpMain
{
	private:
		WebServer *_pServer;
	
	
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
			
			printf(".. Opening Database.\n");
			_pServer = new WebServer;
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



