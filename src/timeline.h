#pragma once
#include "ofApp.h"
#include "control.h"

#ifdef _WIN64
// kinect only works on64 bit windows
#include "kinect2552.h"
#endif

// join data and view, this class knows about both the data and how its shown

namespace Software2552 {
	class Timeline {
	public:
		Timeline();
		void start();
		void setup();
		void update();
		void draw();
		void pause();
		void resume();

		bool readScript(const string& path);

	private:
		void sendClientSigon(shared_ptr<Sender>);
		//bugbug likely to go into ofApp.cpp once things settle down and timeline just goes away
		float x =0;
		float y = 0;
		float x2 = 0;
		float y2 = 0;
		float x3 = 0;
		float y3 = 0;
		float z3 = 0;
		ChannelList playlist;
		ColorList colorlist;
		shared_ptr<Stage> stage = nullptr; // each window is a stage with its own graphics list
		vector <shared_ptr<AppConfiguration>> others; // the config of all other machines on our network
		UDPReceive recUDP;
		UDPBroadcast broadcastUDP;
		// Arduino support
		bool bSetupArduino;
		void setupArduino(const int & version);
		void digitalPinChanged(const int & pinNum);
		void analogPinChanged(const int & pinNum);
		void updateArduino();

	};


}