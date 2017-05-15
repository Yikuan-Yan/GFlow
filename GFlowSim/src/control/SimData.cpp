#include "SimData.hpp"

namespace GFlow {

  SimData::SimData(const Bounds& b, const Bounds& sb) : domain_size(0), domain_capacity(0), edge_size(0), edge_capacity(0), bounds(b), simBounds(sb), wrapX(true), wrapY(true) {};

  void SimData::reserve(int dcap, int ecap) {
    // Reserve all data arrays
    px.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    py.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    vx.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    vy.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    fx.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    fy.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    th.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    om.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    tq.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    sg.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    im.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    iI.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    rp.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    ds.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    cf.reserve2(domain_capacity, dcap, edge_capacity, ecap);
    it.reserve2(domain_capacity, dcap, edge_capacity, ecap, -1);
    // Set sizes
    domain_capacity = dcap;
    edge_capacity  = ecap;
  }

  void SimData::addWall(const Wall& w) {
    walls.push_back(w);
  }

  void SimData::addParticle(const Particle& p) {
    // Check if we have enough room
    if (domain_size==domain_capacity) {
      cout << "Sim data full.\n";
      throw false; // Eventually resize
    }
    // Set arrays
    px[domain_size] = p.position.x;
    py[domain_size] = p.position.y;
    vx[domain_size] = p.velocity.x;
    vy[domain_size] = p.velocity.y;
    fx[domain_size] = p.force.x;
    fy[domain_size] = p.force.y;
    th[domain_size] = p.theta;
    om[domain_size] = p.omega;
    tq[domain_size] = p.torque;
    sg[domain_size] = p.sigma;
    im[domain_size] = p.invMass;
    iI[domain_size] = p.invII;
    rp[domain_size] = p.repulsion;
    ds[domain_size] = p.dissipation;
    cf[domain_size] = p.coeff;
    it[domain_size] = p.interaction;
    // Increment domain_size
    ++domain_size;
  }

  list<Particle> SimData::getParticles() {
    list<Particle> plist;
    for (int i=0; i<domain_size; ++i) {
      if (it.at(i)>-1) {
	Particle P(px.at(i), py.at(i), sg.at(i));
	P.velocity = vec2(vx.at(i), vy.at(i));
	P.force    = vec2(fx.at(i), fy.at(i));
	P.theta    = th.at(i);
	P.omega    = om.at(i);
	P.torque   = tq.at(i);
	P.invMass  = im.at(i);
	P.invII    = iI.at(i);
	P.repulsion = rp.at(i);
	P.dissipation = ds.at(i);
	P.coeff = cf.at(i);
	P.interaction = it.at(i);
	// Push particle into list
	plist.push_back(P);
      }
    }
    return plist;
  }

  void SimData::wrap(RealType& x, RealType& y) {
    if (wrapX) {
      if (x<simBounds.left)
	x = simBounds.right-fmod(simBounds.left-x, simBounds.right-simBounds.left);
      else if (simBounds.right<x) 
	x = fmod(x-simBounds.left, simBounds.right-simBounds.left)+simBounds.left;
    }
    if (wrapY) {
      if (y<simBounds.bottom)
	y = simBounds.top-fmod(simBounds.bottom-y, simBounds.top-bounds.bottom);
      else if (simBounds.top<y)
	y = fmod(y-simBounds.bottom, simBounds.top-simBounds.bottom)+simBounds.bottom;
    }
  }

  void SimData::wrap(RealType& theta) {
    if (theta<0) theta = 2*PI-fmod(-theta, 2*PI);
    else if (2*PI<theta) theta = fmod(theta, 2*PI);
  }

  vec2 SimData::getDisplacement(const RealType ax, const RealType ay, const RealType bx, const RealType by) {
    // Get the correct (minimal) displacement vector pointing from B to A
    double X = ax-bx;
    double Y = ay-by;
    if (wrapX) {
      double dx = (simBounds.right-simBounds.left)-fabs(X);
      if (dx<fabs(X)) X = X>0 ? -dx : dx;
    }
    if (wrapY) {
      double dy =(simBounds.top-simBounds.bottom)-fabs(Y);
      if (dy<fabs(Y)) Y = Y>0 ? -dy : dy;
    }
    return vec2(X,Y);
  }

  vec2 SimData::getDisplacement(const vec2 a, const vec2 b) {
    return getDisplacement(a.x, a.y, b.x, b.y);
  }

#ifdef USE_MPI
  void SimData::atomMove() {

  }

  void SimData::atomCopy() {

  }
#endif
  
}
