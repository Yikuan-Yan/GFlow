#ifndef __STAT_PLOT_H__
#define __STAT_PLOT_H__

#include "Particle.h"

typedef void (*StatPlot) (const vector<Particle>&, vector<double>&, const double, const double);

inline void Plot_Velocity(const vector<Particle>& particles, vector<double>& statVector, const double lower, const double upper) {
  int bins = statVector.size();
  if (bins==0) return;
  // Bin data
  double dq = (upper-lower)/bins;
  for (const auto &p : particles) {
    double data = sqrt(sqr(p.velocity));
    int b = (data-lower)/dq;
    if (b<0) b=0;
    if (bins<=b) b=bins-1;
    ++statVector.at(b);
  }
}

inline void Plot_Force_Vs_Depth(const vector<Particle>& particles, vector<double>& statVector, const double lower, const double upper) {
  int bins = statVector.size();
  if (bins==0) return;
  // Bin data
  double dq = (upper-lower)/bins;
  for (const auto &p : particles) {
    double data = p.position.y;
    double press = sqrt(sqr(p.force))/(2*PI*p.sigma);
    int b = (data-lower)/dq;
    if (b<0) b=0;
    if (bins<=b) b=bins-1;
    statVector.at(b) += press;
  }
}

#endif // __STAT_PLOT_H__
