#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp" 

using namespace std;

FILE * trackFile = fopen("result.txt", "w");
IplImage* unDistort(IplImage* pic);

int nf = 100;
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
	for ( int i = 0; i < nf; i++){
		if (mouseFlag == 2*i)
			mouseFlag = 2*i+1;
	}
    }


}

int main() {
	int delay = 1751;
//	int delay = 2700;
	int n = delay;
	char buffer[10];
	char buffer2[10];
	int signature[nf];
	int duration[nf];
	float distance[nf];
	float sumDistance;
	float avgDistance;

//	double quality_level = 0.05; // 2ndStreet
	double quality_level = 0.005; // quad
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

	IplImage* imgColor = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
	IplImage* img = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	IplImage* imgA = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	IplImage* imgB = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	IplImage* imgC = cvCreateImage(cvSize(540,380),IPL_DEPTH_8U,1);
	IplImage* imgD = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
	IplImage* imgE = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
	IplImage* imgTotal = cvCreateImage(cvSize(640*3,480),IPL_DEPTH_8U,3);
	IplImage* pyramid1 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	IplImage* pyramid2 = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);

	CvVideoWriter *writer_video = cvCreateVideoWriter( "video.avi", CV_FOURCC('M', 'P', '4', '2'), 16, cvSize(640*3,480) );

//	cvNamedWindow( "window", 0 );
//	cvNamedWindow( "select", 0 );
//	cvMoveWindow( "window", 0,0);
//	cvMoveWindow( "select", 640,0);
//	cvResizeWindow( "window", 640, 480 );
//	cvResizeWindow( "select", 640, 480 );
	cvNamedWindow( "total", 0 );
	cvMoveWindow( "total", 0,0);
	cvResizeWindow( "total", 640*3, 480 );

	CvPoint p,q,r,s;
	double angle;
	double hypotenuse;
	double pi = 3.141592;

//	char text[100];
//	CvFont font;
//	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX,.5,.5,0,1,8);
//|CV_FONT_HERSHEY_PLAIN

	for ( int i = 0; i < nf; i++){
		signature[i] = i;
	}
	while(n <=  3001) {

		sprintf(buffer,"/home/hongbin/ARC/tracking/Jul3120142100_boat/img/%010d.jpg", n);
		imgColor = cvLoadImage( buffer );
		unDistort(imgColor);
		cvConvertImage(imgColor,img);
		cvSmooth(img, img, CV_MEDIAN, 5, 0, 0, 0);
		cvSmooth(img, img, CV_MEDIAN, 5, 0, 0, 0);
//		cvSmooth(img, img, CV_MEDIAN, 5, 0, 0, 0);

//		cvAdaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 3, 0 );
		cvAdaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 1 );
		cvSmooth(img, img, CV_MEDIAN, 1, 0, 0, 0);

		IplImage* imgBlank = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);
		cvThreshold( imgBlank, imgBlank, 100, 255, CV_THRESH_BINARY_INV );

		if (n == delay){
//			cvShowImage( "window", imgColor );
//			cvShowImage( "select", imgColor );
			cvConvertImage(img,imgA);

		}

/*		for ( int i = 0; i < nf; i++){
			while (mouseFlag == 2*i){
				cvConvertImage(img,imgA);
				cvSetMouseCallback( "select", on_mouse, 0 );
				cvWaitKey( 33 );
			}
			if (mouseFlag == 2*i+1){
				featuresA[i] = cvPointTo32f(pt);
				mouseFlag = 2*i+2;
			}
		}
*/		//cout << "image number = " << n << endl;
		cvConvertImage(img,imgB);

		cvSetImageROI(img, cvRect(50,50,540,380));
		cvCopy(img,imgC);
		cvResetImageROI(img);
