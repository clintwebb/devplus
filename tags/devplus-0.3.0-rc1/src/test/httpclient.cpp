#include <stdio.h>
#include <string.h>

#include <DevPlus.h>

#define _PORT	8888

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
			DpHttpClient http;
			int code;
			DpDataQueue *pq;
			
			printf("On Startup();\n");
			
			http.SetURL("http://cjdj.org/index.html");
			http.AddParam("first", "parawinkle fred");
			http.AddParam("last", "jones_jones");
			code = http.Get();
			printf("Return code: %d\n", code);
			pq = http.GetBody();
			
			
			
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



