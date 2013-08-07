
#ifndef _GPSConnection_H
#define	_GPSConnection_H

#include <vector>
#include <string>
#include <boost/thread.hpp> 

#include "libgpsmm.h"

class Control;
class Logger;


class GPSConnection {
public:

	GPSConnection(Control* CarController, Logger* Logger);
    	GPSConnection(const GPSConnection& orig);
    	virtual ~GPSConnection();

	bool Open();
	void Start();
	void Stop();
	
	bool GPSState;

	double Latitude;
	double Longitude;
	double Speed;
	double TrackAngle;

private:

	gpsmm* GPSReceiver;

	Logger* GPSLog;

	boost::thread m_Thread;

	bool Run;

	void ProcessMessages();
	void NewPosition();
	void NewSpeed();
	void NewTrack();

	Control* CarControl;

	Logger* Log;

};

#endif	/* _GPSConnection_H */