//		cvAdaptiveThreshold(imgC, imgC, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 3, 0 );

		if (n == delay){
			cvGoodFeaturesToTrack(imgC, NULL, NULL, featuresA, &nf, quality_level, min_distance, NULL, block_size, not_harris, k);
			for ( int i = 0; i < nf; i++){
					featuresA[i].x = featuresA[i].x + 50;
					featuresA[i].y = featuresA[i].y + 50;
			}
		}
		CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );
		IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
		IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );

		CvTermCriteria criteria = cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
		cvCalcOpticalFlowPyrLK(imgA, imgB, pyrA, pyrB, featuresA, featuresB, nf, cvSize(10,10), 7, status, track_error, criteria, 0);
		cvConvertImage(img,imgA);

        /* Find Homography H */
        std::vector<cv::Point2f> featAVec, featBVec, featEst;
        for(int i =0; i < nf; i++)
        {
            featAVec.push_back(cv::Point2f( featuresA[i].x, featuresA[i].y ));
            featBVec.push_back(cv::Point2f( featuresB[i].x, featuresB[i].y ));
        }
        cv::Mat H = cv::findHomography(featAVec, featBVec, CV_RANSAC);
		
        /* perspectiveTransform on featAvec to estimate featBvec */
        cv::perspectiveTransform(featAVec, featEst, H);

        cnt = 0;
        /* Remove feature if featBVec is too far off from estimate*/
        for(int i=0; i<nf; i++)
        {
            double error;
            error = pow(pow(featBVec[i].x - featEst[i].x, 2) + pow(featBVec[i].y - featEst[i].y, 2), 0.5);
            //cout << error << endl;
            if(error > 10)
            {
                featuresA[i].x = 0;
				featuresA[i].y = 0;
				featuresB[i].x = 0;
				featuresB[i].y = 0;
				duration[i] = 0;
				cnt++;

            }
        }




		for ( int i = 0; i < nf; i++){
			if (n == delay){
				duration[i] = 0;
			}
			duration[i] = duration[i] + 1;

            /* if boarder || duration == 200 -> remove feature */
			if (((featuresB[i].x < 10)) || (featuresB[i].x > 630) || (featuresB[i].y < 10) || (featuresB[i].y > 470) || (duration[i] == 200)){
                //cout << "B = " << featuresB[i].x << endl;
				featuresA[i].x = 0;
				featuresA[i].y = 0;
				featuresB[i].x = 0;
				featuresB[i].y = 0;
				duration[i] = 0;
				cnt++;
			}
		}




		
//		cout << "cnt = " << cnt << endl;

        /*  Another set of GoodFeatures to Replace outliers  */
		cvGoodFeaturesToTrack(imgC, NULL, NULL, featuresC, &nf, quality_level, min_distance, NULL, block_size, not_harris, k);

		cvSetImageROI(imgB, cvRect(10,10,620,460));
		cvSetImageROI(imgD, cvRect(10,10,620,460));
		cvConvertImage(imgB,imgD);
		cvAddS(imgD, cvScalar(-100,-100,-100),imgD);
		cvResetImageROI(imgB);
		cvResetImageROI(imgD);
		cvSetImageROI(imgC, cvRect(0,0,540,380));
		cvSetImageROI(imgD, cvRect(50,50,540,380));
		cvConvertImage(imgC,imgD);
		cvResetImageROI(imgC);
		cvResetImageROI(imgD);

		for ( int i = 0; i < nf; i++){
			featuresC[i].x = featuresC[i].x + 50;
			featuresC[i].y = featuresC[i].y + 50;
		}

		j = 0;
		jj = 0;

		for ( int i = 0; i < nf; i++){
            double error;
            error = pow(pow(featBVec[i].x - featEst[i].x, 2) + pow(featBVec[i].y - featEst[i].y, 2), 0.5);
            //cout << error << endl;

            //cvCircle(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), 2, CV_RGB(0,60,255), 1 , 8, 0);
            //cvCircle(imgColor, cvPoint(featEst[i].x, featEst[i].y), 2, CV_RGB(0,255,60), 1 , 8, 0);

            if(error > 10)
                cvLine(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(255,20,20), 1 , CV_AA, 0);
            else
                cvLine(imgColor, cvPoint(featAVec[i].x, featAVec[i].y), cvPoint(featBVec[i].x , featBVec[i].y), CV_RGB(20,255,20), 1 , CV_AA, 0);

 
			if (featuresB[i].x == 0){
				j++;
				featuresA[i].x = featuresC[j].x;
				featuresA[i].y = featuresC[j].y;
				featuresB[i].x = featuresC[j].x;
				featuresB[i].y = featuresC[j].y;
				signature[i] = signature[i] + nf;

			}
			if (featuresB[i].x == 0){
				jj++;
				cout << "jj = " << jj << endl;
			}
/*
			p.x = (int) featuresA[i].x;
			p.y = (int) featuresA[i].y;
			q.x = (int) featuresB[i].x;
			q.y = (int) featuresB[i].y;

			angle = atan2( (double) p.y - q.y, (double) p.x - q.x);
			hypotenuse = sqrt( pow(p.y - q.y, 2) + pow(p.x - q.x, 2));

			q.x = (int) (p.x - 1 * hypotenuse * cos(angle));
			q.y = (int) (p.y - 1 * hypotenuse * sin(angle));
			cvLine(imgColor, p, q, CV_RGB(255, 60, 50), 1, CV_AA, 0);
			p.x = (int) (q.x + 3 * cos(angle + pi / 4));
			p.y = (int) (q.y + 3 * sin(angle + pi/4));
			cvLine(imgColor, p, q, CV_RGB(255, 60, 50), 1, CV_AA, 0);
			p.x = (int) (q.x + 3 * cos(angle - pi/4));
			p.y = (int) (q.y + 3 * sin(angle - pi/4));
			cvLine(imgColor, p, q, CV_RGB(255, 60, 50), 1, CV_AA, 0);

			cvCircle(imgBlank, q, 1, CV_RGB(255, 60, 50), 2, 8);

			q.x = (int) (p.x - 1 * hypotenuse * cos(angle));
			q.y = (int) (p.y - 1 * hypotenuse * sin(angle));
			cvLine(imgD, p, q, CV_RGB(255, 60, 50), 2, CV_AA, 0);
			p.x = (int) (q.x + 3 * cos(angle + pi / 4));
			p.y = (int) (q.y + 3 * sin(angle + pi/4));
			cvLine(imgD, p, q, CV_RGB(255, 60, 50), 2, CV_AA, 0);
			p.x = (int) (q.x + 3 * cos(angle - pi/4));
			p.y = (int) (q.y + 3 * sin(angle - pi/4));
			cvLine(imgD, p, q, CV_RGB(255, 60, 50), 2, CV_AA, 0);
  */
			featuresA[i].x = featuresB[i].x;
			featuresA[i].y = featuresB[i].y;
            
			fprintf(trackFile, "%d \t %d \t %d \t %f \t %f \n", n, i, signature[i], featuresB[i].x, featuresB[i].y);
            char buffer[500];
			sprintf(buffer, "%d \t %d \t %d \t %f \t %f \n", n, i, signature[i], featuresB[i].x, featuresB[i].y);
            printf(buffer);
		}
