#ifndef OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_MotionExplorer_
#define OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_MotionExplorer_
#include <ompl/geometric/planners/quotientspace/algorithms/ExplorerImpl.h>
#include <ompl/geometric/planners/quotientspace/algorithms/QuotientTopology.h>
//#include <ompl/geometric/planners/quotientspace/algorithms/QuotientTopologyControl.h>
#include <type_traits>
#include <queue>

namespace ob = ompl::base;
namespace og = ompl::geometric;

namespace ompl
{
    namespace geometric
    {
        typedef og::MotionExplorerImpl<og::QuotientTopology> MotionExplorer;
    }
}
#endif