#include "ofApp.h"
#include "shaders.h"

// define a default set of shaders, other shaders can be added via json files

//generative art

// only GL3 is supported

//http://www.kynd.info/log/
//http://www.flong.com/
//https://github.com/princemio/ofxMIOFlowGLSL/blob/master/src/ofxMioFlowGLSL.cpp
//http://openglbook.com/chapter-0-preface-what-is-opengl.html
//https://thebookofshaders.com/05/
//https://www.asus.com/support/faq/1013017/

namespace Software2552 {
	// return common header code (private)
	string Shader::codeHeader() {
		string s = "#version 150\n"; // correct version?
#ifdef GL_ES
		s += "precision mediump float;\n";
#endif
		s += "uniform vec2 u_mouse;\n"; // share with all
		s += "uniform vec2 u_resolution;\n"; // share with all
		s += "uniform float u_time;\n"; // share with all
		s += "uniform float u_kick;\n"; // share with all
		s += "uniform float u_snare;\n"; // share with all
		s += "uniform float u_hihat;\n"; // share with all 
		s += "uniform float u_mag;\n"; // share with all 

		s += "uniform float u_0;\n"; // share with all 
		s += "uniform float u_5;\n"; // share with all 
		s += "uniform float u_10;\n"; // share with all 
		s += "uniform float u_15;\n"; // share with all 
		s += "uniform float u_20;\n"; // share with all 
		s += "uniform float u_30;\n"; // share with all 

		s += "out vec4 outputColor;\n";
		return s;
	}
	// for non file based json, ie internal shaders. bugbug add way to load from file some day too
	Json::Value buildCodeJson(const string& name, const string&fragment, const string&vertex) {
		Json::Value val;

		val["name"] = name;
		if (fragment.size() > 0) {
			val["fragment"] = fragment;
		}
		if (vertex.size() > 0) {
			val["vertex"] = vertex;
		}
		return val;
	}
	// data driven
	Json::Value buildCodeJson(const string& name) {
		Json::Value val;

		val["name"] = name;
		if (name == "zigzag") {
			buildCodeJson(name, zigzag(true), zigzag(false));
		}
		return val;
	}

