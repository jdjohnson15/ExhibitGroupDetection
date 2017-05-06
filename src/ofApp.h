#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include <fstream>
#include <iostream>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void setAlpha(double a);
        void signalChange();
    
        bool changeVideoA = false;
        bool changeVideoB = false;
    
        ofVideoPlayer 		videosA[10];
        ofVideoPlayer 		videosB[10];
        int iVidsA = 0;
        int iVidsB = 0;
        int numVidsA = 0;
        int numVidsB = 0;

        double alpha = 0;
    
        //ofxIntSlider =
    
    int width = 1280;
    int height = 720;
    
		
};
