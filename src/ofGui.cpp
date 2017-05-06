#include "ofGui.h"

//--------------------------------------------------------------
void ofGui::setup(){
    
    gui.setup();
    gui.add(sSensitivity.setup("sensitivity", 1, 1, 500));
    gui.add(sBlur.setup("blur", 50, 1, 500));
    gui.add(sBWThresh.setup("contrast threshold", 140, 1, 500));
    gui.add(sTSpeed.setup("transition speed", 140, 1, 500));
    gui.add(sHotSpotSize.setup("hotspot size", 140, 1, 500));
    gui.add(sMoveTime.setup("time before move", 140, 11, 500));
    gui.add(bReferenceReset.setup("Reset Reference"));


}

//--------------------------------------------------------------
void ofGui::update(){
    sensitivity = sSensitivity;
    blur = sBlur;
    bwThresh = sBWThresh;
    tSpeed = sTSpeed;
    hotSpotSize = sHotSpotSize;
    moveTime = sMoveTime;
    if (bReferenceReset)
        referenceReset = true;
    //referenceReset = sMoveTime;
    if (cameraCount != oldCameraCount){
        for (int i = 0; i < cameraCount; i++){
            gui.add(cameraToggle[i].setup("Camera "+to_string(i+1), true));
            oldCameraCount = cameraCount;
        }
    }
}

//--------------------------------------------------------------
void ofGui::draw(){
    
    ofSetColor(100,100,100,255);
    ofDrawRectangle(0, 0, width, height);
    gui.draw();

}

//--------------------------------------------------------------
void ofGui::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofGui::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofGui::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofGui::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofGui::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofGui::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofGui::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofGui::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofGui::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofGui::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofGui::dragEvent(ofDragInfo dragInfo){
    
}

int ofGui::getSensitivity(){
    return sensitivity;
}
int ofGui::getBlur(){
    return blur;
}
int ofGui::getBWThresh(){
    return bwThresh;
}
int ofGui::getTSpeed(){
    return tSpeed;
}
int ofGui::getHotSpotSize(){
    return hotSpotSize;
}
int ofGui::getMoveTime(){
    return moveTime;
}

bool ofGui::getReset(){
    return referenceReset;
}

void ofGui::setReset(bool b){
    referenceReset = b;
}

bool ofGui::getCameraActive(int i){
    return cameraToggle[i];
}

void ofGui::setCameraCount(int c){
    cameraCount = c;
}


