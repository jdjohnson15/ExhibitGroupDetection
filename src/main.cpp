#include "ofMain.h"
#include "ofApp.h"
#include "ofGui.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <thread>
#include <sys/sysctl.h>

//constants
const int MAX_CAMERAS = 10; //reasonable number of webcams available

////namespaces
using namespace cv;
using namespace std;

//constants
const int MAX_WIDTH =  640;
const int MAX_HEIGHT = 480;

//structures
struct Camera{
    string name;
    bool active; //is this camera in use?
    Mat reference; //what frame should look like if space is empty
    double fill; //ratio between occupied and non-occupied space
    int keypointsSize; //number of people (or objects) in scene
    int activePoint; //which target box corner is being changed
    //Point rect1, rect2; //corners of traget box
    bool resizing; //whether target box is being changed or not
    
    void updateFill(Mat img){
        double filled;
        for (int i = 0; i < MAX_WIDTH; i++){
            for (int j = 0; j < MAX_HEIGHT; j++){
                if (img.at<uchar>(cv::Point(i,j)) == 0){
                    filled+=1.0;
                }
            }
        }
        fill = filled/(MAX_WIDTH*MAX_HEIGHT);
    }
    
};

struct Body{
    int ID;// each body gets assigned an ID when created
    cv::Point pos; //most recent spot body was detected moving
    int quant; //assumed number of people in the body (usually one, may be two
};


struct Timer{
    struct timeval startTime, currentTime, stopTime;
    double accumulatedTime; //used for keeping track of time the hotspot is activated: works like a chess clock
    bool started;
    void reset(){
        gettimeofday(&startTime, NULL);
        gettimeofday(&currentTime, NULL);
        gettimeofday(&stopTime, NULL);
        accumulatedTime = 0;
        started = false;
    }
    double updateFromStart(){
        double elapsedTime;
        gettimeofday(&currentTime, NULL);
        elapsedTime = (currentTime.tv_sec - startTime.tv_sec);
        //elapsedTime += (currentTime.tv_usec - startTime.tv_usec) / 1000.0;
        return elapsedTime;
    }
    double update(){
        double elapsedTime;
        gettimeofday(&currentTime, NULL);
        elapsedTime = (currentTime.tv_sec - stopTime.tv_sec);
        //elapsedTime += (currentTime.tv_usec - stopTime.tv_usec) / 1000.0;
        accumulatedTime += elapsedTime;
        return elapsedTime;
    }
    void start(){
        gettimeofday(&stopTime, NULL);
        gettimeofday(&currentTime, NULL);
    }
} timer;

struct Hotspot{
    int cameraID;//what camera the hotspot is currently on. May change to "straddle" multiple camera viewports. For now it does not straddle viewports
    cv::Point box[2];
    cv::Point size;
    //outside parameters. Must be filled by init func
    int x_max;
    int y_max;
    int numCams;
    vector<int> activeCameras;
    double fill; //ratio of box occupied:unoccupied
    void init(int _x, int _y, int _cam){
        size.x = 25;
        size.y = 25;
        x_max = _x;
        y_max = _y;
        for (int i = 0; i < _cam; i++){
            activeCameras.push_back(i);
            cout<<activeCameras[i]<<endl;
        }
        numCams = _cam;
        moveBox();
    }
    void moveBox(){
        cv::Point loc = cv::Point(rand() % (x_max - size.x), rand() % (y_max - size.y));
        box[0] = cv::Point(loc.x, loc.y);
        box[1] = cv::Point(loc.x + size.x, loc.y + size.y);
        
        cameraID = activeCameras[rand() % numCams];
        cout<<"hotspot moved to camera "<<cameraID+1<<endl;
    }
    void resetBox(){
        box[0] = cv::Point(x_max/2 - size.x/2, y_max/2 - size.y/2);
        box[1] = cv::Point(x_max/2 + size.x/2, y_max/2 + size.y/2);
    }
    void updateActiveCams(int n, bool r){
        if (r)
            activeCameras.erase(activeCameras.begin() + n);
        else
            activeCameras.insert(activeCameras.begin() + n, n);
    }
    void resize(int _x, int _y){
        size.x = _x;
        size.y = _y;
        box[1] = cv::Point(box[0].x + size.x, box[0].y + size.y);
    }
    void updateFill(Mat img){
        double filled = 0.0;
        double total = ((double)box[1].x-(double)box[0].x)*((double)box[1].y-(double)box[0].y);
        
        for (int i = box[0].x; i < box[1].x; i++){
            for (int j = box[0].y; j < box[1].y; j++){
                if (img.at<uchar>(cv::Point(i,j)) == 0){
                    filled+=1.0;
                }
            }
        }
        
        fill = filled/total;//(total/(1+exp((-1)*filled))) - (0.5*total);
    }
    
};

Camera camera[MAX_CAMERAS];
int activeCameras[MAX_CAMERAS];
//vector<Body> bodies;

////functions

void performCV();

void playVideo();

void on_trackbar(int, void*);

void on_reset(int, void*);

double checkRectFill(cv::Point p1, cv::Point p2, Mat img);

int detectGroups(double minDist, double singleArea, vector<cv::Point> c, vector<int> q);

double updateOutput(double previous, double current, double rate);

void cameratoggle(int i, bool b);

//void CallBackFunc(int event, int x, int y, int flags, void* userdata);

double dist(cv::Point p1, cv::Point p2);

void resizeTrigger(Camera* c);

int motionTracker(Mat img, Mat& debugImg);

void on_hotspotResize(int, void*);

//working directory
string dir;

//default parametes
VideoCapture cap[MAX_CAMERAS]; //supports up to MAX_CAMERAS (10) cameras
Mat frame;
int cnumber = 0;
int minThresh = 1;
int maxThresh = 50000;
int minArea = 0;
int maxArea = 100000;
int step = 100;
int iBlur = 50;
int minDist = 1000;
int thresh = 10;
int transitionRate = 20;
int hotspotSize = 25;
int hotspotMoveTime = 500; //milliseconds
int sensitivity = 1;

double  avgTotalFill = 0.0,
prevAlpha = 0.0,
currAlpha = 0.0,
activeNumber = 0.0;

Hotspot hotspot;

//for mouse control
cv::Point mousePoint;

bool paramChangeSem = true;

SimpleBlobDetector::Params params;

vector<KeyPoint> keypoints;

SimpleBlobDetector detector;

bool reset = true;

int resetValue = 0;

bool quit = false;

bool debug = false;

//openframeworks pointer
shared_ptr<ofApp> mainApp;
shared_ptr<ofGui> guiApp;

enum Mode {TARGET, TRACKING};
int mode;

// ============ main loop =============

