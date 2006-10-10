#include <stdio.h>

#include <DevPlus.h>

class theApp : public DpMain
{
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
			DpSqlite3 *pDB;
			DpSqlite3Result *pResult;
			
			printf("OnStartup();\n");
			
			printf(".. Opening Database.\n");
			pDB = new DpSqlite3;
			if (pDB->Open("test.db") == false) {
				printf("Unable to open database.\n");
			}
			else {
				printf("Database opened.\n");
				
				// First we insert some data into the table.
// 				pDB->ExecuteNR("INSERT INTO animals (Name, Species) VALUES ('frankie', 'parrot')");
				
				pResult = pDB->Execute("SELECT * FROM animals");
				while(pResult->NextRow()) {
					printf("%d, %s, %s\n", pResult->GetInt("AnimalID"), pResult->GetStr("Name"), pResult->GetStr("Species"));
				}
				delete pResult;
			}
			
			delete pDB;
			
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



