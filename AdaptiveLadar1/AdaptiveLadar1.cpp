// AdaptiveLidar.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "lidar.h"



using namespace std;

float getAdaptive(float x, float y, float fnlDelayMin, float fnlDelayMax, float adapv = 0) {
	printf("Doing a fine scan\n");
	delayNCount maxCount;
	maxCount.count = 0;
	maxCount.delay = 0;
	int maxCountPos = 0;
	int count;
	size_t counter = 0;
	size_t current_dc = 0;
	float dlyitr;
	for (dlyitr = fnlDelayMin; dlyitr < fnlDelayMax; dlyitr = dlyitr + microSteps) {
		setDly(dlyitr);
		count = getCount();
		dc[counter].delay = dlyitr;
		dc[counter].count = count;
		fprintf(fineData, "%f,%f,%f,%d\n", dlyitr, y, x, count);
		if(verbose)
		printf("Fine scan.. Delay: %f\t X: %f\tY: %f\t Count: %d\n", dlyitr, x, y, count);
		if (maxCount.count < count) {
			maxCount.count = count;
			maxCount.delay = dlyitr;
			maxCountPos = counter;

		}
		counter++;
	}

	if (maxCountPos == counter - 1 || maxCountPos == counter - 2)
	{
		bool reachMax = 0;
		//		dlyitr = fnlDelayMax;
		while (!reachMax)
		{
			setDly(dlyitr);
			dlyitr += microSteps;
			count = getCount();
			dc[counter].delay = dlyitr;
			dc[counter].count = count;
			fprintf(fineData, "%f,%f,%f,%d\n", dlyitr, y, x, count);
			if (verbose)
				printf("Fine scan.. Delay: %f\t X: %f\tY: %f\t Count: %d\n", dlyitr, x, y, count);
			if (maxCount.count < count) {
				maxCount.count = count;
				maxCount.delay = dlyitr;
				maxCountPos = counter;
			}
			else
			{
				reachMax = 1;
				//				return maxCount.delay;
			}
			counter++;
		}
	}

/*	if (adapv != 0.0)
	{
		float dlyitr_t = maxCount.delay + 10;
		setDly(dlyitr_t);
		count = getCount();
		if (maxCount.count < count)
		{
			if (dlyitr != maxCount.delay + microSteps)
			{

			}

			maxCount.count = count;
			maxCount.delay = dlyitr;
			maxCountPos = counter;
			dc[counter].delay = dlyitr;
			dc[counter].count = count;
			fprintf(fineData, "%f,%f,%f,%d\n", dlyitr, y, x, count);
		}
	}*/

	if (maxCountPos == 0 || maxCountPos == 1)
	{
		bool reachMax = 0;
		dlyitr = fnlDelayMin;
		while (!reachMax)
		{
			dlyitr -= microSteps;
			setDly(dlyitr);
			count = getCount();
			dc[counter].delay = dlyitr;
			dc[counter].count = count;
			fprintf(fineData, "%f,%f,%f,%d\n", dlyitr, y, x, count);
			if (verbose)
				printf("Fine scan.. Delay: %f\t X: %f\tY: %f\t Count: %d\n", dlyitr, x, y, count);
			if (maxCount.count < count) {
				maxCount.count = count;
				maxCount.delay = dlyitr;
				maxCountPos = counter;
			}
			else
			{
				reachMax = 1;
//				return maxCount.delay;
			}
			counter++;
		}
	}

	

	int checkone, checktwo;
	checkone = dc[maxCountPos - 1].count - dc[maxCountPos - 2].count;
	checktwo = dc[maxCountPos + 1].count - dc[maxCountPos + 2].count;
	if ((checkone > 0) && (checktwo > 0)) {
		return maxCount.delay;
		//printf( "Max from the fine data... Delay: %f, X: %f, Y: %f, Count: %d\n", maxCount.delay, x, y, maxCount.count);
	}
	else
		return 0;

}

