#include "ofApp.h"

namespace Software2552 {

	void setAnimationValues(ofxAnimatable*p, const Json::Value &data, string& curveName, string& repeatType);
	AnimRepeat getRepeatTypeFromString(const string& repeatType);

	objectLifeTimeManager::objectLifeTimeManager() {
		usageCount = 0;     // number of times this animation was used
		objectlifetime = 0; // 0=forever, how long object lives after it starts drawing
		expired = false;    // object is expired
		startTime = 0;
		waitTime = 0;
		refreshRate = 0;
	}

	void objectLifeTimeManager::start() {
		startTime = ofGetElapsedTimef();
	}

	bool objectLifeTimeManager::OKToRemove(shared_ptr<objectLifeTimeManager> me) {
		if (me == nullptr) {
			return false;
		}
		if (me->isExpired()) {
			return true;
		}
		// duration == 0 means never go away, and start == 0 means we have not started yet
		if (me->getObjectLifetime() == 0 || me->startTime == 0) {
			return false; // no time out ever, or we have not started yet
		}
		float t = ofGetElapsedTimef();
		float elapsed = ofGetElapsedTimef() - me->startTime;
		if (me->getWait() > elapsed) {
			return false;
		}
		if (me->getObjectLifetime() > elapsed) {
			return false;
		}
		return true;

	}
	// return true if a refresh was done
	bool objectLifeTimeManager::refreshAnimation() {
		if (startTime == 0) {
			start();
			return false;
		}
		float t = ofGetElapsedTimef() - startTime;

		if (t < getWait()) {
			return false;// waiting to start
		}
		//wait = 0; // skip all future usage of wait once we start
				  // check for expired flag 
		if (getObjectLifetime() > 0 && t > getObjectLifetime()) {
			expired = true; // duration of 0 means no expiration
			return false;
		}
		// at this point we can start the time over w/o a wait
		if (t > getRefreshRate()) {
			startTime = ofGetElapsedTimeMillis();
			return true;
		}
		return false;
	}

	void ActorRole::setActorPosition(ofPoint& p) {
		if (locationAnimation) {
			locationAnimation->setPosition(p);
		}
		else {
			defaultStart = p;
		}
	}

	ofPoint ActorRole::getCurrentPosition() {
		if (locationAnimation) {
			return locationAnimation->getCurrentPosition();
		}
		return defaultStart;// 0,0,0 by default bugbug set on of object vs this saved one
	}
	bool Rotation::setup(const Json::Value &data) {
		Json::Value::Members m = data.getMemberNames();// just to help w/ debugging
		x.setup(data["x"]);
		y.setup(data["y"]);
		z.setup(data["z"]);
		return true;
	}
	FloatAnimation::FloatAnimation(float fromIn, float toIn) :ofxAnimatableFloat(), objectLifeTimeManager() {
		from = fromIn;
		to = toIn;
	}

	// need "scale"{}, animation etc wrapers in json
	bool FloatAnimation::setup(const Json::Value &data) {
		if (data.size() > 0) {
			Json::Value::Members m = data.getMemberNames();// just to help w/ debugging
			objectLifeTimeManager::setup(data);

			READFLOAT(from, data);
			READFLOAT(to, data);
			animateFromTo(from, to); // starts animation right now, will turn off pause, so call be fore SetAnimationValues

			setAnimationValues(this, data, string("LINEAR"), string("LOOP_BACK_AND_FORTH"));

			enabled = true;

		}

		return true;
	}
	// helper
	AnimRepeat getRepeatTypeFromString(const string& repeatType) {
		if (repeatType == "LOOP") {
			return LOOP;
		}
		if (repeatType == "PLAY_ONCE") {
			return PLAY_ONCE;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH") {
			return LOOP_BACK_AND_FORTH;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_ONCE") {
			return LOOP_BACK_AND_FORTH_ONCE;
		}
		else if (repeatType == "PLAY_N_TIMES") {
			return PLAY_N_TIMES;
		}
		else if (repeatType == "LOOP_BACK_AND_FORTH_N_TIMES") {
			return LOOP_BACK_AND_FORTH_N_TIMES;
		}
		return LOOP; // default
	}
	void setAnimationValues(ofxAnimatable*p, const Json::Value &data, string& curveName, string& repeatType) {
		if (p) {
			READSTRING(curveName, data);
			p->setCurve(ofxAnimatable::getCurveFromName(curveName));

			int repeat = 0;
			READINT(repeat, data);
			p->setRepeatTimes(repeat);

			READSTRING(repeatType, data);
			p->setRepeatType(getRepeatTypeFromString(repeatType));

			float duration = 0.55f;
			READFLOAT(duration, data);
			p->setDuration(duration);

		}
	}
	
