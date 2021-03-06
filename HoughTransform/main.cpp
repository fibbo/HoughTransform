#include <iostream>
#include <vector>

/* Define _USE_MATH_DEFINES before including math.h to expose these macro
* definitions for common math constants.  These are placed under an #ifdef
* since these commonly-defined names are not part of the C/C++ standards.
*/

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <TH2F.h>
#include <TF1.h>
#include <TGraph.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <boost/assign/std/vector.hpp>


typedef unsigned int uint;

using namespace boost::assign;

/*def isGood(track):
    return (track.reconstructible_asLong # long track
        and track.true_p > 3000                 # min momentum
        and track.true_pt > 500                  # min transverse momentum
        and track.true_eta > 2                    # pseudorapidity range
        and track.true_eta < 5                    # 
        and abs(track.pid) != 22                 # not an electron/positron
        and track.fromB)                             # B daughter
		*/

struct Track {
	bool reconstructible_asLong;
	bool reconstructible_asUpstream;
	bool fromB;
	int pid;
	double true_p;
	double true_px;
	double true_py;
	double true_pt;
	double true_eta;
	int true_threeCharge;
	int nHits;
	int len;
	double x_hit;
	double y_min_hit;
	double y_max_hit;
	double z_hit;
	int planeCode;
	double x_UT;
	double x_velo;
	double y_velo;
	double z_velo;
	double tx_velo;
	double ty_velo;
	bool hasVelo;
	int vplen;
	double velo_x_hit;
	double velo_y_hit;
	double velo_z_hit;
};

struct viewOutput {
	std::vector<TF1> lines;
	TCanvas c;
	TGraph hPs;
};

struct binCoordValue {
	double i, j;
	int value;
	binCoordValue(double _i, double _j, int _value) { i = _i; j = _j; value = _value; }
};

struct binCoord {
	double i,j;
	binCoord(double _i, double _j) { i = _i, j = _j; }
};

struct HTCoord {
	double r, theta;
	HTCoord(double _r, double _theta) { r = _r; theta = _theta; }
};


TGraph view(std::vector<double> x, std::vector<double> y, std::vector<TF1> lines) {
	double* xx = &x[0];
	double* yy = &x[0];
	int s = x.size();
	TGraph hPs(s, xx, yy);
	return hPs;
}

double round(double d)
{
	return floor(d + 0.5);
}

double get_minimum(std::vector<double> x) {
	double min = x[0];
	for (unsigned int i=1; i<x.size(); i++) {
		if (x[i] < min) min = x[i];
	}
	return min;
}

double get_maximum(std::vector<double> x) {
	double max = x[0];
	for (unsigned int i=1; i<x.size(); i++) {
		if (x[i] > max) max = x[i];
	}
	return max;
}

std::vector<HTCoord> convertBinVal(TH2F* h, std::vector<binCoord> cleanMaxbins) {
	std::vector<HTCoord> M;
	for (unsigned int i=0;i<cleanMaxbins.size(); i++){
		double R = h->GetYaxis()->GetBinCenter(int( round(cleanMaxbins[i].j) ));
		double Theta = h->GetXaxis()->GetBinCenter(int( round(cleanMaxbins[i].i) ));
		M.push_back(HTCoord(R,Theta));
	}
	return M;
}

std::vector<TF1*> findLines(std::vector<HTCoord> myMax, double mx,double Mx) {
	std::vector<TF1*> lines;
	for (unsigned int i=0; i<myMax.size(); i++) {
		char* fname = "test ";
		fname += i;
		lines.push_back(new TF1(fname,"([0]-cos([1])*x)/sin([1])",mx,Mx));
		lines[i]->SetParameter(0,myMax[i].r);
		lines[i]->SetParameter(1,myMax[i].theta);
	}
	return lines;
}


std::vector<binCoord> makeCluster(std::vector<binCoordValue> max, int resX, int resY) {
	std::vector<binCoord> M;
	double a = max[0].i;
	double b = max[0].j;
	for (unsigned int i=1; i<max.size(); i++) {
		double c = max[i].i;
		double d = max[i].j;
		//		std::cout << c << "\t" << d << std::endl;

		if ( (fabs(a-c) <= resX) && (fabs(b-d) <= resY)) {
			a = (a+c)/2.;
			b = (b+d)/2.;
			//M->push_back(binCoord(a,b));
		}
		else {
			M.push_back(binCoord(a,b));
			a = c;
			b = d;
		}


	}
	M.push_back(binCoord(a,b));
	return M;
}

