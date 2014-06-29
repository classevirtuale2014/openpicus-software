#include"SD_HW.h"


int sdsi, sdso, sdcs, sdsc, sddet;
extern int *CNPUs[];
extern int *CNPDs[];
extern int CNPos[];
static int addval;

void pinConfig(int _sdsc, int _sdsi, int _sdso, int _sdcs, int _sddet)
{
	sdsc = _sdsc;
	sdsi = _sdsi;
	sdso = _sdso;
	sdcs = _sdcs;
	sddet = _sddet;
	
	IOInit(sddet, inup);
	//SPI2 pins configuration:
	IOInit(sdsi, SPI_IN);
	IOInit(sdsc, SPICLKOUT);
	IOInit(sdso, SPI_OUT);
	
	// init CS line as ouput, default high
	IOInit(sdcs, out); // SD-CS line
	IOPut(sdcs, on);
	
	//	Pullup resistors configuration
	
	addval = 1 << CNPos[sdsc-1];
	addval = ~addval;
	*CNPDs[sdsc-1] = *CNPDs[sdsc-1] & addval;
	addval = ~addval;
	*CNPUs[sdsc-1] = *CNPUs[sdsc-1] | addval;
	
	
	addval = 1 << CNPos[sdsi-1];
	addval = ~addval;
	*CNPDs[sdsi-1] = *CNPDs[sdsi-1] & addval;
	addval = ~addval;
	*CNPUs[sdsi-1] = *CNPUs[sdsi-1] | addval;

	addval = 1 << CNPos[sdso-1];
	addval = ~addval;
	*CNPDs[sdso-1] = *CNPDs[sdso-1] & addval;
	addval = ~addval;
	*CNPUs[sdso-1] = *CNPUs[sdso-1] | addval;
}


void CS_LOW()
{
	IOPut(sdcs,OFF);
}


void CS_HIGH()
{
	IOPut(sdcs, ON);
}

void MMC_SEL()
{
	IOPut(sdcs, toggle);
}

BOOL sdDetect()
{
	if (sddet > 0)
		return (!(IOGet(sddet)));
	else
		return TRUE;
}
