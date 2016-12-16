#include "ofApp.h"
#include "color.h"

namespace Software2552 {
	shared_ptr<ColorList::colordata> ColorList::privateData = nullptr; // declare static data

	ColorSet::ColorSet(shared_ptr<ColorSet>rhs) {
		if (rhs) {
			defaultColor = rhs->defaultColor;
			group = rhs->group;
			colors = rhs->colors;
		}
	}

	shared_ptr<ColorSet> parseColor(const Json::Value &data)
	{
		ColorList list;
		shared_ptr<ColorSet> results = nullptr;
		Json::Value::Members m = data.getMemberNames();// just to help w/ debugging
		// will update the global copy of the named colorset  
		if (data["colorAnimation"].size() > 0) {
			results = list.read(data["colorAnimation"]);
			if (!results) {
				results = std::make_shared<ColorSet>(); // try to make one only if it does not exist, we could be out of memory
			}
			if (results) {
				results->setup(data["colorAnimation"]);
			}
		}
		if (results) {
			return results;
		}
		return ColorList::getCurrentColor();
	}
	// true if any alpha enabled, keeping in mind with all the pointers in a ColorSet stage can change often
	bool ColorSet::alphaEnbled() {
		for (const auto& c : colors) {
			if (c.second->AlphaEnabled()) {
				return true;
			}
		}

		return false;
	}
	shared_ptr<AnimiatedColor>ColorSet::getAnimatedColor(ColorType type) {
		ColorMap::const_iterator itr = colors.find(type);
		if (itr != colors.end())	{
			return itr->second;
		}
		return nullptr;
	}

	ofColor ColorSet::get(ColorType type) {
		ColorMap::const_iterator itr = colors.find(type);
		if (itr != colors.end()) {
			return itr->second->getCurrentColor();
		}
		return defaultColor;
	}
	void ColorSet::draw() { 
		ColorMap::const_iterator itr = colors.find(Fore);
		if (itr != colors.end()) {
			itr->second->applyCurrentColor();
		}
	}
	// get or create 
	shared_ptr<AnimiatedColor> ColorSet::getOrCreate(ColorType type) {
		shared_ptr<AnimiatedColor> c = getAnimatedColor(type);
		if (!c) {
			c = std::make_shared<AnimiatedColor>();
			addColor(type, c);
		}
		return c;
	}
	// lets us over ride the built in colors bugbug create a basic color than can then be re-done here as needed
	void ColorSet::setup(const Json::Value &data) {
		//bugbug good to read in to/from/color and other things
		//Fore, Back, Lightest, Darkest, Color1, Color2
		shared_ptr<AnimiatedColor> c;
		if (data["fore"].size() > 0 && (c = getOrCreate(Fore))) {
			c->setup(data["fore"]);
		}
		if (data["back"].size() > 0 && (c = getOrCreate(Back))) {
			c->setup(data["back"]);
		}
		if (data["lightest"].size() > 0 && (c = getOrCreate(Lightest))) {
			c->setup(data["lightest"]);
		}
		if (data["darkest"].size() > 0 && (c = getOrCreate(Darkest))) {
			c->setup(data["darkest"]);
		}
		if (data["color1"].size() > 0 && (c = getOrCreate(Color1))) {
			c->setup(data["color1"]);
		}
		if (data["color2"].size() > 0 && (c = getOrCreate(Color2))) {
			c->setup(data["color2"]);
		}
	}
	void ColorSet::update() {
		for (auto& c : colors) {
			c.second->update();
		}
	}
	// set 1 or more colors in the set bugbug function is just an example of var args now
	//void ColorSet::setSetcolors(int c, ...) {
	//	colors.clear();

