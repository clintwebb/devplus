#include <stdio.h>

#include <DpMain.h>

class theApp : public DpMain
{
	public:
		theApp()
		{
		}

		virtual ~theApp()
		{
		}

	protected:
		virtual void OnStartup(void)
		{
			printf("OnStartup();\n");
			
			
			Shutdown();
		}

		
		virtual void OnShutdown(void)
		{
		}
		
		virtual void OnCtrlBreak(void)
		{
		}
				

} myApp;