float nearScan(float x, float y, float fnlDelayMin, float fnlDelayMax, float adapv = 0)
{
	printf("Doing a fine scan\n");
	delayNCount maxCount;
	maxCount.count = 0;
	maxCount.delay = 0;
	int maxCountPos = 0;
	int count;
	size_t counter = 0;
	size_t current_dc = 0;
	float dlyitr;
	for (dlyitr = fnlDelayMin; dlyitr < fnlDelayMax; dlyitr = dlyitr + adapmacroSteps)
	{
		setDly(dlyitr);
		count = getCount();
		dc[counter].delay = dlyitr;
		dc[counter].count = count;
//		fprintf(fineData, "%f,%f,%f,%d\n", dlyitr, y, x, count);
		if (verbose)
			printf("Fine scan.. Delay: %f\t X: %f\tY: %f\t Count: %d\n", dlyitr, x, y, count);
		if (maxCount.count < count) {
			maxCount.count = count;
			maxCount.delay = dlyitr;
			maxCountPos = counter;
		}
		counter++;
	}

	return maxCount.delay;
}

void setDly(float dly) {
	auto comm = std::to_string(dly);
	comm = "DLY " + comm;
	ibwrt(dly_ud, comm.c_str(), comm.length());
	// have to wait till the optical delay is ready
	DWORD sleeptime = (int)(abs((delay_last-dly)) * 15);
	if (sleeptime > 150) {
		Sleep(sleeptime);
	}
	
	delay_last = dly;
}

HANDLE connect_tdc() {
	//TDC initialization
	HRM_STATUS error;

	Ulong moduleCount;
	//HANDLE hdl[1];


	/*
	Initialise variables and clear error report
	*/
	error = HRM_OK;

	HRM_GetLastError(HRM_OK);
	/*
	Detect the number of HRM-TDC modules
	*/
	if (error == HRM_OK)
	{
		HRM_RefreshConnectedModuleList();
		moduleCount = (Ulong)HRM_GetConnectedModuleCount();
	}
	/*
	If a module is present connect to the first module.
	If no module, set error and reason in last error report.
	*/
	if (error == HRM_OK)
	{
		if (moduleCount)
			HRM_GetConnectedModuleList(hdl);
		else
		{
			HRM_GetLastError(HRM_OPEN_USB);
			error = HRM_ERROR;
		}
	}

	/*
	Set the channel edge enables.
	*/
	if (error == HRM_OK)
		error = HRM_SetEdgeSensitivityRegister(hdl[0], CHANN);

	return 0;
}

MTIDevice* connect_mems() {
	printf("creating a new mti\n");
	MTIDevice *mti = new MTIDevice();
	MTIError LastError;

	printf("connecing to the %s\n", portNo);
	mti->ConnectDevice(portNo);
	MTIDeviceParams *params = mti->GetDeviceParams();
	LastError = mti->GetLastError();
	if (LastError != MTIError::MTI_SUCCESS) {
		printf("Error in connecting to the device in the %s\n", portNo);
		exit(-1);
	}
	else printf("Connected to the mirror\n");

	//change the device specific parameters
	params->VdifferenceMax = 125;
	params->HardwareFilterBw = 140;
	params->Vbias = 80;
	//change the user related parameters
	params->DataMode = MTIDataMode::Sample_Output;			// this is default mode, shown here for demo
	params->DataScale = 1;								// example of scaling data/content to 80%
	params->SampleRate = 20000;
	params->DeviceAxes = MTIAxes::Normal;					// this is default mode, shown here for demo
	params->SyncMode = MTISync::Output_DOut0;
	params->DataRotation = 0;

	//set the above changed parameters
	mti->SetDeviceParams(params);
	mti->SetDeviceParam(MTIParam::MEMSDriverEnable, true);
	LastError = mti->GetLastError();
	if (LastError != MTIError::MTI_SUCCESS) {
		printf("Error in changing the device parameters\n");
		exit(-1);
	}

	mti->ResetDevicePosition(); 				// send analog outputs (and device) back to origin in 25ms
	LastError = mti->GetLastError();
	if (LastError != MTIError::MTI_SUCCESS) {
		printf("Error in reseting the device\n");
		exit(-1);
	}
	return mti;
}

int connect_dly() {
	//unsigned int dly_ud;


	printf("Connecting to the delay at address 5\n");

	dly_ud = ibdev(DLY_BOARDID, DLY_PRIMARYADD, DLY_SECONDARYADD, DLY_TIMEOUT, DLY_EOS, DLY_EOT);
	if (dly_ud == -1) {
		printf("Unable to connect to the delay device\n");
		exit(-1);
	}
	else
		printf("connected to the delay device\n");
	return dly_ud;
}