	//va_list args;
	//va_start(args, c);
	//for (int i = 0; i < c; ++i) {
			// null is not saved
			//	if (va_arg(args, shared_ptr<AnimiatedColor>)) {
				//colors.push_back(va_arg(args, shared_ptr<AnimiatedColor>));
				//}
				//}
				//va_end(args);
				//}
	shared_ptr<ColorSet> ColorList::read(const Json::Value &data) {
		if (data.size() > 0) {
			//bugbug where is this allocated
			if (privateData) {
				string colorGroup;
				READSTRING(colorGroup, data);
				if (colorGroup.size() > 0) {
					privateData->currentColorSet = getNextColors(ColorSet::convertStringToGroup(colorGroup), true);
				}
			}
		}
		return privateData->currentColorSet;
	}
	void ColorList::update() {
		if (privateData && privateData->currentColorSet) {
			privateData->currentColorSet->update();
			// remove expired colors
			removeExpiredItems(privateData->colorlist);
			//bugbug call this at the right time
			if (getCurrentColor() && ofRandom(0, 100) > 80) {
				//getNextColors(getCurrentColor()->getGroup(), true); // updates global list
			}
		}
	}
	void ColorList::setCurrentColorSet(shared_ptr<ColorSet>c) {
		if (privateData && c) {
			privateData->currentColorSet = c; //bugbug do need to reset the coloranimation here?
		}
	}
	// load color on demand (save memory at all costs so we can port the heck out of this thing)
	std::forward_list<shared_ptr<ColorSet>>::iterator ColorList::load(ColorSet::ColorGroup group) {
		for (auto& it = privateData->colorlist.begin(); it != privateData->colorlist.end(); ++it) {
			if ((*it)->getGroup() == group) {
				return it; // color already present
			}
		}
		// local helper
		class aColor {
		public:
			aColor(Map *p) { map = p; }
			int get(char c) { 
				Map::const_iterator itr = map->find(c);
				if (itr != map->end()) {
					return itr->second;
				}
				return 0; // default to black
			}

			ofColor color(char c, int alpha=255) {
				return ofColor::fromHex(get(c), alpha); 
			}

			shared_ptr<AnimiatedColor> create(char c, int alpha, char c2, int alpha2=255) {
				return create(color('A', 255), color('B', 255));
			}

			shared_ptr<AnimiatedColor> create(const ofColor& c) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(c);
				}
				return ac;
			}
			shared_ptr<AnimiatedColor> create(const ofColor& start, const ofColor& end) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(start);//bugbug get from json
					ac->animateTo(end);//bugbug look at delays and final color, good json data
				}
				return ac;
			}
			// no animation
			shared_ptr<AnimiatedColor> create(char c) {
				return create(color(c));
			}
			// only alpha changes
			shared_ptr<AnimiatedColor> create(char start, int alpha) {
				shared_ptr<AnimiatedColor>ac = std::make_shared<AnimiatedColor>();
				if (ac) {
					ac->setColor(color(start));
					ac->animateToAlpha(alpha);
				}
				return ac;
			}

			Map *map;
		};
