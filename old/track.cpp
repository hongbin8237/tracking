#include <cv.h>
#include <highgui.h>
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


//IplImage* unDistort(IplImage* pic);
Mat unDistort(Mat pic);


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
    float distance[nf];
    float sumDistance;
    float avgDistance;

    double quality_level = 0.03; // quad
    double min_distance = 20;
    int block_size = 10;
    int not_harris = false;
    double k = 0.04;
    int* numFeatures = &nf;
    int cnt, j, jj;

    char status[nf];
    float track_error[nf];
    CvPoint2D32f featuresA[nf];
    CvPoint2D32f featuresB[nf];
    CvPoint2D32f featuresC[nf];
    
  
//  IplImage* imgColor = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
//  IplImage* img = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
//  IplImage* imgA = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
//  IplImage* imgB = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
//  IplImage* imgC = cvCreateImage(cvSize(540,380),IPL_DEPTH_8U,1);
//  IplImage* imgD = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
//  IplImage* imgE = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
//  IplImage* imgTotal = cvCreateImage(cvSize(640*3,480),IPL_DEPTH_8U,3);
//  IplImage* pyramid1 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
//  IplImage* pyramid2 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
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

    
    CvVideoWriter *writer_video = cvCreateVideoWriter( "video.avi", CV_FOURCC('I', '4', '2', '0'), 16, cvSize(640*3,480) );

    CvPoint p,q,r,s;
    double angle;
    double hypotenuse;
    double pi = 3.141592;

    for ( int i = 0; i < nf; i++){
        signature[i] = i;
    }
    
    while(n <=  3040){
    
        sprintf(buffer,"../../Jul3120142100_boat/img/%010d.jpg", n);

        //imgColor = cvLoadImage( buffer );
        imgColor = imread( buffer );
        
        
        unDistort(imgColor);
        
        
        //cvConvertImage(imgColor,img);
        cvtColor(imgColor,img, CV_BGR2GRAY);
        
        
        //cvSmooth(img, img, CV_MEDIAN, 5, 0, 0, 0);
        //cvSmooth(img, img, CV_MEDIAN, 5, 0, 0, 0);        
        medianBlur(img, img, 5);
        medianBlur(img, img, 5);
        
        
        //cvAdaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 1 );
        adaptiveThreshold(img, img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3, 1 );
        
        
        //cvSmooth(img, img, CV_MEDIAN, 1, 0, 0, 0);
        medianBlur(img, img, 1);
        
        
        //IplImage* imgBlank = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
        Mat imgBlank = Mat(Size(640,480),CV_8UC3);
        
        
        //cvThreshold( imgBlank, imgBlank, 100, 255, CV_THRESH_BINARY_INV );
        threshold( imgBlank, imgBlank, 100, 255, CV_THRESH_BINARY_INV );
        
        
        //IplImage* mask = cvCreateImage( cvGetSize(imgC), 8, 1);
        Mat mask = Mat(imgC.size(), 8, 1);
        
        
        //cvSet(mask, cvScalar(0));
        mask = 0;
        
        
        //cvLine(mask, cvPoint(50, 0), cvPoint(50 , 480), cvScalar(255), 100 , CV_AA, 0);
        //cvLine(mask, cvPoint(490, 0), cvPoint(490 , 480), cvScalar(255), 100 , CV_AA, 0);
        line(mask, cvPoint(50, 0), cvPoint(50 , 480), cvScalar(255), 100 , CV_AA, 0);
        line(mask, cvPoint(490, 0), cvPoint(490 , 480), cvScalar(255), 100 , CV_AA, 0);
        
        
        //cvShowImage( "mask", mask );
        imshow( "mask", mask );
        
        
        if (n == delay)
        {
            //cvConvertImage(img,imgA);
            cvtColor(img,imgA, CV_BGR2GRAY);
        }

        //cvConvertImage(img,imgB);
        cvtColor(img,imgB, CV_BGR2GRAY);
        

        //cvSetImageROI(img, cvRect(50,50,540,380));
        //cvCopy(img,imgC);
        //cvResetImageROI(img);
        Rect rec(50,50,540,380);
        imgC = img(rec);
        
        
        vector<Point2f> corners;
        if (n == delay)
        {
            //cvGoodFeaturesToTrack(imgC, NULL, NULL, featuresA, &nf, quality_level, min_distance, NULL, block_size, not_harris, k);
            /*** NOT SURE ***/
            goodFeaturesToTrack(imgC, corners, nf, quality_level, min_distance, Mat(), block_size, not_harris, k);
            /*** NOT SURE ***/
            
            
            for ( int i = 0; i < nf; i++){
                featuresA[i].x = featuresA[i].x + 50;
                featuresA[i].y = featuresA[i].y + 50;
            }
        }
        
        //CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );
        Size pyr_sz = Size( imgA.cols+8, imgB.rows/3 );
        
        
        //IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
        //IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
        Mat pyrA = Mat( pyr_sz, CV_32FC1 );
        Mat pyrB = Mat( pyr_sz, CV_32FC1 );
        

        //CvTermCriteria criteria = cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
        /*** NOT SURE ***/
        TermCriteria criteria = TermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
        /*** NOT SURE ***/
        
        
        //cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, featuresA, featuresB, nf, cvSize(10,10), 7, status, track_error, criteria, 0);
        /*** NOT SURE ***/
        Mat status;
        Mat track_error;
        calcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, status, track_error, Size(10,10), 7, criteria, 0, 0.0001);
        /*** NOT SURE ***/
        
        
        //cvConvertImage(img,imgA);
        cvtColor(img,imgA, CV_BGR2GRAY);
        

        /* Find Homography H */
        vector<Point2f> featAVec, featBVec, featEst;
        
        for(int i = 0; i < nf; i++)
        {
            featAVec.push_back(Point2f( featuresA[i].x, featuresA[i].y ));
            featBVec.push_back(Point2f( featuresB[i].x, featuresB[i].y ));
        }
        
        Mat H = findHomography(featAVec, featBVec, 8);
        
        /* perspectiveTransform on featAvec to estimate featBvec */
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
        //cvGoodFeaturesToTrack(imgC, NULL, NULL, featuresC, &nf, quality_level, min_distance, NULL, block_size, not_harris, k);
        /*** NOT SURE ***/
        goodFeaturesToTrack(imgC, corners, nf, quality_level, min_distance, Mat(), block_size, not_harris, k);
        /*** NOT SURE ***/
        
        
        //cvSetImageROI(imgB, cvRect(10,10,620,460));
        //cvSetImageROI(imgD, cvRect(10,10,620,460));
        //cvConvertImage(imgB,imgD);
        //cvAddS(imgD, cvScalar(-100,-100,-100),imgD);
        //cvResetImageROI(imgB);
        //cvResetImageROI(imgD);
        Rect rec1(10,10,620,460);
        imgD(rec1) = imgB(rec1);
        add(imgD, Scalar(-100,-100,-100),imgD);
        
        
        //cvSetImageROI(imgC, cvRect(0,0,540,380));
        //cvSetImageROI(imgD, cvRect(50,50,540,380));
        //cvConvertImage(imgC,imgD);
        //cvResetImageROI(imgC);
        //cvResetImageROI(imgD);
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
            
            
                //cvLine(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(255,20,20), 1 , CV_AA, 0);
                line(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(255,20,20), 1 , CV_AA, 0);
                
                
            else
            
                //cvLine(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(20,255,20), 1 , CV_AA, 0);
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
                cout << "jj = " << jj << endl;
            }
            
            featuresA[i].x = featuresB[i].x;
            featuresA[i].y = featuresB[i].y;
            
            fprintf(trackFile, "%d \t %d \t %d \t %f \t %f \n", n, i, signature[i], featuresB[i].x, featuresB[i].y);
            char buffer[500];
            sprintf(buffer, "%d \t %d \t %d \t %f \t %f \n", n, i, signature[i], featuresB[i].x, featuresB[i].y);
            printf(buffer);
        }

        
        //cvSetImageROI(imgD, cvRect(10,10,620,460));
        //cvSetImageROI(imgE, cvRect(10,10,620,460));
        //cvCopy(imgD,imgE);
        //cvResetImageROI(imgD);
        //cvResetImageROI(imgE);
        imgE(rec1) = imgD(rec1);
        
        
        //cvShowImage( "OutputImage", imgColor );
        imshow( "OutputImage", imgColor );
        
        
        /*** NOT SURE ***/
        //cvWriteFrame( writer_video, imgColor );
        VideoWriter(imgColor);
        /*** NOT SURE ***/
        
        
        sprintf(buffer2,"saveImg/%d.jpg", n);
        
        
        //cvSaveImage( buffer2, imgColor );
        imwrite(buffer2, imgColor);
        

        char c = cvWaitKey( 33 );
        n++;
        //cvReleaseImage(&mask);
        //cvReleaseImage( &imgColor );
        //cvReleaseImage( &pyramid1 );
        //cvReleaseImage( &pyramid2 );
        //cvReleaseImage( &pyrA );
        //cvReleaseImage( &pyrB );
    }
    //cvReleaseVideoWriter( &writer_video );
    //cvReleaseImage( &img );
    //cvReleaseImage( &imgA );
    //cvReleaseImage( &imgB );
    //cvReleaseImage( &imgC );
    //cvReleaseImage( &imgD );
    //cvReleaseImage( &imgE );
    //cvDestroyWindow( "window" );

    return( 0 );
}


