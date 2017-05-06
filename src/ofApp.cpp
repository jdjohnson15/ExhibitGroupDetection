#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    std::ifstream configFile;
    chdir("../../../data");
    configFile.open("config.config");
    if (configFile.is_open())
        std::cout<<"the config file loaded successfully"<<std::endl;
    else
        std::cout<<"the config file failed load"<<std::endl;
    
    char current_working_dir[1024];
    getcwd( current_working_dir, 1024 );
    chdir("../../../Iceline_ofDebug.app/Contents/Resources");
    printf("cwd is %s\n", current_working_dir );
    string temp;
    int i = 0;
    bool videosetFlag = false;
    
    while (getline(configFile, temp)){
        if (temp[0] != '#'){
            if (temp.compare("list B") == 0){
                getline(configFile, temp);
                videosetFlag = true;
            }
            if (videosetFlag){
                std::cout<<"loading "<<temp<<" into list B"<<endl;
                videosB[iVidsB].load(temp);
                videosB[iVidsB].setLoopState(OF_LOOP_NORMAL);
                videosB[iVidsB].play();
                videosB[iVidsB++].setVolume(0);
                numVidsB++;
            }
            else{
                if (temp.compare("list A") == 0){
                    getline(configFile, temp);
                    videosetFlag = false;
                }
                std::cout<<"loading "<<temp<<" into list A"<<endl;
                videosA[iVidsA].load(temp);
                videosA[iVidsA].setLoopState(OF_LOOP_NORMAL);
                videosA[iVidsA].play();
                videosA[iVidsA++].setVolume(0);
                numVidsA++;
            }
        }
    }
    iVidsA = 0;
    iVidsB = 0;
    
    ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::update(){

    //update current and next video
    videosA[iVidsA].update();
    iVidsA + 1 >= numVidsA ? videosA[0].update() : videosA[iVidsA + 1].update();
        
    videosB[iVidsB].update();
    iVidsB + 1 >= numVidsB ? videosB[0].update() : videosB[iVidsB + 1].update();
    
    videosA[iVidsA].setVolume(alpha);
    videosB[iVidsB].setVolume(1.0 - alpha);

    if (changeVideoB){
        
        videosB[iVidsB].setVolume(0);
        if (++iVidsB >= numVidsB)
            iVidsB = 0;
        
        changeVideoB = false;
        changeVideoA = true;
    }
    if (changeVideoA){
        if (alpha == 0.0){
            videosA[iVidsA].setVolume(0);
            if (++iVidsA >= numVidsA)
              iVidsA = 0;
            changeVideoA = false;
        }
        
    }

}

//--------------------------------------------------------------
void ofApp::draw(){

    //play both videos
    ofSetColor(255,255,255,255);
    ofDrawRectangle(0, 0, width, height);
    videosA[iVidsA].draw(0,0,width, height);
    ofSetColor(255,255,255,255 - alpha*255);
    videosB[iVidsB].draw(0,0,width, height);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::setAlpha(double a){
    alpha = a;
}

void ofApp::signalChange(){
    changeVideoB = true;
}
