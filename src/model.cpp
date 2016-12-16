#include "ofApp.h"
#include "color.h"
#ifdef _WIN64
#include "inc\Kinect.h" // needed for enums
#endif
#include "model.h"
#include "scenes.h"
// maps json to drawing and animation tools

namespace Software2552 {
	float scaleIt(float val, float scale) {
		return val*scale;
	}

	// data read in as a % of screen x,y; z is not converted
	void Point3D::convert(float xpercent, float ypercent, float zpercent) {
		x = scaleIt(ofGetWidth(), xpercent);
		y = scaleIt(ofGetWidth(), ypercent);
		z = zpercent;//not sure how to do this yet
	}
	bool Point3D::setup(const Json::Value &data) {
		float x = 0.0f, y = 00.0f, z = 00.0f;
		if (data.size() > 0) {
			READFLOAT(x, data);
			READFLOAT(y, data);
			READFLOAT(z, data);
			convert(x*0.01f, y*0.01f, z*0.01f);
			return true;
		}
		return false;
	}

	vector<shared_ptr<Channel>>& ChannelList::getList() {
		return list;
	}
	// since list is maintained 0 is always current
	shared_ptr<Channel> ChannelList::getCurrent() {
		// find first non skipped channel
		for (const auto& channel : list) {
			if (!channel->getSkip()) {
				return channel;
			}
		}
		return nullptr;
	}
	shared_ptr<Channel> ChannelList::getbyNumber(int i) {
		if (i < list.size() && i > 0) {
			return list[i];
		}
		return nullptr;
	}

	// first channel of type 
	shared_ptr<Channel> ChannelList::getbyType(Channel::ChannelType type, int number) {
		// find first non skipped channel
		int count = 0;
		for (const auto& channel : list) {
			if (!channel->getSkip() && channel->getType() == type) {
				if (count == number) {
					return channel;
				}
				++count;
			}
		}
		return nullptr;
	}
	shared_ptr<Channel> ChannelList::getbyName(const string&name) {
		// find first non skipped channel
		for (const auto& channel : list) {
			if (!channel->getSkip() && channel->getKeyName() == name) {
				return channel;
			}
		}
		return nullptr;
	}

