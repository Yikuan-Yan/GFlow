#include "GField.h"

GField::GField() {
  zeroPointers();
}

GField::GField(Shape s) {
  zeroPointers();
  initialize(s, true);
}

GField::GField(int* sizes, int dims) {
  zeroPointers();
  Shape s(sizes, dims);
  initialize(s, true);
}

GField::~GField() {
  clean();
}

GField& GField::operator=(const GField& F) {
  clean();
  shape = F.shape;
  total = F.total;
  int rank = shape.getRank();
  create(F.spacing, spacing, rank);
  create(F.stride, stride, rank);
  create(F.array, array, total);
  create(F.lowerBounds, lowerBounds, rank);
  create(F.upperBounds, upperBounds, rank);
  create(F.wrapping, wrapping, rank);
}

void GField::at_address(int& add, Index index) const { // Doesn't check (for speed)
  for (int i=0; i<index.size(); i++) add += stride[i]*index.at(i);
}

void GField::at_address(int& add, vector<int> index) const { // Doesn't check (for speed)
  for (int i=0; i<index.size(); i++) add += stride[i]*index.at(i);
}

void GField::initialize(Shape s, bool all) {
  int lastRank = shape.getRank(), lastTotal = shape.getTotal();
  int rank = s.getRank();
  if (rank==0 || s.getTotal()==0) return;
  total = s.getTotal();
  shape = s;
  // Set stride array (recalculate on change of Shape)
  if (stride) delete [] stride;
  stride = new int[rank];
  int count = 1;
  for (int i=0; i<rank; i++) {
    count *= shape.dims[i];
    stride[i] = total/count;
  }
  // Set up array (recalculate on change of Shape)
  if (total!=lastTotal || all) {
    if (array) delete [] array;
    array = 0;
  }
  if (array==nullptr) array = new double[total];
  for (int i=0; i<total; i++) array[i] = 0.; // Always reset values
  // Set up bounds (recalculate when given)
  if (shape.getRank()!=lastRank || all) {
    if (lowerBounds) delete [] lowerBounds;
    lowerBounds = new double[rank];
    for (int i=0; i<rank; i++) lowerBounds[i] = -1.;
    if (upperBounds) delete [] upperBounds;
    upperBounds = new double[rank];
    for (int i=0; i<rank; i++) upperBounds[i] = 1.;
  }
  // Set up wrapping (recalculate when given)
  if (shape.getRank()!=lastRank || all) {
    if (wrapping) delete [] wrapping;
    wrapping = new bool[rank];
    for (int i=0; i<rank; i++) wrapping[i] = false;
  }
  // Set up spacing (recalculate on change of Shape or bounds)
  if (spacing) delete [] spacing;
  spacing = new double[rank];
  for (int i=0; i<rank; i++) spacing[i] = (upperBounds[i]-lowerBounds[i])/shape.at(i);
}

double& GField::at(Index index) {
  int address = 0;
  at_address(address, index);
  if (total<=address) throw GFieldOutOfBounds();
  return array[address];
}

double GField::at(Index index) const {
  int address = 0;
  at_address(address, index);
  if (total<=address) throw GFieldOutOfBounds();
  return array[address];
}

double& GField::at(vector<int> index) {
  int address = 0;
  at_address(address, index);
  if (total<=address) throw GFieldOutOfBounds();
  return array[address];
}

double GField::at(vector<int> index) const {
  int address = 0;
  at_address(address, index);
  if (total<=address) throw GFieldOutOfBounds();
  return array[address];
}

double GField::derivative(Index I, Index pos) const {
  // Assumes a single first derivative
  Index up = pos+I, lw = pos-I;
  int ind1 = mod(up);
  int ind2 = mod(lw);
  // Find which index we are differentiating
  int index = 0;
  for (int i=0; i<I.size(); i++)
    if (I.at(i)==1) {
      index = i;
      break;
    }
  double invh = 1./spacing[index];
  double der = 0;
  if (ind1==0 && ind2==0) der = (at(up)-at(lw))*0.5*invh;
  else if (ind1!=0) der = (at(I)-at(lw))*invh;
  else der = (at(up)-at(I))*invh;

  return der;
}

gVector GField::getPosition(vector<int> indices) const {
  gVector pos;
  pos.resize(indices.size());
  for (int i=0; i<indices.size(); i++) 
    pos.at(i) = indices.at(i)*spacing[i]+lowerBounds[i];
  return pos;
}

