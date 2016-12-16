#pragma once

// ties to to things together

namespace Software2552 {

	//bugbug timers is windows only, when porting maybe add android_ndk_perf and/or Linux Perf, or maybe PAPI or find something, just need a quick test
	class Timer {
	public:
		double PCFreq;
		unsigned long long CounterStart;
		LARGE_INTEGER ticksPerSecond;
		LARGE_INTEGER tick;
		LARGE_INTEGER timeStart;

		void StartTimer() {
			if (!QueryPerformanceFrequency(&ticksPerSecond)) {
				ofLogError("Timer") << "QueryPerformanceFrequency failed";
			}

			PCFreq = double(ticksPerSecond.QuadPart) / 1000.0; // milli seconds

			QueryPerformanceCounter(&tick);
			CounterStart = tick.QuadPart;
		}

		double Timer::GetElapsedTime()
		{
			LARGE_INTEGER tick2;
			QueryPerformanceCounter(&tick2);
			return double(tick2.QuadPart - CounterStart) / PCFreq;
		}

	};
	DataType mapPortToType(OurPorts ports);

	// can be used even w/o kinect installed
	class Sender : public Server {
	public:
#ifdef _WIN64
		void setupKinect();
#endif
		bool kinectIREnabled();
		bool KinectBodyIndexEndabled();
		bool KinectBodyEnabled();
	};

	class StagedClient  {
	public:
		StagedClient(DataType idIn = UnknownID) { id = idIn; }
		void set(shared_ptr<Stage> stage) { backStagePass.push_back(stage); }
	protected:
		vector<shared_ptr<Stage>> backStagePass;
		DataType id;// optional bugbug DataType not supported any more, remove it when ready

	};
	// read known size ofPixels
	class PixelsClient : public TCPPixels, public StagedClient {
	public:
		PixelsClient(DataType id) :StagedClient(id) {}
		virtual void myUpdate(shared_ptr<ofPixels> pixels);
		ofPoint pt; // optional starting point for drawing
	};
	
	// easily add in more types here such as...

	class TCPKinectClient : public TCPClient, public StagedClient {
	public:
		virtual void myUpdate(shared_ptr<ReadTCPPacket> packet);// let dervied class do their thing, q not used

	};

}