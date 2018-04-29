#ifndef _PAYLOAD_PARSER_H_
#define _PAYLOAD_PARSER_H_

#include <vector>
#include "amste_msg.h"

class cpuStatTest
{
    public:
        cpuStatTest();
        ~cpuStatTest();
    
        void parseStatus(std::vector<AmsteMsg*>& parsedPayload);
  
        void getStatus(std::vector<AmsteMsg*>& status);
};
#endif
