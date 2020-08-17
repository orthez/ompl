#ifndef OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_PATH_SECTION__
#define OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_PATH_SECTION__
#include <ompl/multilevel/datastructures/BundleSpaceGraph.h>
#include <ompl/util/ClassForward.h>

namespace ompl
{
    namespace multilevel
    {
        OMPL_CLASS_FORWARD(PathRestriction);

        class PathSection
        {
          public:
            using Configuration = ompl::multilevel::BundleSpaceGraph::Configuration;

            PathSection() = delete;
            PathSection(PathRestriction*);
            virtual ~PathSection();
            
            void interpolateL2(
                const base::State* fiberStart, 
                const base::State* fiberGoal);

            void interpolateL1FiberFirst(
                const base::State* fiberStart, 
                const base::State* fiberGoal);

            void interpolateL1FiberLast(
                const base::State* fiberStart, 
                const base::State* fiberGoal);

            /** \brief Checks if section is feasible
             *
             *  @retval True if feasible and false if only partially feasible
             */
            // bool checkMotion(std::pair<base::State *, double>& lastValid_);
            bool checkMotion(
                Configuration* const xStart, 
                Configuration* const xGoal, 
                std::pair<base::State *, double>& lastValid_);

            /** \brief checks if section is feasible */
            void sanityCheck();

            double getLastValidBasePathLocation();

            int getLastValidBasePathIndex();

            /** \brief Add vertex for sNext and edge to xLast by assuming motion
             * is valid  */
            Configuration *addFeasibleSegment(Configuration *xLast, base::State *sNext);

            void addFeasibleGoalSegment(Configuration *const xLast, Configuration *const xGoal);

            Configuration *getLastValidConfiguration();

          protected:

            PathRestriction *restriction_;

            /** \brief Interpolated section along restriction */
            std::vector<base::State*> section_;

            std::vector<int> sectionBaseStateIndices_;

            Configuration *xBundleLastValid_;

            /** \brief Last valid state on feasible segment */
            std::pair<base::State *, double> lastValid_;

            double lastValidLocationOnBasePath_;
            int lastValidIndexOnBasePath_;

            base::State *xBaseTmp_{nullptr};
            base::State *xBundleTmp_{nullptr};

            base::State *xFiberStart_{nullptr};
            base::State *xFiberGoal_{nullptr};
            base::State *xFiberTmp_{nullptr};
        };
    }
}
#endif
