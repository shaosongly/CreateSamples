#pragma once

#include "resource.h"
#include "cv.h"
#include "highgui.h"
#include <string>
using namespace std;

void icvWriteVecHeader( FILE* file, int count, int width, int height )
{
    int vecsize;
    short tmp;

    /* number of samples */
    fwrite( &count, sizeof( count ), 1, file );
    /* vector size */
    vecsize = width * height;
    fwrite( &vecsize, sizeof( vecsize ), 1, file );
    /* min/max values */
    tmp = 0;
    fwrite( &tmp, sizeof( tmp ), 1, file );
    fwrite( &tmp, sizeof( tmp ), 1, file );
}

void icvWriteVecSample( FILE* file, CvArr* sample ,int channel)
{
    CvMat* mat, stub;
    int r, c;
    uchar tmp;
    uchar chartmp;

    mat = cvGetMat( sample, &stub );
    chartmp = 0;
    fwrite( &chartmp, sizeof( chartmp ), 1, file );

    for( r = 0; r < mat->rows; r++ )
    {
        for( c = 0; c < mat->cols; c++ )
        {
            tmp = ((uchar*)(mat->data.ptr+r*mat->step))[channel*c];
            fwrite( &tmp, sizeof( tmp ), 1, file );
			if(channel==3)
			{
				tmp = ((uchar*)(mat->data.ptr+r*mat->step))[channel*c+1];
				fwrite( &tmp, sizeof( tmp ), 1, file );
				tmp = ((uchar*)(mat->data.ptr+r*mat->step))[channel*c+2];
				fwrite( &tmp, sizeof( tmp ), 1, file );
			}
        }
    }
}


int cvCreateTrainingSamplesFromInfo( const char* infoname, const char* vecfilename,int num,int winwidth, int winheight,int channel )
{
    string dirname="G:/pedestrian_detection/trainData/";
	string picPath;
    char  filename[100];

    FILE* info;
    FILE* vec;
    IplImage* src=0;
    IplImage* sample;
    int line;
    int error;
    int i;
    int x, y, width, height;
    int total;
	//cvNamedWindow("main",CV_WINDOW_AUTOSIZE);

    assert( infoname != NULL );
    assert( vecfilename != NULL );
    total = 0;
	
    info = fopen( infoname, "r" );
    if( info == NULL )
    {
        fprintf( stderr, "Unable to open file: %s\n", infoname );
        return total;
    }

    vec = fopen( vecfilename, "wb" );
    if( vec == NULL )
    {
        fprintf( stderr, "Unable to open file: %s\n", vecfilename );
        fclose( info );
        return total;
    }
	
    sample = cvCreateImage( cvSize( winwidth, winheight ), IPL_DEPTH_8U, channel );
    icvWriteVecHeader( vec, num, sample->width, sample->height );

	
    for( line = 1, error = 0, total = 0; total < num ;line++ )
    {
        int count;
        error = ( fscanf( info, "%s %d", filename, &count ) != 2 );
        if( !error )
        {
			picPath=dirname+filename;
			src = cvLoadImage( picPath.c_str(), CV_LOAD_IMAGE_ANYCOLOR);
            error = ( src == NULL );
            if( error )
            {
                fprintf( stderr, "Unable to open image: %s\n", picPath );
            }
        }
	
        for( i = 0; (i < count) && (total < num); i++, total++ )
        {
            error = ( fscanf( info, "%d %d %d %d", &x, &y, &width, &height ) != 4 );
            if( error ) break;
            cvSetImageROI( src, cvRect( x, y, width, height ) );
            cvResize( src, sample, width >= sample->width &&
                      height >= sample->height ? CV_INTER_AREA : CV_INTER_LINEAR );
			
            icvWriteVecSample(vec, sample,channel );
			//cvShowImage("main",&sample);
			//cvWaitKey(100);
        }
	
        if( src )
        {
            cvReleaseImage( &src );
        }
        if( error )
        {
            fprintf( stderr, "%s(%d) : parse error", infoname, line );
            break;
        }
	
    }

    if( sample )
    {
        cvReleaseImage( &sample );
    }
    fclose( vec );
    fclose( info );
	
    return total;
}