	bool objectLifeTimeManager::setup(const Json::Value &data) {
		usageCount = 0;     // number of times this animation was used
		objectlifetime = 0; // 0=forever, how long object lives after it starts drawing
		expired = false;    // object is expired
		startTime = 0;
		waitTime = 0;
		refreshRate = 0;

		READFLOAT(waitTime, data);
		READFLOAT(objectlifetime, data);
		READFLOAT(refreshRate, data);
		
		return true;
	}

		// only read in one time, to make things more dynamic change at run time
	bool PointAnimation::setup(const Json::Value &data) {
		if (data.size() > 0) {
			objectLifeTimeManager::setup(data);

			Point3D point0; // defaults to 0,0,0
			point0.setup(data["from"]);
			setPosition(point0);

			Point3D pointEnd; // defaults to 0,0,0
			pointEnd.setup(data["to"]);
			animateTo(pointEnd); // starts animation will pause if needed setAnimationValues

			setAnimationValues(this, data, string("EASE_IN"), string("LOOP_BACK_AND_FORTH"));
		}
		return true;
	}
	void FloatAnimation::update() {
		float dt = 1.0f / 60.0f;
		ofxAnimatableFloat::update(dt);
	}

	void PointAnimation::update() {
		float dt = 1.0f / 60.0f;
		ofxAnimatableOfPoint::update(dt);
	}
	// try to keep wrappers out of site to avoid clutter
	// we want to run w/o crashing in very low memory so we need to check all our pointers, we can chug along
	// until memory frees up
	
	bool ActorRole::isAnimating() { return (locationAnimation)?locationAnimation->isAnimating() : false; }
	void ActorRole::animateTo(const ofPoint& p) { if (locationAnimation)locationAnimation->animateTo(p); }
	bool ActorRole::refreshAnimation() { return (locationAnimation) ? locationAnimation->refreshAnimation() : false; }
	float ActorRole::getTimeBeforeStart(float t) { return (locationAnimation) ? locationAnimation->getWait() : 0; }
	void ActorRole::pause(){ if (locationAnimation)locationAnimation->pause();}
	void ActorRole::resume() { if (locationAnimation)locationAnimation->pause(); }
	float ActorRole::getObjectLifetime() { return (locationAnimation) ? locationAnimation->getObjectLifetime() : 0; }
	void ActorRole::setRefreshRate(uint64_t rateIn) { if (locationAnimation)locationAnimation->setRefreshRate(rateIn); }
	float ActorRole::getWait() { return (locationAnimation) ? locationAnimation->getWait() : 0; }

	void AnimiatedColor::setAlpha(float val) {
		setAlphaOnly(val);
	}
	bool ActorRole::setup(const Json::Value &data) {
		fill = true; // set default
		drawOrder = 0;
		references = nullptr;
		locationAnimation = nullptr;
		scaleAnimation = nullptr;
		rotationAnimation = nullptr;
		repeating = false;
		Json::Value::Members m = data.getMemberNames();// just to help w/ debugging

		if (data.size()) {
			READSTRING(name, data);
			READSTRING(title, data);
			READSTRING(notes, data);
			READSTRING(locationPath, data);
			READBOOL(repeating, data);
			setupRepeatingItem(data);
			READBOOL(showSoundGraph, data);
			READBOOL(fill, data);
			READINT(drawOrder, data);
			Point3D point0; // defaults to 0,0,0
			if (point0.setup(data["position"])) {
				setActorPosition(point0); // set default position before any of the items below which may move it again
			}
			string s = getLocationPath();//just for debug
			// any actor can have a reference
			references = parseList<Reference>(data["references"]);
			locationAnimation = parseNoList<PointAnimation>(data["animation"]);
			scaleAnimation = parseNoList<ScaleAnimation>(data["scale"]);
			rotationAnimation = parseNoList<Rotation>(data["rotation"]);
		}

		colorHelper = parseColor(data); // always some kind of helper present

		// let helper objects deal with empty data in their own way

		// actors can have a lot of attributes, but if not no memory is used
		font.setup(data);

		// read derived class data
		return mysetup(data);
	}
	// return current color, track its usage count
	shared_ptr<ColorSet> ColorList::getCurrentColor() {
		if (privateData) {
			if (privateData->currentColorSet) {
				++(*(privateData->currentColorSet)); // mark usage if data has been set
				return privateData->currentColorSet;
			}
		}
		// should never happen ... but return a reasonble value if it does
		return nullptr;
	}

