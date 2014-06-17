#include "taskFlyport.h"
#include "grovelib.h"
#include "rht03.h"


	unsigned char Humd = 0;
	unsigned char Tempt = 0;
	unsigned char Terr = 0; 
	unsigned char Lum = 0;		

    //comunicazione con server
	char arraysens[5];      				    //array dati sensori
	char arrayset[9];   						//array parametri arrivati dal server
	char arraymess[9];  						//array dei messaggi
	
	//sensori
	char temperature[3];
    char humidity[3];
    char terrain[3];
	char light[3];
    
	//server
	int ID = 1;
	
	//comandi
	int pompa;
	int RGB_R;
	int RGB_G;
	int RGB_B;
	int RGB_RB;
	int RGB_RG;
	
	//alimentazioni
	int alimentatore;
    float battery_perc;
	
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
	UARTWrite(1, "Invio aggiornamento al server");
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
	
	if(!TCPisConn(WorkSocket))
	{
		return FALSE; 
	}
	
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
	}
	
	TCPClientClose(WorkSocket);
	
	return TRUE;
}
/*
BOOL SendWarning()
{
	TCP_SOCKET
}*/

void operazioni()
{ 
arraysens[0]= (char) humidity;
arraysens[1]= (char) temperature;
arraysens[2]= (char) terrain;
arraysens[3]= (char) light;

if(arraysens[0]<arrayset[0])
  {
	// operazione da effetture se l'umidità è minore del parametro : invia al server : spostami in un luogo più umido ( messaggio 1)
	arraymess[1]=33+1;
  }
if(arraysens[0]>arrayset[4])
  {
	// operazione da effetture se l'umidità è maggiore del parametro : invia al server : spostami in un luogo più asciutto	(messaggio 2)
	arraymess[2]=33+2;
  }
if(arraysens[1]<arrayset[1])
  {
	// operazione da effetture se la temperatura è minore del parametro : invia al server: ho freddo (messaggio 3)
	arraymess[3]=33+3;
  }
if(arraysens[1]>arrayset[5])
  {
	// operazione da effetture se la temperature è maggiore del parametro : invia al server : ho caldo (messaggio 4)
	arraymess[4]=33+4;
  }
if(arraysens[2]<arrayset[2])
  {
	// operazione da effetture se l'umidità del terreno è minore del parametro : accendo la pompa (messaggio 5)
	pompa = on;
	arraymess[5]=33+5;
  }
  if(arraysens[2]>arrayset[6])
  {
	// operazione da effetture se l'umidità del terreno è maggiore del parametro : regola la pompa in modo che la pompa lavori di meno 
  }
if(arraysens[3]<arrayset[3])
  {
	// operazione da effetture se la luce è minore del parametro : invia al server : ho bisogno di luce (messaggio 6)
	IOPut(RGB_RG, on);
	arraymess[6]=33+6;
  }
if(arraysens[3]>arrayset[7])
  {
	// operazione da effetture se la luce è maggiore del parametro : invia al server: spostami in un posto con un po' di ombra (messaggio 7) 
	arraymess[7]=33+7;
  }
}

void serbatoio()
{   
	IOInit(p5, inup);                           // digital pin5 (DIG3)
	if (IOButtonState(p5)== pressed)
	{ 
		IOPut(RGB_B, on);                       //RGB: blu light
	    arraymess[7]=33+7;                         //comunica al server: rabboccare acqua (messagio 8)
	}
}

void pompa_acqua()
{
	const int pot = 100;                        //Initialize % of power
	int time = 10;                              //Delay
	IOInit(p10,out);
	
	if(RGB_RB == off)
	{
	  if(pompa == on)
	  {
	PWMInit(1,1000,pot);                        //Initialize PWM1 to work at 1000 Hz, pot % of duty
	PWMOn(p10, 1);                              //Assign pin 10 as PWM1 and turns it on
	for(; time<=0; time--)
	{
	PWMDuty(pot, 1);                            //Change the duty at pot value
	vTaskDelay(1000);                     
	}
		}
	else 
	{
	  IOPut(10, off);
	}	
}
}

void alimentazione()                            // controllo se la scheda è collegata alla corrente di rete o alla batteria
{   
	IOInit(p6,inup);				            // controllo corrente di rete   
	 if (IOButtonState(p6) == pressed) 
	 {
		alimentatore = 1;
	 }
	
	 void *board = new(GroveNest);
	IOInit(p23,inup);                           // controllo batteria
	int battery;
	void *batt = new (An_i);
	attachToBoard(board, batt, p23);
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


void led()
{
    IOInit(p11,out);                            //connected to Red led
    IOInit(p12,out);                			//connected to Green led
    IOInit(p14,out);                			//connected to Blue led

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

	
	attachToBoard(board,rht03,DIG1);            // Attach Temperature-Humidity, on digital port1 (DIG1)
	attachToBoard(board, light_sensor, AN3);    // Light attached to board, on analog port 3 (AN3)
	attachToBoard(board, moisture_sensor, AN2); // Attach Moisture, on analog port 2 (AN2)                
	
	configure(rht03, 3);

	int Send = 0;
	
	while(1)
	{
			// lettura sensori
		{
			// Reading external humidity 
			Humd = (unsigned char)  get(rht03, HUMD);
			sprintf (humidity, "%3d\n", Humd);
               
			// Reading external temperature 
			Tempt = (unsigned char) get(rht03, TEMP);
			sprintf (temperature, "%3d\n", Tempt);
		
			// Reading terren humidity
			Terr = (unsigned char) get(moisture_sensor);
			sprintf (terrain, "%3d\n", Terr);
	
			// Reading light
			Lum = (unsigned char) get(light_sensor);
			sprintf (light, "%3d\n", Lum);
		}
		operazioni();
		led();
		serbatoio();
		led();
		pompa_acqua();
		led();
		alimentazione();
		/*char msg[100];
		sprintf(msg,"%d %d %d %d %d %d %d \r\n",arrayset[0],arrayset[1],arrayset[2],arrayset[3],arrayset[4],arrayset[5],arrayset[6]);
		UARTWrite(1, msg);*/
		vTaskDelay(500);
		Send++;
		if(Send == 1)
		{
			Send = 0;
			SendUpdate();
		}
	}
}

