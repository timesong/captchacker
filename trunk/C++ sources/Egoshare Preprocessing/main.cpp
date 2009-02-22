#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#if defined (WIN32)
#pragma comment(lib,"cv")
#pragma comment(lib,"cvaux")
#pragma comment(lib,"cxcore")
#pragma comment(lib,"highgui")
#pragma comment(lib,"cvcam")
#endif

using namespace std;


#define WIDTH 20
#define HEIGHT 20



template <class T>
inline std::string to_string (const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}




typedef struct CC
{
	CvConnectedComp *comp;
	IplImage *mask;
};




inline int func_compare_area_cc(const void *a, const void *b)
{
	//Ordre d�croissant par aire
	return (*((CC**) b))->comp->area - (*((CC**) a))->comp->area;
}

inline int func_compare_pos_cc(const void *a, const void *b)
{
	//cout << "COMPARISON: " << (*((CC**) a))->comp->rect.x << " " << (*((CC**) b))->comp->rect.x << endl;
	//Ordre croissant par position
	return (*((CC**) a))->comp->rect.x - (*((CC**) b))->comp->rect.x;
}










int main(int argc, char *argv[])
{
	//Noms de fichiers
	string filenameIN;
	if (argc < 2)
		filenameIN = "test.jpg";
	else
		filenameIN = argv[1];


	//Seuillage
	int threshold = 150, maxValue = 255;
	int thresholdType = CV_THRESH_BINARY;

	IplImage *srcImg=0, *grayThresh=0, *gray=0;
	srcImg = cvLoadImage(filenameIN.c_str(),1);

	grayThresh = cvCreateImage( cvSize(srcImg->width, srcImg->height), IPL_DEPTH_8U, 1 );
	cvCvtColor(srcImg, grayThresh, CV_BGR2GRAY );
	cvThreshold(grayThresh, grayThresh, threshold, maxValue, thresholdType);

	//On commence par chercher les composantes 8-connexes
	int connectivity = 4;
	std::vector<CC*> CCs;

	while (1)
	{
		gray = cvCloneImage(grayThresh);

		//S�lection de toutes les composantes connexes en noir
		CCs.clear();
		for (int i=0; i<gray->width; ++i)
		{
			for (int j=0; j<gray->height; ++j)
			{
				if (cvGet2D(gray, j, i).val[0] == 0)
				{
					CvConnectedComp *comp = new CvConnectedComp;

					IplImage *mask = cvCreateImage(cvSize(gray->width+2, gray->height+2), IPL_DEPTH_8U, 1);
					cvZero(mask);

					cvFloodFill(gray, cvPoint(i,j), cvScalar(128),cvScalarAll(0),cvScalarAll(0),comp, connectivity, mask);

					CC *cc = new CC;
					cc->mask = mask;
					cc->comp = comp;
					CCs.push_back(cc);
				}
			}
		}

		//cout << CCs.size() << " connected components found." << endl;


		//cout << "BEFORE" << endl;
		//for (int i=0; i<CCs.size(); ++i)
		//	cout << CCs[i]->comp->area << " ";
		//cout << endl;
		//Tri d�croissant selon l'aire des composantes connexes
		qsort(&CCs[0], CCs.size(), sizeof(CCs[0]), func_compare_area_cc);

		//cout << "AFTER AREA SORT" << endl;
		//for (int i=0; i<CCs.size(); ++i)
		//	cout << CCs[i]->comp->area << " ";
		//cout << endl;

		//On ne garde que 3 composantes connexes
		int size = CCs.size();
		for (int i=3; i<size; ++i)
			CCs.pop_back();

		if (connectivity == 4)
		{
			//Si une des composantes 4-connexes est un fragment de caract�re,
			//on cherche les composantes 8-connexes
			if (CCs[2]->comp->area < 30)
			{
				connectivity = 8;
				continue;
			}
		}


		//cout << "AFTER POP" << endl;
		//for (int i=0; i<CCs.size(); ++i)
		//	cout << CCs[i]->comp->area << " ";
		//cout << endl;

		//cout << "BEFORE POS SORT" << endl;
		//for (int i=0; i<CCs.size(); ++i)
		//	cout << CCs[i]->comp->rect.x << " ";
		//cout << endl;

		//Tri croissant selon l'abscisse de la composante connexe
		qsort(&CCs[0], CCs.size(), sizeof(CCs[0]), func_compare_pos_cc);

		//cout << "AFTER POS SORT" << endl;
		//for (int i=0; i<CCs.size(); ++i)
		//	cout << CCs[i]->comp->rect.x << " ";
		//cout << endl;

		break;


	}













	std::vector<IplImage*> letters;
	for (int i=0; i<3; ++i)
	{
		IplImage *letter = new IplImage;
		letter = cvCreateImage( cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1 );
		cvFillImage(letter, 255);
		letters.push_back(letter);
	}

	//Remplissage des imagettes par les sous-rectangles de l'image threshold�e
	for (int index_image=0; index_image<letters.size(); ++index_image)
	{
		IplImage *letter = letters[index_image];
		CC *cc = CCs[index_image];

		int offsetx = (WIDTH -  cc->comp->rect.width)/2;
		int offsety = (HEIGHT - cc->comp->rect.height)/2;

		//cout << cc->comp->rect.width<< " " << cc->comp->rect.height << endl;
		//Recopiage de la sous-image
		for (int i=1; i<cc->mask->width-1; ++i)
		{
			for (int j=1; j<cc->mask->height-1; ++j)
			{
				if (cvGet2D(cc->mask, j, i).val[0] == 1)
				{
					int Y = j - cc->comp->rect.y + offsety;
					int X = i - cc->comp->rect.x + offsetx;

					if ((X>0) && (X<WIDTH) && (Y>0) && (Y<HEIGHT))
					{
						cvSet2D(letter,
							j - cc->comp->rect.y + offsety,
							i - cc->comp->rect.x + offsetx,
							cvScalar(0));
					}
				}
			}
		}

		//cout << "Lettre " << index_image << " preprocessed!" << endl;
	}



	//Sauvegardes des imagettes
	std::string filename = "Letter";;
	for (int i=0; i<3; ++i)
		cvSaveImage((filename+to_string(i+1)+".bmp").c_str(), letters[i]);


	//if(!cvSaveImage("testOUT.bmp".c_str(),grayThresh)){
	//    cout << "WARNING: Pic can't be saved" <<endl;
	//    exit(2);
	//}
	//cout << "testOUT.bmp" << " successfully written" << endl;

	//cvWaitKey(0);
	cvReleaseImage( &grayThresh );
	cvReleaseImage( &srcImg );

	//system("pause");

	return 0;


}