	bool ChannelList::setup(const Json::Value &data) {

		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<Channel> channel = std::make_shared<Channel>();
			channel->setup(data[j]);
			if (!channel->getSkip()) {
				list.push_back(channel);
			}
		}
		return true;
	}

	// get a string from json
	bool readStringFromJson(string &value, const Json::Value& data) {
		if (readJsonValue(value, data)) {
			value = data.asString();
			return true;
		}
		return false;
	}
	template<typename T> bool readJsonValue(T &value, const Json::Value& data) {
		try {
			if (!data.empty()) {
				switch (data.type()) {
				case Json::nullValue:
					break;
				case Json::booleanValue:
					value = data.asBool();
					break;
				case Json::stringValue:
					return true; // templates get confused to readJsonValue the string else where, use this to validate
					break;
				case Json::intValue:
					value = data.asInt();
					break;
				case Json::realValue:
					value = data.asFloat();
					break;
				case Json::objectValue:
					ofLogError("readJsonValue") << "objectValue called at wrong time";
					break;
				case Json::arrayValue:
					ofLogError("readJsonValue") << "arrayValue called at wrong time";
					break;
				default:
					// ignore?
					ofLogError("readJsonValue") << "value not found";
					break;
				}
				return true;
			}
		}
		catch (std::exception e) {
			ofLogError("readJsonValue") << e.what();
		}
		return false;
	}
	DateAndTime::DateAndTime() : Poco::DateTime(0, 1, 1) {
		timeZoneDifferential = 0;
		bc = 0;
	}
	void DateAndTime::operator=(const DateAndTime& rhs) {
		timeZoneDifferential = rhs.timeZoneDifferential;
		bc = rhs.bc;
		assign(rhs.year(), rhs.month(), rhs.day(), rhs.hour(), rhs.minute(), rhs.second(), rhs.microsecond(), rhs.microsecond());
	}
	bool Rectangle::mysetup(const Json::Value &data) {
		width = 0;
		height = 0;
		READINT(height, data);
		READINT(width, data);
		return true;
	}
	void Rectangle::myDraw() {
		ofSetRectMode(OF_RECTMODE_CENTER);	// center around the position
		ofRect(0, 0, width, height);
	}

	bool Ball::mysetup(const Json::Value &data) {
		// can read any of these items from json here
		READFLOAT(radius, data);
		return true;
	}
	bool Channel::setup(const Json::Value &data) {
		READSTRING(keyname, data);
		float lifetime = 0;
		READFLOAT(lifetime, data);
		setObjectLifetime(lifetime);
		READBOOL(skip, data);
		string type = "any";
		readStringFromJson(type, data["channelType"]);
		if (type == "history") {
			setType(History);
		}
		else if (type == "sports") {
			setType(Sports);
		}
		else {
			setType(Any);
		}
		return true;
	}

	bool Dates::setup(const Json::Value &data) {
		timelineDate.setup(data["timelineDate"]); // date item existed
		lastUpdateDate.setup(data["lastUpdateDate"]); // last time object was updated
		itemDate.setup(data["itemDate"]);
		return true;
	}

	bool Reference::setup(const Json::Value &data) {
		Dates::setup(data["dates"]);
		if (Dates::setup(data)) { // ignore reference as an array or w/o data at this point
								  // no base class so it repeats some data in base class ReferencedItem
			READSTRING(location, data[STRINGIFY(Reference)]);
			READSTRING(locationPath, data[STRINGIFY(Reference)]);
			READSTRING(source, data[STRINGIFY(Reference)]);
			return true;
		}
		return false;
	}

	bool DateAndTime::setup(const Json::Value &data) {

		if (READINT(bc, data)) {
			return true;
		}

		string str; // scratch varible, enables either string or ints to pull a date
		if (READSTRING(str, data)) {
			if (!Poco::DateTimeParser::tryParse(str, *this, timeZoneDifferential)) {
				ofLogError("DateAndTime", str) << "invalid AD date";
				return false;
			}
			makeUTC(timeZoneDifferential);
		}

		return true;
	}

	bool Text::mysetup(const Json::Value &data) {
		READSTRING(text, data);
		return true;
	}

	// return true if text read in
	bool Paragraph::mysetup(const Json::Value &data) {

		string text;
		READSTRING(text, data);
		worker.setText(text);

		int indent;
		int leading;
		int spacing;
		string alignment; // paragraph is a data type in this usage

		if (READINT(indent, data)) {
			worker.setIndent(indent);
		}
		if (READINT(leading, data)) {
			worker.setLeading(leading);
		}
		if (READINT(spacing, data)) {
			worker.setSpacing(spacing);
		}
		READSTRING(alignment, data);
		if (alignment == "center") { //bugbug ignore case
			worker.setAlignment(ofxParagraph::ALIGN_CENTER);
		}
		else if (alignment == "right") { //bugbug ignore case
			worker.setAlignment(ofxParagraph::ALIGN_RIGHT);
		}

		worker.setFont(getFontPointer());

		// object holds it own color bugbug maybe just set current color right before draw?
		worker.setColor(colorHelper->getForeground());

		return true;
	}

	void Face::update(const Json::Value &data) {
		points.clear();
		elipses.clear();

		if (data.size() == 0) {
			return;
		}
		//bugbug figure out a way to scale to all screens, like do the ratio of 1000x1000 are the values we use then mult by scale, not sure, but its
		// drawing thing more than a data thing
		Json::Value::Members m = data.getMemberNames();

		if (m.size() < 3) {
			return; // not enough data to matter
		}

		float ratioX = ratioColorToScreenX();
		float ratioY = ratioColorToScreenY();

		// setup upper left
		rectangle.set(data["boundingBox"]["left"].asFloat()*ratioX, data["boundingBox"]["top"].asFloat()*ratioY,
			data["boundingBox"]["right"].asFloat()*ratioX - data["boundingBox"]["left"].asFloat()*ratioX,
			data["boundingBox"]["bottom"].asFloat()*ratioY - data["boundingBox"]["top"].asFloat()*ratioY);

		pitch = data["rotation"]["pitch"].asFloat();
		yaw = data["rotation"]["yaw"].asFloat();
		roll = data["rotation"]["roll"].asFloat(); // bugbug rotate this in 3d

		float x = data["eye"]["left"]["x"].asFloat() * ratioX;
		float y = data["eye"]["left"]["y"].asFloat() * ratioY;

		float radius = (data["eye"]["left"]["closed"].asBool()) ? 15 : 25;
		elipses.push_back(ofVec4f(x, y, 10, 20));

		x = data["eye"]["right"]["x"].asFloat()*ratioX;
		y = data["eye"]["right"]["y"].asFloat()*ratioY;
		radius = (data["face"]["eye"]["right"]["closed"].asBool()) ? 5 : 10;
		elipses.push_back(ofVec4f(x, y, 10, 20));

		x = data["nose"]["x"].asFloat()*ratioX;
		y = data["nose"]["y"].asFloat()*ratioY;
		elipses.push_back(ofVec4f(x, y, 10, 15));

		x = data["mouth"]["left"]["x"].asFloat()*ratioX;
		y = data["mouth"]["left"]["y"].asFloat()*ratioY;
		float x2 = data["mouth"]["right"]["x"].asFloat()*ratioX;
		float y2 = data["mouth"]["right"]["y"].asFloat()*ratioY;

		if (data["happy"].asBool()) {
			elipses.push_back(ofVec4f(x, y, 15, 50));//bugbug for now just one circule, make better mouth later
		}
		else if (data["mouth"]["open"].asBool()) {
			elipses.push_back(ofVec4f(x, y, abs(x - x2), 10));//bugbug for now just one circule, make better mouth later
		}
		else if (data["mouth"]["moved"].asBool()) {
			elipses.push_back(ofVec4f(x, y, 5 + abs(x - x2), ofRandom(15)));//bugbug for now just one circule, make better mouth later
		}
		else {
			points.push_back(ofPoint(x, y, 15));
			points.push_back(ofPoint(x2, y2, 15));
		}

	}
	void KinectItem::setup() {
		setFixed(true);
		setRealTime();
		frames.setFrameCount(ofGetFrameRate()/5); // keep around enough to avoid flicker
	}
	// draw face separte from body
	void Face::myDraw() {
		ofSetColor(ofColor::blue); //bugbug clean changing up to fit in with rest of app, also each user gets a color
		ofPushStyle();
		//ofFill();
		if (rectangle.width != 0) {
			//ofPushMatrix();
			//ofTranslate(rectangle.width / 2, rectangle.height / 2, 0);//move pivot to centre
			//ofRotateZ(yaw); // bugbug figure out rotation
			//ofTranslate(rectangle.width/2, rectangle.height/2);
			//ofRotateX(roll);
			//ofRotateY(pitch);
			//ofPushMatrix();
			//ofTranslate(-rectangle.width / 2, -rectangle.height / 2);
			ofDrawRectRounded(rectangle, 15);
			//ofPopMatrix();
			//ofRotateX(pitch);
			//ofPushMatrix();
			//rectangle.draw(-rectangle.width / 2, -rectangle.height / 2);//move back by the centre offset
			//ofPopMatrix();
			//ofPopMatrix();
		}
		ofSetColor(ofColor::red); //bugbug clean changing up to fit in with rest of app, also each user gets a color
		for (const auto&face : points) {
			if (face.x == face.y && face.x == 0) {
				continue; // noise, bugbug but I am not sure about a real 0,0 yet
			}
			ofDrawCircle(face.x, face.y, face.z); // z is radius
		}
		ofSetColor(ofColor::pink); //bugbug clean changing up to fit in with rest of app, also each user gets a color
		ofDrawCircle(0, 0, 111); // z is radius
		ofSetColor(ofColor::green); //bugbug clean changing up to fit in with rest of app, also each user gets a color
		for (const auto&e : elipses) {
			if (e.x == e.y && e.x == 0) {
				continue; // noise, bugbug but I am not sure about a real 0,0 yet
			}
			ofDrawEllipse(e.x, e.y, e.z, e.w);
		}
		ofPopStyle();

	}
	void Kinect::bodyFromTCP(const char * bytes, const size_t numBytes) {
		ofxJSON data;
		if (!data.parse(bytes)) {
			ofLogError("bodyFromTCP") << "invalid json " << bytes;// lets hope its null terminated
			return;
		}
		update(data);
	}

	// need to add this to our model etc
	void Kinect::myDraw() {

		face.myDraw();

		ofColor color = ofColor::orange;

		ofNoFill();
		for (const auto&circle : points) {
			ofSetColor(color); //bugbug clean changing up to fit in with rest of app
			color.setHue(color.getHue() + 6.0f);
			if (circle.x == circle.y && circle.x == 0) {
				continue; // not valid data
			}
			ofSetLineWidth(2);//bugbug make json attribute
			ofDrawCircle(circle.x, circle.y, circle.z); // z is radius
		}

	}
	void Kinect::setFace(const Json::Value &data) {
		face.update(data);
	}

	void Kinect::setHand(const Json::Value &data, float x, float y, float size) {
		if (data["state"] == "open") {
			points.push_back(ofPoint(x, y, size * 2));
		}
		else if (data["state"] == "lasso") {
			points.push_back(ofPoint(x, y, size));
		}
		else if (data["state"] == "closed") {
			points.push_back(ofPoint(x, y, size / 4));
		}
	}
	void Kinect::update(ofxJSON& data) {
		points.clear();
		float f1 = (float)ofGetScreenHeight();
		float f2 = getDepthFrameHeight();
		float ratioX = ratioDepthToScreenX();
		float ratioY = ratioDepthToScreenY();

		string s = data.getRawString(false); // too large for UDP

		for (Json::ArrayIndex i = 0; i < data["body"].size(); ++i) {
			face.update(data["body"][i]["face"]);
			Json::Value::Members m = data["body"][i].getMemberNames();
			for (Json::ArrayIndex j = 0; j < data["body"][i]["joint"].size(); ++j) {
				float x = data["body"][i]["joint"][j]["depth"]["x"].asFloat() * ratioX; // using depth coords which will not match the face
				float y = data["body"][i]["joint"][j]["depth"]["y"].asFloat() * ratioY;

				if (data["body"][i]["joint"][j]["jointType"] == JointType::JointType_HandRight) {
					setHand(data["body"][i]["joint"][j]["right"], x, y, ratioX * 25);
				}
				else if (data["body"][i]["joint"][j]["jointType"] == JointType::JointType_HandLeft) {
					setHand(data["body"][i]["joint"][j]["left"], x, y, ratioX * 25);
				}
				else if (data["body"][i]["joint"][j]["jointType"] == JointType::JointType_Head) {
					points.push_back(ofPoint(x, y, ratioX * 25));// bugbug add color etc
				}
				else {
					// just the joint gets drawn, its name other than JointType_Head (hand above head)
					// is not super key as we track face/hands separatly 
					points.push_back(ofPoint(x, y, ratioX * 10));// bugbug add color etc
				}
			}
		}
	}
	void PixelsManager::mySetup() {
		//bugbug doing this or just about any other thing draws blck iamage image.allocate(pixels->getWidth(), pixels->getHeight(), OF_IMAGE_COLOR);//bugbug get OF_IMAGE_COLOR from data if this works
		image.getPixelsRef() = pixels;
		setFixed(true); 
		frames.setFrameCount(((ofApp*)ofGetAppPtr())->appconfig.getFramerate());//bugbug how to avoid flash?
	}
	void PixelsManager::myUpdate() {
		image.update();
	}

	void PixelsManager::myDraw() { 
		if (image.getTexture().isAllocated()) {
			// some times texuture does not get allocated 
			ofTranslate(getCurrentPosition());
			ofSetColor(255, 255, 255); // this one is always pure bugbug just while learning
			image.draw(0, 0);//bugbug some times texture is missing
		}
	}


	bool Visual::mysetup(const Json::Value &data) {
		READINT(width, data);
		READINT(height, data);
		setType(ActorRole::draw2d);
		return true;
	}
	//, "carride.mp4"
	bool Video::mysetup(const Json::Value &data) {
		Visual::mysetup(data);
		float speed = 0;
		READFLOAT(speed, data);
		if (speed != 0) {
			worker.setSpeed(speed);
		}
		float volume = 1;//default
		READFLOAT(volume, data);
		worker.setVolume(volume);
		return true;
	}
	bool Audio::mysetup(const Json::Value &data) {
		float volume = 1;//default
		READFLOAT(volume, data);
		worker.setVolume(volume);
		worker.setMultiPlay(true);
		worker.setSpeed(ofRandom(0.8, 1.2));// get from data

		return true;
	}
	void MovingCamera::orbit() {
		float time = ofGetElapsedTimef();
		float longitude = 10 * time;
		float latitude = 10 * sin(time*0.8);
		float radius = 600 + 50 * sin(time*0.4);
		worker.orbit(longitude, latitude, radius, ofPoint(0, 0, 0));
	}

	bool Camera::setup(const Json::Value &data) {
		worker.disableMouseInput();//bugbug note this is here
		return mysetup(data);
	}
	bool MovingCamera::mysetup(const Json::Value &data) {
		return true;
	}
	bool FixedCamera::mysetup(const Json::Value &data) {
		return true;
	}
	void Material::begin() {
		// the light highlight of the material  
		if (colorHelper) {
			setSpecularColor(colorHelper->getForeground());
		}
		else {
			setSpecularColor(ofColor(255, 255, 255, 255)); // harsh, but default only
		}
		ofMaterial::begin();
	}

	bool Material::setup(const Json::Value &data) {
		// shininess is a value between 0 - 128, 128 being the most shiny // 
		float shininess = 90;
		READFLOAT(shininess, data);
		setShininess(shininess);
		colorHelper = parseColor(data);

		return true;
	}
	bool Light::setup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		//could get from json? not sure yet getAnimationHelper()->setPositionX(ofGetWidth()*.2);
		setLoc(ofRandom(-100, 100), 0, ofRandom(400, 600));
		// help http://www.glprogramming.com/red/chapter05.html
		colorHelper = parseColor(data);
		if (colorHelper) {
			worker.setDiffuseColor(colorHelper->getLightest());
			worker.setSpecularColor(colorHelper->getDarkest());
			worker.setAmbientColor(colorHelper->getBackground());
		}
		//bugbug what about alpha?
		return mysetup(data);
	}
	bool PointLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		// specular color, the highlight/shininess color //
		setLoc(ofRandom(ofGetWidth()*.2, ofGetWidth()*.4), ofRandom(ofGetHeight()*.2, ofGetHeight()*.4), ofRandom(500, 700));
		//could get from json? not sure yet getAnimationHelper()->setPositionY(ofGetHeight()*.2);
		return true;
	}
	bool DirectionalLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		worker.setOrientation(ofVec3f(0, 90, 0));
		setLoc(ofGetWidth() / 2, ofGetHeight() / 2, 260);
		return true;
	}
	bool SpotLight::mysetup(const Json::Value &data) {
		//bugbug fill in as an option, use Settings for color, or the defaults
		//get from json player.setDiffuseColor(ofColor(0.f, 255.f, 0.f));
		// specular color, the highlight/shininess color //
		//get from json player.setSpecularColor(ofColor(255.f, 0, 0));
		//could get from json? not sure yet getAnimationHelper()->setPositionX(ofGetWidth()*.2);
		//could get from json? not sure yet getAnimationHelper()->setPositionY(ofGetHeight()*.2);
		//directionalLight->player.setOrientation(ofVec3f(0, 90, 0));
		setLoc(ofGetWidth()*.1, ofGetHeight()*.1, 220);
		// size of the cone of emitted light, angle between light axis and side of cone //
		// angle range between 0 - 90 in degrees //
		worker.setSpotlightCutOff(50);

		// rate of falloff, illumitation decreases as the angle from the cone axis increases //
		// range 0 - 128, zero is even illumination, 128 is max falloff //
		worker.setSpotConcentration(2);
		setLoc(-ofGetWidth()*.1, ofGetHeight()*.1, 100);
		return true;
	}
	void Ball::myDraw() {
		// starting position determined by caller
		ofCircle(0, 0, radius*scale());
	}

	bool Arrow::mysetup(const Json::Value &data) {
		end.x = ofGetWidth() / 2;
		end.z = 600;
		if (data.size() > 0) {
			start.setup(data);
			end.setup(data);
			READFLOAT(headSize, data);
		}
		return true;
	}

	void Text::myDraw() {
		//position set by caller
		drawText(text, 0, 0);
	}

	void Text::drawText(const string &s, int x, int y) {
		FontHelper font;
		font.get()->setLineHeight(444);
		font.get()->drawString(s, x, y);
	}

	bool Plane::derivedMysetup(const Json::Value &data) {
		return true;
	}
	DrawingPrimitive3d::~DrawingPrimitive3d() {
		if (get()) {
			delete get();
		}
	}

	DrawingPrimitive3d::DrawingPrimitive3d(of3dPrimitive *p, drawtype type) : ActorRole() {
		node = p;
		setType(type);
		if (get()) {
			get()->enableColors();
		}
	}

	bool DrawingPrimitive3d::mysetup(const Json::Value &data) {
		///ofPolyRenderMode renderType = OF_MESH_WIREFRAME; //bugbug enable phase II
		bool wireFrame = true;
		READBOOL(wireFrame, data);
		setWireframe(wireFrame);
		// pass on current animation
		material.colorHelper = colorHelper;
		material.setup(data);
		return derivedMysetup(data);
	}
	void DrawingPrimitive3d::myUpdate() {
		if (get()) {
			get()->setPosition(getCurrentPosition());
		}
	}
	// private draw helper
	void DrawingPrimitive3d::basicDraw() {

		if (get()) {
			get()->setScale(scale());
			ofPoint scale = get()->getScale();
			ofPoint save = scale;
			if (useWireframe()) {
				//get()->setScale(scale.x + 0.01f, scale.y + 0.01f, scale.z + 0.01f);
				get()->setScale(scale.x + BAND(0), scale.y + BAND(5), scale.z + BAND(15) * 3);
				get()->drawWireframe();
			}
			else {
				scale.z = BAND(15);
				get()->setScale(scale);
				get()->draw();
			}
			get()->setScale(save);
		}
	}
	// assumes push/pop handled by caller
	void DrawingPrimitive3d::myDraw() {
		ofSetColor(0, 0, 0);//color comes from the light
		material.begin();
		if (!useFill()) {
			ofNoFill();
		}
		else {
			ofFill();
		}
		basicDraw();
		material.end();
	}

	bool Cube::derivedMysetup(const Json::Value &data) {
		float size = 100;//default
		READFLOAT(size, data);
		if (get()) {
			setWireframe(true);
			get()->set(size);
			get()->roll(20.0f);// just as an example
		}
		return true;
	}
	bool Cylinder::derivedMysetup(const Json::Value &data) {
		return true;
	}
	bool Cone::derivedMysetup(const Json::Value &data) {
		return true;
	}
	bool Sphere::derivedMysetup(const Json::Value &data) {
		if (get()) {
			float radius = 100;//default
			READFLOAT(radius, data);
			get()->setRadius(radius);

			float resolution = 100;//default
			READFLOAT(resolution, data);
			get()->setResolution(resolution);

			// can be moving too, let json decide, need camera too
			setType(ActorRole::draw3dFixedCamera);
			setFill();
			get()->setMode(OF_PRIMITIVE_TRIANGLES);
		}
		return true;
	}
	bool Background::mysetup(const Json::Value &data) {

		string type;
		readStringFromJson(type, data["colortype"]);
		if (type == "fixed") {
			setType(ColorFixed);
		}
		else if (type == "changing") {
			setType(ColorChanging);
		}
		else {
			setType(none);
		}
		type = "";
		readStringFromJson(type, data["gradient"]);
		if (type == "linear") {
			ofMode = OF_GRADIENT_LINEAR;
			setGradientMode(linear);
		}
		else if (type == "bar") {
			ofMode = OF_GRADIENT_BAR;
			setGradientMode(bar);
		}
		else if (type == "circular") {
			ofMode = OF_GRADIENT_CIRCULAR;
			setGradientMode(circular);
		}
		else if (type == "flat") {
			setGradientMode(flat);
		}
		else if (type == "musicGradient") {
			setGradientMode(musicGradient);
		}
		else {
			setGradientMode(noGradient);
		}

		//bugbug finish this 
		setRefreshRate(60000);// just set something different while in dev
		drawOrder = 100000; // always draw first, if someone really wants to draw behind the background they just need to exced this number which is fine

		if (!data["image"].empty()) {
			add<Image>(data["image"], drawOrder+1);
		}

		if (!data["video"].empty()) {
			add<Video>(data["video"], drawOrder + 2);
		}

		if (!data["rainbow"].empty()) {
			add<Rainbow>(data["rainbow"], drawOrder + 3);
		}
		return true;
	}
	void Visual::myUpdate() {
		if (fullsize) {
			ofPoint pt;// upper left in a centered world
			pt.x = -ofGetWidth() / 2;
			pt.y = -ofGetHeight() / 2;
			width = ofGetWidth();
			height = ofGetHeight();
			setActorPosition(pt);
		}
	}
	void Background::myDraw() {
		if (colorHelper) {
			if (mode == flat) {
				// just a plane background
				ofBackground(colorHelper->getBackground());
			}
			else if(mode == musicGradient) {
				ofColor c1(ofMap(BAND(3), 0, 2, 50, 240), ofMap(BAND(5), 0, 2, 50, 200), ofMap(BAND(23), 0, 2, 50, 240));
				c1.setBrightness(50);
				c1.setSaturation(50);
				ofColor c2 = c1;
				c2.setBrightness(255);
				c2.setSaturation(255);
				ofBackgroundGradient(c2, c1, OF_GRADIENT_CIRCULAR);
			}
			else if (mode != noGradient) {
				ofBackgroundGradient(colorHelper->getLightest(), colorHelper->getDarkest(), ofMode);
			}
			if (type == none) {
				return;
			}
			// set by default since this is set first other usage of fore color will override
			ofSetColor(colorHelper->getForeground());
		}
	}
	bool Rainbow::mysetup(const Json::Value &data) {
		width = ofGetWidth();
		height = ofGetHeight(); // default
		Visual::mysetup(data);
		sizex = 0;
		sizey = 0;// save after first update
		return true;
	}
	void Rainbow::myUpdate() {
		Visual::myUpdate();
		width = ofGetWidth()/4;
		height = ofGetHeight()/4; 

		if (width != sizex && height != sizey) {
			sizex = width;
			sizey = height;

			worker.allocate(width, height, OF_IMAGE_COLOR);

			for (float y = 0; y<height; y++) {
				for (float x = 0; x<width; x++) {

					float hue = x / width * 255;
					float sat = ofMap(y, 0, height / 2, 0, 255, true);
					float bri = ofMap(y, height / 2, height, 255, 0, true);

					worker.setColor(x, y, ofColor::fromHsb(hue, sat, bri));
				}
			}
			worker.update();
		}
	}
	void Rainbow::myDraw() {
		ofSetColor(ofColor::white);
		float f = MAG;
		if (KICK && MAG > 0.3f) {
			degrees += increment;
			if (abs(degrees) > 90) {
				degrees = increment;
				increment = -increment;
			}
			ofRotateY(degrees);
		}
		worker.draw(-ofGetWidth()/8, -ofGetHeight() / 8);
	}
	// colors and background change over time but not at the same time
	void Background::myUpdate() {
		if (type == ColorChanging && refreshAnimation()) {
			if (mode != noGradient) {
				//bugbug test out refreshAnimation
				switch ((int)ofRandom(0, 3)) {
				case 0:
					ofMode = OF_GRADIENT_LINEAR;
					break;
				case 1:
					ofMode = OF_GRADIENT_CIRCULAR;
					break;
				case 2:
					ofMode = OF_GRADIENT_BAR;
					break;
				}
			}
		}

	}
	void Paragraph::myDraw() {
		worker.setPosition(0, 0);
		worker.draw();
	}
	bool ChannelList::skipChannel(const string&keyname) {
		for (auto& item : list) {
			if (item->getKeyName() == keyname) {
				return item->getSkip();
			}
		}
		return true;
	}
	// match the keynames 
	bool ChannelList::setStage(shared_ptr<Stage> p) {
		if (p != nullptr) {
			for (auto& item : list) {
				if (item->getKeyName() == p->getKeyName()) {
					item->setStage(p);
					return true;
				}
			}
		}
		return false;
	}
	bool ChannelList::read(const string&path) {
		ofxJSON json;
		ofxJSON data;
		data["scenes"][0]["code"] = "44";
		data["scenes"][1]["code"] = "88";
		data["scenes"][3]["dog"] = "99";
		string s;
		s = data.getRawString(false);// send this
		json.parse(s);

		readStringFromJson(s, data["United Kingdom"]["code"]);

		if (!json.open(path)) {
			ofLogError("ChannelList::read", path) << "open failed";
			return false;
		}
		try {

			setup(json["channelList"]);
			if (getList().size() == 0) {
				ofLogError("ChannelList::read") << "missing channelList";
				return false;
			}

			// read all the scenes
			for (Json::ArrayIndex i = 0; i < json["scenes"].size(); ++i) {
				string sceneType;
				if (readStringFromJson(sceneType, json["scenes"][i]["sceneType"])) {
					shared_ptr<Stage> p = getScene(sceneType);
					// read common items here
					//p->settings.setup(json["scenes"][i]["settings"]);
					readStringFromJson(p->getKeyName(), json["scenes"][i]["keyname"]);
					if (skipChannel(p->getKeyName())) {
						continue;
					}
					if (p->getKeyName() == "ClydeBellecourt") {
						int i = 1; // just for debugging
					}
					// save with right playitem
					//if (p->setup(json["scenes"][i])) {
						// find stage and set it
						if (!setStage(p)) {
							ofLogNotice("ChannelList::read") << "scene not in playlist (ignored) " << p->getKeyName();
						}
					//}
				}
			}
			// remove unattached stages, user forgot them in the input file
			std::vector<shared_ptr<Channel>>::const_iterator iter = list.begin();
			while (iter != list.end()) {
				if ((*iter)->getStage() == nullptr) {
					iter = list.erase(iter);
					ofLogNotice("ChannelList::read") << "item in playlist not found in json (ignored) " << (*iter)->getKeyName();
				}
				else {
					++iter;
				}
			}
		}
		catch (std::exception e) {
			ofLogError("ChannelList::read") << "exception " << e.what();
			return false;
		}
		return true;
	}

	// add this one http://clab.concordia.ca/?page_id=944
	void Video::myDraw() {

		if (width == 0 || height == 0) {
			worker.draw(0, 0);
		}
		else {
			worker.draw(0, 0, width, height);
		}
	}
	Video::~Video() {
		if (isLoaded) {
			worker.stop();
		}
	}
	void Video::mySetup() {
		string debug = getLocationPath();
		if (!isLoaded) {
			if (!worker.load(getLocationPath())) {
				ofLogError("Video") << "setup video Player " << getLocationPath();
			}
			isLoaded = true; // avoid keep on trying
		}
		worker.play();

	}
	void Video::myUpdate() {
		Visual::myUpdate();
		worker.update();
	}

	void Image::myUpdate() {
		update();
	}
	void Image::myDraw() {
		if (pixels.isAllocated()) {
			ofImage image;
			image.setFromPixels(pixels);
			image.draw(0,0);
		}
	}
	// setup each row
	bool Image::mySetup(const Json::Value &val) {
		if (val["locationPath"].isString()) {
			string path = val["locationPath"].asString();
			if (!ofLoadImage(pixels, path)) {
				ofLogError("Picture") << "setup Picture Player " << path;
			}
			else {
				// set pixels size if needed
				READINT(height, val);
				READINT(width, val);
				if (width == -1 || height == -1) {
					pixels.resize(ofGetScreenWidth(), ofGetScreenHeight()); // if either is -1 set both
				}
				else if (width != 0 && height != 0) {
					pixels.resize(width, height); // but must be set to change size
				}

				return true;
			}
		}
		return false;
	}

	float Video::getTimeBeforeStart(float t) {

		// if json sets a wait use it
		if (getWait() > 0) {
			setIfGreater(t, getWait());
		}
		else {
			// will need to load it now to get the true lenght
			if (!worker.isLoaded()) {
				worker.load(getLocationPath());
			}
			float duration = getObjectLifetime();
			setIfGreater(t, duration);
		}
		return t;
	}
	
	void Audio::mySetup() {
		if (!worker.load(getLocationPath())) {
			ofLogError("Audio") << "setup audio Player " << getLocationPath();
		}
		// some of this data could come from data in the future
		worker.play();
	}
	int CameraGrabber::find() {
		//bugbug does Kintect show up?
		vector<ofVideoDevice> devices = worker.listDevices();
		vector<ofVideoDevice>::const_iterator it = devices.begin();
		for (; it != devices.end(); ++it) {
			if (getLocationPath().size() == 0) {
				return it->id;// first found unless one is named
			}
			if (it->deviceName == getLocationPath()) {
				return it->id;
			}
		}
		return 0;// try first found as a default
	}
	void CameraGrabber::myUpdate() {
		if (worker.isInitialized()) {
			worker.update();
		}
	}

	bool CameraGrabber::loadGrabber(int wIn, int hIn) {
		id = find();
		worker.setDeviceID(id);
		worker.setDesiredFrameRate(30);
		bool b = worker.initGrabber(wIn, hIn);
		return b;
	}

	void CameraGrabber::myDraw() {
		if (worker.isInitialized()) {
			worker.draw(0, 0);
		}
	}
	bool CameraGrabber::mysetup(const Json::Value &data) {
		//"Logitech HD Pro Webcam C920"

		return true;
	}
	bool TextureVideo::mybind() {
		if (worker.isInitialized() && fbo.isUsingTexture()) {
			worker.getTexture().bind();
			return true;
		}
		return false;
	}
	bool TextureVideo::myunbind() {
		if (worker.isInitialized() && worker.isUsingTexture()) {
			worker.getTexture().unbind();
			return true;
		}
		return false;
	}
	bool TextureVideo::mysetup(const Json::Value &data) {
		//bugbug fill this in with json reads as needed
		if (!worker.isLoaded()) {
			if (!worker.load(getLocationPath())) {
				ofLogError("TextureVideo") << "setup TextureVideo Player " << getLocationPath();
				return false;
			}
			worker.play();
		}
		return true;
	}
	ofTexture& TextureVideo::getVideoTexture() {
		return worker.getTexture();
	}

	void VideoSphere::myDraw() {
		//bugbug just need to do this one time, maybe set a flag
		if (video->getVideoTexture().isAllocated() && !set) {
			sphere.get()->mapTexCoordsFromTexture(video->getVideoTexture());
			sphere.get()->rotate(180, 0, 1, 0.0);
			set = true;
		}

		video->mybind();
		getSphere().myDraw();
		video->myunbind();
	}
	bool VideoSphere::mysetup(const Json::Value &data) {

		setType(ActorRole::draw3dFixedCamera);
		video = std::make_shared<TextureVideo>();
		if (video) {
			if (video->setup(data)) {//bugbug is this data correct?
				getStage()->addToAnimatable(video);
			}
		}
		getSphere().get()->set(250, 180);// set default
		getSphere().get()->rotate(180, 1, 0, 0); // flip right side up
		getSphere().get()->rotate(180, 0, 1, 0); // turn seam to the back, just one time
		getSphere().mysetup(data); // do not call base class, just get specific items, baseclass items handled in Video
		getSphere().setWireframe(true); // looks much cooler
		return true;
	}
	void TextureFromImage::create(const string& name, float w, float h) {
		// create texture
		ofLoadImage(*this, name);
		if (w == 0 || h == 0) {
			int i = 0;
		}
		fbo.allocate(w, h, GL_RGB);
		fbo.begin();//fbo does drawing
		ofSetColor(ofColor::white); // no image color change when using white
		draw(0, 0, w, h);
		fbo.end();// normal drawing resumes
	}

	bool SolarSystem::mysetup(const Json::Value &data) {
		Json::Value::Members m = data.getMemberNames();
		if (data["videoSphere"].size() > 0) {
			shared_ptr<VideoSphere> vs = std::make_shared<VideoSphere>();
			if (vs) {
				if (vs->setup(data["videoSphere"])) {
					addPlanets(data["planets"], vs->getSphere());
					getStage()->addToAnimatable(vs);
				}
			}
		}
		return true;
	}
	void SolarSystem::addPlanets(const Json::Value &data, Sphere &parent) {

		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<Planet> planet = std::make_shared<Planet>();
			if (planet) {
				planet->rotateAround = ofPoint(parent.get()->getRadius(), parent.get()->getRadius(), parent.get()->getRadius());
				if (planet->setup(data[j])) {
					getStage()->addToAnimatable(planet);
				}
			}
		}
	}

	bool Planet::mysetup(const Json::Value &data) {
		setType(ActorRole::draw3dMovingCamera);
		getSphere().mysetup(data); // do not call base class, just get specific items, baseclass items handled in Planet
								   //override some settings as this is a helper object vs. on json really wants to do much with
		getSphere().setWireframe(false);
		float max = (rotateAround.x > 0) ? (rotateAround.x / 3) : ofGetWidth() / 10;
		float r = ofRandom(5, max);// never bigger than center planet
		getSphere().get()->set(r, 60);
		//bugbug as this objet is used get data from json, right now this is more demo than production
		ofPoint point;
		if (rotateAround.x == 0) {
			rotateAround.x = ofGetWidth() / 10;
		}
		if (rotateAround.y == 0) {
			rotateAround.y = ofGetHeight() / 10;
		}
		if (rotateAround.z == 0) {
			rotateAround.z = ofGetWidth() / 100;
		}
		point.x = ofRandom(-rotateAround.x, rotateAround.x * 2);
		point.y = ofRandom(rotateAround.y, rotateAround.y * 2);
		point.z = ofRandom(rotateAround.z, rotateAround.z * 2);
		getSphere().get()->setPosition(point); // data stored as pointer so this updates the list

		getTexturePtr()->create(getLocationPath(), r * 2, r * 2);

		getSphere().get()->mapTexCoordsFromTexture(getTexturePtr()->getImageTexture());
		getSphere().get()->rotate(180, 1, 0, 0);
		return true;
	}
	void Planet::myDraw() {
		getTexturePtr()->bind();
		sphere.get()->rotate(45, 0, 1.0, 0.0);
		sphere.myDraw();
		getTexturePtr()->unbind();
	}
	void RandomDots::draw() {
		for (int i = 0; i < ofGetMouseX() * 5; i++) {
			ofSetColor(ofRandom(96));
			ofRect(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 4, 4);
		}
	}
	void ComplexLines::draw() {
		for (const auto& line : lines) {
			ofDrawLine(line.a, line.b);
		}
	}
	//bugbug convert to code vs mouse using random
	void ComplexLines::mouseDragged(int x, int y, int button) {
		for (auto point : drawnPoints) {
			ofPoint mouse;
			mouse.set(x, y);
			float dist = (mouse - point).length();
			if (dist < 30) {
				Line lineTemp;
				lineTemp.a = mouse;
				lineTemp.b = point;
				lines.push_back(lineTemp);
			}
		}
		drawnPoints.push_back(ofPoint(x, y));
	}
	void VectorPattern::matrix(int twistx, int shifty) {
		ofPushMatrix();
		for (int i = -50; i < 50; ++i) {
			ofTranslate(0, i*shifty);
			ofRotate(i*twistx);
			stripe(true);
		}
		ofPopMatrix();
	}
	void VectorPattern::stripe(bool rotate) {
		ofSetColor(ofColor::black);
		ofSetLineWidth(3);
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofTranslate(i * 20, 0);
			if (rotate) {
				ofRotate(i * 5);
			}
			ofLine(0, -100, 0, 100);
			ofPopMatrix();
		}
	}
	void VectorPattern::triangle(bool rotate) {
		ofSetColor(ofColor::black);
		ofSetLineWidth(3);
		ofNoFill();
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofTranslate(i * 20, 0);
			if (rotate) {
				ofRotate(i * 5);
			}
			ofScale(6, 6); // enlarge 6x
			ofTriangle(0, 0, -50, 100, 50, 100);
			ofPopMatrix();
		}
	}
	void VectorPattern::shape(int twistx, int shifty, bool rect, bool fill, int rotate, int alpha) {
		ofColor color = ofColor::black;
		color.a = alpha;
		ofSetColor(color);
		ofSetLineWidth(1);
		if (fill) {
			ofFill();
		}
		else {
			ofNoFill();
		}
		for (int i = -50; i < 50; ++i) {
			ofPushMatrix();
			ofRotate(i * twistx);
			ofTranslate(i * 20, shifty);
			ofRotate(rotate);
			ofScale(6, 6); // enlarge 6x
			if (rect) {
				ofRect(-50, -50, 100, 100);
			}
			else {
				ofTriangle(0, 0, -50, 100, 50, 100);
			}
			ofPopMatrix();
		}
		ofScale(6, 6); // enlarge 6x
		ofTriangle(0, 0, -50, 100, 50, 100);
	}



	void Line3D::setup() {
		//required ofEnableDepthTest();
		baseNode.setPosition(0, 0, 0);
		childNode.setParent(baseNode);
		childNode.setPosition(0, 0, 200);
		grandChildNode.setParent(childNode);
		grandChildNode.setPosition(0, 50, 0);
	}
	void Line3D::update() {
		baseNode.pan(1);
		childNode.tilt(3);

		line.addVertex(grandChildNode.getGlobalPosition());
		if (line.size() > 200) {
			line.getVertices().erase(
				line.getVertices().begin()
			);
		}
	}
	void Line3D::draw() {
		//uncomment these 3 lines to understand how nodes are moving
		//baseNode.draw();
		//childNode.draw();
		//grandChildNode.draw();
		line.draw();
	}



	void CrazyMesh::setup() {
		for (int i = 0; i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				addColor(color);
			}
		}
	}
	void CrazyMesh::update() {
		clearVertices();
		for (int i = 0; i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				float x = sin(i*0.1 + ofGetElapsedTimef()) * 1000;
				float y = sin(j*0.15 + ofGetElapsedTimef()) * 1000;
				float z = x + y;
				addVertex(ofVec3f(x, y, z));
			}
		}
	}
	void CrazyMesh::draw() {
		ofPushStyle();
		ofSetHexColor(0xffffff);
		glPointSize(2);
		glEnable(GL_POINT_SMOOTH);
		drawVertices();
		ofPopStyle();
	}

	void MeshEngine::setup() {
		//setMode(OF_PRIMITIVE_POINTS);
		setMode(OF_PRIMITIVE_LINES);
		//setMode(OF_PRIMITIVE_LINE_STRIP);
		//setMode(OF_PRIMITIVE_LINE_LOOP);
		enableColors();
		enableIndices();
		ofVec3f top(100.0, 50.0, 0.0);
		ofVec3f left(50.0, 150.0, 0.0);
		ofVec3f right(150.0, 150.0, 0.0);
		addVertex(top);
		//		addColor(Colors::getFloatColor(Colors::getDarkest()));

		addVertex(left);
		//		addColor(Colors::getFloatColor(Colors::getLightest()));

		addVertex(right);
		//bugbug this is where we add in more colors, lets see how many before we make 
		// changes, but somthing like Color1, Color2 etc
		//	addColor(Colors::getFloatColor(Colors::getDarkest()));

		addIndex(0);
		addIndex(1);
		addIndex(1);
		addIndex(2);
		addIndex(2);
		addIndex(0);

	}
	void MoreMesh::setup() {
		image.load("stars.png");
		image.resize(200, 200); // use less data
		setMode(OF_PRIMITIVE_LINES);
		enableColors();

		float intensityThreshold = 150.0;
		int w = image.getWidth();
		int h = image.getHeight();
		for (int x = 0; x<w; ++x) {
			for (int y = 0; y<h; ++y) {
				ofColor c = image.getColor(x, y);
				float intensity = c.getLightness();
				if (intensity >= intensityThreshold) {
					float saturation = c.getSaturation();
					float z = ofMap(saturation, 0, 255, -100, 100);
					// We shrunk our image by a factor of 4, so we need to multiply our pixel
					// locations by 4 in order to have our mesh cover the openFrameworks window
					ofVec3f pos(x * 4, y * 4, z);
					addVertex(pos);
					// When addColor(...), the mesh will automatically convert
					// the ofColor to an ofFloatColor
					addColor(c);
				}
			}
		}
		// Let's add some lines!
		float connectionDistance = 30;
		int numVerts = getNumVertices();
		for (int a = 0; a < numVerts; ++a) {
			ofVec3f verta = getVertex(a);
			for (int b = a + 1; b < numVerts; ++b) {
				ofVec3f vertb = getVertex(b);
				float distance = verta.distance(vertb);
				if (distance <= connectionDistance) {
					// In OF_PRIMITIVE_LINES, every pair of vertices or indices will be
					// connected to form a line
					addIndex(a);
					addIndex(b);
				}
			}
		}
	}
	void MoreMesh::draw() {
		ofColor centerColor = ofColor(85, 78, 68);
		ofColor edgeColor(0, 0, 0);
		ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);
		easyCam.begin();
		ofPushMatrix();
		ofTranslate(-ofGetWidth() / 2, -ofGetHeight() / 2);
		ofMesh::draw();
		ofPopMatrix();
		easyCam.end();
	}

	// figure out rotationsfor face
	void rotateToNormal(ofVec3f normal) {
		normal.normalize();

		float rotationAmount;
		ofVec3f rotationAngle;
		ofQuaternion rotation;

		ofVec3f axis(0, 0, 1);
		rotation.makeRotate(axis, normal);
		rotation.getRotate(rotationAmount, rotationAngle);
		ofRotate(rotationAmount, rotationAngle.x, rotationAngle.y, rotationAngle.z);
	}

}