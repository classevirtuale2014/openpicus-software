#include "taskFlyport.h"
#include "grovelib.h"
#include "rht03.h"

	//Parameters configuration
	
	TCP_SOCKET sockHttp;
    BOOL inExecution = TRUE;
    int requestType = 0; // -1=Idle, 0=GET method, 1=HTTP POST without additional params, 2=HTTP POST with additional params
    int currentAction = 0;
    BYTE regStat = NO_REG;
 
    // Setup here your params!
    char httpServName[] = "";  // inserire nome sito
    char httpServPort[] = "80";
	char requestGetURL[] = "recive.php";
    char requestURL[] = "recive";
	
	
void FlyportTask()
{
	// Flyport connects to default network
	WFConnect(WF_DEFAULT);
	while(WFGetStat() != CONNECTED);
	vTaskDelay(25);
	UARTWrite(1,"Flyport Wi-fi G connected...hello world!\r\n");
    

	
	
	
	
	
	
	void *board = new(GroveNest);
	void *rht03 = new(RHT03);
	void *light_sensor = new (An_i);
 
    configure(rht03, 3);
 
	void *moisture_sensor = new (An_i);
	attachToBoard(board,rht03,DIG1); // Attach Temperature-Humidity, on digital port1 (DIG1)
	attachToBoard(board, light_sensor, AN3); // Light attached to board, on analog port 3 (AN3)
	attachToBoard(board, moisture_sensor, AN2); // Attach Moisture, on analog port 2 (AN2)
 
	float Humd = 0;
	float Tempt = 0;
	float Terr = 0.0;
	float Lum = 0.0;
	char array[400];
	char arrayhum[100];
	char arraytemp[100];
	char arrayterr[100];
	char arrayli[100];
		
	
 
    while(1)
	{
		// Reading external humidity 
		Humd = get(rht03, HUMD);
		if(!readError())
		{
			sprintf(arrayhum,"humidity %3.2f;", (double) Humd);
		}
               
		// Reading external temperature 
		Tempt = get(rht03, TEMP);
		if(!readError())
		{
			sprintf(arraytemp,"temperature %3.2f;", (double) Tempt);
		}
        // Reading terren humidity
        Terr = get(moisture_sensor);
		if(!readError())
		{
		sprintf(arrayterr, "moisture value: %3.2f;", (double) Terr);
		}
		// Reading light
        Lum = get(light_sensor);
		if(!readError())
		{
		sprintf(arrayli, "light value: %3.2f;", (double) Lum);
		}
		strcat (array,arrayhum);
		strcat (array,arraytemp);
		strcat (array,arrayterr);
        strcat (array,arrayli);
		
}
}

