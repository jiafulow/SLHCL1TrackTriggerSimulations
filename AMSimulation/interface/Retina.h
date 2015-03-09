#ifndef AMSimulation_Retina_h_
#define AMSimulation_Retina_h_

#include <vector>


struct xyPoint {
  double x;
  double y;
  unsigned hitRef;
};

struct pqPoint_i {
  int p;
  int q;
};

struct pqPoint {
  double p;
  double q;
  double w;
};

enum FitView { XY, RZ };

class Retina {

 private:

  std::vector<xyPoint> hits;
  unsigned int pbins;
  unsigned int qbins;
  double pmin;
  double pmax;
  double offset;
  double qmin;
  double qmax;
  double pbinsize;
  double qbinsize;
  std::vector<double> sigma;
  double minWeight;
  unsigned int para;
  FitView view;

  std::vector <std::vector <double> > Grid;
  std::vector <pqPoint> pqCollection;

  void    makeGrid();
  double  getResponseXpXm(double x_plus, double x_minus);
  pqPoint findMaximumInterpolated(pqPoint_i point_i, double w);

 public:

  Retina(std::vector<xyPoint> hits_, unsigned int pbins_, unsigned int qbins_,
         double pmin_, double pmax_, double qmin_, double qmax_,
         std::vector<double> sigma_, double minWeight_, FitView view_);
  ~Retina();

  void fillGrid();
  void dumpGrid(int eventNum=0,int step=1,int imax=0);
  void findMaxima();
  void printMaxima();
  std::vector <pqPoint> getMaxima();
  pqPoint getBestPQ();

};

#endif
