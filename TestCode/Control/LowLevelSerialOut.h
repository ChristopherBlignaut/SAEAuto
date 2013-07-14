
#ifndef _LowLevelSerialOut_H
#define	_LowLevelSerialOut_H

#define BAUDRATE 9600
#define DEVICE "/dev/ttyUSB0"

#include <vector>
#include <string>
#include <boost/thread.hpp> 

#include "AsyncSerial.h"


class Control;
class Logger;

class LowLevelSerialOut {
public:

	LowLevelSerialOut(Control* CarController, Logger* Logger);
    	LowLevelSerialOut(const LowLevelSerialOut& orig);
    	virtual ~LowLevelSerialOut();

	bool Open();
	void Start();
	void Stop();
	bool Send(std::string Command);
	
	bool SerialState;

private:

	std::vector<char> RxBuffer;

	CallbackAsyncSerial* Serial;

	boost::thread m_Thread;

	bool Run;

	void Receive(const char *data, unsigned int len);

	void ProcessMessage();

	void SendCurrent();

	Control* CarControl;

	Logger* Log;

};

#endif	/* _LowLevelSerialOut_H */

