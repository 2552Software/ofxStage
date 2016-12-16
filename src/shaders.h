#pragma once

// shader helpers

namespace Software2552 {
	string zigzag(bool fragment=true); // shader functions
	string basic(bool fragment = true);
	string digits(bool fragment = true);
	string green(bool fragment = true);
	string grid(bool fragment = true);
	string mosaic(bool fragment = true);
	string smooth(bool fragment = true);
	string groovy(bool fragment = true);
	string red(bool fragment = true);
	string greenCircle(bool fragment = true);
	string sea(bool fragment = true);

	Json::Value buildCodeJson(const string& name, const string&fragment, const string&vertex);
	Json::Value buildCodeJson(const string& name);
		
	class Shader : public ActorRole {
	public:
		virtual void myDraw();
		static string codeHeader();
	private:
		ofShader shader;
		void startDrawing();
		bool mysetup(const Json::Value &val);
	};
}