#define CREATE(a) c.create(a)
#define CREATE2(a,b) c.create(a,b)
		// colors http://cloford.com/resources/colours/namedcol.htm list
		// nice palettes etc http://www.color-hex.com/color-palettes/ http://www.color-hex.com/popular-colors.php
		if (group == ColorSet::Modern || group == ColorSet::ModernAnimated) {
			aColor c(&privateData->modern);
			shared_ptr<AnimiatedColor>A = CREATE('A');
			shared_ptr<AnimiatedColor>C = CREATE('C');
			//add the range here with the helper
			addbasic(ColorSet::ModernAnimated, c.create('A', 255, 'B'), CREATE2('C', 100), C, A);

			//jabc, lightest:c, darkest: a, note sharing pointers is ok too, like using the pointer A in more places,
			// but in that case it will changes across colors, something that may be wanted
			shared_ptr<AnimiatedColor>J = CREATE2('J', 1);
			shared_ptr<AnimiatedColor>B = CREATE('B');
			addfull(ColorSet::ModernAnimated, J, A, B, CREATE('C'), CREATE('C'), CREATE('A'));

			// jobl a pointer one that will change as others change
			addfull(ColorSet::Modern, CREATE('J'), CREATE('O'), B, CREATE('L'), CREATE('B'), CREATE('O'));
		}
		else if (group == ColorSet::UserDefined) {
			aColor c(nullptr);
			addbasic(ColorSet::UserDefined, CREATE(ofColor::white));// user edits via json
		}
		else if (group == ColorSet::White) {
			aColor c(nullptr);
			addbasic(ColorSet::White, CREATE(ofColor::white));//bugbug add "createColor":"white" kind of thing to json
		}
		else if (group == ColorSet::Orange || group == ColorSet::OrangeAnimated) {
			aColor c(nullptr);
			addbasic(ColorSet::OrangeAnimated, CREATE2(ofColor::orange, ofColor::orangeRed));
			addbasic(ColorSet::Orange, CREATE(ofColor::orange));
		}
		else if (group == ColorSet::Blue || group == ColorSet::BlueAnimated) {
			aColor c(nullptr);
			addbasic(ColorSet::BlueAnimated, CREATE2(ofColor::lightBlue, ofColor::darkBlue));
			addbasic(ColorSet::Blue, CREATE(ofColor::blue));
		}
		return privateData->colorlist.begin();
	}
	// get next color based on type and usage count, this will set the color globally
	// example: type==cool gets the next cool type, type=Random gets any next color
	shared_ptr<ColorSet> ColorList::getNextColors(ColorSet::ColorGroup group, bool global) {
		shared_ptr<ColorSet> ret = nullptr;
		if (getCurrentColor() != nullptr) {
			if (getCurrentColor()->getGroup() != group) {
				// new group, delete current group
				if (global) {
					setCurrentColorSet(nullptr);
				}
			}
		}
		// load group if needed
		std::forward_list<shared_ptr<ColorSet>>::iterator it = load(group);

		// find a match
		for (; it != privateData->colorlist.end(); ++it) {
			if ((*it)->getGroup() == group) {
				if (getCurrentColor() == nullptr || getCurrentColor()->getUsage() >= (*it)->getUsage()) {
					// first time in or a color as less usage than current color
					if (global) {
						setCurrentColorSet(*it);
					}
					ret = *it;
					break;
				}
			}
		}
		if (!ret) {
			return std::make_shared<ColorSet>(); // always return something
		}
		return ret;
	}
	// add basic list
	shared_ptr<ColorSet>  ColorList::addbasic(const ColorSet::ColorGroup group, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) {
		shared_ptr<ColorSet> s = std::make_shared<ColorSet>(group, fore, back, lightest, darkest);
		if (s) {
			privateData->colorlist.push_front(s);
		}
		return s;
	}
	// add a full color list
	void ColorList::addfull(const ColorSet::ColorGroup group, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor>back,
		shared_ptr<AnimiatedColor> color1, shared_ptr<AnimiatedColor> color2,
		shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest)	{
		shared_ptr<ColorSet> s = addbasic(group, fore, back, lightest, darkest);
		if (s) {
			s->addColor(ColorSet::Color1, color1);
			s->addColor(ColorSet::Color2, color2);
		}
	}

	ColorSet::ColorGroup ColorSet::convertStringToGroup(const string&name) {
		if (name == "Modern") {
			return Modern;
		}
		if (name == "ModernAnimated") {
			return ModernAnimated;
		}
		else if (name == "UserDefined") {
			return UserDefined;
		}
		else if (name == "Smart") {
			return Smart;
		}
		else if (name == "Extreme") {
			return Extreme;
		}
		else if (name == "EarthTone") {
			return EarthTone;
		}
		else if (name == "Orange") {
			return Orange;
		}
		else if (name == "OrangeAnimated") {
			return OrangeAnimated;
		}
		else if (name == "Black") {
			return Black;
		}
		else if (name == "White") {
			return White;
		}
		else if (name == "Blue") {
			return Blue;
		}
		else if (name == "BlueAnimated") {
			return BlueAnimated;
		}
		else if (name == "Random") {
			return Random;
		}
		else {
			return Default;
		}
	}

	ColorList::ColorList() {
	}
	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore, shared_ptr<AnimiatedColor> back, shared_ptr<AnimiatedColor> lightest, shared_ptr<AnimiatedColor> darkest) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;
		if (fore) {
			colors[Fore] = fore; // never set a null
		}
		if (back) {
			colors[Back] = back; // never set a null
		}
		if (lightest) {
			colors[Lightest] = lightest; // never set a null
		}
		if (darkest) {
			colors[Darkest] = darkest; // never set a null
		}
	}
	ofColor ColorSet::getColor1() {
		ColorMap::const_iterator itr = colors.find(Color1);
		if (itr == colors.end()) {
			ofColor c = getBackground();// start with fore color
			c.setSaturation(c.getSaturation() + 50);// may wrap around?
			c.setBrightness(c.getBrightness() + 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ofColor ColorSet::getBackground() {
		ColorMap::const_iterator itr = colors.find(Back);
		if (itr == colors.end()) {
			return get(Fore).getInverted();//only color we can assume is Fore, but maybe we can do more to make it unique
		}
		return itr->second->getCurrentColor();
	}

	ofColor ColorSet::getColor2() {
		ColorMap::const_iterator itr = colors.find(Color2);
		if (itr == colors.end()) {
			ofColor c = getForeground();// start with fore color
			c.setSaturation(c.getSaturation() + 10);// may wrap around?
			c.setBrightness(c.getBrightness() + 10);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}

	ofColor ColorSet::getLightest() {
		// assume data is propery allocated in the colors vector
		ColorMap::const_iterator itr = colors.find(Lightest);
		if (itr == colors.end()) {
			ofColor c = get(Fore);// start with fore color
			c.setSaturation(c.getSaturation() - 50);// may wrap around?
			c.setBrightness(c.getBrightness() - 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ofColor ColorSet::getDarkest() {
		ColorMap::const_iterator itr = colors.find(Darkest);
		if (itr == colors.end()) {
			ofColor c = getBackground();// start with fore color
			c.setSaturation(c.getSaturation() + 50);// may wrap around?
			c.setBrightness(c.getBrightness() + 20);// may wrap around?
			return c;
		}
		return itr->second->getCurrentColor();
	}
	ColorSet::ColorSet() {
		group = Modern;
		defaultColor = ofColor::yellow;
		shared_ptr<AnimiatedColor>c = std::make_shared<AnimiatedColor>();
		if (c) {
			c->setColor(defaultColor);
			colors[Fore] = c;
		}
	};

	ColorSet::ColorSet(const ColorGroup groupIn, shared_ptr<AnimiatedColor> fore) : objectLifeTimeManager() {
		group = groupIn;
		defaultColor = ofColor::greenYellow;//c.invert(); lerp ofColor r = ofColor::red;
									  ///     ofColor b = ofColor::blue;
									  ///     b.lerp(r, 0.5); // now purple
		if (fore) {
			colors[Fore] = fore; // never set a null
		}
	}

	//http://www.creativecolorschemes.com/resources/free-color-schemes/art-deco-color-scheme.shtml
	void ColorList::setup() {
		// assumes static data so color info is shared across app
		if (privateData == nullptr) {
			privateData = std::make_shared<colordata>();
		}
		if (privateData == nullptr) {
			return;
		}
		
		//bugbug phase II read from json
		// colors http://cloford.com/resources/colours/namedcol.htm list
		// nice palettes etc http://www.color-hex.com/color-palettes/ http://www.color-hex.com/popular-colors.php
		// set some matching built in colors
		if (privateData->modern.empty()) {
			privateData->modern =
			{ {'A', 0x003F53}, {'B', 0x3994B3}, {'C', 0x64B1D1 }, {'D', 0x00626D }, {'E', 0x079CBA }, {'F', 0x60CDD9 },
			 {'G', 0x003E6B }, {'H', 0x0073A0 }, {'I', 0xBAECE9 }, {'J', 0xD0FC0E }, {'K', 0xFDB075 }, {'L', 0xFFD76E },
			 {'M', 0x4D5E6C }, {'N', 0x858E9C }, {'O', 0xCCD1D5 } };
			privateData->smart = {
			{ 'A',0x312659},{'B', 0x373B81},{ 'C', 0x425096},{ 'D', 0x0D60A5 },{ 'E', 0x297BC6 },{ 'F', 0x3EA0ED },{ 'G', 0x169FAD },
			{ 'H', 0x30C1D8 },{ 'I', 0x7FE3F7 },{ 'J', 0xB01116 },{ 'K', 0xD71920 },{ 'L', 0xFEAB07 },{ 'M', 0xFED341 },{ 'N', 0xFFDA7A },{ 'O', 0xFFEEBC } };

			privateData->extreme = {
				{ 'A',0x023D7B },{ 'B', 0x1B4C8C },{ 'C', 0x448BE3 },{ 'D', 0x025B8F },{ 'E', 0x088BB3 },
				{ 'F',  0x02CAE6 },{ 'G',  0xC61630 },{ 'H',0xFE243E },{ 'I',  0xFE3951 },{ 'J',  0xF03624 },
				{ 'K',  0xF3541B },{ 'L',  0xFE872B },{ 'M',  0x8FD173 },{ 'N',  0xB7B96B },{ 'O',  0xAD985F } };

			privateData->earthtone = {
			{ 'A',0x493829 },{ 'B',  0x816C5B },{ 'C',  0xA9A18C },{ 'D',  0x613318 },{ 'E',  0x855723 },{ 'F',  0xB99C6B },{ 'G',  0x8F3B1B },{ 'H', 0xD57500 },
			{ 'I',  0xDBCA69 },{ 'J',  0x404F24 },{ 'K',  0x668D3C },{ 'L',  0xBDD09F },{ 'M',  0x4E6172 },{ 'N',  0x83929F },{ 'O',  0xA3ADB8} };
			//A C B D A C see the color doc to fill these in. use the 4 colors then pick the lightest and darkest 
		}
		if (privateData->currentColorSet == nullptr) {
			privateData->currentColorSet = getNextColors(ColorSet::Modern, true);// make sure there is a current color
		}

	}

	

}