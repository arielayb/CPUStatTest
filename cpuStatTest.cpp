#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#ifdef _WIN32
    #include <Windows.h>
	#include <tchar.h>
	#include <pdh.h>
	#include <pdhmsg.h>
	#pragma comment(lib, "pdh.lib")
#endif

#include "cpuStatTest.h"

cpuStatTest::cpuStatTest()
{}

cpuStatTest::~cpuStatTest()
{}

void cpuStatTest::parseStatus(std::vector<AmsteMsg*>& parsedPayload)
{
    //A.A: Windows implementation...
#ifdef _WIN32
    
	//A.A: Get the CPU usage from windows
	static PDH_HQUERY cpuQuery;
	static PDH_HCOUNTER cpuTotal;
	
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, _T("\\Processor(_Total)\\% Processor Time"), NULL, &cpuTotal);
    	PdhCollectQueryData(cpuQuery);
	
	PDH_FMT_COUNTERVALUE counterVal;

    	PdhCollectQueryData(cpuQuery);
    	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	std::cout << "CPU usage: " << counterVal.doubleValue << std::endl;
	parsedPayload.push_back(new AmsteMsg("CPU", std::to_string(counterVal.doubleValue));
	
    	//A.A: Fetch the memory info status from the Windows OS.
    	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	
	//A.A: Get Total RAM
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    	std::cout << "Total RAM: " << totalPhysMem << std::endl;
	parsedPayload.push_back(new AmsteMsg("RAMTOTAL", std::to_string(totalPhysMem));
							
    	//A.A: Get the used amount of RAM
 	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    	std::cout << "Available RAM: " << physMemUsed << std::endl;
	parsedPayload.push_back(new AmsteMsg("RAMUSED", std::to_string(physMemUsed));
							
	//A.A: Free amount of RAM available (to be continued)
   	DWORDLONG freePhysMem = memInfo.ullAvailPhys;
    	std::cout << "Total RAM: " << freePhysMem << std::endl;
	parsedPayload.push_back(new AmsteMsg("RAMFREE", std::to_string(freePhysMem));
	
    //A.A: Linux implementation...
#else
    

    //FILE* fp = std::fopen("/proc/loadavg", "r");
    std::ifstream cpuUseFile, memInfoFile;
    
    //std::string memFile = "/proc/meminfo";
    cpuUseFile.open("/proc/loadavg");
    memInfoFile.open("/proc/meminfo");
    
    //A.A: temp vector to store pieces of string data from the file
    std::vector<std::string> tempData;
	
    //A.A:I need to store these calculated values in KV.
    if((cpuUseFile.good() && memInfoFile.good()))
    {
        char cpuUseTarget = ' ';
        std::string cpuUsageOutput, memInfoOutput;

        std::getline(cpuUseFile, cpuUsageOutput, cpuUseTarget);
        const char * payloadMsg = cpuUsageOutput.c_str();
        
        parsedPayload.push_back(new AmsteMsg("CPU", payloadMsg));
	cpuUseFile.close();
		
		//A.A: the struct will obtain needed system information using sys/sysinfo.h
//		struct sysinfo memInfo;
//		sysinfo (&memInfo);
//		
//		//A.A: fetch the total amount of RAM
//		unsigned long totalPhysMem = memInfo.totalram;
//		
//		//Multiply in next statement to avoid int overflow on right hand side...
//		totalPhysMem *= memInfo.mem_unit*0.001f;
//		unsigned long totalMemCalculation = totalPhysMem/1024;
//		parsedPayload.push_back(new AmsteMsg("RAMTOTAL", std::to_string(totalMemCalculation)));
//		std::cout << "What is the RAMTOTAL: " << parsedPayload[1]->getVal() << std::endl;
//		
//		//A.A: fetch the used amount of RAM
//		unsigned long physMemUsed = memInfo.totalram - memInfo.freeram;
//		
//		//Multiply in next statement to avoid int overflow on right hand side...
//		physMemUsed *= memInfo.mem_unit*0.001f;
//		unsigned long usedMemCalculation = physMemUsed/1024;
//		parsedPayload.push_back(new AmsteMsg("RAMUSED", std::to_string(usedMemCalculation)));
//		std::cout << "What is the RAMUSED: " << parsedPayload[2]->getVal() << std::endl;
//
//		
//		//A.A: fetch the free amount of RAM
//		unsigned long physMemFree = memInfo.freeram;
//		
//		//Multiply in next statement to avoid int overflow on right hand side...
//		physMemFree *= memInfo.mem_unit*0.001f;
//		unsigned long freeMemCalculation = physMemFree/1024;
//		parsedPayload.push_back(new AmsteMsg("RAMFREE", std::to_string(freeMemCalculation)));
//		std::cout << "What is the RAMFREE: " << parsedPayload[3]->getVal() << std::endl;

		
		//A.A: The temp vector will be used to rip string data from the file
        while(std::getline(memInfoFile, memInfoOutput))
        {
            const char * processor = memInfoOutput.c_str();
            tempData.push_back(processor);
        }
        
        int totalMem = 0;
        std::string memMB;
        int totalMemCalculation = 0;
        std::stringstream ss(tempData[0]);
    	
        if(std::getline(ss, memMB, ':'))
        {
            ss >> memMB;
            totalMem = std::stoi(memMB);
            totalMemCalculation = totalMem / 1024;
			
           	std::string totalRAM = std::to_string(totalMemCalculation);
			parsedPayload.push_back(new AmsteMsg("RAMTOTAL", totalRAM));
        }
               
        int freeMem = 0;
        std::string memFreeMB;
	int freeMemCalculation = 0;
        std::stringstream streamFree(tempData[1]);
    	
        if(std::getline(streamFree, memFreeMB, ':'))
        {
            streamFree >> memFreeMB;
            freeMem = std::stoi(memFreeMB);
            freeMemCalculation = freeMem / 1024;
            
			std::string freeRAM = std::to_string(freeMemCalculation);
			parsedPayload.push_back(new AmsteMsg("RAMFREE", freeRAM));
        }
        
	//A.A: need to fetch the cache memory to calculate the RAM usage
	int cacheMem = 0;
        std::string memCacheMB;
	int chacheMemCalculation = 0;
        std::stringstream streamCache(tempData[4]);
	    
        if(std::getline(streamCache, memCacheMB, ':'))
        {
            streamCache >> memCacheMB;
            cacheMem = std::stoi(memCacheMB);
            chacheMemCalculation = cacheMem / 1024;
            
        }
		
		int usedMem = 0;
		int usedMemCalculation = 0;
		usedMemCalculation = totalMemCalculation - freeMemCalculation - chacheMemCalculation;
            
		std::string usedRAM = std::to_string(usedMemCalculation);
		parsedPayload.push_back(new AmsteMsg("RAMUSED", usedRAM));	

	 	memInfoFile.close();
	 	getStatus(parsedPayload);
    }
    else
    {
        std::perror("Failed to open/find the file");
    }
    
    
    usleep(3000000);

    parseStatus(parsedPayload);
#endif
}

void cpuStatTest::getStatus(std::vector<AmsteMsg*>& status)
{
 	for(int i = 0; i < status.size(); i++)
	{
		std::cout << status[i]->getKey() << " : "<< status[i]->getVal() << std::endl;
	}
}
                         
int main()
{
    std::vector<AmsteMsg*> parsedPayload;
    cpuStatTest parse;
    parse.parseStatus(parsedPayload);
	
    std::vector<AmsteMsg*>::iterator iter = parsedPayload.begin();
    for (iter; iter != parsedPayload.end(); ++iter)
	if (*iter)
		delete *iter;
}
