#pragma once
#include "ofApp.h"
#include "ofSoundPlayer.h"
// home of custom drawing

namespace Software2552 {

	// simple drawing 

	// internal helpers
	class RandomDots {
	public:

		void draw();
	};

	class Line {
	public:
		ofPoint a;
		ofPoint b;
	};



	// https://github.com/openframeworks/ofBook/blob/master/chapters/lines/chapter.md
	class ComplexLines {
	public:
		void draw();
		//bugbug convert to code vs mouse using random
		void mouseDragged(int x, int y, int button);
	private:
		vector < ofPoint > drawnPoints;
		vector < Line > lines;
	};
	class Line3D {
	public:
		void setup();
		void update();
		void draw();

	private:
		ofNode baseNode;
		ofNode childNode;
		ofNode grandChildNode;
		ofPolyline line;
	};
	class SoundOut {
	public:
		static void setup();
		static void update();
		static void draw();
		static void audioOut(ofSoundBuffer &outBuffer);

		class dataOut {
		public:
			double wavePhase;
			double pulsePhase;
			double sampleRate;
			mutex audioMutex;
			ofSoundBuffer lastBuffer;
			ofPolyline waveform;
			float rms;
		};

	private:
		static dataOut soundDataOut;
	};

	class SoundIn {
	public:
		static void setup();
		static void update();
		static void audioIn(float * input, int bufferSize, int nChannels);
		class dataIn {
		public:
			vector <float> left;
			vector <float> right;
			vector <float> volHistory;
			int 	bufferCounter;
			int 	drawCounter;
			float smoothedVol;
			float scaledVol;
			ofSoundStream soundStream;
		};

	private:
		static dataIn soundDataIn;
	};
	class Graphics2552 {
	public:
		static void rotateToNormal(ofVec3f normal);

	};


	class CrazyMesh : public ofMesh {
	public:
		CrazyMesh(const ofFloatColor&colorIn = ofFloatColor::orange) : ofMesh() { color = colorIn; }
		virtual void setup();
		virtual void update();
		virtual void draw();
	private:
		ofFloatColor color;
		int w = 200;
		int h = 200;
	};

	// basic mesh stuff for learning
	class MeshEngine : public ofMesh {
	public:
		void setup();
	private:
	};

	//http://openframeworks.cc/ofBook/chapters/generativemesh.html
	class MoreMesh : public MeshEngine {
	public:
		void setup();
		void draw();
	private:
		ofImage		image;
		ofEasyCam	easyCam;
		ofLight		light;

	};





}