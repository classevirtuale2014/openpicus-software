#include "taskFlyport.h"
#include "grovelib.h"
#include "rht03.h"

    
	
	unsigned char Humd ;
	unsigned char Tempt ;
	unsigned char Terr ; 
	unsigned char Lum ;		

    //comunicazione con server
	unsigned char arraysens[5];      		    //array dati sensori
	char arrayset[9];   						//array parametri arrivati dal server
	char arraymess[9];  						//array dei messaggi
	
	//sensori
	//char temperature[5];
    //char humidity[5];
    //char terrain[5];
	//char light[5];
    
	//server
	int ID = 1;
	
	//comandi
	int pompa;
	int RGB_R = off;
	int RGB_G;
	int RGB_B = off;
	int RGB_RB = off;    						//purple
	int RGB_RG;									//yellow
	
	//alimentazioni
	int alimentatore;
    float battery_perc;
	
	int p=0;
	
	void *Relay;
	
BOOL FirstConnect()
{		
	//Dichiaro il socket per la connessione
	TCP_SOCKET WorkSocket = INVALID_SOCKET;
	
	char OutBuffer[3];
	
	//Dato che è la prima connessione il messaggio deve essere lungo 2 char: primo lunghezza, secondo tipo di messaggio (0 - prima registrazione)
	
	_dbgwrite("\nTento la connessione iniziale...");
	
	//Apro la connessione al server
 	WorkSocket = TCPClientOpen("192.168.33.112", "10111");
	
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
	
BOOL connessione_server()
{
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
	for(i = 0; i < 6; i++)
	{
		int index = i + 3;
		OutBuffer[index] = arraysens[i];
	}
	
	WorkSocket = TCPClientOpen("192.168.33.112", "10111");

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
	
	/*BOOL Continue = FALSE;
	while(!Continue)
	{
		while ((RxLen = TCPRxLen(WorkSocket))>0)
		{
			TCPRead(WorkSocket, InBuffer, RxLen);
			Continue = TRUE;
		}
	}
		
	if(InBuffer[2] == (char)2)
	{
		if(FirstConnect())
		{
			if(!SendUpdate())
			{
				return FALSE;
			}
		}
		else
		{
			//Accendi led di non connessione
			return FALSE;
		}
	}*/
	
	TCPClientClose(WorkSocket);
	
	UARTWrite(1,"ChiusoSocketUpdate\n");
	
	return TRUE;
}
/*
BOOL SendWarning()
{
	TCP_SOCKET
}*/

void operazioni()
{ 



if(Humd<arrayset[0])
  {
	// operazione da effetture se l'umidità è minore del parametro : invia al server : spostami in un luogo più umido ( messaggio 1)
	arraymess[1]=33+1;
  }
if(Humd>arrayset[4])
  {
	// operazione da effetture se l'umidità è maggiore del parametro : invia al server : spostami in un luogo più asciutto	(messaggio 2)
	arraymess[2]=33+2;
  }
if(Tempt<arrayset[1])
  {
	// operazione da effetture se la temperatura è minore del parametro : invia al server: ho freddo (messaggio 3)
	arraymess[3]=33+3;
  }
if(Tempt>arrayset[5])
  {
	// operazione da effetture se la temperature è maggiore del parametro : invia al server : ho caldo (messaggio 4)
	arraymess[4]=33+4;
  }
if(Terr<(unsigned char)arrayset[2])
  { 
	  UARTWrite(1, "Terr minore");
	p=p+1
	if(p==3)
	{
			pompa = on;
			p=0;
	}	

	
	arraymess[5]=33+5;
	// operazione da effetture se l'umidità del terreno è minore del parametro : accendo la pompa (messaggio 5)
	
  }
  if(Terr>(unsigned char)arrayset[6])
  {
	  UARTWrite(1, "Terr maggiore");
	  p=0;
	  pompa=off;
	// operazione da effetture se l'umidità del terreno è maggiore del parametro : regola la pompa in modo che la pompa lavori di meno 
  }
if(Lum<arrayset[3])
  {
	// operazione da effetture se la luce è minore del parametro : invia al server : ho bisogno di luce (messaggio 6)
	IOPut(RGB_RG, on);
	arraymess[6]=33+6;
  }
if(Lum>arrayset[7])
  {
	// operazione da effetture se la luce è maggiore del parametro : invia al server: spostami in un posto con un po' di ombra (messaggio 7) 
	arraymess[7]=33+7;
  }
}
/*
void serbatoio()
{   
	IOInit(p5, inup);                           // digital pin5 (DIG3)
	if (IOButtonState(p5)== pressed)
	{ 
		IOPut(RGB_B, on);                       //RGB: blu light
	    arraymess[7]=33+7;                         //comunica al server: rabboccare acqua (messagio 8)
	}
}
*/
/*
void pompa_acqua()
{
	const int pot = 100;                        //Initialize % of power
	int time = 10;                              //Delay
	IOInit(p10,out);
	
	if(RGB_RB == off)
	{
	  if(pompa == on)
	  {
	  IOPut(10, on);

	  vTaskDelay(5000);
	 
	  IOPut(10, off);
	  }
	}
}
*/

void led()
{
    IOInit(p11,out);                            //connected to Red led
    IOInit(p12,out);                			//connected to Green led
    IOInit(p14,out);                			//connected to Blue led
    if(RGB_G == off)
	{
		return;
	}
	if(RGB_G == on)
	{
		IOPut(p11, on);
		IOPut(p12, off);
		IOPut(p13, on);
		if(RGB_B == on)
		{
			IOPut(p11, on);
		    IOPut(p12, on);
		    IOPut(p13, off);
			vTaskDelay(200);
			if(RGB_RG == on)
			{
				IOPut(p11, off);
		        IOPut(p12, off);
		        IOPut(p13, on);
				vTaskDelay(200);
				IOPut(p11, on);
		        IOPut(p12, on);
		        IOPut(p13, off);
			    vTaskDelay(200);
			}	
	    if(RGB_RG == on)
		{
			IOPut(p11, off);
			IOPut(p12, off);
			IOPut(p13, on);
			vTaskDelay(200);
		}
		}
	}
   if(RGB_RB == on)
	  {
		IOPut(p11, off);
		IOPut(p12, on);
		IOPut(p13, off);
		vTaskDelay(200);
		if(RGB_RG == on)
		{
			IOPut(p11, off);
		    IOPut(p12, off);
		    IOPut(p13, on);
			vTaskDelay(200);
			IOPut(p11, off);
		    IOPut(p12, on);
		    IOPut(p13, off);
			vTaskDelay(200);
		}
	 if(RGB_R == on)
	  {
		IOPut(p11, off);
		IOPut(p12, on);
		IOPut(p13, on);
		vTaskDelay(200);
		if(RGB_RG == on)
		 {
			IOPut(p11, off);
		    IOPut(p12, off);
		    IOPut(p13, on);
			vTaskDelay(200);
			IOPut(p11, off);
		    IOPut(p12, on);
		    IOPut(p13, on);
			vTaskDelay(200);
		 }
	    }
	  }
}




void FlyportTask()
{
	// Flyport connects to default network
	WFConnect(WF_DEFAULT);
	while(WFGetStat() != CONNECTED);
	vTaskDelay(25);
	UARTWrite(1,"Flyport Wi-fi G connected...hello world!\r\n");
	
	if(!connessione_server())
	{
		//Accendi led non connessione
	}

    void *board = new(GroveNest);
    void *rht03 = new(RHT03);
	void *light_sensor = new (An_i);
    void *moisture_sensor = new (An_i);
	void *relay = new(Dig_io, OUT); 
	attachToBoard(board, relay, DIG3);          // Device attached to board, on digital p10 (DIG3)
    
	set(relay, ON);								// Turns ON the relay and Closes the relay contact
    set(relay, OFF);							// Turns OFF the relay and Opens the relay contact	

	
	attachToBoard(board,rht03,DIG1);            // Attach Temperature-Humidity, on digital port1 (DIG1)
	attachToBoard(board, light_sensor, AN3);    // Light attached to board, on analog port 3 (AN3)
	attachToBoard(board, moisture_sensor, AN2); // Attach Moisture, on analog port 2 (AN2)                
	 
	configure(rht03, 3);
	IOInit(p6,inup);
	IOInit(p23,inup);
	void *batt = new (An_i);
	attachToBoard(board, batt, p23);
	
	char msg[100];
	int Send = 0;
	
	while(1)
	{
												// sensor reading
		{
			// Reading external humidity 
			Humd = (unsigned char)  get(rht03, HUMD);
			//sprintf (humidity, "%3d\r", Humd);
			
			arraysens[0] = Humd;
			
			// Reading external temperature 
			Tempt = (unsigned char) get(rht03, TEMP);
			//sprintf (temperature, "%3d\r", Tempt);
			
			arraysens[1] = Tempt;
			
			// Reading terren humidity
			Terr = (unsigned char) get(moisture_sensor);
			//sprintf (terrain, "%3d\r", Terr);
	
			arraysens[2] = Terr;
			
			// Reading light
			Lum = (unsigned char) get(light_sensor);
			//sprintf (light, "%3d\r", Lum);
			arraysens[3] = Lum;
		}

/*												// alimentation control
		{   
	
							             
	 if (IOGet(p6) == on) 
	 {
		alimentatore = 1;
	 }
	
	 
	                           // battery control
	int battery;
    int battery_max= 37 ;       // 37=3.7V
	int battery_min= 33 ;       // 33=3.3V
	
    battery = get(batt);
	battery_perc = (battery/255)*100;
	arraysens[4]=battery_perc;
	
	if( alimentatore == 1)
	{
		IOPut(RGB_G, on);                       // RGB: green light
	}
	else
	{
		IOPut(RGB_RB, on);                      // RGB: violet light
	}
	
	if (battery_perc <= 15)
	{
		IOPut(RGB_R, on);                       // RGB: red light
		arraymess[0]=33;				            // comunica al server: batteria quasi scarica (messagio 0) 
	}
	
}
*/

        //serbatoio();
		led();
		operazioni();
		if(pompa==on)
		{
			set(relay, on);					        	// open relay contact
		    vTaskDelay(1000);
			set(relay, off);					        // close relay contact	
			pompa=off;
		}
		led();
		
		
		
		sprintf(msg,"%d %d %d %d %d %d %d \r\n",arrayset[0],arrayset[1],arrayset[2],arrayset[3],arrayset[4],arrayset[5],arrayset[6]);
		UARTWrite(1, msg);
		char msgtemp[50];
		char msghum[50];
        char msgterr[50];
		char msglig[50];
		sprintf(msgtemp,"temperatura: %d \n", Tempt);
		sprintf(msghum,"umidità aria: %d \n", Humd);
		sprintf(msgterr,"umidità terreno: %d \n", Terr);
		sprintf(msglig,"luce: %d \n \n ", Lum);
		UARTWrite(1, msgtemp);
		UARTWrite(1, msghum);
		UARTWrite(1, msgterr);
		UARTWrite(1, msglig);
		vTaskDelay(500);
		Send++;
		if(Send == 1)
		{
			Send = 0;
			SendUpdate();
		}
	}
}

