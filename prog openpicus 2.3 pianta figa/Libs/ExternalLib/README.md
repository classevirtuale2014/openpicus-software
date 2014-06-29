lib_sd_card_fat_file_system
============================

Flyport library for FAT32 File System on SD-Card, released under GPL v.3.<br>
The library allows to manage files and directories on a FAT32 formatted SD-Card. <BR>
More info on wiki.openpicus.com.
<br>1) import files inside Flyport IDE using the external libs button.<br>
2) add following code example in FlyportTask.c:

<pre>
#include "taskFlyport.h"
#include "sd_manager.h"

void FlyportTask()
{
	BOOL repeatInit = TRUE;
	BOOL initRes = FALSE;
	BOOL resultSD = FALSE;
	int error = 0;
	
	vTaskDelay(100);
	UARTWrite(1,"Flyport SD-Card FAT32 library test!\r\n");
	
	while(1)
	{
		// Check for SD-Card detect signal
		if(IOGet(p5) == TRUE)
		{
			if(repeatInit == FALSE)
			{
				UARTWrite(1, "SDUnMount()...\n");
				resultSD = SDUnMount();
				repeatInit = TRUE;
			}	
		}
		else
		{
			if(repeatInit == TRUE)
			{
				UARTWrite(1, "SDInit()...");
				
				// Please, choose your pinout with 
				//	SDInit(...);
				// or use instead
				// 	SDOnNest(SD_GROVE, 15);
				// 	SDOnNest(SD_MUSIC, 15);
				// if you are using a Grove Nest or Music Nest Carrier Board
				initRes = SDInit(p8, p12, p10, p7, p5, 15);
				
				if (initRes)
				{
					repeatInit = FALSE;
					UARTWrite(1,"Initialized\n");
				}
				else
				{
					error = SDGetErr();
					if (error == SD_NOT_PRESENT)
					{
						UARTWrite(1, "SD Card Not Present\n");
					}	
					else if (error == SD_FS_NOT_INIT)
					{
						UARTWrite(1, "Generic error\n");
						while (1); // Lock here for generic error!
					}	
				}	
			}
		}
		
		// INSERT HERE YOUR CODE
	}
}
</pre>

This example initializes automatically the SD-Card when is inserted.<br>
Leave this code snippet inside the while(1) loop to let Flyport check if the SD-Card is present or not, and then add your code.