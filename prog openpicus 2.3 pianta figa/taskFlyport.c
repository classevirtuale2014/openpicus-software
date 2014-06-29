#include "taskFlyport.h"
#include "grovelib.h"
#include "rht03.h"
#include "sd_manager.h"

    char sdfreespace[110];
    char sderror[110];
	
	unsigned char Humd ;
	unsigned char Tempt ;
	unsigned char Terr ; 
	unsigned char Lum ;		

    //comunicazione con server
	unsigned char arraysens[5];      				    //array dati sensori
	unsigned char arrayset[9];   						//array parametri arrivati dal server
	unsigned char arraymess[9];  						//array dei messaggi
	
    
	//server
	int ID = 20;
	
	//comandi
	
	int pompa = off;
	int RGB_R = off;
	int RGB_G = off;
	int RGB_B = off;
	int RGB_RB = off;    							    //purple led
	int RGB_RG = off;									//yellow led
	
	int counter_pompa=0;
	
    float battery_perc=0;
	
	
    BOOL repeatInit = TRUE;
    BOOL initRes = FALSE;
    BOOL resultSD = FALSE;
    int error = 0;
  
	BOOL IsError = FALSE;
	
	int DelayTime = 0;
	
	void SDCheck()
{
    // Check for SD-Card detect signal
    if(IOGet(p19) == TRUE)
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
            //  SDInit(...);
            // or use instead
              SDOnNest(SD_GROVE, 15);
            
            // if you are using a Grove Nest or Music Nest Carrier Board
            initRes = SDInit(p18, p7, p17, p9, p19, 15);
 
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
}
	
	
BOOL FirstConnect()
{		
	//Dichiaro il socket per la connessione
	TCP_SOCKET WorkSocket = INVALID_SOCKET;
	
	char OutBuffer[3];
	
	//Dato che è la prima connessione il messaggio deve essere lungo 2 char: primo lunghezza, secondo tipo di messaggio (0 - prima registrazione)
	
	_dbgwrite("\nTento la connessione iniziale...");
	
	//Apro la connessione al server
 	WorkSocket = TCPClientOpen("172.17.11.87", "10111");
	
	vTaskDelay(250);
	
	if(!TCPisConn(WorkSocket))
	{
		UARTWrite(1,"\nConnessione al server fallita\n Riprovo...\n");
		return FALSE;
	}

	UARTWrite(1,"\nConnessione al server riuscita");

	vTaskDelay(100);
	
	OutBuffer[0] = (char)3; //Lunghezza
	OutBuffer[1] = (char)0; //Tipo di messaggio (0 - registrazione)
	OutBuffer[2] = (char)ID; //ID della pianta (sarà autenticato nel server)
	
	_dbgwrite("\nInvio del messaggio al server");
	
	//Invio del messaggio (OutBuffer) sul socket WorkSocket di lunghezza OutBuffer[0] (2)
	int len = TCPWrite(WorkSocket, OutBuffer, OutBuffer[0]);
	
	UARTWrite(1,"\nPachetto inviato");
	
	//Buffer in ricezione
	char InBuffer[15];
	
	int RxLen = 0;
	
	BOOL Continue = FALSE;
	while(!Continue)
	{
		while ((RxLen = TCPRxLen(WorkSocket))>0)
		{
			TCPRead(WorkSocket, InBuffer, RxLen);
			Continue = TRUE;
		}
	}
	
	UARTWrite(1, "\nChiusura Socket");
	
	TCPClientClose(WorkSocket);

	UARTWrite(1, InBuffer);
	
	int n = 0;
	for(n = 2; n < InBuffer[0]; n++)
	{
		int Index2 = n - 2;
		arrayset[Index2] = InBuffer[n];
	}
	
	if((InBuffer[1] == (char)0)||(InBuffer[1] == (char)2))
	{
		UARTWrite(1,"\nErrore nella ricezione di Max e Min");
		return FALSE;
	} 

	return TRUE;
}
	
