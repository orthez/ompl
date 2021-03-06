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

#ifndef OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_BUNDLE_SUBSPACE_
#define OMPL_MULTILEVEL_PLANNERS_BUNDLESPACE_BUNDLE_SUBSPACE_
#include <ompl/base/State.h>
#include <ompl/base/StateSpace.h>
#include <ompl/base/StateSpaceTypes.h>
#include "BundleSpaceComponentTypes.h"

namespace ompl
{
    namespace multilevel
    {
        class BundleSpaceComponent
        {
        public:
            BundleSpaceComponent(
                base::StateSpacePtr BundleSpace, 
                base::StateSpacePtr BaseSpace);

            virtual ~BundleSpaceComponent() = default;

            virtual void projectFiber(
                const ompl::base::State *xBundle, 
                ompl::base::State *xFiber) const = 0;

            virtual void projectBase(
                const ompl::base::State *xBundle, 
                ompl::base::State *xBase) const = 0;

            virtual void liftState(
                const ompl::base::State *xBase, 
                const ompl::base::State *xFiber,
                ompl::base::State *xBundle) const = 0;

            ompl::base::StateSpacePtr getFiberSpace() const;

            void initFiberSpace();

            bool isDynamic() const;

            /// Dimension of Fiber Space
            unsigned int getFiberDimension() const;
            /// Dimension of Base Space
            unsigned int getBaseDimension() const;
            /// Dimension of Bundle Space
            unsigned int getDimension() const;
            /// Type of Bundle Space
            BundleSpaceComponentType getType() const;
            void setType(BundleSpaceComponentType &);

            std::string getTypeAsString() const;
            std::string getFiberTypeAsString() const;
            std::string getBundleTypeAsString() const;
            std::string getBaseTypeAsString() const;

            friend std::ostream &operator<<(
                std::ostream &out, 
                const BundleSpaceComponent &);

        protected:
            virtual ompl::base::StateSpacePtr computeFiberSpace() = 0;
            virtual void print(std::ostream &out) const;
            std::string stateTypeToString(base::StateSpacePtr) const;

            base::StateSpacePtr BundleSpace_{nullptr};
            base::StateSpacePtr BaseSpace_{nullptr};
            base::StateSpacePtr FiberSpace_{nullptr};

            bool isDynamic_{false};

            BundleSpaceComponentType type_;
        };
    }
}

#endif