//		cout << ", j = " << j << ", jj = " << jj << endl;

		cvSetImageROI(imgD, cvRect(10,10,620,460));
		cvSetImageROI(imgE, cvRect(10,10,620,460));
		cvCopy(imgD,imgE);
		cvResetImageROI(imgD);
		cvResetImageROI(imgE);
    
        /*  
		cvSetImageROI(imgTotal, cvRect(0,0,640,480));
		cvCopy(imgColor,imgTotal);
		cvResetImageROI(imgTotal);
		cvSetImageROI(imgTotal, cvRect(640,0,640,480));
		cvCopy(imgE,imgTotal);
		cvResetImageROI(imgTotal);
		cvSetImageROI(imgTotal, cvRect(1280,0,640,480));
		cvConvert(imgBlank,imgTotal);
		cvResetImageROI(imgTotal);
        */
//		cvShowImage( "window", imgColor );
//		cvShowImage( "features", imgBlank );
//		if (n != delay){
//		cvShowImage( "select", imgE );
//		}
//		cvResizeWindow( "features", 640, 480 );
		cvShowImage( "OutputImage", imgColor );
	

		cvWriteFrame( writer_video, imgTotal );
		sprintf(buffer2,"saveImg/%d.jpg", n);
		cvSaveImage( buffer2, imgColor );

		char c = cvWaitKey( 33 );
		n++;
		cvReleaseImage( &imgColor );
		cvReleaseImage( &pyramid1 );
		cvReleaseImage( &pyramid2 );
		cvReleaseImage( &pyrA );
		cvReleaseImage( &pyrB );
	}
	cvReleaseVideoWriter( &writer_video );
	cvReleaseImage( &img );
	cvReleaseImage( &imgA );
	cvReleaseImage( &imgB );
	cvReleaseImage( &imgC );
	cvReleaseImage( &imgD );
	cvReleaseImage( &imgE );
	cvDestroyWindow( "window" );

	return( 0 );
}

//! 2014-08-19 calibration results
IplImage* unDistort(IplImage* pic){
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
