//
//  ofGui.hpp
//  Iceline_of
//
//  Created by Jesse Johnson on 1/28/17.
//
//
#pragma once
#ifndef ofGui_h
#define ofGui_h

#include "ofMain.h"
#include "ofxGui.h"
#include <fstream>
#include <iostream>

class ofGui : public ofBaseApp{
    
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
    
    int getSensitivity();
    int getBlur();
    int getBWThresh();
    int getTSpeed();
    int getHotSpotSize();
    int getMoveTime();
    bool getReset();
    void setReset(bool b);
    bool getCameraActive(int i);
    void setCameraCount(int c);

    ofxPanel     gui;
    ofxIntSlider sSensitivity;
    ofxIntSlider sBlur;
    ofxIntSlider sBWThresh;
    ofxIntSlider sTSpeed;
    ofxIntSlider sHotSpotSize;
    ofxIntSlider sMoveTime;
    ofxButton    bReferenceReset;
    ofxToggle    cameraToggle[10];
    
    int cameraCount = 0;
    int oldCameraCount = 0;
    
    int blur = 1;
    int sensitivity;
    int bwThresh;
    int tSpeed;
    int hotSpotSize;
    int moveTime;
    bool referenceReset = false;

int width = 1280;
int height = 720;
};
#endif /* ofGui_hpp */
