#pragma once
namespace Software2552 {
	// kinect stuff 
#ifndef _WIN64
#ifndef _JointType_
#define _JointType_
	typedef enum _JointType JointType;


	enum _JointType
	{
		JointType_SpineBase = 0,
		JointType_SpineMid = 1,
		JointType_Neck = 2,
		JointType_Head = 3,
		JointType_ShoulderLeft = 4,
		JointType_ElbowLeft = 5,
		JointType_WristLeft = 6,
		JointType_HandLeft = 7,
		JointType_ShoulderRight = 8,
		JointType_ElbowRight = 9,
		JointType_WristRight = 10,
		JointType_HandRight = 11,
		JointType_HipLeft = 12,
		JointType_KneeLeft = 13,
		JointType_AnkleLeft = 14,
		JointType_FootLeft = 15,
		JointType_HipRight = 16,
		JointType_KneeRight = 17,
		JointType_AnkleRight = 18,
		JointType_FootRight = 19,
		JointType_SpineShoulder = 20,
		JointType_HandTipLeft = 21,
		JointType_ThumbLeft = 22,
		JointType_HandTipRight = 23,
		JointType_ThumbRight = 24,
		JointType_Count = (JointType_ThumbRight + 1)
	};
#endif // _JointType_
#endif

	static float getDepthFrameWidth() { return 512.0f; }
	static float getDepthFrameHeight() { return 424.0f; }
	static float getIRFrameWidth() { return 512.0f; }
	static float getIRFrameHeight() { return 424.0f; }
	static float getColorFrameWidth() { return 1920.0f; }
	static float getColorFrameHeight() { return 1080.0f; }

	static float ratioColorToScreenX() { return (float)ofGetWindowWidth() / getColorFrameWidth(); }
	static float ratioColorToScreenY() { return (float)ofGetWindowHeight() / getColorFrameHeight(); }

	static float ratioDepthToScreenX() { return (float)ofGetWindowWidth() / getDepthFrameWidth(); }
	static float ratioDepthToScreenY() { return (float)ofGetWindowHeight() / getDepthFrameHeight(); }

	static float ratioColorToDepthX() { return (float)getDepthFrameWidth() / getColorFrameWidth(); }
	static float ratioColorToDepthY() { return (float)getDepthFrameHeight() / getColorFrameHeight(); }

	enum DataType : char {
		TCPID = 't',	//116
		BodyIndexID = 'x',//120
		IrID = 'i',		//105
		BodyID = 'b',	// 98
		JsonID = 'j',	//106
		Pixels = 'p',		//12
		UnknownID = 'k'
	};
}
