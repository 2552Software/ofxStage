#include "ofApp.h"
#ifdef _WIN64
#include "inc\Kinect.h" // needed for enums 
#endif
#include "scenes.h"

// connect things together

namespace Software2552 {
	DataType mapPortToType(OurPorts port) {
		switch (port) {
		case TCP:
			return TCPID;
		case TCPKinectIR:
			return IrID;
		case TCPKinectBodyIndex:
			return BodyIndexID;
		case TCPKinectBody:
			return BodyID;
		default:
			ofLogError("mapPortToType") << "invalid port " << port;
			return UnknownID;
		}
	}
	bool Sender::kinectIREnabled() {
#ifdef _WIN64
		return enabled(TCPKinectIR);
#else
		return false;
#endif
	}
	bool Sender::KinectBodyIndexEndabled() {
#ifdef _WIN64
		return enabled(TCPKinectBodyIndex);
#else
		return false;
#endif
	}
	bool Sender::KinectBodyEnabled() {
#ifdef _WIN64
		return enabled(TCPKinectBody);
#else
		return false;
#endif
	}
#ifdef _WIN64
	void Sender::setupKinect() {
		addTCPServer(TCPKinectIR, true);
		addTCPServer(TCPKinectBodyIndex, true);
		addTCPServer(TCPKinectBody, true);
	}
#endif
	void PixelsClient::myUpdate(shared_ptr<ofPixels> pixels) {
		if (pixels) {
			// map data to stage
			for (const auto&pass : backStagePass) {
				shared_ptr<PixelsManager>p = std::make_shared<PixelsManager>(id);
				if (p) {
					p->pixels = *pixels; // drawing needs to occur in main thread to make OpenGL work
					p->setActorPosition(pt);
					// add to all stages bugbug until we figure left/right etc and groupings
					pass->addToAnimatable(p);
				}
			}
		}
	}
	void TCPKinectClient::myUpdate(shared_ptr<ReadTCPPacket> packet) {
		if (packet) {
				// add to all stages bugbug until we figure left/right etc and groupings
				for (const auto&pass : backStagePass) {
					shared_ptr<Kinect>k = std::make_shared<Kinect>();
					if (k) {
						ofPoint pt;// start at 0,0
						k->bodyFromTCP(packet->data.c_str(), packet->data.size());
						k->setup();
						pt.x = 0;
						pt.y = getDepthFrameHeight();// *ratioDepthToScreenY();
						k->setActorPosition(pt);
						pass->addToAnimatable(k);
				}
			}
		}
	}
}