	void ActorRole::setFullSize() {
		fullsize = true;
		pause(); // make sure animation is off
				 //bugbug get this somewhere setSpeed(0.25f);
	}


	void ActorRole::updateForDrawing() {

		update();

		if (colorHelper) {
			colorHelper->update();
		}
		if (locationAnimation) {
			locationAnimation->update();
		}
		if (scaleAnimation) {
			scaleAnimation->update();
		}
		if (rotationAnimation) {
			rotationAnimation->update();
		}
		myUpdate(); // call derived classes
	};
	void Rotation::update() {
		x.update();
		y.update();
		z.update();
	}
	void ActorRole::applyColor() {
		if (colorHelper) {
			colorHelper->draw(); // always returns a pointer
		}
	}
	string &ActorRole::getLocationPath() {
		return locationPath;
	}
	void ActorRole::rotate() {

		if (rotationAnimation) {
			if (rotationAnimation->x.enabled)
			{
				ofRotateX(rotationAnimation->x.val());  // <- rotate the circle around the x axis by some amount.   
			}
			if (rotationAnimation->y.enabled)
			{
				ofRotateY(rotationAnimation->y.val());  // <- rotate the circle around the x axis by some amount.   
			}
			if (rotationAnimation->z.enabled)
			{
				ofRotateZ(rotationAnimation->z.val());  // <- rotate the circle around the z axis by some amount.   
			}
		}
	}
	float ActorRole::scale() {
		if (scaleAnimation) {
			return scaleAnimation->getCurrentValue();
		}
		return 1.0f;
	}
	// derived class needs to fill this in
	void ActorRole::myDraw() {
	};

	bool ActorRole::drawIt(drawtype type) {
		if (okToDraw(type)) {
			if (useFill()) {
				ofFill();
			}
			else {
				ofNoFill();
			}
			bool disableEAP = false;
			if (colorHelper && colorHelper->alphaEnbled()) {
				disableEAP = true;
				ofEnableAlphaBlending(); // only use when needed for performance
			}
			if (getType() == draw2d) {
				applyColor(); // set color here in 2d, 3d color comes from lights etc
				ofPushMatrix();
				if (!getFixed()) {
					ofTranslate(getCurrentPosition());
					rotate(); // fixed does not do rotation here either
				}
				else {
					// should go to home, bugbug can I just go to 0,0 no matter where I am?
					ofTranslate(-ofGetWidth() / 2, -ofGetHeight() / 2);// assume fixed uses screen as it sees fit
				}
				if (!frames.isInfinite()) {
					frames.decrementFrameCount();
				}
				myDraw();
				ofPopMatrix();
			}
			else {
				// 3d does movement differently
				ofPushMatrix();
				if (!frames.isInfinite()) {
					frames.decrementFrameCount();
				}
				myDraw();
				ofPopMatrix();
			}
			if (disableEAP) {
				ofDisableAlphaBlending(); 
			}
			if (showSoundGraph) {
				ofPushMatrix();
				ofNoFill();
				drawSoundGraph();
				ofPopMatrix();
			}
			return true;
		}
		return false;
	};
	bool ActorRole::OKToRemove(shared_ptr<ActorRole> me) {
		if (me) {
			if (me->repeating) {
				return false; // never delete
			}
			bool b = me->frames.getFrameCountMaxHit() || objectLifeTimeManager::OKToRemove(me->locationAnimation);
			if (b) {
				int i = 0;
			}
			return me->frames.getFrameCountMaxHit() || objectLifeTimeManager::OKToRemove(me->locationAnimation);
		}
		return true; // ok to remove nullptr?
	}

