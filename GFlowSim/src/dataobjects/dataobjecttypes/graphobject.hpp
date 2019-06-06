#ifndef __GRAPH_OBJECT_HPP__GFLOW__
#define __GRAPH_OBJECT_HPP__GFLOW__

#include "../../base/dataobject.hpp"

namespace GFlowSimulation {

  /**
  *  \brief The parent class for data objects whose output is a csv file of (x, f(x)) data
  *  that can be printed in a regular, 2d graph.
  *
  */
  class GraphObject : public DataObject {
  public:
    //! \brief Default constructor.
    GraphObject(GFlow*, const string&);

    //! \brief Axis name setting constructor.
    GraphObject(GFlow*, const string&, const string&, const string&);

    //! \brief Clear the data.
    virtual void pre_integrate() override;

    //! \brief Add an entry to the data array.
    void addEntry(RealType, RealType);

    //! \brief Compute averages of data.
    RealType ave() const;

    //! \brief The first entry in data.
    RPair first() const;
    
    //! \brief The last entry in data.
    RPair last() const;

    //! \brief The size of data.
    int size() const;

    //! \brief Set the names of the axes.
    void setAxes(const string&, const string&);

    //! \brief Set the print plot flag.
    void setPrintPlot(bool);

    //! \brief Write data to a file - if true, the string is a path, and you should use your own name as the file name.
    //!
    //! Returns true for success.
    virtual bool writeToFile(string, bool=true) override;

  protected:

    //! \brief The data, as a vector of pairs of real numbers
    vector<RPair> data;

    //! \brief The axis labels.
    string axis_x="x", axis_y="y";

    //! \brief If true, we use vistools to print the graph.
    bool print_plot = true;
  };

}
#endif // __GRAPH_OBJECT_HPP__GFLOW__