int getCount() {
	int i, j, k;
	/*
	Clear the memory.
	*/
	if (error == HRM_OK)
		error = HRM_InitMemory(hdl[0], 0, 0x200000, 0);
	/*
	Start the histogram running.
	*/

	if (error == HRM_OK)
		error = HRM_StartHistogramFSM(hdl[0], TCSPC, 0);
	/*
	Run the histogram for the programmed time in ms.
	*/

	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	LARGE_INTEGER freq;
	if (error == HRM_OK)
	{
		QueryPerformanceFrequency(&freq);
		double frequency = (double)freq.QuadPart;
		QueryPerformanceCounter(&start);
		double timep = 0.0;
		for (; timep < tdcTime;)
		{
			QueryPerformanceCounter(&stop);
			timep = (double)(stop.QuadPart - start.QuadPart) / frequency * 1000;
		}
	}
	//	for (i = GetTickCount(); (Uword)(GetTickCount() - i) < tdcTime;);
	/*
	Stop the histogram process.
	*/
	if (error == HRM_OK)
		error = HRM_SetModeBitsRegister(hdl[0], 0x0030);

	/*
	Read all the memory into the buffer.
	*/
	if (error == HRM_OK)
		error = HRM_ReadMemory(hdl[0], 0x0030, CH2_ADDR, CH3_ADDR, (BYTE*)buffer);
	/*
	Open the file and set the headings.
	*/
	if (error == HRM_OK) {

		for (i = 0, j = 0, k = 0; i != CH1_ADDR; i++)
		{
			j = (j == 0 && buffer[i]) ? i : j;
			k = (Ulong)(k + buffer[i]);
		}
		//fprintf(fl, "%f,%f,%f,%d\n", dly_itr, yitr, xitr, k);
		//printf("the first non zero occured at: %d", j);
	}

	return k;
}

void usage() {
	printf("Adaptive Scanning:\n");
	printf("-xmi for X min\n");
	printf("-xma for X max\n");
	printf("-ymi for Y min\n");
	printf("-yma for Y max\n");
	printf("-xst for X steps\n");
	printf("-yst for Y steps\n");
	printf("-fnm for File name\n");
	printf("-fn2 for File name2\n");
	printf("-pno for Port no\n");
	printf("-dmi for delay minimum\n");
	printf("-dma for delay maximam\n");
	printf("-mad for macro delay steps\n");
	printf("-mid for micro delay steps\n");
	printf("-pch for the peak check step\n");
	printf("-ths for the thershold\n");
	printf("-tdc for the TDC integration time\n ");
	printf("-ver for enable verbose or detailed comments");

}

float getPeak(float x, float y) {
	printf("Doing macro scan for the peak\n");
	delayNCount maxCount;
	maxCount.count = 0;
	maxCount.delay = 0;
	float dlyitr;
	// do the macrostep scan to locate where the peak might occur
	for (dlyitr = delayMin; dlyitr < delayMax; dlyitr = dlyitr + macroSteps) {
		dlyCntMxchk(x, y, dlyitr, &maxCount.delay, &maxCount.count);
	}
	//printf("The max count: %d for x: %f, y: %f occured at delay: %f\n", maxCount.count, x, y, maxCount.delay);

	//check if this the real macro peak or if it's the side small peak
	dlyCntMxchk( x,  y, maxCount.delay + peakCheck, &maxCount.delay, &maxCount.count);
	printf("Peak Check Max..  Delay: %f, X: %f, Y: %f, Count: %d\n", maxCount.delay, x, y, maxCount.count);
	return maxCount.delay;

}




void dlyCntMxchk(float x, float y, float dly, float * maxdly, int* maxcnt) {
	int count;
	setDly(dly);
	count = getCount();
	fprintf(peakData, "%f,%f,%f,%d\n", dly, x, y, count);
	if (verbose)
		printf("Peak Check...  Delay: %f, X: %f, Y: %f, Count: %d\n", dly, x, y, count);
	if (*maxcnt < count) {
		*maxcnt = count;
		*maxdly = dly;
	}

}


