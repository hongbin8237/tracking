/*
 * =====================================================================================
 *
 *       Filename:  track.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/28/2014 10:23:41 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hong-Bin Yoon (HBY), yoon48@illinois.edu
 *   Organization:  Aerospace Robotics and Controls Lab (ARC)
 *
 * =====================================================================================
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
using namespace cv;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description: finds goodfeaturesToTrack from images in Image Path and writes  
 * =====================================================================================
 */
    int
main ( int argc, char *argv[] )
{
    int startT = 2300;
    int endT = 4150;
    int n = startT;
    int maxCorners = 100;
    if( argc!=1 )
    {
        printf(" Usage: %s \n", argv[0]);
        exit(EXIT_FAILURE);
    }


    while(n <= endT)
    {
        char buffer[100];
        /* Image Path */
        //sprintf(buffer,"~/ARC/slam/raw/Jul1920140545/img/%010d.jpg", n);
		sprintf(buffer,"/home/hongbin/ARC/slam/raw/Jul1920140545/img/%010d.jpg", n);
        /* Read Image */
        Mat src, src_gray, img;
        src = imread( buffer, 1 );
        if(src.empty())
        {
            std::cout << "failed to load image: " << buffer <<std::endl;
            exit(EXIT_FAILURE);
        }
        img = src.clone();
        cvtColor( src, src_gray, COLOR_BGR2GRAY );

        /* Apply smoothing */
        medianBlur(img, img, 5);
        medianBlur(img, img, 5);

        adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 1 );

        /* Shi-Tomasi Algorithm Parameters */
        std::vector<Point2f> corners;
        double qualityLevel = 0.01;
        double minDistance = 10;
        int blockSize = 3;
        bool useHarris = false;
        double k = 0.04;

        /* Apply Corner Detection */
        goodFeaturesToTrack( src_gray,
                corners,
                maxCorners,
                qualityLevel,
                minDistance,
                Mat(),
                blockSize,
                useHarris,
                k);


    } /* --- end of while --- */
    return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */

