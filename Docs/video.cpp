#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
 
using namespace std;
using namespace cv;
//initial min and max HSV filter values.
 
int H_MIN_U = 92;
int H_MAX_U = 255;
int S_MIN_U = 135;
int S_MAX_U = 255;
int V_MIN_U = 176;
int V_MAX_U = 255;
 
int H_MIN_G = 54;
int H_MAX_G = 255;
int S_MIN_G = 152;
int S_MAX_G = 255;
int V_MIN_G = 146;
int V_MAX_G = 227;
 
int H_MIN_R = 0;
int H_MAX_R = 68;
int S_MIN_R = 193;
int S_MAX_R = 256;
int V_MIN_R = 0;
int V_MAX_R = 256;
 
class Players
{
public:
    int x;
    int y;
};
 
Players players[2];
 
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName1 = "HSV Image";
const std::string windowName2 = "Thresholded Image";
const std::string windowName4 = "Thresholded Image2";
const std::string windowName5 = "Thresholded Image3";
const std::string windowName3 = "After Morphological Operations";
const std::string trackbarWindowName = "Trackbars";
const std::string trackbarWindowName2 = "Trackbars2";
 
 
void on_mouse(int e, int x, int y, int d, void *ptr)
{
    if (e == EVENT_LBUTTONDOWN)
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
}
 
void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
}
 
string intToString(int number) {
 
 
    std::stringstream ss;
    ss << number;
    return ss.str();
}
 
void drawObject(int x, int y, Mat &frame) {
 
    //use some of the openCV drawing functions to draw crosshairs
    //on your tracked image!
 
    //UPDATE:JUNE 18TH, 2013
    //added 'if' and 'else' statements to prevent
    //memory errors from writing off the screen (ie. (-25,-25) is not within the window!)
 
    circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
    if (y - 25 > 0)
        line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
    if (y + 25 < FRAME_HEIGHT)
        line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
    if (x - 25 > 0)
        line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
    if (x + 25 < FRAME_WIDTH)
        line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
    else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);
 
    putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
    //cout << "x,y: " << x << ", " << y;
 
}
void morphOps(Mat &thresh) {
 
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
 
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));
 
    erode(thresh, thresh, erodeElement);
    erode(thresh, thresh, erodeElement);
 
    dilate(thresh, thresh, dilateElement);
    dilate(thresh, thresh, dilateElement);
}
 
void socket()
{
    int client, server;
    int portNum = 20231;
    bool isExit = false;
    int bufsize=9;
    char buffer[9]={'f','b','l','r','f','b','l','r','f'};
 
    struct sockaddr_in server_addr;
    socklen_t size;
 
    client = socket(AF_INET, SOCK_STREAM, 0);
 
    if (client < 0)
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }
 
    cout << "\n=> Socket server has been created..." << endl;
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("193.226.12.217");
    server_addr.sin_port = htons(portNum);
   
    int clientCount = 1;
 
    if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        cout << "=> Connection to the server port number: " << portNum << endl;
 
    for(int i =0 ;i<bufsize;i++){
        printf("Trimit %c\n", buffer[i]);
            send(client, &buffer[i], 1, 0);
        sleep(1);
    }
}
 
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed) {
 
    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if (numObjects < MAX_NUM_OBJECTS) {
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
 
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
 
                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter(0
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
                    x = moment.m10 / area;
                    y = moment.m01 / area;
                    objectFound = true;
                    refArea = area;
                }
                else objectFound = false;
 
 
            }
            //let user know you found an object
            if (objectFound == true) {
                putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
                //draw object location on screen
                //cout << x << "," << y;
                drawObject(x, y, cameraFeed);
               
                players[state].x=x;
                players[state].y=y;
                printf("found player [%d] at [%d][%d]\n",state,x,y);
 
                if(state == 1){
                    //oppenent found
                }  
 
                state = 1 - state;
        }
        else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
    }
}
int main(int argc, char* argv[])
{
    socket();
 
    //some boolean variables for different functionality within this
    //program
    bool trackObjects = true;
    char buffer[100];
    int socket1;
    socket1=socket(193.226.12.217
    bool useMorphOps = true;
 
    Point p;
    //Matrix to store each frame of the webcam feed
    Mat cameraFeed;
    //matrix storage for HSV image
    Mat HSV;
    //matrix storage for binary threshold image
    Mat threshold_U;
    Mat threshold_G;
    Mat threshold_R;
    //x and y values for the location of the object
    int x = 0, y = 0, x1=0,y1=0,x2=0,y2=0;
    //create slider bars for HSV filtering
    //createTrackbars(trackbarWindowName);
    //createTrackbars(trackbarWindowName2);
    //video capture object to acquire webcam feed
    VideoCapture capture;
    //open capture object at location zero (default location for webcam)
    capture.open("rtmp://172.16.254.63/live/live");
    //set height and width of capture frame
    capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    //start an infinite loop where webcam feed is copied to cameraFeed matrix
    //all of our operations will be performed within this loop
 
 
 
   
    while (1) {
 
 
        //store image to matrix
        capture.read(cameraFeed);
        //convert frame from BGR to HSV colorspace
        cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
        //filter HSV image between values and store filtered image to
        //threshold matrixthreshold2
        inRange(HSV, Scalar(H_MIN_U, S_MIN_U, V_MIN_U), Scalar(H_MAX_U, S_MAX_U, V_MAX_U),threshold_U);
      //capture.read(cameraFeed);
     cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
        inRange(HSV, Scalar(H_MIN_G, S_MIN_G, V_MIN_G), Scalar(H_MAX_G, S_MAX_G, V_MAX_G),threshold_G);
          //capture.read(cameraFeed);
      cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
    inRange(HSV, Scalar(H_MIN_R, S_MIN_R, V_MIN_R), Scalar(H_MAX_R, S_MAX_R, V_MAX_R),threshold_R);
        //perform morphological operations on thresholded image to eliminate noise
        //and emphasize the filtered object(s)
        if (useMorphOps)
            {
                morphOps(threshold_U);
                morphOps(threshold_G);
        morphOps(threshold_R);
            }
        //pass in thresholded frame to our object tracking function
        //this function will return the x and y coordinates of the
        //filtered object
        if (trackObjects)
{
            trackFilteredObject(x, y, threshold_U, cameraFeed);
            trackFilteredObject(x1, y1, threshold_G, cameraFeed);
        trackFilteredObject(x2, y2, threshold_R, cameraFeed);
}
 
        //show frames
        imshow(windowName5, threshold_U);
    imshow(windowName2, threshold_R);
    imshow(windowName4, threshold_G);
        imshow(windowName, cameraFeed);
        //imshow(windowName1, HSV);
        setMouseCallback("Original Image", on_mouse, &p);
        //delay 30ms so that screen can refresh.
        //image will not appear without this waitKey() command
        waitKey(30);
    }
    return 0;
}