/*** NOT SURE ***/
//! 2014-08-19 calibration results
//IplImage* unDistort(IplImage* pic){
Mat unDistort(Mat pic){


    CvMat* intrinsic = cvCreateMat(3,3,CV_64F);
    cvmSet(intrinsic,0,0,766.48320);   
    cvmSet(intrinsic,0,1,0);
    cvmSet(intrinsic,0,2,314.63026);
    cvmSet(intrinsic,1,0,0);
    cvmSet(intrinsic,1,1,765.13273);
    cvmSet(intrinsic,1,2,228.36923);
    cvmSet(intrinsic,2,0,0);
    cvmSet(intrinsic,2,1,0);
    cvmSet(intrinsic,2,2,1);

    CvMat* distortion = cvCreateMat(1,5,CV_64F);
    cvmSet(distortion,0,0,0.19343);
    cvmSet(distortion,0,1,-1.13072);
    cvmSet(distortion,0,2,0.00472);
    cvmSet(distortion,0,3,-0.00052);
    cvmSet(distortion,0,4,0.00000);

    IplImage* mapx = cvCreateImage( cvGetSize(pic), IPL_DEPTH_32F, 1 );
    IplImage* mapy = cvCreateImage( cvGetSize(pic), IPL_DEPTH_32F, 1 );

    cvInitUndistortMap(intrinsic,distortion,mapx,mapy);

    IplImage *t = cvCloneImage(pic);
    cvRemap( t, pic, mapx, mapy );
    cvReleaseImage(&mapx);
    cvReleaseImage(&mapy);
    cvReleaseImage(&t);

    return pic;
}
/*** NOT SURE ***/