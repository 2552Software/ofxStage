#include "ofApp.h"
#include "scenes.h"

//By default, the screen is cleared with every draw() call.We can change that with 
//  ofSetBackgroundAuto(...).Passing in a value of false turns off the automatic background clearing.

namespace Software2552 {
	
	shared_ptr<Stage> stage=nullptr; // one global stage, makes it easy to change between stages
	// always valid stage pointer
	shared_ptr<Stage> getStage() {
		if (stage == nullptr) {
			getScene("Generic"); // default
		}
		return stage;
	}
	// convert name to object
	shared_ptr<Stage> getScene(const string&name)
	{
		if (name == "Test") {
			return stage = std::make_shared<TestScene>();
		}
		if (name == "Generic") {
			return stage = std::make_shared<GenericScene>();
		}
		ofLogError("Stage") << "name not known (ignored) using default scene " << name;
		return stage = std::make_shared<GenericScene>();
	}

	// return a possibly modifed version such as camera moved
	shared_ptr<FixedCamera> Director::pickem(vector<shared_ptr<FixedCamera>>&cameras, bool orbiting) {
		for (auto it = cameras.begin(); it != cameras.end(); ++it) {
			if (orbiting) {
				if ((*it)->isOrbiting()) {
					return (*it);
				}
			}
			else {
				if (!(*it)->isOrbiting()) {
					//bugbug return only fixed cameras
					return (*it);// stuff data in the Camera class
									  // lots to do here to make things nice, learn and see how to bring this in
									  //void lookAt(const ofNode& lookAtNode, const ofVec3f& upVector = ofVec3f(0, 1, 0));
									  //if (j == 1) {
									  //cameras[j].move(-100,0,0);
									  //cameras[j].pan(5);
									  //}
									  //else {
									  //cameras[j].move(100, 0, 0);
									  //}
									  //bugbug just one thing we can direct http://halfdanj.github.io/ofDocGenerator/ofEasyCam.html#CameraSettings
									  //float f = cameras[j].getDistance();
									  //figure all this out, with times, animation etc:
									  //cameras[j].setDistance(cameras[j].getDistance() + ofRandom(-100,100));
									  //return &cameras[j];
				}

			}
		}
		ofLogError("Director") << "no camera";
		return nullptr;
	}
	// set background object at location 0 every time
	shared_ptr<Background> Stage::CreateReadAndaddBackgroundItems(const Json::Value &data) {
		if (!data.empty()) {
			shared_ptr<Background> b = std::make_shared<Background>();
			if (b != nullptr) {
				if (b->setup(data)) {
					// only save if data was read in 
					addToAnimatable(b, true);
				}
			}
			return b;
		}
		else {
			return nullptr;
		}
	}
	///bigger the number the more forward
	bool compareOrder(shared_ptr<ActorRole>first, shared_ptr<ActorRole>second)	{
		return (first->getDrawOrder()  < second->getDrawOrder());
	}
	// samples https://sites.google.com/site/ofauckland/examples

	// these are hard code helpers just for use in reading in Stage data
#define ADDANIMATION(name,type)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddAnimatable<type>(this, data[STRINGIFY(name)])
#define ADDLIGHTS(name,type)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddLight<type>(this, data[STRINGIFY(name)])
#define ADDCAMERAS(name,type)	if (!data[STRINGIFY(name)].empty()) CreateReadAndaddCamera<type>(this, data[STRINGIFY(name)])