	bool Shader::mysetup(const Json::Value &val) {

		string fragment;
		string vertex;
		Json::Value::Members m = val.getMemberNames();//here for debug
		string n = val["name"].asString();
		ofLogNotice("Shader::getShader") << val["name"];

		if (val["name"] == "zigzag") {
			fragment = zigzag(true);
			vertex = zigzag(false);
		}
		else if (val["name"] == "basic") {
			fragment = basic(true);
			vertex = basic(false);
		}
		else if (val["name"] == "sea") {
			fragment = sea(true);
			vertex = sea(false);
		}
		else if (val["name"] == "green") {
			fragment = green(true);
			vertex = green(false);
		}
		else if (val["name"] == "grid") {
			fragment = grid(true);
			vertex = grid(false);
		}
		else if (val["name"] == "mosaic") {
			fragment = mosaic(true);
			vertex = mosaic(false);
		}
		else if (val["name"] == "greenCircle") {
			fragment = greenCircle(true);
			vertex = greenCircle(false);
		}
		else if (val["name"] == "smooth") {
			fragment = smooth(true);
			vertex = smooth(false);
		}
		else if (val["name"] == "groovy") {
			fragment = groovy(true);
			vertex = groovy(false);
		}
		else if (val["name"] == "red") {
			fragment = red(true);
			vertex = red(false);
		}
		else if (val["name"] == "userdefined") {
			if (val["fragment"].asString().size() > 0) {
				fragment = Shader::codeHeader() + val["fragment"].asString();
			}
			vertex = val["vertex"].asString(); // bugbug do we need to check size first?
		}
		if (vertex.size() > 0) {
			shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		}
		if (fragment.size() > 0) {
			shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
		}
		if (vertex.size() > 0 || fragment.size() > 0) {
			shader.bindDefaults();
			shader.linkProgram();
		}
		return shader.isLoaded();


	}
	void Shader::startDrawing() {
		shader.begin();
		// true for all our shaders (from https://thebookofshaders.com)
		shader.setUniform1f("u_kick", KICK);
		shader.setUniform1f("u_snare", SNARE);
		shader.setUniform1f("u_hihat", HIHAT);
		shader.setUniform1f("u_hihat", HIHAT);
		shader.setUniform1f("u_mag", HIHAT);
		shader.setUniform1f("u_time", ofGetElapsedTimef());
		shader.setUniform2f("u_resolution", ofGetWidth(), ofGetHeight());
		shader.setUniform1f("u_0", BAND(0));
		shader.setUniform1f("u_5", BAND(5));
		shader.setUniform1f("u_10", BAND(10));
		shader.setUniform1f("u_15", BAND(15));
		shader.setUniform1f("u_20", BAND(20));
		shader.setUniform1f("u_30", BAND(30));
		//bugbug add kinect stuff, voice stuff go beyond mouse
		shader.setUniform2f("u_mouse", ((ofApp*)ofGetAppPtr())->mouseX, ((ofApp*)ofGetAppPtr())->mouseY);
	}
	void Shader::myDraw() {
		startDrawing();
		ofDrawRectangle(-ofGetWidth() / 2, -ofGetHeight() / 2, ofGetWidth(), ofGetHeight());
		shader.end();
	}
	//http://glslsandbox.com/e#32867.0
	string groovy(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
			void main(void)			{

				vec2 uv = (gl_FragCoord.xy / u_resolution.xy)*4.0;

				vec2 uv0 = uv;
				float i0 = 1.0;
				float i1 = 1.0;
				float i2 = 1.0;
				float i4 = 0.0;
				for (int s = 0; s < 7; s++)				{
					vec2 r;
					r = vec2(cos(uv.y*i0 - i4 + u_time / i1), sin(uv.x*i0 - i4/ i1)) / i2;
					r += vec2(-r.y, r.x)*0.3;
					uv.xy += r;

					i0 *= 1.93;
					i1 *= 1.15;
					i2 *= 1.7;
					i4 += 0.05 + 0.1*u_time*i1;
				}
				float r = sin(uv.x - u_time)*0.5 + 0.5;
				float b = sin(uv.y + u_time)*0.5 + 0.5;
				float g = sin((sqrt(uv.x*uv.x + uv.y*uv.y) + u_5))*0.5 + 0.5;
				vec3 c = vec3(r, g, b);
				outputColor = vec4(c, 1.0);

				outputColor *= vec4(c - 0.5*sqrt(uv.x*uv.x + uv.y*uv.y), 1.0);
				outputColor *= vec4(c - 1.0*sqrt(uv.x*uv.x + uv.y*uv.y), 1.0);
			}
			);
		}
		else {
			return "";
		}
	}
	// Author unknown but from http://glslsandbox.com/e#32842.2
	// Title: smooth
	string smooth(bool fragment) {
		string frag = Shader::codeHeader();
		if (fragment) {
		
			return frag + STRINGIFY(
			void main(void)			{
				vec2 uv = (gl_FragCoord.xy / u_resolution.xy)*4.0;

				vec2 uv0 = uv ;
				float i0 = 1.2+u_10;
				float i1 = 0.95;
				float i2 = 1.5;
				vec2 i4 = vec2(0.0, 0.0);
				for (int s = 0; s < 4; s++)				{
					vec2 r;
					r = vec2(cos(uv.y*i0 - i4.y + u_mag / i1), sin(uv.x*i0 + i4.x + u_mag / i1)) / i2;
					r += vec2(-r.y, r.x)*0.2;
					uv.xy += r+u_15;

					i0 *= 1.93;
					i1 *= 1.25;
					i2 *= 1.7;
					i4 += r.xy*1.0 + 0.5*u_time*i1;
				}
				float r = sin(uv.x - u_5)*0.5 + 0.5;
				float b = sin(uv.y + u_30)*0.5 + 0.5;
				float g = sin((sqrt(uv.x*uv.x + uv.y*uv.y) + u_15))*0.5 + 0.5;
				vec3 c = vec3(r, g, b);
				outputColor = vec4(c, 1.0);
			}
			);
		}
		else {
			return frag + STRINGIFY(

				uniform mat4 modelViewProjectionMatrix;

			in vec4 position;

			void main() {
				gl_Position = modelViewProjectionMatrix * position;
			}
			);
		}

	}
	string shadertemplate(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
				hi
			);
		}
		else {
			return "";// return vector
		}

	}
	// http://glslsandbox.com/e#32904.1
	string greenCircle(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
				void main(void) {
				vec2 uv = (2.0 * gl_FragCoord.xy / u_resolution.xy - 1.0) * vec2(u_resolution.x / u_resolution.y, 1.0);
				float a = 2.0*atan(uv.y / uv.x);
				uv /= 02.5 + 0.02 * sin(15.0 * a - u_time * 1889999.0);

				uv *= u_mag;

				float f = 0.40 + 0.2 * sin(u_time * 0.14);
				float d = (abs(length(uv) - f) * 180.0);

				outputColor += vec4(0.989 / d, 2.2 / d, 0.22 / d, 1); 
			}
			);
		}
		else {
			return "";// return vector
		}

	}
	// http://glslsandbox.com/e#32783.0
	string sea(bool fragment) {
		// "Seascape" by Alexander Alekseev aka TDM - 2014
		// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
		if (fragment) {
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
			const int NUM_STEPS = 16; \n
			const float PI = 3.1415;\n
			const float EPSILON = 1e-3;\n
			float EPSILON_NRM = 0.; \n

			const int ITER_GEOMETRY = 3; \n
			const int ITER_FRAGMENT = 5; \n
			const float SEA_HEIGHT = 0.6; \n
			const float SEA_CHOPPY = 5.0; \n
			const float SEA_SPEED = 1.0; \n
			const float SEA_FREQ = 0.16; \n
			const vec3 SEA_BASE = vec3(0.1, 0.19, 0.22); \n
			const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6); \n
			float SEA_TIME = 0.0;\n
			);
			frag += STRINGIFY(
				mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6); \n
				mat3 fromEuler(vec3 ang) {\n
				vec2 a1 = vec2(sin(ang.x), cos(ang.x)); \n
				vec2 a2 = vec2(sin(ang.y), cos(ang.y)); \n
				vec2 a3 = vec2(sin(ang.z), cos(ang.z)); \n
				mat3 m; \n
				m[0] = vec3(a1.y*a3.y + a1.x*a2.x*a3.x, a1.y*a2.x*a3.x + a3.y*a1.x, -a2.y*a3.x); \n
				m[1] = vec3(-a2.y*a1.x, a1.y*a2.y, a2.x); \n
				m[2] = vec3(a3.y*a1.x*a2.x + a1.y*a3.x, a1.x*a3.x - a1.y*a3.y*a2.x, a2.y*a3.y); \n
				return m; \n
				}\n
				float hash(vec2 p) {				\n
					float h = dot(p, vec2(127.1, 311.7)); \n
					return fract(sin(h)*43758.5453123); \n
			}\n
				float noise(in vec2 p) {					\n
						vec2 i = floor(p); \n
						vec2 f = fract(p); \n
						vec2 u = f*f*(3.0 - 2.0*f); \n
						return -1.0 + 2.0*mix(mix(hash(i + vec2(0.0, 0.0)), \n
							hash(i + vec2(1.0, 0.0)), u.x), \n
							mix(hash(i + vec2(0.0, 1.0)), \n
								hash(i + vec2(1.0, 1.0)), u.x), u.y); \n
				}\n
			float diffuse(vec3 n, vec3 l, float p) {				\n
					return pow(dot(n, l) * 0.4 + 0.6, p); \n
			}\n
				float specular(vec3 n, vec3 l, vec3 e, float s) {					\n
						float nrm = (s + 8.0) / (3.1415 * 8.0); \n
						return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm; \n
				}\n
					vec3 getSkyColor(vec3 e) {						\n
							e.y = max(e.y, 0.0); \n
							vec3 ret; \n
							ret.x = pow(1.0 - e.y, 2.0); \n
							ret.y = 1.0 - e.y; \n
							ret.z = 0.6 + (1.0 - e.y)*0.4; \n
							return ret; \n
					}\n
						float sea_octave(vec2 uv, float choppy) {							\n
								uv += noise(uv); \n
								vec2 wv = 1.0 - abs(sin(uv)); \n
								vec2 swv = abs(cos(uv)); \n
								wv = mix(wv, swv, wv); \n
								return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy); \n
						}\n
							);
			frag += STRINGIFY(
				float map(vec3 p) {				\n
					float freq = SEA_FREQ; \n
					float amp = SEA_HEIGHT+u_mag; \n
					float choppy = SEA_CHOPPY+u_10; \n
					vec2 uv = p.xz; uv.x *= 0.75; \n
					float d; float h = 0.0; \n
					for (int i = 0; i < ITER_GEOMETRY; i++) {
						\n
							d = sea_octave((uv + SEA_TIME)*freq, choppy); \n
							d += sea_octave((uv - SEA_TIME)*freq, choppy); \n
							h += d * amp; \n
							uv *= octave_m; freq *= 1.9; amp *= 0.22; \n
							choppy = mix(choppy, 1.0, 0.2); \n
					}\n
						return p.y - h; \n
			}\n
				float map_detailed(vec3 p) {		\n
								float freq = SEA_FREQ; \n
								float amp = SEA_HEIGHT + u_mag; \n
								float choppy = SEA_CHOPPY + u_10; \n
								vec2 uv = p.xz; uv.x *= 0.75; \n
								float d;\n float h = 0.0; \n
				for (int i = 0; i < ITER_FRAGMENT; i++) {\n
					d = sea_octave((uv + SEA_TIME)*freq, choppy); \n
					d += sea_octave((uv - SEA_TIME)*freq, choppy); \n
					h += d * amp; \n
					uv *= octave_m; freq *= 1.9; amp *= 0.22; \n
					choppy = mix(choppy, 1.0, 0.2); \n
				}\n
				return p.y - h; \n
			}\n
			vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {\n
				float fresnel = 1.0 - max(dot(n, -eye), 0.0); \n
				fresnel = pow(fresnel, 3.0) * 0.65; \n
				vec3 reflected = getSkyColor(reflect(eye, n)); \n
				vec3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12; \n
				vec3 color = mix(refracted, reflected, fresnel); \n
				float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0); \n
				color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT + u_mag) * 0.18 * atten; \n
				color += vec3(specular(n, l, eye, 60.0)); \n
				return color; \n
			}\n
			vec3 getNormal(vec3 p, float eps) {\n
				vec3 n; \n
				n.y = map_detailed(p); \n
				n.x = map_detailed(vec3(p.x + eps, p.y, p.z)) - n.y; \n
				n.z = map_detailed(vec3(p.x, p.y, p.z + eps)) - n.y; \n
				n.y = eps; \n
				return normalize(n); \n
			}\n
			float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {\n
				float tm = 0.0; \n
				float tx = 1000.0; \n
				float hx = map(ori + dir * tx); \n
				if (hx > 0.0) return tx; \n
				float hm = map(ori + dir * tm); \n
				float tmid = 0.0; \n
				for (int i = 0; i < NUM_STEPS; i++) {\n
					tmid = mix(tm, tx, hm / (hm - hx)); \n
					p = ori + dir * tmid;
					float hmid = map(p); \n
					if (hmid < 0.0) {\n
						tx = tmid; \n
						hx = hmid; \n
					}\n
					else {\n
						tm = tmid; \n
						hm = hmid; \n
					}\n
				}\n
				return tmid;\n
			}\n
			);
			frag += STRINGIFY(
				\nvoid main(void) {\n
			EPSILON_NRM = 0.1 / u_resolution.x; \n
			SEA_TIME = u_time * SEA_SPEED; \n

			vec2 uv = gl_FragCoord.xy / u_resolution.xy;
			uv = uv * 2.0 - 1.0;
			uv.x *= u_resolution.x / u_resolution.y;
			float time = u_time * 0.3 + u_mouse.x*0.01;

			vec3 ang = vec3(3);
			vec3 ori = vec3(u_mouse.x*100.0, 3.5, 5.0);
			vec3 dir = normalize(vec3(uv.xy, -2.0));
			dir.z += length(uv) * 0.15;
			dir = normalize(dir) * fromEuler(ang);

			vec3 p;
			heightMapTracing(ori, dir, p);
			vec3 dist = p - ori;
			vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
			vec3 light = normalize(vec3(0.0, 1.0, 0.8));

			vec3 color = mix(
				getSkyColor(dir),
				getSeaColor(p, n, light, dir, dist),
				pow(smoothstep(0.0, -0.05, dir.y), 0.3));

			outputColor = vec4(pow(color, vec3(0.75)), 1.0);
			}
				);
			return frag;
		}
		else {
			return "";
		}

	}
	// http://glslsandbox.com/e#32902.0
	string red(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(
				void main(void) {\n
				vec2 uv = (2.0 * gl_FragCoord.xy / u_resolution.xy - 1.0) * vec2(u_resolution.x / u_resolution.y, 9.50); \n

				float a = 71.90*atan(uv.y / uv.x); \n
				uv /= 0.5 + 0.202 * sin(115.0 * a - u_time * 16.0); \n

				float f2 = .60 * sin(u_time * 4786.14); \n
				float d = (abs(length(uv) - f2) * 1.0); \n
					
				outputColor += vec4(9.3 / d + u_hihat, 0.62*u_mag / d, 0.22+u_15 / d, 1); \n

				}
			);
		}
		else {
			return "";
		}

	}
	// Author @patriciogv - 2015
	// Title: Mosaic
	string mosaic(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(

			float random(vec2 st) {
				return fract(sin(dot(st.xy,
					vec2(12.9898, 78.233)))*
					43758.5453123);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;

				st *= 10.0; // Scale the coordinate system by 10*u_mag
				vec2 ipos = floor(st);  // get the integer coords
				vec2 fpos = fract(st);  // get the fractional coords

										// Assign a random value based on the integer coord
				vec3 color = vec3(random(ipos));
				//color *= u_mag;
				// Uncomment to see the subdivided grid
				color = vec3(fpos,0.0);
				outputColor = vec4(color, 1.0);

			}
			);
		}
		else {
			return "";
		}
	}
	// for now all shaders start here, but in phase 1b people can add them in via json, once more is understood how things will work in this regard
	// so there is a name in the shader json, but that can become UserDefined bugbug
	string basic(bool fragment) {
		if (fragment) {
			string frag = Shader::codeHeader();
			return frag + STRINGIFY(

				void main() {
				float r = gl_FragCoord.x / u_resolution.x;
				float g = gl_FragCoord.y / u_resolution.y;
				outputColor = vec4(r+u_20, g+u_0, u_kick, u_mag);
			}
			);
		}
		else {
			return "";
		}
	}
	// Author @patriciogv - 2015
	// http://patriciogonzalezvivo.com
	string green(bool fragment) {
		if (fragment) {

			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				float plot(vec2 _st, float _pct) {
				return  smoothstep(_pct - 0.01, _pct, _st.y) -
					smoothstep(_pct, _pct + 0.01+ u_15, _st.y);
			}

			float random(in float _x) {
				return fract(sin(_x)*43758.5453);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;
				st.x *= u_resolution.x / u_resolution.y;
				vec3 color = vec3(0.0);

				float y = random(st.x*0.001 + u_5);

				// color = vec3(y);
				float pct = plot(st, y);
				color = (1.0 - pct)*color + pct*vec3(u_0, u_mag, u_30);

				outputColor = vec4(color, 1.0);
			}
			);
			return frag;
		}
		else {
			return "";
		}
	}
	// Author @patriciogv - 2015
	// Title: Ikeda Numered Grid
	string grid(bool fragment) {
		if (fragment) {
			//bugbug define json, include a reference tag 
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				float random(in float x) { return fract(sin(x)*43758.5453); }\n
				// float random(in vec2 st){ return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453); }
				float random(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }\n

				float bin(vec2 ipos, float n) {\n
					float remain = mod(n, 33554432.); \n
					for (float i = 0.0; i < 25.0; i++) {\n
						if (floor(i / 3.0) == ipos.y && mod(i, 3.0) == ipos.x) {\n
							return step(1.0, mod(remain, 2.0)); \n
						}\n
						remain = ceil(remain / 2.0);\n
					}
					return 0.0; \n
				}\n
				float charX(vec2 st, float n) {
					st.x = st.x*2. - 0.5; \n
					st.y = st.y*1.2 - 0.1; \n

					vec2 grid = vec2(3.0, 5.0); \n

					vec2 ipos = floor(st*grid); \n
					vec2 fpos = fract(st*grid); \n

					n = floor(mod(n, 10.0)); \n
					float digit = 0.0; \n
					if (n < 1.0) { digit = 31600.0; }
					else if (n < 2.0) { digit = 9363.0; }
					else if (n < 3.0) { digit = 31184.0; }
					else if (n < 4.0) { digit = 31208.0; }
					else if (n < 5.0) { digit = 23525.0; }
					else if (n < 6.0) { digit = 29672.0; }
					else if (n < 7.0) { digit = 29680.0; }
					else if (n < 8.0) { digit = 31013.0; }
					else if (n < 9.0) { digit = 31728.0; }
					else if (n < 10.0) { digit = 31717.0; }
					float pct = bin(ipos, digit); \n

					vec2 borders = vec2(1.0); \n
					// borders *= step(0.01,fpos.x) * step(0.01,fpos.y);   // inner
					borders *= step(0.0, st)*step(0.0, 1.0 - st);            // outer

					return step(0.5, 1.0 - pct) * borders.x * borders.y; \n
				}

				float grid(vec2 st, float res) {\n
					vec2 grid = fract(st*res); \n
					return 1.0 - (step(res, grid.x) * step(res, grid.y)); \n
				}

				float box(in vec2 st, in vec2 size) {
					size = vec2(0.5) - size*0.5; \n
					vec2 uv = smoothstep(size, size + vec2(0.001), st); \n
					uv *= smoothstep(size,size + vec2(0.001),vec2(1.0) - st);
					return uv.x*uv.y;\n
				}

				float cross(in vec2 st, vec2 size) {
					return  clamp(box(st, vec2(size.x*0.5, size.y*0.125)) +	box(st, vec2(size.y*0.125, size.x*0.5)), 0.0, 1.0);\n
				}

				void main() {
					vec2 st = gl_FragCoord.st / u_resolution.xy;\n
					st.x *= u_resolution.x / u_resolution.y;\n
					vec3 color = vec3(0.0);\n
					// Grid
					vec2 grid_st = st*300.0; \n
					color += vec3(0.5, 0.0, 0.0)*grid(grid_st, 0.01); \n
					color += vec3(0.2, 0.0, 0.0)*grid(grid_st, 0.02); \n
					color += vec3(0.2)*grid(grid_st, 0.1); \n

					// Crosses
					vec2 crosses_st = st + 0.5; \n
					crosses_st *= 3.0; \n
					vec2 crosses_st_f = fract(crosses_st); \n
					color *= 1. - cross(crosses_st_f, vec2(0.3, 0.3)); \n
					color += vec3(.9)*cross(crosses_st_f, vec2(0.2, 0.2)); \n

					// Digits
					vec2 digits_st = mod(st*60.0, 20.0); \n
					vec2 digits_st_i = floor(digits_st); \n
					if (digits_st_i.y == 1.0 &&	digits_st_i.x > 0.0 && digits_st_i.x < 6.0) {
						vec2 digits_st_f = fract(digits_st); \n
						float pct = random(digits_st_i + floor(crosses_st) + floor(u_time*20.0));\n
						color += vec3(charX(digits_st_f, 100.0*pct));\n
					}
					else if (digits_st_i.y == 2.0 && digits_st_i.x > 0.0 && digits_st_i.x < 8.0) {
						vec2 digits_st_f = fract(digits_st); \n
						float pct = random(digits_st_i + floor(crosses_st) + floor(u_time*20.0)); \n
						color += vec3(charX(digits_st_f, 100.0*pct)); \n
					}
					outputColor = vec4(color, 1.0);\n
				}
			);
			return frag;
		}
		else {
			return "";
		}

	}
	// return json string for Zigzag
	// Author @patriciogv - 2015
	// Title: Zigzag
	string zigzag(bool fragment) {
		if (fragment) {
			//bugbug define json, include a reference tag 
			string frag = Shader::codeHeader();
			frag += STRINGIFY(
				vec2 mirrorTile(vec2 _st, float _zoom) {
				_st *= _zoom;
				if (fract(_st.y * 0.5) > 0.5) {
					_st.x = _st.x + 0.5;
					_st.y = 1.0 - _st.y;
				}
				return fract(_st);
			}

			float fillY(vec2 _st, float _pct, float _antia) {
				return  smoothstep(_pct - _antia, _pct, _st.y);
			}

			void main() {
				vec2 st = gl_FragCoord.xy / u_resolution.xy;
				vec3 color = vec3(0.0);

				st = mirrorTile(st*vec2(1., 2.), 5.);
				float x = st.x*2.;
				float a = floor(1. + sin(x*3.14));
				float b = floor(1. + sin((x + 1.)*3.14));
				float f = fract(x);

				color = vec3(fillY(st, mix(a, b, f), 0.01));

				outputColor = vec4(color, 1.0);
			}
		);
			return frag;
		}
		else {
			return "";
		}
	}


}