int main(int argc, char * argv[])
{
    dir = getcwd(NULL, 0);
    int ccount = 0;
    
    
    mode = TARGET;
    
    
    
    //find all connected cameras
    while(ccount != -1){
        
        
        cap[ccount].open(ccount);
        if (!cap[ccount].isOpened()) {
            cnumber = ccount;
            ccount = -1;
        }
        else { //init camera info
            int c = ccount + 1;
            camera[ccount].name = "camera " + to_string(c);
            namedWindow(camera[ccount].name, WINDOW_NORMAL);
            
            camera[ccount].active = true;
            camera[ccount].keypointsSize  = 0;
            camera[ccount].resizing = false;
            //setMouseCallback(camera[ccount].name, CallBackFunc, &camera[ccount]);
            
            cnumber = ++ccount;
            
        }
    }
    if (cnumber == 0) {
        cout << "no cameras detected\n";
        
        return -1;
    }
    //================== initialze hot spot and other important values =================
    
    
    
    hotspot.init(MAX_WIDTH, MAX_HEIGHT, cnumber);
    
    
    //================== control panel setup ==================
    //Move all gui stuff to openFrameworks window
    
    /*
    namedWindow("control", WINDOW_NORMAL);
 
    createTrackbar("blur", "control", &iBlur, 100, on_trackbar);

    createTrackbar("b/w thresh", "control", &thresh, 256, on_trackbar);
    
    createTrackbar("transition speed", "control", &transitionRate, 100, on_trackbar);
    
    createTrackbar("hotspot size", "control", &hotspotSize, hotspotSize * 10, on_hotspotResize);
    
    createTrackbar("hotspot move time", "control", &hotspotMoveTime, hotspotMoveTime * 3, on_trackbar);
    
    createTrackbar("manual reset background","control", &resetValue, 1, on_reset);
    
    Mat controlPanel = Mat::zeros(300, 200, CV_WINDOW_NORMAL);*/
    
    cout << cnumber << " cameras detected\n";
    
    
    //================== setup for main output thread ==================
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofGLWindowSettings settings;
    
    //openFrameworks = new ofApp();
    
    settings.width = 1920;//openFrameworks->width;
    settings.height = 1080;//openFrameworks->height;
    settings.setPosition(ofVec2f(300,0));
    auto mainWindow = ofCreateWindow(settings);
    
    
    
    
    //openFrameworksGui = new ofGui();
    settings.width = 300;
    settings.height = 300;
    settings.setPosition(ofVec2f(0,0));
    auto guiWindow = ofCreateWindow(settings);
    
    guiApp = make_shared<ofGui>();
    mainApp = make_shared<ofApp>();
    
    guiApp->setCameraCount(cnumber);
    
    ofRunApp(mainWindow, mainApp);
    ofRunApp(guiWindow, guiApp);
    

    
    //================== setup for GUI and camera output ==================
    
    
    
    
    
    
    
    // ================== main display loop ==================
    
    thread cvThread (performCV);
    //ofRunApp(openFrameworksGui);
    //thread ofThread (ofRunMainLoop);
    ofRunMainLoop();
    
    timer.reset(); //init time right before loop
    
    //shut it all down
    cout<<"shutdown initiated"<<endl;
    //quit = true;
    
    for (int i = 0; i < cnumber; i++){
        cap[i].release();
    }
    cvThread.join();
    //ofThread.join();
    return 0;
}