	void Stage::readCameras(const Json::Value &data) {
		if (!data.empty()) {
			if (deleteExisting(data)) {
				getCameras().clear();
			}
			ADDCAMERAS(cameraFixed, FixedCamera);
			ADDCAMERAS(camera, MovingCamera);
		}
		// set a default camera if none exist
		if (getCameras().size() == 0) {
			shared_ptr<FixedCamera> camera = std::make_shared<FixedCamera>();
			if (camera) {
				if (camera->setup(data)) {
					camera->worker.setPosition(0, 0, 0);//bugbug clean up the rand stuff via data and more organized random
					add(camera);
				}
			}
			shared_ptr<MovingCamera> camera2 = std::make_shared<MovingCamera>();
			if (camera2) {
				if (camera2->setup(data)) {
					camera2->worker.setPosition(0, 0, 600);//bugbug clean up the rand stuff via data and more organized random
					add(camera2);
				}
			}
		}

	}
	void Stage::readLights(const Json::Value &data) {
		if (!data.empty()) {
			if (deleteExisting(data)) {
				getLights().clear();
			}
			ADDLIGHTS(light, Light);
			ADDLIGHTS(pointLight, PointLight);
			ADDLIGHTS(directionalLight, DirectionalLight);
			ADDLIGHTS(spotLight, SpotLight);
		}

		// set a default light if none exist
		if (getLights().size() == 0) {
			shared_ptr<Light> light = std::make_shared<Light>();
			if (light) {
				if (light->setup(data)) {
					add(light);
				}
			}
		}

	}
	// delete existing data
	bool Stage::deleteExisting(const Json::Value &data) {
		bool deleteExisting = false;// delete existing
		READBOOL(deleteExisting, data);
		return deleteExisting;

	}

	//recursive reader
	void Stage::readGraphics(const Json::Value &data) {
		// read from input (web, osc etc and queue input, resort by priority post each read and remove timed out data
		if (!data.empty()) {
			if (deleteExisting(data)) {
				getAnimatables().clear();
			}
			ADDANIMATION(spheres, Sphere);
			ADDANIMATION(spiral, Spiral);
			ADDANIMATION(shaders, Shader);
			ADDANIMATION(visibleMusic, VisibleMusic);
			ADDANIMATION(graphMusic, GraphMusic);
			ADDANIMATION(videos, Video);
			ADDANIMATION(rainbows, Rainbow);
			ADDANIMATION(planets, Planet);
			//ADD_REPEATING_ANIMATION(images, Image);
			return;
			
			ADDANIMATION(spiral, Spiral);// sound phase 1a
			ADDANIMATION(audio, Audio);// sound phase 1a
			ADDANIMATION(videos, Video);
			ADDANIMATION(circles, Ball);
			ADDANIMATION(cubes, Cube);
			ADDANIMATION(grabbers, CameraGrabber);
			ADDANIMATION(texts, Text);
			ADDANIMATION(paragraphs, Paragraph);
			ADDANIMATION(videoSpheres, VideoSphere);
			ADDANIMATION(solarSystems, SolarSystem);//bugbug forgot how to rotate around clyde, does not run w/ others
			getAnimatables().sort(compareOrder);

		}
		return;
	}

	bool Stage::updateData(shared_ptr<ofxJSON> data) {
		if (data) {
			Json::Value::Members m = data->getMemberNames();
			readGraphics((*data)["graphics"]);
			
			CreateReadAndaddBackgroundItems((*data)["background"]);
			readLights((*data)["lights"]);
			readCameras((*data)["cameras"]);
		}

		return true;
	};

	void Stage::draw() {
		preDraw();
		drawn = false;
		if (drawIn2d) {
			ofPushStyle();
			ofPushMatrix();
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);// default position, cameras may change location
			draw2d();
			ofPopMatrix();
			ofPopStyle();
		}
		if (drawIn3dMoving || drawIn3dFixed) {
			ofPushStyle();
			ofPushMatrix();
			draw3d();
			ofPopMatrix();
			ofPopStyle();
		}
		postDraw();
		if (!drawn) {
			for (auto& a : animatables) {
				a->frames.resetIFNeeded();
			}
		}

	}
	// pause them all
	void Stage::pause() {
		for (auto& a : animatables) {
			a->pause();
		}
		//bugbug pause moving camera, grabber etc
		myPause();
	}
	void Stage::resume() {
		for (auto& a : animatables) {
			a->resume();
		}
		//bugbug pause moving camera, grabber etc
		myResume();
	}
	// clear objects
	void Stage::clear(bool force) {
		if (force) {
			animatables.clear();
			cameras.clear();
			lights.clear();
		}
		else {
			removeExpiredItems(animatables);
			removeExpiredItems<FixedCamera>(cameras);
			removeExpiredItems<Light>(lights);
		}
		myClear(force);
	}
	void Stage::removeExpiredItems(list<shared_ptr<ActorRole>>&v) {
		mutex.lock();
		v.remove_if(ActorRole::OKToRemove);
		mutex.unlock();
	}

	void Stage::setup(shared_ptr<TCPKinectClient>clientIn) {
		tcpKinectClient = clientIn;
		mySetup();
	}
	void Stage::update() {
		
		removeExpiredItems(animatables); // keep things clean 

		for (auto& a : animatables) {
			a->updateForDrawing();
		}
		myUpdate();// dervived class update

	}
	// setup light and material for drawing
	void Stage::installLightAndMaterialThenDraw(shared_ptr<FixedCamera>cam, bool drawFixed) {
		if (cam != nullptr) {
			cam->worker.begin();
			cam->orbit(); 
			for (auto& light : lights) {
				ofPoint p = light->worker.getPosition();
				light->worker.setPosition(light->loc);
				light->worker.enable();
				light->worker.draw();
			}
			if (cam->isOrbiting()) {
				if (drawIn3dMoving && !drawFixed) {
					draw3dMoving();
				}
			}
			else {
				if (drawIn3dFixed && drawFixed) {
					draw3dFixed();
				}
			}
			cam->worker.end();
		}
		else {
			// draw w/o a camera
			ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2); // center when not using a camera
			for (auto& light : lights) {
				light->worker.setPosition(0, 0, 600);
				light->worker.enable();
				light->worker.draw();
			}
			if (drawIn3dMoving && !drawFixed) {
				draw3dMoving();
			}
			if (drawIn3dFixed && drawFixed) {
				draw3dFixed();
			}
		}
	}

	std::list<shared_ptr<ActorRole>>::iterator Stage::find(DataType type) {
		std::list<shared_ptr<ActorRole>>::iterator it = std::find_if(animatables.begin(), animatables.end(), [&type](shared_ptr<ActorRole>p) {
			return p->id == type;
		});
		return it;
	}


	void Stage::addToAnimatable(shared_ptr<ActorRole>p, bool inFront) {
		// only save working pointers
		if (p != nullptr) {
			if (animatables.size() > maxAnimatables()) {
				// remove a few old ones bugbug see how this goes
				for (int i = 0; i < maxAnimatables()/20; ++i) {
					animatables.pop_back();
				}
			}
			ActorRole::drawtype tp = p->getType();
			if (p->getType() == ActorRole::draw3dFixedCamera) {
				fixed3d(true); // do not set to false in case its already set
			}
			if (p->getType() == ActorRole::draw3dMovingCamera) {
				moving3d(true); // do not set to false in case its already set
			}
			if (p->getType() == ActorRole::draw2d) {
				fixed2d(true); // do not set to false in case its already set
			}
			p->setupForDrawing();
			mutex.lock();
			if (inFront) {
				animatables.push_front(p);
			}
			else {
				animatables.push_back(p);
			}
			mutex.unlock();
		}
	}

	void Stage::preDraw() {
		ofDisableDepthTest(); // bugbug make this a var read in via json so we can make more advanced graphics later
		//ofBackground(ofColor::blue); // white enables all colors in pictures/videos
		ofSetSmoothLighting(true);
		ofDisableAlphaBlending();

		//ofEnableDepthTest();
	}
	void Stage::postDraw() {
		// clean up
		ofEnableDepthTest();
		for (auto& light : lights) {
			light->worker.disable();
		}
		ofDisableLighting();

	}
	void Stage::draw3d() {

		// fixed camera
		if (drawIn3dFixed) {
			shared_ptr<FixedCamera> cam = director.pickem(cameras, false);
			if (cam != nullptr) {
				installLightAndMaterialThenDraw(cam, true);
			}
		}
		if (drawIn3dMoving) {
			shared_ptr<FixedCamera> cam = director.pickem(cameras, true);
			if (cam != nullptr) {
				installLightAndMaterialThenDraw(cam, false);
			}
		}

	}
	// find overall duration of a scene
	float Stage::findMaxWait() {
		float f = 0;
		
		for (const auto& a : getAnimatables()) {
			setIfGreater(f, a->getObjectLifetime() + a->getWait());
		}

		return f;
	}
	// show light location, great for debugging and kind of fun too
	void Stage::drawlights() {
		for (const auto& light : getLights()) {
			ofSetColor(light->worker.getDiffuseColor());
			ofPoint pos = light->worker.getPosition();
			ofDrawSphere(light->worker.getPosition(), 20.f);
		}
	}

	void Stage::draw2d() {

		myDraw2d();// allow easy extensions

		// draw fast items, then draw based on count

		for (auto& a : animatables) {
			if (!a->realtimeDrawing()) {
			
				// draw all infinite, then only draw based on count where a count is set
				// this allows the building of graphics
				if (a->frames.isInfinite()) {
					a->drawIt(ActorRole::draw2d);
				}
				else if (!a->frames.getFrameCountMaxHit()) {
					if (a->drawIt(ActorRole::draw2d)) {
						drawn = true;
					}
					break; // forces serialized drawing
				}
			}
		}
		for (auto& a : animatables) {
			if (a->realtimeDrawing() && !a->frames.getFrameCountMaxHit()) {
				if (a->drawIt(ActorRole::draw2d)) {
					drawn = true;
				}
			}
		}

		//ofBackground(ofColor::black);
		//bugbug option is to add vs replace:ofEnableBlendMode(OF_BLENDMODE_ADD);//bugbug can make these attributes somewhere
		//ofEnableAlphaBlending();
	}
	// juse need to draw the SpaceScene, base class does the rest
	void Stage::draw3dMoving() {
		myDraw3dMoving();
		// draw moving items
		for (auto& a : animatables) {
			if (a->frames.isInfinite()) {
				a->drawIt(ActorRole::draw3dMovingCamera);
			}
			else if (!a->frames.getFrameCountMaxHit()) {
				if (a->drawIt(ActorRole::draw3dMovingCamera)) {
					drawn = true;
				}
				break; // forces serialized drawing
			}
		}
	}
	void Stage::draw3dFixed() {
		myDraw3dFixed();
		// draw fixed items
		for (auto& a : animatables) {
			if (a->frames.isInfinite()) {
				a->drawIt(ActorRole::draw3dFixedCamera);
			}
			else if (!a->frames.getFrameCountMaxHit()) {
				if (a->drawIt(ActorRole::draw3dFixedCamera)) {
					drawn = true;
				}
				break; // forces serialized drawing
			}
		}
	}
	template<typename T>void Stage::CreateReadAndaddCamera(Stage*stage, const Json::Value &data) {
		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<T> camera = std::make_shared<T>();
			if (camera) {
				if (camera->setup(data[j])) {
					camera->worker.setPosition(0, 0, ofRandom(100, 500));//bugbug clean up the rand stuff via data and more organized random
					add(camera);
				}
			}
		}
	}
	template<typename T>void Stage::CreateReadAndaddLight(Stage*stage, const Json::Value &data) {

		for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
			shared_ptr<T> light = std::make_shared<T>();
			if (light) {
				if (light->setup(data[j])) {
					add(light);
				}
			}
		}
	}

	// bugbug all items in test() to come from json or are this is done in Director
	bool TestScene::myCreate(const Json::Value &data) {

		return true;

	}
	void TestScene::myUpdate() {
		mesh.update();
	}
	void TestScene::myDraw3dFixed() {
	
		//drawlights();

		return; // comment out to draw by vertice, less confusing but feel free to add it back in
		//bugbug item below needs to be put in a drawing class or should be removed
		ofSetHexColor(0xffffff);
		glPointSize(2);
		glEnable(GL_POINT_SMOOTH);
		mesh.drawVertices();
	}
	void TestScene::mySetup() {
		ofSetSmoothLighting(true);
		mesh.setup();
	}
	 
}