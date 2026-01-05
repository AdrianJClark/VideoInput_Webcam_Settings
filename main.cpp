#include <opencv\cv.h>
#include <opencv\highgui.h>

#include "videoinput.h"
#include "tinyxml.h"

bool swapColour = true;
bool flipImage = true;

int deviceNumber = 0;
int deviceWidth = 640;
int deviceHeight = 480;

bool capandexit=false;

using namespace std;
void readXML(string filename);
void writeXML(string filename);

void readStartup(string filename);

videoInput *vi;
void main() {
	vi = new videoInput();
	vi->listDevices();
	readStartup("startup.xml");

	vi->setupDevice(deviceNumber, deviceWidth, deviceHeight);
	readXML("settings.xml");
	
	if (!capandexit) vi->showSettingsWindow(deviceNumber);

	bool running = true;
	while (running) {
		IplImage *myImage = cvCreateImage(cvSize(deviceWidth,deviceHeight), IPL_DEPTH_8U, 3);

		myImage->imageData = (char*)vi->getPixels(deviceNumber, !swapColour, flipImage);

		if (capandexit) {
			long i=0; char s[50]; 
			while (true) {
				sprintf(s, "%ld.jpg", i++);
				if (fopen(s, "r")==NULL) { 	
					cvSaveImage(s, myImage); cvReleaseImage(&myImage); delete vi; return;
				}
			}
		}

		cvShowImage("Frame", myImage);
		
		switch (cvWaitKey(1)) {
			case 27:
				running = false; break;
			case 's':
				vi->showSettingsWindow(deviceNumber); break;
			case 13:
				writeXML("settings.xml"); break;

		};

		cvReleaseImage(&myImage);

	}
	delete vi;
}

void readStartup(string filename) {
	TiXmlDocument doc(filename.c_str());
	if (!doc.LoadFile()) return;

	TiXmlElement* setting = doc.FirstChildElement();

	if (setting->Attribute("device")) deviceNumber = atoi(setting->Attribute("device"));
	if (setting->Attribute("width")) deviceWidth = atoi(setting->Attribute("width"));
	if (setting->Attribute("height")) deviceHeight = atoi(setting->Attribute("height"));
	if (setting->Attribute("flipimage")) flipImage = atoi(setting->Attribute("flipimage"))==1;
	if (setting->Attribute("swapcolor")) swapColour = atoi(setting->Attribute("swapcolor"))==1;
	if (setting->Attribute("capandexit")) capandexit = atoi(setting->Attribute("capandexit"))==1;

}

void setCameraProperty(TiXmlElement *setting, long _property, bool isFilter) {
	long min, max, steppingDelta, currentValue, flags, defaultval;
	min = atol(setting->Attribute("min"));
	max = atol(setting->Attribute("max"));
	steppingDelta = atol(setting->Attribute("steppingDelta"));
	currentValue = atol(setting->Attribute("currentValue"));
	flags = atol(setting->Attribute("flags"));
	defaultval = atol(setting->Attribute("defaultval"));

	if (min==max && min==steppingDelta && min==currentValue && min==flags && min==defaultval) return;

	if (isFilter) 
		vi->setVideoSettingFilter(deviceNumber, _property, currentValue, flags, false);
	else 
		vi->setVideoSettingCamera(deviceNumber, _property, currentValue, flags, false);
}