void performCV(){
    for (;;) {
        if (quit) break;
        //call when reseting the camera's reference image
        if (reset){
            hotspot.resetBox();
            for (int i = 0; i < cnumber; i++){
                Mat temp;
                cap[i]>>temp;
                resize(temp, temp, cv::Size(MAX_WIDTH, MAX_HEIGHT));
                //temp.convertTo(temp, -1, 1.0 + alpha/100.0, beta);
                //cvtColor(temp, temp, CV_BGR2GRAY);
                //blur(temp, temp, cv::Size(1+iBlur,1+iBlur));
                camera[i].reference = temp;
            }
            reset = false;
            guiApp->setReset(false);
        }
        
        //get values from GUI
        sensitivity = guiApp->getSensitivity();
        iBlur = guiApp->getBlur();
        thresh= guiApp->getBWThresh();
        hotspotSize = guiApp->getHotSpotSize();//openFrameworksGui->hotSpotSize;
        hotspotMoveTime = guiApp->getMoveTime();
        transitionRate = guiApp->getTSpeed();
        reset = guiApp->getReset();
        
        hotspot.resize(hotspotSize, hotspotSize);
        
        avgTotalFill = 0.0;
        activeNumber = 0.0;
        for (int i = 0; i < cnumber; i++)
        {
            if (camera[i].active){
                
                cap[i] >> frame;
                
                resize(frame, frame, cv::Size(640, 480));
                
                Mat test;
                
                frame.copyTo(test);
                cvtColor(test, test, CV_BGR2GRAY);
                
                Mat diff;
                absdiff(camera[i].reference, frame, diff);//extract new objects
                
                Scalar color = Scalar( 255, 255, 255 );
                
                blur(diff, diff, cv::Size(1+iBlur,1+iBlur));
                cvtColor(diff, diff, CV_BGR2GRAY);
                diff = diff > thresh;
                bitwise_not(diff, diff);
                rectangle(diff, cv::Point(0,0), cv::Point(frame.cols, frame.rows), color, 8);
                
                
                //get contours of diff array and predict number of people on camera
                
                Mat diffCopy;
                diff.copyTo(diffCopy);
                
                camera[i].updateFill(diffCopy);
                
                //draw hotspot onto selected camera veiwport (chosen at random within the object)
                if (i == hotspot.cameraID){
                    hotspot.updateFill(diffCopy);
                    rectangle(test, hotspot.box[0], hotspot.box[1], Scalar(255,255,255));
                }
                Mat final;
                addWeighted(diffCopy, 0.5, test, 0.5, 0.0, test);
                putText(test,"hot spot fill rato: "+to_string(hotspot.fill), cv::Point(0, 60),1,1,color,2);
                putText(test,"total fill ratio: "+to_string(camera[i].fill), cv::Point(0, 30),1,1,color,2);
                putText(test,"time till box move: "+to_string((hotspotMoveTime - timer.accumulatedTime)), cv::Point(0, 90),1,1,color,2);
                putText(test,"prevalpha: "+to_string(prevAlpha), cv::Point(0, 120),1,1,color,2);
                
                imshow(camera[i].name, test);
                
                avgTotalFill+=camera[i].fill;
                ++activeNumber;
                
            }
        }
        
        //calculate crossfade
        avgTotalFill /= activeNumber;
        
        currAlpha = (avgTotalFill <= hotspot.fill*((double)sensitivity/500)  ? 1.0 : 0.0);
        
        if (currAlpha == 1.0){
            if (!timer.started){
                timer.start();
                timer.started = true;
                timer.update();
                cout<<"starting timer"<<endl;
            }
            else
                timer.update();
        }else
            timer.started = false;
        
        
        //update alpha and send to openFrameworks to direct video outputprev
        double c = updateOutput(prevAlpha, currAlpha, (double)transitionRate / 1000.0);
        prevAlpha = c;
        
        mainApp->setAlpha(c);
        
        //move hotspot and signal video player to change video when timer reaches limit
        if ( timer.accumulatedTime >= hotspotMoveTime && c == 1.0){
            hotspot.moveBox();
            timer.reset();
            mainApp->signalChange();
        }
        
        //update info on control panel
        /*
        int input = waitKey(100);
        
        if (input == 27) break;
        if (input == 32) reset = true;*/
        for (int i = 0; i < cnumber; i++){
            if (guiApp->getCameraActive(i))
                cameratoggle(i, true);
            else
                cameratoggle(i, false);
        }
    }
}

void playVideo(){
    cout<<"output thread started"<<endl;
    
    
    //string vid1Path, vid2Path;
    /*
    //load two videos to switch between.
    
     cout<<"path for video 1: ";
     cin>>vid1Path;
     cout<<"path for video 2: ";
     cin>>vid2Path;
     
    
    VideoCapture vid1(dir + "/1.mp4"), vid2(dir + "/2.mp4");
    
    // holders for webcam capture, samples from the videos, and the final output image
    Mat img1,
    img2,
    output;
    img1 = imread("1.jpg");
    img2 = imread("2.jpg");
    while (!quit){
        //grab next frame of the video files
        //vid1 >> img1;
        //vid2 >> img2;
        //resize(img1, img1, cv::Size(640, 480));
        //resize(img2, img2, cv::Size(640, 480));
        
        double c = updateOutput(prevAlpha, currAlpha, (double)transitionRate / 1000.0);
        prevAlpha = c;
        //alpha = c;
        //addWeighted(img1, c, img2, 1 - c, 0.0, output);
        
        //update volume in audio tracks (to match video cross-fade
        //audio[0].volume = 128.0 * c;
        //audio[1].volume = 128.0 - (128.0*c);
        
        //display main image
        //imshow("output", output);
        
        //update info on control panel
        //waitKey(100);
    }/*
    vid1.release();
    vid2.release();
    cout<<"output thread shut down"<<endl;*/
}

//callback for tracksbars
void on_trackbar(int, void*) {
    paramChangeSem = true;
}

void on_reset(int, void*) {
    if (resetValue > 0)
        reset = true;
    else
        reset = false;
}

void on_hotspotResize(int, void*){
    hotspot.resize(hotspotSize, hotspotSize);
}