gVector GField::getPosition(Index indices) const {
  gVector pos;
  pos.resize(indices.size());
  for (int i=0; i<indices.size(); i++)
    pos.at(i) = indices.at(i)*spacing[i]+lowerBounds[i];
  return pos;
}

double GField::getPosition(int index, int grid) const {
  if (shape.rank<=index) throw GFieldIndexOutOfBounds();
  return grid*spacing[index]+lowerBounds[index];
}

double GField::integrate() {
  double total = 0.;
  for (int i=0; i<shape.getTotal(); i++) total += array[i];
  double dV = 1;
  for (int i=0; i<shape.getRank(); i++) dV *= spacing[i];
  return total*dV;
}

double GField::integrate(int index, Index pos) {
  int add = 0;
  pos.at(index) = 0; // Start at the beginning
  at_address(add, pos);
  // Integrate
  int step = stride[index];
  double total = 0.;
  for (int i=0; i<shape.at(index); i++, add+=step) total += array[add];  
  total *= spacing[index];
  return total;
}

GField& GField::operator+=(const GField& F) {
  if (F.shape!=shape) throw GFieldMismatch();
  for (int i=0; i<total; i++)
    array[i] += F.array[i];
}

/// Not safe (no checking) version
void plusEqNS(GField& G, const GField& H, double mult) {
  for (int i=0; i<G.total; i++) G.array[i] += mult*H.array[i];
}

void GField::setWrapping(int i, bool w) {
  if (shape.rank<=i) GFieldDimensionMismatch();  
  wrapping[i] = w;
}

void GField::setBounds(int i, double l, double u) {
  if (shape.rank<=i) throw GFieldDimensionMismatch();
  upperBounds[i] = u; lowerBounds[i] = l;
  spacing[i] = (upperBounds[i]-lowerBounds[i])/shape.at(i);
}

void GField::set(gFunction function) {
  Index index;
  index.resize(shape.rank);
  GField::setHelper(index, 0, function, *this);
}

void GField::parabolaZero(int index, int N) {
  
}

std::ostream& operator<<(std::ostream& out, const GField& G) {
  vector<int> indices;
  string str = "{", str2;
  stringstream stream;
  GField::writeHelper(indices, stream, G);
  stream >> str2;
  str += str2;
  str.pop_back(); // Put back the last ','
  str += "}";
  out << str;
  return out;
}

inline void GField::writeHelper(vector<int> indices, std::ostream& out, const GField& G) {
  int step = indices.size();
  if (step==G.shape.rank-1) { // Base case
    for (int i=0; i<G.shape.dims[step]; i++) {
      indices.push_back(i); // Add i
      gVector coord = G.getPosition(indices);
      out << '{' << bare_representation(coord) << ',' << G.at(indices) << '}';
      indices.pop_back();   // Remove i
      out << ",";
    }
  }
  else {
    for (int i=0; i<G.shape.dims[step]; i++) {
      indices.push_back(i); // Add i
      writeHelper(indices, out, G);
      indices.pop_back();   // Remove i
    }
  }
}

std::istream& operator>>(std::ostream& in, GField& G) {
  
}

inline void GField::setHelper(Index index, int step, gFunction function, GField& G) {
  if (step==G.shape.rank-1) { // Base case
    index.at(step)=0;
    for (int i=0; i<G.shape.dims[step]; i++) {
      gVector coord = G.getPosition(index);
      G.at(index) = function(coord);
      index.at(step)++;
    }
  }
  else {
    index.at(step)=0;
    for (int i=0; i<G.shape.dims[step]; i++) {
      setHelper(index, step+1, function, G);
      index.at(step)++;
    }
  }
}

int GField::mod(Index& index) const {
  int oob = 0;
  for (int i=0; i<index.size(); i++) {
    int &c = index.at(i);
    bool w = wrapping[i];

    if (c<0) {
      if (w) c += shape.at(i);
      else oob = -1;
    }
    else if (shape.at(i)<=c) {
      if (w) c -= shape.at(i);
      else oob = 1;
    }
  }
  return oob;
}

void GField::clean() {
  if (spacing) delete [] spacing;
  if (stride) delete [] stride;
  if (array) delete [] array;
  if (lowerBounds) delete [] lowerBounds;
  if (upperBounds) delete [] upperBounds;
  if (wrapping) delete [] wrapping;
  zeroPointers();
}

void GField::zeroPointers() {
  spacing = nullptr;
  stride = nullptr;
  array = nullptr;
  lowerBounds = nullptr;
  upperBounds = nullptr;
  wrapping = nullptr;
}
