#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/tracking.hpp"

using namespace std;
using namespace cv;

FILE* trackFile = fopen("result.txt", "w");

int nf = 200;
int mouseFlag = 0;
int renewFlag = 0;
CvPoint pt;

void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( event == CV_EVENT_LBUTTONDOWN ){
        if( mouseFlag != 0)
            renewFlag = renewFlag+nf;
            
        mouseFlag = 0;
    }
    
    if( event == CV_EVENT_RBUTTONDOWN ){ // click right button four times 
        pt = cvPoint(x,y);
    
        for ( int i = 0; i < nf; i++)
        {
            if (mouseFlag == 2*i)
                mouseFlag = 2*i+1;
        }
    }
}

int main() {
    int delay = 1751;
    int n = delay;
    char buffer[100];
    char buffer2[10];
    int signature[nf];
    int duration[nf];

    double quality_level = 0.03; // quad
    double min_distance = 20;
    int block_size = 10;
    int not_harris = false;
    double k = 0.04;
    int cnt, j, jj;

    vector<uchar> status;
    vector<float> track_error;
    vector<Point2f> featuresA;
    vector<Point2f> featuresB;
    vector<Point2f> featuresC;
    
    Mat imgColor = Mat(Size(640,480),CV_8UC3);
    Mat img = Mat(Size(640,480),CV_8UC1);
    Mat imgA = Mat(Size(640,480),CV_8UC1);
    Mat imgB = Mat(Size(640,480),CV_8UC1);
    Mat imgC = Mat(Size(540,380),CV_8UC1);
    Mat imgD = Mat(Size(640,480),CV_8UC3);
    Mat imgE = Mat(Size(640,480),CV_8UC3);
    Mat imgTotal = Mat(Size(640*3,480),CV_8UC3);
    Mat pyramid1 = Mat(Size(640,480),CV_8UC1);
    Mat pyramid2 = Mat(Size(640,480),CV_8UC1);

    for ( int i = 0; i < nf; i++){
        signature[i] = i;
    }
    
    while(n <= 1900){
    
        sprintf(buffer,"../Jul3120142100_boat/img/%010d.jpg", n);

        Mat src = imread( buffer );
        
        Mat cameraMatrix = (Mat_<double>(3,3) << 766.48320, 0, 314.63026, 0, 765.13273, 228.36923, 0, 0, 1);
        Mat disCoeffs = (Mat_<double>(1,5) << 0.19343, -1.13072, 0.00472, -0.00052, 0);
        
        undistort(src, imgColor, cameraMatrix, disCoeffs);

        cvtColor(imgColor, img, CV_BGR2GRAY);
        
        medianBlur(img, img, 5);
        medianBlur(img, img, 5);
        
        adaptiveThreshold(img, img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3, 1 );
        
        medianBlur(img, img, 1);
        
        Mat imgBlank = Mat(Size(640,480),CV_8UC3);
        
        threshold( imgBlank, imgBlank, 100, 255, CV_THRESH_BINARY_INV );
        
        Mat mask = Mat::zeros(imgC.rows, imgC.cols, CV_8UC1);
        
        line(mask, cvPoint(50, 0), cvPoint(50 , 480), cvScalar(255), 100 , CV_AA, 0);
        line(mask, cvPoint(490, 0), cvPoint(490 , 480), cvScalar(255), 100 , CV_AA, 0);

        if (n == delay)
        {
            img.convertTo(imgA, CV_8UC1);
        }
        
        img.convertTo(imgB, CV_8UC1);
        
        Rect rec(50,50,540,380);
        imgC = img(rec);
        
        
        if (n == delay)
        {
            goodFeaturesToTrack(imgC, featuresA, nf, quality_level, min_distance, noArray(), block_size, not_harris, k);
            
            
            for ( int i = 0; i < nf; i++){
                featuresA[i].x = featuresA[i].x + 50;
                featuresA[i].y = featuresA[i].y + 50;
            }
        }
        
        TermCriteria criteria = TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
        
        calcOpticalFlowPyrLK(imgA, imgB, featuresA, featuresB, status, track_error, Size(10,10), 7, criteria);
        

        /* Find Homography H */
        vector<Point2f> featAVec, featBVec, featEst;
        
        for(int i = 0; i < nf; i++)
        {
            featAVec.push_back(Point2f( featuresA[i].x, featuresA[i].y ));
            featBVec.push_back(Point2f( featuresB[i].x, featuresB[i].y ));
        }
        
        Mat H = findHomography(featAVec, featBVec, 8);
        
        perspectiveTransform(featAVec, featEst, H);

        cnt = 0;
        /* Remove feature if featBVec is too far off from estimate*/
        for(int i = 0; i < nf; i++)
        {
            double error;
            error = pow(pow(featBVec[i].x - featEst[i].x, 2) + pow(featBVec[i].y - featEst[i].y, 2), 0.5);
            if(error > 5)
            {
                featuresA[i].x = 0;
                featuresA[i].y = 0;
                featuresB[i].x = 0;
                featuresB[i].y = 0;
                duration[i] = 0;
                cnt++;

            }
        }

        /* clear features every n frames */
        if(n%20 == 0)
        {
            for(int i=0; i<nf; i++)
            {
                featuresA[i].x = 0;
                featuresA[i].y = 0;
                featuresB[i].x = 0;
                featuresB[i].y = 0;
                duration[i] = 0;
            }
        }


        for( int i = 0; i < nf; i++){
            if(n == delay){
                duration[i] = 0;
            }
            duration[i] = duration[i] + 1;

            /* if boarder || duration == 200 -> remove feature */
            if (((featuresB[i].x < 10)) || (featuresB[i].x > 630) || (featuresB[i].y < 10) || (featuresB[i].y > 470) || (duration[i] == 50)){
                featuresA[i].x = 0;
                featuresA[i].y = 0;
                featuresB[i].x = 0;
                featuresB[i].y = 0;
                duration[i] = 0;
                cnt++;
            }
        }

        /*  Another set of GoodFeatures to Replace outliers  */
        goodFeaturesToTrack(imgC, featuresC, nf, quality_level, min_distance, noArray(), block_size, not_harris, k);
        
        Rect rec1(10,10,620,460);
        imgD(rec1) = imgB(rec1);
        add(imgD, Scalar(-100,-100,-100),imgD);
        
        Rect rec2(0,0,540,380);
        imgD(rec) = imgB(rec2);
        
        
        for( int i = 0; i < nf; i++){
            featuresC[i].x = featuresC[i].x + 50;
            featuresC[i].y = featuresC[i].y + 50;
        }

        j = 0;
        jj = 0;

        for( int i = 0; i < nf; i++){
            double error;
            error = pow(pow(featBVec[i].x - featEst[i].x, 2) + pow(featBVec[i].y - featEst[i].y, 2), 0.5);

            if(error > 5)
                line(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(255,20,20), 1 , CV_AA, 0);
                
                
            else
                line(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(20,255,20), 1 , CV_AA, 0);
                
                
            if(featuresB[i].x == 0){
                j++;
                featuresA[i].x = featuresC[j].x;
                featuresA[i].y = featuresC[j].y;
                featuresB[i].x = featuresC[j].x;
                featuresB[i].y = featuresC[j].y;
                signature[i] = signature[i] + nf;

            }
            if(featuresB[i].x == 0){
                jj++;
            }
            
            featuresA[i].x = featuresB[i].x;
            featuresA[i].y = featuresB[i].y;
            
            fprintf(trackFile, "%d \t %d \t %d \t %f \t %f \n", n, i, -1*signature[i], featuresB[i].x, featuresB[i].y);
            char buffer[500];
            sprintf(buffer, "%d \t %d \t %d \t %f \t %f \n", n, i, -1*signature[i], featuresB[i].x, featuresB[i].y);
            printf(buffer);
        }
        
        imgE(rec1) = imgD(rec1);
        
        imshow( "OutputImage", imgColor );
        
        /*
        VideoWriter writer;
        writer.open("video.avi", CV_FOURCC('I', '4', '2', '0'), 16, Size(640*3,480), true);
        
        if(writer.isOpened()){
        writer << imgColor;
        }
        
        
        sprintf(buffer2,"saveImg/%d.jpg", n);
        imwrite(buffer2, imgColor);
        */
        
        char c = waitKey( 33 );
        
        n++;    
    }
    
    return( 0 );
}