#include "ofapp.h"
#include "color.h"
#include "model.h"
#include "scenes.h"

namespace Software2552 {
	//bugbug maybe add this https://github.com/arturoc/ofxDepthStreamCompression
	Timeline::Timeline(){
	}
	// read one act and save it to the list of acts (ie a story)
	bool Timeline::readScript(const string& path) {
		//return playlist.read(path);
		return true;
	}
	void Timeline::start() {
	}
	void Timeline::resume() {
	}
	void Timeline::pause() {
	}
	void  Timeline::sendClientSigon(shared_ptr<Sender> sender) {
		if (sender) {
			sender->sendOsc(((ofApp*)ofGetAppPtr())->appconfig.getsignon(), SignOnClientOscAddress);
		}
	}

	void Timeline::setup() {
		//ofSeedRandom(); // turn of to debug if needed

		recUDP.setup();
		broadcastUDP.setup();

		// design to run in low mem on slow devices, many items can not be allocated and graphics should still display

		//bugbug use ofxOscMessage ofxOscReceiver (cool way to find server ip for all things, server may need to broad cast this now and then
		// or advertise I guess for new folks that come on line
		stage = std::make_shared<Software2552::Stage>();
		if (!stage) {
			return; //things would be really messed up...
		}

		if (!globalinstance.oscClient) {
			globalinstance.oscClient = std::make_shared<Software2552::ReadOsc>();
			if (globalinstance.oscClient) {
				globalinstance.oscClient->setup();
			}
		}

		if (!globalinstance.router) {
			globalinstance.router = std::make_shared<Software2552::Sender>();
			if (globalinstance.router) {
				globalinstance.router->setup();
				globalinstance.router->addTCPServer(TCP, false); // general server
			}
		}

#ifdef _WIN64
		if (((ofApp*)ofGetAppPtr())->appconfig.getseekKinect() && !globalinstance.kinectDevice) {
			globalinstance.kinectDevice = std::make_shared<KinectDevice>();
			if (globalinstance.kinectDevice) {
				// build out a full kinect
				globalinstance.router->setupKinect();
				if (globalinstance.kinectDevice->setup(globalinstance.router, stage, 25)) { // give after a while
					globalinstance.kinectBody = std::make_shared<Software2552::KinectBody>(globalinstance.kinectDevice);
					if (globalinstance.kinectBody) {
						globalinstance.kinectBody->useFaces(std::make_shared<Software2552::KinectFaces>(globalinstance.kinectDevice));
						globalinstance.kinectBody->useAudio(std::make_shared<Software2552::KinectAudio>(globalinstance.kinectDevice));
					}
					shared_ptr<ofxOscMessage> signon = std::make_shared<ofxOscMessage>();
					if (signon) {
						signon->addStringArg(globalinstance.kinectDevice->getId());
						globalinstance.router->sendOsc(signon, SignOnKinectServerOscAddress);
					}
				}
			}
		}
#endif
		if (((ofApp*)ofGetAppPtr())->appconfig.getseekSound()) {
			addMusic("keep.mp3"); // add sample
		}
		if (((ofApp*)ofGetAppPtr())->appconfig.getSoundGenerator()) {
			((ofApp*)ofGetAppPtr())->appconfig.soundout = std::make_shared<SoundOut>();
			if (((ofApp*)ofGetAppPtr())->appconfig.soundout) {
				((ofApp*)ofGetAppPtr())->appconfig.soundout	->setup();
			}
		}

		if (((ofApp*)ofGetAppPtr())->appconfig.getseekArduino() && !globalinstance.arduino) {
			globalinstance.arduino = std::make_shared<ofArduino>();
			if (globalinstance.arduino) {
				globalinstance.arduino->sendFirmwareVersionRequest();
				// replace the string below with the serial port for your Arduino board
				// you can get this from the Arduino application or via command line
				// for OSX, in your terminal type "ls /dev/tty.*" to get a list of serial devices
				globalinstance.arduino->connect("COM3", 57600);

				// listen for EInitialized notification. this indicates that
				// the arduino is ready to receive commands and it is safe to
				// call setupArduino()
				ofAddListener(globalinstance.arduino->EInitialized, this, &Timeline::setupArduino);
				bSetupArduino = false;	// flag so we setup arduino when its ready, you don't need to touch this :)
			}
		}

		ofSetVerticalSync(false);
		((ofApp*)ofGetAppPtr())->appconfig.setFrameRateinOF();
		colorlist.setup();
		
		ofxJSON data;
		for (const auto& file : ((ofApp*)ofGetAppPtr())->appconfig.jsonFile) {
			//bugbug write a better iterator etc so we can loop through jsons for ever
			shared_ptr<ofxJSON> data = make_shared<ofxJSON>();
			if (stage && data) {
				if (data->open(file)) {
					stage->updateData(data);
				}
			}

		}
		return;

	}
	void Timeline::setupArduino(const int & version) {

		if (!globalinstance.arduino) {
			return;
		}

		// remove listener because we don't need it anymore
		ofRemoveListener(globalinstance.arduino->EInitialized, this, &Timeline::setupArduino);

		// it is now safe to send commands to the Arduino
		bSetupArduino = true;

		// print firmware name and version to the console
		ofLogNotice() << globalinstance.arduino->getFirmwareName();
		ofLogNotice() << "firmata v" << globalinstance.arduino->getMajorFirmwareVersion() << "." << globalinstance.arduino->getMinorFirmwareVersion();

		// Note: pins A0 - A5 can be used as digital input and output.
		// Refer to them as pins 14 - 19 if using StandardFirmata from Arduino 1.0.
		// If using Arduino 0022 or older, then use 16 - 21.
		// Firmata pin numbering changed in version 2.3 (which is included in Arduino 1.0)

		// set pins D2 and A5 to digital input
		globalinstance.arduino->sendDigitalPinMode(2, ARD_INPUT);
		globalinstance.arduino->sendDigitalPinMode(19, ARD_INPUT);  // pin 21 if using StandardFirmata from Arduino 0022 or older

													// set pin A0 to analog input
		globalinstance.arduino->sendAnalogPinReporting(0, ARD_ANALOG);

		// set pin D13 as digital output
		globalinstance.arduino->sendDigitalPinMode(13, ARD_OUTPUT);
		// set pin A4 as digital output
		globalinstance.arduino->sendDigitalPinMode(18, ARD_OUTPUT);  // pin 20 if using StandardFirmata from Arduino 0022 or older

													 // set pin D11 as PWM (analog output)
		globalinstance.arduino->sendDigitalPinMode(11, ARD_PWM);

		// attach a servo to pin D9
		// servo motors can only be attached to pin D3, D5, D6, D9, D10, or D11
		globalinstance.arduino->sendServoAttach(9);

		// Listen for changes on the digital and analog pins
		ofAddListener(globalinstance.arduino->EDigitalPinChanged, this, &Timeline::digitalPinChanged);
		ofAddListener(globalinstance.arduino->EAnalogPinChanged, this, &Timeline::analogPinChanged);
	}