void fillHistogram(std::vector<double> x, std::vector<double> y, TH2F* h, int nTheta) {
	double step = M_PI/nTheta;
	double tmp = 0.;
	std::vector<double> thetas;

	for (int i=0; i<nTheta;i++) {
		tmp += step;
		thetas.push_back(tmp);
	}
	double mtheta = 0.;
	double Mtheta = M_PI;
	double mrho = -10;
	double Mrho = 10;

	for (unsigned int i = 0; i<x.size(); i++) {
		for (unsigned int t = 0; t<thetas.size(); t++) {
			double r = x[i]*cos(thetas[t]) + y[i]*sin(thetas[t]);
			//std::cout << r << "\t" << t << std::endl;
			h->Fill(thetas[t],r);
		}
	}
}

std::vector<binCoordValue> getCoords(TH2F* h, int nRho, int nTheta, int thr) {
	std::vector<binCoordValue> preliminaryMax;	
	for (int i=0;i<nTheta;i++) {
		for (int j=0;j<nRho;j++) {
			int tmp = h->GetBinContent(i,j);
			if (tmp >= thr) { 
				preliminaryMax.push_back(binCoordValue(i,j,tmp));
			}
		}
	}

	return preliminaryMax;
}

void drawGraphs(TH2F* h, std::vector<TF1> lines, TGraph hPs) {
	TCanvas* c2 = new TCanvas("c2","c2");
	h->GetXaxis()->SetTitle("#theta");
	h->GetYaxis()->SetTitle("#rho");
	h->Draw("colz");


}

void HT(std::vector<double> x, std::vector<double> y, int thr) {

	int nTheta = 1000;
	int nRho = 1000;
	int resX = 2;
	int resY = 2;
	double mtheta = 0.;
	double Mtheta = M_PI;
	double mrho = -10;
	double Mrho = 10;

	TH2F* h = new TH2F("h","h",nTheta,mtheta,Mtheta,nRho,mrho,Mrho);
	fillHistogram(x, y, h, nTheta);

	double mx = get_minimum(x);
	double Mx = get_maximum(x);

	std::vector<binCoordValue> binCoords = getCoords(h, nRho, nTheta, thr);
	std::vector<binCoord> M = makeCluster(binCoords, resX, resY);
	std::vector<HTCoord> cleanMax = convertBinVal(h, M);
	std::vector<TF1*> lines = findLines(cleanMax, mx, Mx);
	TGraph* hPs = new TGraph(x.size(), &x[0], &y[0]);// &x[0] converts vector to double* array, same for y

	//drawGraphs(h, lines, hPs);

	TCanvas* c2 = new TCanvas("c2","c2");
	h->GetXaxis()->SetTitle("#theta");
	h->GetYaxis()->SetTitle("#rho");
	h->Draw("colz");


	TCanvas* c4 = new TCanvas("c4","found lines");
	hPs->Draw("A*");
	for (unsigned int i=0; i<lines.size(); i++){
		//if (i == 0) lines->at(i).Draw();
		//else lines->at(i).Draw("same");
		lines[i]->Draw("same");
	}
	c4->Update();

}



int main(int argc, char *argv[]) {

		int FLAG;
	std::cout << "Enter 1 to run Data from Rootfile, press 0 to run Data from trivial example" << std::endl;
	std::cin >> FLAG;
	TApplication theApp("App",&argc,argv);

	if (FLAG == 1) {
		struct POINT {
			Float_t x,y,z;};

		TFile* file = new TFile("VPHits.root");
		TTree* tree = (TTree*) file->Get("VPHits/VPHits");
		static POINT point;
		//static POINT point(tree->GetEntries());;
		std::vector<double> x,y,z;
		tree->SetBranchAddress("x_hit",&point.x);
		tree->SetBranchAddress("y_hit",&point.y);
		tree->SetBranchAddress("z_hit",&point.z);
		TBranch* branch = tree->GetBranch("x_hit");
		for (unsigned long i=0; i<tree->GetEntries(); i++) {
			tree->GetEntry(i);
			x.push_back((double)point.x);
			y.push_back((double)point.y);
			z.push_back((double)point.z);
		}
		int threshold = 10;
		HT(y,z,threshold); // or HT(x,z);
		theApp.Run();
	}
	else if (FLAG == 0) {
		std::vector<double> x,y;
		x += 4.0,3.2,1.8,1.0,1.0,2.0,3.0,1.0,4.0,2.7;
		y += 4.0,3.2,1.8,1.0,3.5,2.5,1.5,5.0,1.2,4.1;
		int threshold = 3;
		HT(x,y,threshold);	
		theApp.Run();
	}
	
	return 0;
}