int main(int argv, const char** argc)
{
	clock_t start, end;
	double cpu_time_used;

	start = clock();
	verbose = 0;
	minX = -0.55;
	minY = -0.55;
	maxX = 0.55;
	maxY = 0.55;
	xSteps = 0.1;
	ySteps = 0.1;
	delayMin = 0;
	delayMax = 100;
	strcpy(fileName, "alldata.csv");
	strcpy(fileName2, "adaptive.csv");

	strcpy(portNo, "COM3");
	macroSteps = 25;
	adapmacroSteps = 5;
	microSteps = 2;
	peakCheck = 10;
	threshold = 100;
	tdcTime = 100;
	int argitr;
	float adaptive = 0;
	float peak;
	unsigned char out = OUTCHAR;
	//FILE *alldata, *adpdata;

	//processing the inputs
	if (argv > 1) {
		if (argv % 2 == 0) {
			usage();
			return 0;
		}

		for (argitr = 1; argitr < argv; argitr++) {

			if (strncmp(argc[argitr], "-xmi", 4) == 0) {
				minX = (float)atof(argc[++argitr]);
			}

			else if (strncmp(argc[argitr], "-ymi", 4) == 0) {
				minY = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-xma", 4) == 0) {
				maxX = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-yma", 4) == 0) {
				maxY = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-xst", 4) == 0) {
				xSteps = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-yst", 4) == 0) {
				ySteps = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-fnm", 4) == 0) {
				strcpy_s(fileName, argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-fn2", 4) == 0) {
				strcpy_s(fileName2, argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-pno", 4) == 0) {
				strcpy_s(portNo, argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-mad", 4) == 0) {
				macroSteps = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-ams", 4) == 0) {
				adapmacroSteps = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-mid", 4) == 0) {
				microSteps = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-pch", 4) == 0) {
				peakCheck = (float)atof(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-tdc", 4) == 0) {
				tdcTime = (int)atoi(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-ver", 4) == 0) {
				verbose = (int)atoi(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-ths", 4) == 0) {
				threshold = (int)atoi(argc[++argitr]);
			}
			else if (strncmp(argc[argitr], "-dmi", 4) == 0) {
				delayMin = (float)atof(argc[++argitr]);

			}
			else if (strncmp(argc[argitr], "-dma", 4) == 0) {
				delayMax = (float)atof(argc[++argitr]);

			}
			else {
				usage();
				return 0;
			}
		}
	}

	printf("Adaptive scanning\n");
	printf("Xmin: %f \tXmax: %f and will be incremented in %f steps\n", minX, maxX, xSteps);
	printf("Ymin: %f \tYmax: %f and will be incremented in %f steps\n", minY, maxY, ySteps);
	printf("Delay Macro steps: %f \tMicro steps: %f\n", macroSteps, microSteps);
	printf("Tdc integration time: %d\n", tdcTime);
	//printf("Looking for the mirror in the port: %s\n", portNo);
	printf("And the data will be saved to the file: %s\n", fileName);

	connect_dly();
	mti = connect_mems();
	connect_tdc();

	fopen_s(&fineData, fileName, "w+t");
	fprintf(fineData, "delay, y, x, counts\n ");
	fopen_s(&peakData, fileName2, "w+t");
	fprintf(peakData, "delay, y, x, counts\n ");
	delay_last = 0;

	for (float xitr = minX; xitr <= maxX; xitr = xitr + xSteps) 
	{
		for (float yitr = minY; yitr <= maxY; yitr = yitr + ySteps) 
		{
			mti->SendDataStream(&xitr, &yitr, &out, 1);

			LastError = mti->GetLastError();
			if (LastError != MTIError::MTI_SUCCESS) {
				printf("Error settinglocation");

			}

//			peak = getPeak(xitr, yitr);
//			adaptive = getAdaptive(xitr, yitr, roundf(peak - macroSteps / 3), roundf(peak + macroSteps / 3));

			if (adaptive) 
			{
				adaptive = nearScan(xitr, yitr, adaptive - macroSteps, adaptive + macroSteps);
				adaptive = getAdaptive(xitr, yitr, adaptive - macroSteps / 4, adaptive + macroSteps / 4);
			}
			else {
				
				peak = getPeak(xitr, yitr);
				adaptive = getAdaptive(xitr, yitr, roundf(peak - macroSteps / 3), roundf(peak + macroSteps / 3));
			}
		}
		adaptive = 0;
	}
	// End the program by returning the device to origin
	mti->ResetDevicePosition(); // send analog outputs (and device) back to origin in 25ms
	mti->SetDeviceParam(MTIParam::MEMSDriverEnable, false);		// turn off the MEMS driver
	mti->DisconnectDevice();
	mti->DeleteDevice();
	ibonl(dly_ud, 0);

	if (peakData)
		fclose(peakData);
	if (fineData)
		fclose(fineData);

	end = clock();
	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

	printf("the time taken for the scan is %f\n", cpu_time_used);


	return 0;
}