void readXML(string filename) {

	TiXmlDocument doc(filename.c_str());
	if (!doc.LoadFile()) return;

	TiXmlElement* setting = doc.FirstChildElement();

	while (setting!=NULL) {
		std::string value(setting->Value());	 
		if (value == "BackLightCompensation") setCameraProperty(setting, vi->propBacklightCompensation, true);
		else if (value == "Brightness") setCameraProperty(setting, vi->propBrightness, true);
		else if (value == "Contrast") setCameraProperty(setting, vi->propContrast, true);
		else if (value == "Gain") setCameraProperty(setting, vi->propGain, true);
		else if (value == "Gamma") setCameraProperty(setting, vi->propGamma, true);
		else if (value == "Hue") setCameraProperty(setting, vi->propHue, true);
		else if (value == "Saturation") setCameraProperty(setting, vi->propSaturation, true);
		else if (value == "Sharpness") setCameraProperty(setting, vi->propSharpness, true);
		else if (value == "WhiteBalance") setCameraProperty(setting, vi->propWhiteBalance, true);

		else if (value == "ColourEnable") setCameraProperty(setting, vi->propColorEnable, false);
		else if (value == "Exposure") setCameraProperty(setting, vi->propExposure, false);
		else if (value == "Focus") setCameraProperty(setting, vi->propFocus, false);
		else if (value == "Iris") setCameraProperty(setting, vi->propIris, false);
		else if (value == "Pan") setCameraProperty(setting, vi->propPan, false);
		else if (value == "Roll") setCameraProperty(setting, vi->propRoll, false);
		else if (value == "Tilt") setCameraProperty(setting, vi->propTilt, false);
		else if (value == "Zoom") setCameraProperty(setting, vi->propZoom, false);

		setting=setting->NextSiblingElement();
	}

}

void createElement(TiXmlDocument *doc, long _property, string propertyName, bool isFilter) {
	long min, max, steppingDelta, currentValue, flags, defaultval;
	bool success;
	if (isFilter) 
		success = vi->getVideoSettingFilter(deviceNumber, _property, min, max, steppingDelta, currentValue, flags, defaultval);
	else 
		success = vi->getVideoSettingCamera(deviceNumber, _property, min, max, steppingDelta, currentValue, flags, defaultval);

	if (min==max && min==steppingDelta && min==currentValue && min==flags && min==defaultval) return;

	if (success) {
		TiXmlElement *tElem = new TiXmlElement(propertyName.c_str());
		stringstream st;
		st << min; tElem->SetAttribute("min", st.str().c_str()); st.str("");
		st << max; tElem->SetAttribute("max", st.str().c_str()); st.str("");
		st << steppingDelta; tElem->SetAttribute("steppingDelta", st.str().c_str()); st.str("");
		st << currentValue; tElem->SetAttribute("currentValue", st.str().c_str()); st.str("");
		st << flags; tElem->SetAttribute("flags", st.str().c_str()); st.str("");
		st << defaultval; tElem->SetAttribute("defaultval", st.str().c_str()); st.str("");
		doc->LinkEndChild(tElem);
	}
}

void writeXML(string filename) {
	TiXmlDocument doc(filename.c_str());

	{
		/*FILTERS
		vi->propBacklightCompensation;
		vi->propBrightness;
		vi->propContrast;
		vi->propGain;
		vi->propGamma;
		vi->propHue;
		vi->propSaturation;
		vi->propSharpness;
		vi->propWhiteBalance;
		*/
		createElement(&doc, vi->propBacklightCompensation, "BackLightCompensation", true);
		createElement(&doc, vi->propBrightness, "Brightness", true);
		createElement(&doc, vi->propContrast, "Contrast", true);
		createElement(&doc, vi->propGain, "Gain", true);
		createElement(&doc, vi->propGamma, "Gamma", true);
		createElement(&doc, vi->propHue, "Hue", true);
		createElement(&doc, vi->propSaturation, "Saturation", true);
		createElement(&doc, vi->propSharpness, "Sharpness", true);
		createElement(&doc, vi->propWhiteBalance, "WhiteBalance", true);
	}

	{
		/*CAMERA SETTINGS
		vi->propColorEnable;
		vi->propExposure;
		vi->propFocus;
		vi->propIris;
		vi->propPan;
		vi->propRoll;
		vi->propTilt;
		vi->propZoom;
		*/

		createElement(&doc, vi->propColorEnable, "ColourEnable", false);
		createElement(&doc, vi->propExposure, "Exposure", false);
		createElement(&doc, vi->propFocus, "Focus", false);
		createElement(&doc, vi->propIris, "Iris", false);
		createElement(&doc, vi->propPan, "Pan", false);
		createElement(&doc, vi->propRoll, "Roll", false);
		createElement(&doc, vi->propTilt, "Tilt", false);
		createElement(&doc, vi->propZoom, "Zoom", false);
	}

	doc.SaveFile();
	
}
