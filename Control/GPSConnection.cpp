/* 
 * File:   GPSConnection.cpp
 * Author: T. Drage
 * 
 * A wrapper for the GPSD C++ API.
 *
 * Created on 23/7/13
 */


#include <iostream>
#include <boost/thread.hpp> 
#include <boost/lexical_cast.hpp>
#include <exception>
#include <vector>

#include <string>

#include "libgpsmm.h"

#include "GPSConnection.h"
#include "Logger.h"
#include "Control.h"
#include "Fusion.h"

using namespace std;

/**
 * Purpose: Creates a new instance of the GPSConnection object.
 * Inputs : None.
 * Outputs: None.
 */
GPSConnection::GPSConnection(Control* CarController, Logger* Logger) {

	CarControl = CarController;
 	Log = Logger;	

	GPSState = false;
	Run = false;

}

/**
 * Purpose: Creates a new instance of the GPSConnection object.
 * Inputs : An Network object.
 * Outputs: None.
 */
GPSConnection::GPSConnection(const GPSConnection& orig) {
}

/**
 * Purpose: Destroys the instance of the GPSConnection object.
 * Inputs : None.
 * Outputs: None.
 */
GPSConnection::~GPSConnection() {
}

bool GPSConnection::Open() {

	std::string LogPath = CarControl->LogDir + "/gps.log";

	GPSLog = new Logger(LogPath.c_str());

	GPSReceiver = new gpsmm("localhost", DEFAULT_GPSD_PORT);

    	if (GPSReceiver->stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
    	    	Log->WriteLogLine("GPS - No GPSD running.");
		CarControl->Trip(7);
        	return false;
   	}

	GPSState = true;
	return true;

}

void GPSConnection::Start() {
    
	if(GPSState) {	

		Run = true;

		m_Thread = boost::thread(&GPSConnection::ProcessMessages, this);
		m_Thread.detach();

		s_Thread = boost::thread(&GPSConnection::Monitor, this);
		s_Thread.detach();

	}
        
}



void GPSConnection::ProcessMessages() {

	while(Run) {

		struct gps_data_t* NewData;

		if (! GPSReceiver->waiting(10000)) { continue; }

		if ((NewData = GPSReceiver->read()) == NULL) {
	    		Log->WriteLogLine("GPS - Read error!");
			CarControl->Trip(7);
		} 
		else {
			if (NewData->set & TIME_SET) {
				Time = NewData->fix.time;
			}
			if(NewData->set & TRACK_SET) {
				TrackAngle = NewData->fix.track;
				NewTrack();
			}
			if((NewData->set & LATLON_SET) && (NewData->set & SPEED_SET)) {
				Latitude = NewData->fix.latitude - CarControl->LatOffset;
				Longitude = NewData->fix.longitude - CarControl->LongOffset;
				Speed = NewData->fix.speed;
				NewSpeedAndPosition();
			}
			
		}
    }

}

void GPSConnection::Stop() {

	Run = false;
	GPSLog->CloseLog();
	sleep(1);

}

void GPSConnection::NewSpeedAndPosition() {

	VECTOR_2D Position = CarControl->LatLongToXY(Latitude, Longitude);

	if(CarControl->ExtLogging) {
		GPSLog->WriteLogLine("P," + boost::lexical_cast<std::string>(CarControl->TimeStamp()) + "," + boost::lexical_cast<std::string>(Latitude) + "," + boost::lexical_cast<std::string>(Longitude) + "," + boost::lexical_cast<std::string>(Position.x) + "," + boost::lexical_cast<std::string>(Position.y), true);
		GPSLog->WriteLogLine("S," + boost::lexical_cast<std::string>(CarControl->TimeStamp()) + "," + boost::lexical_cast<std::string>(Speed), true);
	}	

	CarControl->Fuser->GPSUpdate(Position, Speed);

}

void GPSConnection::NewTrack() {

	CarControl->Fuser->GPSTrackAngleUpdate(TrackAngle);

	if(CarControl->ExtLogging) {
		GPSLog->WriteLogLine("T," + boost::lexical_cast<std::string>(CarControl->TimeStamp()) + "," + boost::lexical_cast<std::string>(TrackAngle), true);
	}
}

void GPSConnection::Monitor() {

	while(Run) {

		//Log->WriteLogLine("old " + boost::lexical_cast<std::string>(OldTime) + " new " + boost::lexical_cast<std::string>(Time));

		if(CarControl->AutoRun) {
			if(! (Time > (OldTime + 0.1))) { Log->WriteLogLine("GPS - GPS Fix is old."); CarControl->Trip(7); } 
		}

		OldTime = Time;

		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	}

}