	//--------------------------------------------------------------
	void Timeline::updateArduino() {

		if (globalinstance.arduino) {
			// update the arduino, get any data or messages.
			// the call to ard.update() is required
			globalinstance.arduino->update();

			// do not send anything until the arduino has been set up
			if (bSetupArduino) {
				// fade the led connected to pin D11
				globalinstance.arduino->sendPwm(11, (int)(128 + 128 * sin(ofGetElapsedTimef())));   // pwm...
			}
		}
	}

	// digital pin event handler, called whenever a digital pin value has changed
	// note: if an analog pin has been set as a digital pin, it will be handled
	// by the digitalPinChanged function rather than the analogPinChanged function.

	//--------------------------------------------------------------
	void Timeline::digitalPinChanged(const int & pinNum) {
		// do something with the digital input. 
	}

	// analog pin event handler, called whenever an analog pin value has changed

	//--------------------------------------------------------------
	void Timeline::analogPinChanged(const int & pinNum) {
		// do something with the analog input. 
	}


	// keep this super fast
	void Timeline::update() { 

		
		string message;
		if (recUDP.update(message)) {
			string address;
			int port;
			broadcastUDP.update(message);
			recUDP.reader.GetRemoteAddr(address, port);
			ofLogNotice("Timeline::update()") << message << "from " << address + ":" << port;
		}

		ofSoundUpdate();
		updateArduino();

#ifdef _WIN64
		// kinect can only go 30fps 
		if (globalinstance.kinectBody) {
			globalinstance.kinectBody->update();
		}
#endif
		if (!((ofApp*)ofGetAppPtr())->appconfig.installed && globalinstance.tcpKinectClient) {
			globalinstance.tcpKinectClient->set(stage); // unique stages
			stage->setup(globalinstance.tcpKinectClient);
			globalinstance.tcpBodyIndex->set(stage);
			((ofApp*)ofGetAppPtr())->appconfig.installed = true;
		}
		// router updates itself and builds a queue of input
		// check for sign on/off etc of things
		shared_ptr<ofxOscMessage> signon = globalinstance.oscClient->getMessage(SignOnKinectServerOscAddress);
		// if there is a valid message, if I am not the kinect sending, and I am not already signed in the sign on is requested then ...
		if ((!((ofApp*)ofGetAppPtr())->appconfig.getseekKinect()) && signon && globalinstance.kinectServerIP.empty()) {
			ofLogNotice("Timeline::update()") << " client sign on for kinect ID " << signon->getArgAsString(0);
			globalinstance.kinectServerIP = signon->getRemoteIp();//bugbug only 1 kinect for now, needs to be a vector for > 1 kinect
			globalinstance.tcpKinectClient = std::make_shared<Software2552::TCPKinectClient>(); // frees of any existing
			if (globalinstance.tcpKinectClient) {
				globalinstance.tcpKinectClient->setup(globalinstance.kinectServerIP, TCPKinectBody, true);
			}
			globalinstance.tcpBodyIndex = std::make_shared<Software2552::PixelsClient>(BodyIndexID); // frees of any existing
			if (globalinstance.tcpBodyIndex) {
				globalinstance.tcpBodyIndex->setup(globalinstance.kinectServerIP, TCPKinectBodyIndex, true);
			}
			/* ir takes too much from our little Kinect server, maybe the server needs to be high powered, like this dev box (or the one I gave away :() 
			tcpIRIndex = std::make_shared<Software2552::PixelsClient>(); // frees of any existing
			if (tcpIRIndex) {
				tcpIRIndex->set(stage);
				tcpIRIndex->setup(kinectServerIP, TCPKinectIR, true);
				tcpIRIndex->pt.x = getDepthFrameWidth();// *ratioDepthToScreenX();
			}
			*/
		}
		if (stage) {
			stage->update();
		}
		// let new people know where are here once time per minute (every window will call this)
		uint64_t frame = ofGetFrameNum();
		uint64_t rate = ((ofApp*)ofGetAppPtr())->appconfig.getFramerate() * 60;
		if ((frame % rate ) == 0 && globalinstance.router) {
			sendClientSigon(globalinstance.router);
		}
		// see if someone else checked in, every window is a client (but we do not care really, we just care that its a client)
		// its a bit bizzare in that we may talk to ourself via osc but thats ok on an internal network with out much traffic usign osc
		// and these sign on items are not time sensative
		signon = globalinstance.oscClient->getMessage(SignOnClientOscAddress);
		if (signon) {
			// add or update client bubug we do not use this data yet
			//MachineConfiguration
			string name;
			AppConfiguration::getName(name, signon);
			name += ofToString(AppConfiguration::getWindowNumber(signon));
			if (name != ((ofApp*)ofGetAppPtr())->appconfig.getName()+ ofToString(((ofApp*)ofGetAppPtr())->appconfig.getWindowNumber())) { // skip our own signon messages
				std::vector <shared_ptr<AppConfiguration>>::iterator it = std::find_if(others.begin(), others.end(), [&name](shared_ptr<AppConfiguration>p) {
					return p->getName() == name;
				});
				if (it == others.end()) {
					shared_ptr<AppConfiguration> p = std::make_shared<AppConfiguration>(signon);
					if (p) {
						others.push_back(p);
					}
				}
				else {
					*it = std::make_shared<AppConfiguration>(signon); //bugbug does this really work? It should
				}
			}
		}
		return;
		playlist.removeExpiredItems();
		if (playlist.getCurrent() != nullptr) {
			playlist.getCurrent()->getStage()->update();
		}
		else {
			ofExit();
		}
		colorlist.update();
	};

	// keep as fast as possible
	void Timeline::draw() {
		if (stage) {
			stage->draw();
		}
		//sound source https://www.freesound.org/search/?q=frequency&f=&s=score+desc&advanced=0&g=1
		//mesh.draw();
	};

}