//fall-back plan to detect large group of people gathered in a certain spot
//pros: super fast, easy to implement, little glitching
//cons: cant detect separate groups of people, wont detect if the people in the spot are grouped together


//more intelligent process of detecting groups of people. Will make a "group" object when either
//a. a contour of a certain size is found and/or
//b. there are at least two contours within a minimum distance from each other
int detectGroups(double minDist, double singleArea, vector<cv::Point> c, vector<int> q){
    int largestGroup = 0;
    for (int i = 0; i < c.size(); i++){
        int total = 0;
        for (int j = 0; j < c.size(); j++){
            if (dist(c.at(i), c.at(j))<=minDist)
                total += q.at(j);
        }
        if (total > largestGroup)
            largestGroup = total;
    }
    return largestGroup;
}



//provides smooth transition between the two pieces
double updateOutput(double previous, double current, double rate){
    if (abs(previous - current) <= rate) return current;
    if (previous < current) return previous + rate;
    if (previous > current) return previous - rate;
    else return current;
}
//control for individual cameras
void cameratoggle(int i, bool b){
    if (camera[i].active && !b){
        camera[i].active = false;
        hotspot.updateActiveCams(i, true);
        destroyWindow(camera[i].name);
        cout<<camera[i].name<<" inactive"<<endl;
    }
    else if (!camera[i].active && b){
        camera[i].active = true;
        namedWindow(camera[i].name, 1);
        hotspot.updateActiveCams(i, false);
        cout<<camera[i].name<<" active"<<endl;
    }
}
/*
 void CallBackFunc(int event, int x, int y, int flags, void* userdata)
 {
 //mouse primarily controls size of trigger box through drag and drop interface
 Camera* c = (Camera*)userdata;
 
 if  ( event == EVENT_LBUTTONDOWN )
 {
 if (c->resizing){
 c->resizing = false;
 }else{
 Point point[4];
 point[0] = c->rect1;
 point[1] = cv::Point(c->rect2.x, c->rect1.y);
 point[2] = c->rect2;
 point[3] = cv::Point(c->rect1.x, c->rect2.y);
 
 //default to first corner
 double smallest = dist(mousePoint, point[0]);
 c->activePoint = 0;
 
 for (int i = 1; i < 4; i++){
 double d = dist(mousePoint, point[i]);
 if ( d < smallest){
 smallest = d;
 c->activePoint = i;
 }
 }
 c->resizing = true;
 }
 }
 else if  ( event == EVENT_MOUSEMOVE )
 {
 mousePoint.x = x; mousePoint.y = y;
 if (c->resizing)
 resizeTrigger(c);
 }
 }
 */
double dist(cv::Point p1, cv::Point p2){
    return sqrt(pow(p2.x-p1.x, 2) + pow(p2.y-p1.y, 2));
}
/*
 void resizeTrigger(Camera* c){
 switch (c->activePoint){
 
 case 0: c->rect1 = mousePoint; break;
 
 case 1: c->rect2.x = mousePoint.x; c->rect1.y = mousePoint.y; break;
 
 case 2: c->rect2 = mousePoint; break;
 
 case 3: c->rect1.x = mousePoint.x; c->rect2.y = mousePoint.y; break;
 
 default: break;
 }
 }
 
 int motionTracker(Mat frame1, Mat frame2, Mat& debugImg){
 
 Mat temp;
 absdiff(frame1, frame2, temp);
 vector< vector<Point> > contours;
 vector<Vec4i> hierarchy;
 
 //findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
 findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours
 int num = contours.size();
 
 /// Approximate contours to polygons + get bounding rects and circles
 vector<vector<Point> > contours_poly( contours.size() );
 vector<Rect> boundRect( num );
 vector<Point2f>center( num );
 vector<float>radius( num );
 
 for( int i = 0; i < contours.size(); i++ )
 { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
 boundRect[i] = boundingRect( Mat(contours_poly[i]) );
 minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
 }
 
 /// Draw polygonal contour + bonding rects + circles
 Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
 for( int i = 0; i< contours.size(); i++ )
 {
 Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
 drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, cv::Point() );
 rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
 circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
 }
 
 
 
 return num;
 }*/