BOOL connessione_server(BOOL Connection)
{
	if(!Connection)
		return FALSE;
	int TimeError = 0;
	while(!FirstConnect())
	{
		vTaskDelay(3000);
		TimeError++;
		if(TimeError == 5)
		{
			_dbgwrite("\nTimeout error");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL SendUpdate()
{	
	UARTWrite(1, "Invio aggiornamento al server \n");
	TCP_SOCKET WorkSocket = INVALID_SOCKET;
	char OutBuffer[9];
	OutBuffer[0] = (char)9;
	OutBuffer[1] = (char)1;
	OutBuffer[2] = (char)ID;
	
	int i;
	for(i = 0; i < 4; i++)
	{
		int index = i + 3;
		OutBuffer[index] = arraysens[i];
	}
	
	OutBuffer[7] = arraymess[7]; 
	OutBuffer[8] = arraysens[4]; 
	
	WorkSocket = TCPClientOpen("172.17.11.87", "10111");
	vTaskDelay(250);
	if(!TCPisConn(WorkSocket))
	{
		return FALSE; 
	}
	char msg[9];
	
	sprintf(msg,"%d %d %d %d %d %d %d %d %d\r\n",OutBuffer[0],OutBuffer[1],OutBuffer[2],OutBuffer[3],OutBuffer[4],OutBuffer[5],OutBuffer[6], OutBuffer[7], OutBuffer[8]);
	
	int len = TCPWrite(WorkSocket, OutBuffer, OutBuffer[0]);
	
	char InBuffer[15];
	
	int RxLen = 0;
	
	BOOL Continue = FALSE;
	while(!Continue)
	{
		while ((RxLen = TCPRxLen(WorkSocket))>0)
		{
			TCPRead(WorkSocket, InBuffer, RxLen);
			Continue = TRUE;
		}
	}
		
	if(InBuffer[1] == (char)2)
	{
		if(FirstConnect())
		{
			if(!SendUpdate(TRUE))
			{
				return FALSE;
			}
		}
		else
		{
			//TODO: Accendi led di non connessione
			return FALSE;
		}
	}
	
	if(InBuffer[1] == 0)
	{
		DelayTime = InBuffer[2];
		//TODO: DelayDiProva
	}
	
	TCPClientClose(WorkSocket);
	
	UARTWrite(1,"ChiusoSocketUpdate\n");
	
	return TRUE;
}

void SendWarning()
{
	TCP_SOCKET WorkSocket = INVALID_SOCKET;
	char OutBuffer[11];
	
	//Dato che è la prima connessione il messaggio deve essere lungo 2 char: primo lunghezza, secondo tipo di messaggio (0 - prima registrazione)
	
	_dbgwrite("\nInvio errore\n");
	
	//Apro la connessione al server
 	WorkSocket = TCPClientOpen("172.17.11.87", "10111");
	vTaskDelay(250);

	if(!TCPisConn(WorkSocket))
	{
		return; 
	}
	
	OutBuffer[0] = (char)11;
	OutBuffer[1] = (char)2;
	OutBuffer[2] = (char)ID;
	
	int i;
	for(i = 3; i < 11; i++)
	{
		OutBuffer[i] = arraymess[i - 3];
	}
	
	int len = TCPWrite(WorkSocket, OutBuffer, OutBuffer[0]);
	
	char InBuffer[15];
	
	int RxLen = 0;
	
	BOOL Continue = FALSE;
	while(!Continue)
	{
		while ((RxLen = TCPRxLen(WorkSocket))>0)
		{
			TCPRead(WorkSocket, InBuffer, RxLen);
			Continue = TRUE;
		}
	}
	
	TCPClientClose(WorkSocket);
}

void SendHistory(char Lectures[], int lenght)
{
	TCP_SOCKET WorkSocket = INVALID_SOCKET;
	
	char OutBuffer[lenght + 3];
	OutBuffer[0] = (char)lenght + 3;
	OutBuffer[1] = (char)1;
	OutBuffer[2] = (char)ID;
	int i;
	for(i = 0; i < lenght; i++)
	{
		OutBuffer[i + 3] = Lectures[i];
	}
	
	WorkSocket = TCPClientOpen("172.17.80.197", "10111");
	vTaskDelay(250);
	if(!TCPisConn(WorkSocket))
	{
		return; 
	}

	int Len = TCPWrite(WorkSocket, OutBuffer, OutBuffer[0]);
	
	char InBuffer[15];
	
	int RxLen = 0;
	
	BOOL Continue = FALSE;
	while(!Continue)
	{
		while ((RxLen = TCPRxLen(WorkSocket))>0)
		{
			TCPRead(WorkSocket, InBuffer, RxLen);
			Continue = TRUE;
		}
	}
	TCPClientClose(WorkSocket);
	
	//TODO: DELAY TIME:
	//int indexDelayTime = OutBuffer[0] - 1;
	//DelayTime = OutBuffer[indexDelayTime];
}

void operazioni()
{ 
	if(Humd<arrayset[4])
	{
		// operazione da effetture se l'umidità è minore del parametro : invia al server : spostami in un luogo più umido ( messaggio 1)
		arraymess[1]= (unsigned char)1;
		IsError = TRUE;
	}
	if(Humd>arrayset[0])
	{
		// operazione da effetture se l'umidità è maggiore del parametro : invia al server : spostami in un luogo più asciutto	(messaggio 2)
		arraymess[2]= (unsigned char)1;
		IsError = TRUE;
	}
	if(Tempt<arrayset[5])
	{
		// operazione da effetture se la temperatura è minore del parametro : invia al server: ho freddo (messaggio 3)
		arraymess[3]=(unsigned char)1;
		IsError = TRUE;
	}
	if(Tempt>arrayset[1])
	{
		// operazione da effetture se la temperature è maggiore del parametro : invia al server : ho caldo (messaggio 4)
		arraymess[4]=(unsigned char)1;
		IsError = TRUE;
	}
	if(Terr<=arrayset[6])
	{ 
		counter_pompa=counter_pompa+1;
		if(counter_pompa == 5)
		{
			pompa = on;
			counter_pompa = 0;
		}	
		arraymess[5] = (unsigned char)1;
		IsError = TRUE;
		// operazione da effetture se l'umidità del terreno è minore del parametro : accendo la pompa (messaggio 5)
	}
	if(Terr>arrayset[2])
	{
		counter_pompa = 0;
		pompa = off;
		// operazione da effetture se l'umidità del terreno è maggiore del parametro : regola la pompa in modo che la pompa lavori di meno 
	}
	if(Lum<arrayset[3])
	{
		// operazione da effetture se la luce è minore del parametro : invia al server : ho bisogno di luce (messaggio 6)
		//IOPut(RGB_RG, on);
		RGB_RG = on;
		arraymess[6]=(unsigned char)1;
		IsError = TRUE;
	}
	if(Lum>arrayset[3]) // arrayset[7]
	{
		// operazione da effetture se la luce è maggiore del parametro : invia al server (messaggio 7) 
		RGB_RG = off;
		arraymess[7]=(unsigned char)1;
		IsError = TRUE;
	}
}

void serbatoio()
{   
	IOInit(p5, inup);                  	          // digital pin5 (DIG3)
	int c = IOGet(p5);
	
		if (c == on)
		{ 
			RGB_B = on;             		          //RGB: blu light
			arraymess[7]= (unsigned char)1;			  //comunica al server: rabboccare acqua (messagio 8)
		}
		else
		{
			RGB_B = off;
			arraymess[7]= (unsigned char)0;
		}
}

void FlyportTask()
{
	BOOL ThereIsConnection = FALSE;
	//CONNECTO TO WI-FI
		
	// Flyport connects to default network
	
	WFConnect(WF_DEFAULT);
	while(WFGetStat() == CONNECTING);
	
	if(WFGetStat() == CONNECTED)
	{
		ThereIsConnection = TRUE;
		UARTWrite(1, "\n\n\nWIFI CONNECTED");
	}
	else
	{
		UARTWrite(1, "\n\n\nWIFI NOT CONNECTED");
	}
	vTaskDelay(25);
	UARTWrite(1,"Flyport Wi-fi G connected...hello world!\r\n");
	
	SDCheck();
	
	sprintf(sdfreespace, "%lu\n", SDFreeSpace());
    UARTWrite(1, sdfreespace);
	
	if(sdfreespace > 0)
	{
		if (SDFileCheck("max_min.txt"))
		{
			UARTWrite(1, "File max_min exists!\n");
		}
		else
		{
			if(SDFileCreate("max_min.txt"))
				UARTWrite(1, "File max_min created!\n");
			else
			{
				UARTWrite(1, "File max_min not created!\n");
				sprintf(sderror, "Error:%d\n", SDGetErr());
				UARTWrite(1, sderror);
			}
		}	
	
		if (SDFileCheck("history.txt"))
		{
			UARTWrite(1, "File istorical exists!\n");
		}
		else
		{
			if(SDFileCreate("history.txt"))
				UARTWrite(1, "File istorical created!\n");
			else
			{
				UARTWrite(1, "File istorical not created!\n");
				sprintf(sderror, "Error:%d\n", SDGetErr());
				UARTWrite(1, sderror);
			}
		}	
	}		
	
	//CONNECTO TO SERVER
	if(!connessione_server(ThereIsConnection))
	{
		//TODO: Accendi led non connessione
		DWORD filSize = SDFileSize("max_min.txt");
		if(filSize != -1)
		{
			char msgf[9];
			SDFileRead("max_min.txt",msgf, 9);
			int a = 0;
			for(a=0; a<=9; a++)
			{
				arrayset[a] = msgf[a];
			}	
		}
	}
	else
	{  
		//TODO: Store MaxMin data on SD
		char msgm[10];
		sprintf(msgm,"%s\n", arrayset);
		SDFileAppend("max_min.txt",msgm,10);
		
		DWORD filSize = SDFileSize("history.txt");
		if(filSize != -1)
		{
			char Lectures[filSize];
			SDFileRead("history.txt",Lectures, filSize);
		}
	}
	
	//IO pins for leds
	IOInit(p11,out);                            //connected to Red led
    IOInit(p12,out);                			//connected to Green led
    IOInit(p14,out);                			//connected to Blue led

    void *board = new(GroveNest);
    void *rht03 = new(RHT03);
	void *light_sensor = new (An_i);
    void *moisture_sensor = new (An_i);
	void *relay = new(Dig_io, OUT); 
	attachToBoard(board, relay, DIG3);          // Device attached to board, on digital p10 (DIG3)
    
	set(relay, ON);								// Turns ON the relay and Closes the relay contact
    set(relay, OFF);							// Turns OFF the relay and Opens the relay contact	

	
	attachToBoard(board,rht03,DIG1);            // Attach Temperature-Humidity, on digital port1 (DIG1)
	attachToBoard(board, light_sensor, AN1);    // Light attached to board, on analog port 3 (AN3)
	attachToBoard(board, moisture_sensor, AN2); // Attach Moisture, on analog port 2 (AN2)                
	 
	configure(rht03, 3);
	IOInit(p4,inup);
	IOInit(p6,inup);
	void *batt = new (An_i);
	attachToBoard(board, batt, AN3);
	
	//alimentazioni
	//Valore numerico dell'alimentazione dalla rete (0 rete 1 batteria)
	int alimentatore = 0;
	//Valore della batteria 
	unsigned char battery = 0;
    
	int battery_max= 174 ;       // 4.2V
    int battery_min= 150 ;       // 3.6V
	
	int flusso = 0;
	int durata = 2000;           // 7s
	
	
	//Output message
	char msg[100];
	int Send = 0;

	pompa = off;
	counter_pompa=0;
	
	while(1)
	{
		SDCheck();
		//LETTURA DEI SENSORI E ASSEGNAZIONE AD ARRAYSENS
		{
			// Reading external humidity 
			Humd = (unsigned char)  get(rht03, HUMD);
			arraysens[0] = Humd;
			
			// Reading external temperature 
			Tempt = (unsigned char) get(rht03, TEMP);
			arraysens[1] = Tempt;
			
			// Reading terrain humidity
			Terr = (unsigned char) get(moisture_sensor);
			arraysens[2] = Terr;
			
			// Reading light
			Lum = (unsigned char) get(light_sensor);
			arraysens[3] = Lum;
		}

		//CONTROLLO IL TIPO DI ALIMENTAZIONE
		{
			battery = (unsigned char) get(batt);
			
			//***STAMPA BATTERY
			char as[3];
			battery_perc = ((battery - battery_min)*100)/(battery_max - battery_min);
			sprintf(as, "BATTERIA %d\nALIMENTAZIONE %d\n percentuale batteria %d\n", battery, alimentatore,(unsigned char) battery_perc);
			
			UARTWrite(1, as); 
			//******
			
			//Se il pin6 (alimentazione di rete) è impostato su on, alimentatore viene spostato su on, altrimenti su off.
			if (IOGet(p6) != on)
			{
				alimentatore = off;
			}
			else
			{
				alimentatore = on;
			}
			//Calcolo la percentuale della batteria
			
			//battery_perc = battery_perc/(battery_max - battery_min);
			arraysens[4] = battery_perc;
			
			if(alimentatore == on)
			{
				//Se il vaso è attaccato alla rete
				//accendi il led verde e spegni il led viola
				RGB_G = on;// RGB: green light
				RGB_RB = off;
				RGB_R = off;
			}
			else
			{
				//Se il vaso si alimenta dalla batteria
				//accendi il led viola e spegni il led verde
				RGB_RB = on;
				RGB_G = off;
				RGB_R = off;
			}
			
			if (battery_perc <= 15)
			{
				RGB_R = on;                       // RGB: red light
				arraymess[0]=(unsigned char)1;				            // comunica al server: batteria quasi scarica (messagio 0) 
			}
			
		}
			
		//CONTROLLO SE MASSIMI E MINIMI SONO COERENTI CON I VLAORI LETTI
		
		
		operazioni();
		
		
		serbatoio();
		
		
		if((IsError)&&(ThereIsConnection))
		{
			UARTWrite(1, "SendWarning");
			SendWarning();
		}
		 
		//CONTROLLO POMPA
		if(pompa == on)
		{
	    	if(RGB_RB == off)
			{
				set(relay, on);								// close relay contact
				vTaskDelay(1000);
				
				int j=0;
				char fluss[20];
				for(j=0; j<=(durata/1000); j++)
				{
					flusso = IOGet(p4);
					if(flusso != on)
					{
						vTaskDelay(250);
						sprintf(fluss,"%d\n\r", flusso);
						UARTWrite(1,fluss);
					}
					else
					{
						j = (durata/1000);
						flusso = IOGet(p4);
						pompa = off;
						sprintf(fluss,"%d\n\r", flusso);
						UARTWrite(1,fluss);
					}
				}
				
				set(relay, off);				      // open relay contact
			}
		pompa = off;
		}
		
		// Leds control

		{
		if(RGB_G == on)
	    {
			IOPut(p11, off);
			IOPut(p12, on);
			IOPut(p14, off);
		
			if(RGB_B == on)
				{
				IOPut(p11, off);
				IOPut(p12, off);
				IOPut(p14, on);
				vTaskDelay(200);
			
				if(RGB_RG == on)
					{
					IOPut(p11, on);
					IOPut(p12, on);
					IOPut(p14, off);
					vTaskDelay(200);
					IOPut(p11, off);
					IOPut(p12, off);
					IOPut(p14, on);
					vTaskDelay(200);
					}	
				}
		if(RGB_RG == on)
		{
			IOPut(p11, on);
			IOPut(p12, on);
			IOPut(p14, off);
		}
		}
		
		if((RGB_R == on)&&(RGB_RB == on))
			{
			IOPut(p11, on);
			IOPut(p12, off);
			IOPut(p14, off);
			vTaskDelay(200);
			if(RGB_RG == on)
				{
				IOPut(p11, on);
				IOPut(p12, on);
				IOPut(p14, off);
				vTaskDelay(200);
				IOPut(p11, on);
				IOPut(p12, off);
				IOPut(p14, off);
				vTaskDelay(200);
				}
			}
		if((RGB_RB == on)&&(RGB_R == off))
		{
			IOPut(p11, on);
			IOPut(p12, off);
			IOPut(p14, on);
			vTaskDelay(200);
			if(RGB_RG == on)
			{
				IOPut(p11, on);
				IOPut(p12, on);
				IOPut(p14, off);
				vTaskDelay(200);
				IOPut(p11, on);
				IOPut(p12, off);
				IOPut(p14, on);
				vTaskDelay(200);
			}
		}
		}
		
		
		sprintf(msg,"%d %d %d %d %d %d %d %d \r\n",arrayset[0],arrayset[1],arrayset[2],arrayset[3],arrayset[4],arrayset[5],arrayset[6],arrayset[7]);
		UARTWrite(1, msg);
		char msgsens[400];
		sprintf(msgsens,"temperatura: %d \numidità aria: %d \numidità terreno: %d \nluce: %d \n \n", Tempt,Humd,Terr,Lum);
		UARTWrite(1, msgsens);
		UARTWrite(1, batt);
		
		int assegnazione=0;
		
		for(assegnazione=0; assegnazione<=8; assegnazione++)
		{
			arraymess[assegnazione] = (unsigned char)0;
		}
		
		if(!ThereIsConnection)
		{
			WFConnect(WF_DEFAULT);
			while(WFGetStat() == CONNECTING);
	
			if(WFGetStat() == CONNECTED)
			{
				ThereIsConnection = TRUE;
				UARTWrite(1, "\n\n\nWIFI CONNECTED");
				UARTWrite(1,"Flyport Wi-fi G connected...hello world!\r\n");
			}
			else
			{
				UARTWrite(1, "\n\n\nWIFI NOT CONNECTED");
			}
			vTaskDelay(25);
		}
		
		if((Send == 0)&&(ThereIsConnection))
		{
			UARTWrite(1,"Send update");
			if(!SendUpdate())
			{
				//TODO: Save data on SD
				char msgd[20];
				char sprintarray[6];
				sprintarray[5] = arraymess[7];
	         
				//TODO: Aggiongere waterlevel:
				sprintf(msgd,"%s", sprintarray);
				SDFileAppend("history.txt", sprintarray, 6);
			}	
		}
		
		vTaskDelay(250);
		
		Send++;
		
		if(Send == (DelayTime*60))
		{
			Send = 0;
		}
		//TODO:COMMENTO THIS LINE
		Send = 0;
	}
}
	

