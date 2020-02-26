/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2020, University of Stuttgart
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
 *   * Neither the name of the University of Stuttgart nor the names
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

/* Author: Andreas Orthey, Sohaib Akbar */

#ifndef OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_QMPSTAR_
#define OMPL_GEOMETRIC_PLANNERS_QUOTIENTSPACE_QMPSTAR_
#include <ompl/geometric/planners/quotientspace/datastructures/BundleSpaceSequence.h>
#include <ompl/geometric/planners/quotientspace/algorithms/QMPStarImpl.h>

namespace ompl
{
    namespace geometric
    {
        /**
             @anchor QMPStar

             @par Short description
             Quotient space roadMap Planner Star (QMP*) generalizes the PRM* algorithm to bundle spaces. 

             @par External documentation (QMP)
             A. Orthey, A. Escande and E. Yoshida,
             Quotient-Space Motion Planning,
             in <em>International Conference on Intelligent Robots and Systems</em>, 2018,
             [[PDF]](https://arxiv.org/abs/1807.09468)

             @par External documentation (PRM*)
             S. Karaman and E. Frazzoli, Sampling-based
             Algorithms for Optimal Motion Planning, International Journal of Robotics
             Research, vol. 30, no.7, pp. 846-894, 2011.
             DOI: [10.1177/0278364911406761](http://dx.doi.org/10.1177/0278364911406761)<br>
        */

        /** \brief Quotient-space roadMap Planner Start (QMPStar) Algorithm */
        typedef ompl::geometric::BundleSpaceSequence<ompl::geometric::QMPStarImpl> QMPStar;

    }  // namespace geometric
}  // namespace ompl

#endif
