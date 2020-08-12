/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2020, 
 *  Max Planck Institute for Intelligent Systems (MPI-IS).
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the MPI-IS nor the names
 *     of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Andreas Orthey */

#ifndef OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_PATH_SECTION_
#define OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_PATH_SECTION_
#include <ompl/multilevel/datastructures/BundleSpaceGraph.h>

namespace ompl
{
    namespace base
    {
        OMPL_CLASS_FORWARD(Path);
    }
    namespace geometric
    {
        OMPL_CLASS_FORWARD(PathGeometric);
    }
    namespace multilevel
    {
        OMPL_CLASS_FORWARD(BundleSpaceGraph);

        /// \brief Representation of path restriction (set of all elements of bundle space
        // which project onto a given base path --- i.e. the union of fibers over base path).
        //
        // This class has additional
        // functionalities to find path sections (paths lying inside path
        // restriction) using different interpolation methods (shortest L1, L2
        // paths)

        class BundleSpacePathRestriction
        {
        public:
            using Configuration = ompl::multilevel::BundleSpaceGraph::Configuration;
            BundleSpacePathRestriction() = delete;
            BundleSpacePathRestriction(BundleSpaceGraph *);

            virtual ~BundleSpacePathRestriction();

            virtual void clear();

            void setBasePath(base::PathPtr);
            void setBasePath(std::vector<base::State *>);

            bool checkSection(Configuration *const xStart, Configuration *const xGoal);

            bool checkSectionRecursiveRepair(Configuration *const xStart, Configuration *const xGoal,
                                             const std::vector<base::State *> basePath, bool interpolateL1 = true,
                                             unsigned int depth = 0, double startLength = 0.0);

            bool sideStepAlongFiber(const base::State *xBase, base::State *xBundle);

            void sanityCheckSection();

            Configuration *addFeasibleSegment(Configuration *xLast, base::State *sNext);

            void addFeasibleGoalSegment(Configuration *const xLast, Configuration *const xGoal);

            // Note:
            // const ptr* means that the pointer itself is const
            // ptr* const means that the content of the pointer is const (but ptr
            // can change)

            bool hasFeasibleSection(Configuration *const, Configuration *const);

            //\brief Interpolate along restriction using L2 metric
            //  ---------------
            //            ____x
            //       ____/
            //   ___/
            //  x
            //  ---------------
            std::vector<base::State *> interpolateSectionL2(const base::State *xFiberStart,
                                                            const base::State *xFiberGoal,
                                                            const std::vector<base::State *> basePath);

            //\brief Interpolate along restriction using L1 metric
            //  ---------------
            //                x
            //                |
            //                |
            //  x_____________|
            //  ---------------
            std::vector<base::State *> interpolateSectionL1FL(const base::State *xFiberStart,
                                                              const base::State *xFiberGoal,
                                                              const std::vector<base::State *> basePath);

            //\brief Interpolate along restriction using L1 metric, but first
            // interpolate along fiber
            //  ---------------
            //   _____________x
            //  |
            //  |
            //  x
            //  ---------------
            std::vector<base::State *> interpolateSectionL1FF(const base::State *xFiberStart,
                                                              const base::State *xFiberGoal,
                                                              const std::vector<base::State *> basePath);

        protected:
            BundleSpaceGraph *bundleSpaceGraph_;

            std::vector<base::State *> basePath_;

            double lengthBasePath_{0.0};
            std::vector<double> intermediateLengthsBasePath_;

            base::State *xBaseTmp_{nullptr};
            base::State *xBundleTmp_{nullptr};

            base::State *xFiberStart_{nullptr};
            base::State *xFiberGoal_{nullptr};
            base::State *xFiberTmp_{nullptr};

            std::pair<base::State *, double> lastValid_;
        };
    }
}

#endif