	bool ActorRole::okToDraw(drawtype type) {
		drawtype dt = getType();
		if (type != getType() || frames.getFrameCountMaxHit()){
			return false;
		}
		if (repeating) {
			return true;//bugbug need a way to rotate through json
		}
		// bugbug objectLifeTimeManager will handle these also
		if (locationAnimation == nullptr) {
			return true; 
		}
		if (locationAnimation && locationAnimation->isExpired()) {
			return false;
		}
		// if still in wait threshold
		float t = ofGetElapsedTimef() - locationAnimation->getStart();
		if (t < locationAnimation->getWait()) {
			return false; // in wait mode, nothing else to do
		}
		//wait = 0; // skip all future usage of wait once we start
				  // duration 0 means always draw
		if (locationAnimation->getObjectLifetime() == 0) {
			return true;
		}
		if (t < locationAnimation->getObjectLifetime()) {
			return true;
		}
		else {
			float olt = locationAnimation->getObjectLifetime();
			locationAnimation->setExpired(true);
			return false;
		}
	}

	// actor that draws over and over
	void ActorRole::setupRepeatingItem(const Json::Value & data) {
		if (data["frames"].isInt()) {
			frames = FrameCounter(data["frames"].asInt());
		}
		else {
			frames = FrameCounter();
		}
		if (data["repeating"].isBool()) {
			repeating = data["repeating"].asBool();
		}
	}

	void ActorRole::update() {
	}


	// helpers
	ofTrueTypeFont* FontHelper::get() {
		if (font == nullptr) {
			getPointer();
		}
		if (font != nullptr) {
			return &font->ttf;
		}
		return nullptr;
	}

	shared_ptr<ofxSmartFont> FontHelper::getPointer() {
		if (font == nullptr) {
			font = ofxSmartFont::get(defaultFontFile, defaultFontSize);
			if (font == nullptr) {
				// name is not unqiue, just a helper of some kind I guess
				font = ofxSmartFont::add(defaultFontFile, defaultFontSize, defaultFontName);
			}
		}
		if (font == nullptr) {
			ofLogError("FontHelper") << "font is null";
		}
		return font;
	}
	bool FontHelper::setup(const Json::Value &data) {
		if (data.size() > 0 && !data["font"].empty()) {

			string name;
			int size = defaultFontSize;
			string filename;

			readStringFromJson(name, data["font"]["name"]);
			readStringFromJson(filename, data["font"]["file"]);
			readJsonValue(size, data["font"]["size"]);

			// filename required to create a font, else default font is used
			if (filename.size() != 0) {
				font = ofxSmartFont::get(filename, size);
				if (font == nullptr) {
					// name is not unqiue, just a helper of some kind I guess
					font = ofxSmartFont::add(filename, size, name);
				}
				if (font == nullptr) {
					ofLogError("FontHelper") << "font file issue";
					return false;
				}
			}
		}
		return true;
	}
	void AnimiatedColor::update() {
		float dt = 1.0f / 60.0f;//bugbug does this time to frame count? I think so
		ofxAnimatableOfColor::update(dt);
	}
	void AnimiatedColor::draw() {
		if (isAnimating()) {
			applyCurrentColor();
		}
	}
	// all drawing is done using AnimiatedColor, even if no animation is used, color info still stored by doing a set color
	bool AnimiatedColor::setup(const Json::Value &data) {
		if (!data.empty()) {
			Json::Value::Members m = data.getMemberNames();// just to help w/ debugging

			READFLOAT(from, data);
			READFLOAT(to, data);
			if (from != 255) {
				setAlphaOnly(from);
			}
			// anmation requested
			if (from != to && from != 255) {
				animateToAlpha(to); // will not animate color, just alpha
			}

			string color;//hex color
			if (READSTRING(color, data)) {
				setColor(ofColor().fromHex(ofHexToInt(color), from));
			}
			string colorTo;
			if (READSTRING(colorTo, data)) {
				animateTo(ofColor().fromHex(ofHexToInt(colorTo), to));
			}
			setAnimationValues(this, data, string("LINEAR"), string("LOOP_BACK_AND_FORTH"));
		}
		return true;
	}

}