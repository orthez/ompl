#include <ompl/multilevel/datastructures/pathrestriction/PathRestriction.h>
#include <ompl/multilevel/datastructures/pathrestriction/PathSection.h>
#include <ompl/multilevel/datastructures/pathrestriction/BasePathHead.h>
#include <ompl/multilevel/datastructures/pathrestriction/FindSection.h>
#include <ompl/multilevel/datastructures/graphsampler/GraphSampler.h>

namespace ompl
{
    namespace magic
    {
        static const unsigned int PATH_SECTION_MAX_WRIGGLING = 10;
        static const unsigned int PATH_SECTION_MAX_DEPTH = 3;
        static const unsigned int PATH_SECTION_MAX_BRANCHING = 20;
        static const unsigned int PATH_SECTION_MAX_TUNNELING = 100;

        static const unsigned int PATH_SECTION_MAX_FIBER_SAMPLING = 10;
    }
}

using namespace ompl::multilevel;

FindSection::FindSection(PathRestriction* restriction):
  restriction_(restriction)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    if (graph->getFiberDimension() > 0)
    {
        base::SpaceInformationPtr fiber = graph->getFiber();
        xFiberStart_ = fiber->allocState();
        xFiberGoal_ = fiber->allocState();
        xFiberTmp_ =  fiber->allocState();
        validFiberSpaceSegmentLength_ = fiber->getStateSpace()->getLongestValidSegmentLength();
    }
    if (graph->getBaseDimension() > 0)
    {
        base::SpaceInformationPtr base = graph->getBase();
        xBaseTmp_ = base->allocState();
        validBaseSpaceSegmentLength_ = base->getStateSpace()->getLongestValidSegmentLength();
    }
    base::SpaceInformationPtr bundle = graph->getBundle();
    xBundleTmp_ = bundle->allocState();
    xBundleTemporaries_.resize(magic::PATH_SECTION_MAX_DEPTH * magic::PATH_SECTION_MAX_BRANCHING);
    bundle->allocStates(xBundleTemporaries_);

    validBundleSpaceSegmentLength_ = bundle->getStateSpace()->getLongestValidSegmentLength();

    neighborhoodRadiusBaseSpaceLambda_ = 1e-4;

    neighborhoodRadiusBaseSpace_.setLambda(neighborhoodRadiusBaseSpaceLambda_);
    neighborhoodRadiusBaseSpace_.setInitValue(0.0);
    neighborhoodRadiusBaseSpace_.setTargetValue(10*validBaseSpaceSegmentLength_);
}

FindSection::~FindSection()
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    if (graph->getFiberDimension() > 0)
    {
        base::SpaceInformationPtr fiber = graph->getFiber();
        fiber->freeState(xFiberStart_);
        fiber->freeState(xFiberGoal_);
        fiber->freeState(xFiberTmp_);
    }
    if (graph->getBaseDimension() > 0)
    {
        base::SpaceInformationPtr base = graph->getBase();
        base->freeState(xBaseTmp_);
    }
    base::SpaceInformationPtr bundle = graph->getBundle();
    bundle->freeState(xBundleTmp_);
    bundle->freeStates(xBundleTemporaries_);
}

bool FindSection::solve(BasePathHeadPtr& head)
{
    BasePathHeadPtr head2(head);

    ompl::time::point tStart = ompl::time::now();
    bool foundFeasibleSection = recursivePatternSearch(head);
    ompl::time::point t1 = ompl::time::now();

    OMPL_WARN("FindSection required %.2fs.", ompl::time::seconds(t1 - tStart));

    if(!foundFeasibleSection)
    {
        foundFeasibleSection = recursivePatternSearch(head2, false);
        ompl::time::point t2 = ompl::time::now();
        OMPL_WARN("FindSection2 required %.2fs.", ompl::time::seconds(t2 - t1));
    }

    return foundFeasibleSection;
}

bool FindSection::findFeasibleStateOnFiber(
    const base::State *xBase, 
    base::State *xBundle)
{
    unsigned int ctr = 0;
    bool found = false;

    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    base::SpaceInformationPtr bundle = graph->getBundle();
    base::SpaceInformationPtr base = graph->getBundle();
    // const ompl::base::StateSamplerPtr samplerBase = graph->getBaseSamplerPtr();

    while (ctr++ < magic::PATH_SECTION_MAX_FIBER_SAMPLING && !found)
    {
        // sample model fiber
        // samplerBase->sampleUniformNear(xBaseTmp_, xBase, validBaseSpaceSegmentLength_);

        graph->sampleFiber(xFiberTmp_);

        graph->liftState(xBase, xFiberTmp_, xBundle);

        //New sample must be valid AND not reachable from last valid
        if (bundle->isValid(xBundle))
        {
            found = true;
        }
    }
    return found;
}

bool FindSection::sideStepAlongFiber(Configuration* &xOrigin, base::State *state)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    base::SpaceInformationPtr bundle = graph->getBundle();
    if (bundle->checkMotion(xOrigin->state, state))
    {
        //#########################################################
        // side step was successful.
        // Now interpolate from there to goal
        //#########################################################

        Configuration *xSideStep = new Configuration(bundle, state);
        graph->addConfiguration(xSideStep);
        graph->addBundleEdge(xOrigin, xSideStep);

        xOrigin = xSideStep;

        return true;

    }
    return false;
}

bool FindSection::tripleStep(
    BasePathHeadPtr& head,
    const base::State *sBundleGoal,
    double locationOnBasePathGoal)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    base::SpaceInformationPtr bundle = graph->getBundle();
    base::SpaceInformationPtr base = graph->getBase();
    base::SpaceInformationPtr fiber = graph->getFiber();

    base::State* xBundleStartTmp = bundle->allocState();
    base::State* xBundleGoalTmp = bundle->allocState();
    base::State* xBase = base->cloneState(head->getStateBase());
    const base::State *sBundleStart = head->getState();

    // const ompl::base::StateSamplerPtr fiberSampler = graph->getFiberSamplerPtr();
    // const ompl::base::StateSamplerPtr baseSampler = graph->getBaseSamplerPtr();

    graph->projectFiber(sBundleStart, xFiberStart_);
    graph->projectFiber(sBundleGoal, xFiberGoal_);

    bool found = false;

    //mid point heuristic 
    fiber->getStateSpace()->interpolate(xFiberStart_, xFiberGoal_, 0.5, xFiberTmp_);

    double location = head->getLocationOnBasePath() - validBaseSpaceSegmentLength_;

    //Triple step connection attempt
    // xBundleStartTmp <------- xBundleStart
    //     |
    //     |
    //     |
    //     v
    // xBundleGoalTmp -------> xBundleGoal

    while(!found && location >= 0)
    {
        restriction_->interpolateBasePath(location, xBase);

        graph->liftState(xBase, xFiberTmp_, xBundleStartTmp);

        if (bundle->isValid(xBundleStartTmp))
        {
            graph->liftState(xBase, xFiberStart_, xBundleStartTmp);
            graph->liftState(xBase, xFiberGoal_, xBundleGoalTmp);

            if(bundle->isValid(xBundleStartTmp)
                && bundle->isValid(xBundleGoalTmp))
            {
                if(bundle->checkMotion(xBundleStartTmp, xBundleGoalTmp))
                {
                    
                    bool feasible = true;

                    double fiberDist = fiber->distance(xFiberStart_, xFiberGoal_);
                    double fiberStepSize = validFiberSpaceSegmentLength_;

                    if(!bundle->checkMotion(sBundleStart, xBundleStartTmp))
                    {
                      feasible = false;

                      double fiberLocation = 0.5*fiberDist;
                      do{
                        fiberLocation -= fiberStepSize;
                        
                        fiber->getStateSpace()->interpolate(
                            xFiberStart_, xFiberGoal_, fiberLocation/fiberDist, xFiberTmp_);


                        graph->liftState(xBase, xFiberTmp_, xBundleStartTmp);

                        if(bundle->checkMotion(sBundleStart, xBundleStartTmp)
                            && bundle->checkMotion(xBundleStartTmp, xBundleGoalTmp))
                        {
                          feasible = true;
                          break;
                        }
                      }while(fiberLocation > -0.5*fiberDist);
                      //try to repair
                    }
                    if(feasible && !bundle->checkMotion(xBundleGoalTmp, sBundleGoal))
                    {
                      feasible = false;

                      double fiberLocation = 0.5*fiberDist;
                      do{
                        fiberLocation += fiberStepSize;

                        fiber->getStateSpace()->interpolate(
                            xFiberStart_, xFiberGoal_, fiberLocation/fiberDist, xFiberTmp_);

                        // graph->liftState(xBaseTmp_, xFiberTmp_, xBundleGoalTmp);
                        graph->liftState(xBase, xFiberTmp_, xBundleGoalTmp);

                        if(bundle->checkMotion(xBundleGoalTmp, sBundleGoal)
                            && bundle->checkMotion(xBundleStartTmp, xBundleGoalTmp))
                        {
                          feasible = true;
                          break;
                        }

                      }while(fiberLocation < 1.5*fiberDist);
                    }
                    if(feasible)
                    {
                        found = true;
                    }
                    break;
                }
            }
        }

        location -= validBaseSpaceSegmentLength_;
    }

    if(found)
    {
        Configuration *xBackStep = new Configuration(bundle, xBundleStartTmp);
        graph->addConfiguration(xBackStep);
        graph->addBundleEdge(head->getConfiguration(), xBackStep);


        Configuration *xSideStep = new Configuration(bundle, xBundleGoalTmp);
        graph->addConfiguration(xSideStep);
        graph->addBundleEdge(xBackStep, xSideStep);

        //xBaseTmp_ is on last valid fiber. 
        Configuration *xGoal = new Configuration(bundle, sBundleGoal);
        graph->addConfiguration(xGoal);
        graph->addBundleEdge(xSideStep, xGoal);

        // std::cout << "INSERT TRIPLE STEP (BWD-SIDE-FWD)" << std::endl;
        // bundle->printState(xBundleLastValid->state);
        // bundle->printState(xBackStep->state);
        // bundle->printState(xSideStep->state);
        // bundle->printState(xGoal->state);

        head->setCurrent(xGoal, locationOnBasePathGoal);
    }

    bundle->freeState(xBundleStartTmp);
    bundle->freeState(xBundleGoalTmp);
    base->freeState(xBase);
    return found;
}

bool FindSection::tunneling(
    BasePathHeadPtr& head)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    const ompl::base::StateSamplerPtr bundleSampler = graph->getBundleSamplerPtr();
    const ompl::base::StateSamplerPtr baseSampler = graph->getBaseSamplerPtr();
    ompl::base::SpaceInformationPtr bundle = graph->getBundle();
    ompl::base::SpaceInformationPtr base = graph->getBase();

    double curLocation = head->getLocationOnBasePath();

    bool found = false;

    bool hitTunnel = false;

    const ompl::base::StateSamplerPtr fiberSampler = graph->getFiberSamplerPtr();

    while (curLocation < restriction_->getLengthBasePath() && !found)
    {
        curLocation += validBaseSpaceSegmentLength_;

        restriction_->interpolateBasePath(curLocation, xBaseTmp_);

        for(uint k = 0; k < 5; k++)
        {
            fiberSampler->sampleUniformNear(xFiberTmp_, 
                head->getStateFiber(), validFiberSpaceSegmentLength_);

            graph->liftState(xBaseTmp_, xFiberTmp_, xBundleTmp_);

            if(bundle->isValid(xBundleTmp_))
            {
                if(hitTunnel)
                {
                    found = true;
                    break;
                }
            }else{
                hitTunnel = true;
            }
        }
    }
    if(!found)
    {
    }else{

        //length of tunnel


        if(found)
        {
            found = false;


            const double locationEndTunnel = curLocation;
            // const double lengthTunnel = curLocation - head->getLocationOnBasePath();
            const base::State* xBundleTunnelEnd = bundle->cloneState(xBundleTmp_);

            Configuration *last = head->getConfiguration();

            curLocation = head->getLocationOnBasePath();

            double bestDistance = bundle->distance(last->state, xBundleTunnelEnd);

            bool makingProgress = true;

            base::State* xBase = base->allocState();

            while (curLocation < locationEndTunnel && makingProgress)
            {
                if(bundle->checkMotion(last->state, xBundleTunnelEnd))
                {
                    Configuration *xTunnel = new Configuration(bundle, xBundleTunnelEnd);
                    graph->addConfiguration(xTunnel);
                    graph->addBundleEdge(last, xTunnel);

                    head->setCurrent(xTunnel, locationEndTunnel);

                    base->freeState(xBase);

                    return true;
                }

                curLocation += validBaseSpaceSegmentLength_;

                restriction_->interpolateBasePath(curLocation, xBaseTmp_);

                makingProgress = false;

                unsigned int ctr = 0;

                while(ctr++ < magic::PATH_SECTION_MAX_TUNNELING)
                {
                    double d = neighborhoodRadiusBaseSpace_();

                    baseSampler->sampleUniformNear(xBase, xBaseTmp_, d);

                    fiberSampler->sampleUniformNear(xFiberTmp_, head->getStateFiber(), 
                        4*validFiberSpaceSegmentLength_);

                    graph->liftState(xBase, xFiberTmp_, xBundleTmp_);

                    if(bundle->isValid(xBundleTmp_))
                    {
                        double d = bundle->distance(xBundleTmp_, xBundleTunnelEnd);
                        if( d < bestDistance)
                        {
                            if(bundle->checkMotion(last->state, xBundleTmp_))
                            {
                                Configuration *xTunnelStep = new Configuration(bundle, xBundleTmp_);
                                graph->addConfiguration(xTunnelStep);
                                graph->addBundleEdge(last, xTunnelStep);

                                last = xTunnelStep;

                                makingProgress = true;
                                bestDistance = d;
                                //add new configurations, but do not change head
                                //yet
                            }
                        }
                    }
                }
            }
            // if(!makingProgress)
            // {
            //   std::cout << "Stopped tunnel at " << curLocation << " of tunnel distance "
            //    << head->getLocationOnBasePath() << " to " << locationEndTunnel << std::endl;
            // }
            base->freeState(xBase);
        }
    }

    return false;
}

bool FindSection::wriggleFree(BasePathHeadPtr& head)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    ompl::base::SpaceInformationPtr bundle = graph->getBundle();

    double curLocation = head->getLocationOnBasePath() + validBaseSpaceSegmentLength_;

    double epsilon = 2*validFiberSpaceSegmentLength_;

    const ompl::base::StateSamplerPtr fiberSampler = graph->getFiberSamplerPtr();
    const ompl::base::StateSamplerPtr baseSampler = graph->getBaseSamplerPtr();

    int steps = 0;
    while (curLocation < restriction_->getLengthBasePath())
    {
        unsigned int ctr = 0;
        bool madeProgress = false;

        restriction_->interpolateBasePath(curLocation, xBaseTmp_);

        while(ctr++ < magic::PATH_SECTION_MAX_WRIGGLING)
        {
            double d = neighborhoodRadiusBaseSpace_();

            baseSampler->sampleUniformNear(xBaseTmp_, xBaseTmp_, d);

            fiberSampler->sampleUniformNear(xFiberTmp_, head->getStateFiber(), epsilon);

            graph->liftState(xBaseTmp_, xFiberTmp_, xBundleTmp_);

            if(bundle->isValid(xBundleTmp_) &&
                bundle->checkMotion(head->getState(), xBundleTmp_))
            {
                Configuration *xWriggleStep = new Configuration(bundle, xBundleTmp_);
                graph->addConfiguration(xWriggleStep);
                graph->addBundleEdge(head->getConfiguration(), xWriggleStep);

                head->setCurrent(xWriggleStep, curLocation);

                madeProgress = true;
                steps++;
                break;
            }
        }
        if(!madeProgress)
        {
            break;
        }
        curLocation += validBaseSpaceSegmentLength_;
    }
    return (steps > 0);
}


#define COUT(depth) (std::cout << std::string(4*depth, '>'))

bool FindSection::recursivePatternSearch(
    BasePathHeadPtr& head,
    bool interpolateFiberFirst, 
    unsigned int depth)
{
    BundleSpaceGraph *graph = restriction_->getBundleSpaceGraph();
    base::SpaceInformationPtr bundle = graph->getBundle();
    base::SpaceInformationPtr base = graph->getBase();
    base::SpaceInformationPtr fiber = graph->getFiber();

    PathSectionPtr section = std::make_shared<PathSection>(restriction_);

    double prevLocation = head->getLocationOnBasePath();

    if (interpolateFiberFirst)
    {
        section->interpolateL1FiberFirst(head);
    }
    else
    {
        section->interpolateL1FiberLast(head);
    }


    if(section->checkMotion(head))
    {
        section->sanityCheck();
        return true;
    }

    static_cast<BundleSpaceGraph *>(graph->getParent())
       ->getGraphSampler()
       ->setPathBiasStartSegment(head->getLocationOnBasePath());

    //############################################################################
    //Get last valid state information
    //############################################################################

    if (depth >= magic::PATH_SECTION_MAX_DEPTH)
    {
        return false;
    }

    //############################################################################
    //Try different strategies to locally resolve constraint violation
    //Then call function recursively with clipped base path
    //############################################################################

    if(wriggleFree(head))
    {
        BasePathHeadPtr newHead(head);

        bool feasibleSection = recursivePatternSearch(newHead, false, depth + 1);
        if(feasibleSection)
        {
            return true;
        }
    }

    // if(tunneling(head))
    // {
    //     BasePathHeadPtr newHead(head);
    //     bool feasibleSection = recursivePatternSearch(newHead, false, depth + 1);
    //     if(feasibleSection)
    //     {
    //         return true;
    //     }
    // }

    double curLocation = head->getLocationOnBasePath();

    if(curLocation <= prevLocation)
    {
        //stuck in crevice
        return false;
    }

    ////search for alternative openings
    unsigned int infeasibleCtr = 0;

    // unsigned int size = 0;

    // int offset = depth * magic::PATH_SECTION_MAX_DEPTH;

    double location = head->getLocationOnBasePath() + validBaseSpaceSegmentLength_;

    for (unsigned int j = 0; j < magic::PATH_SECTION_MAX_BRANCHING; j++)
    {
        //############################################################################
        // Move base state forward by validsegmentlength_ to penetrate slightly
        // the constraints. This will ensure that sampling along fiber space
        // later will not hit the current feasible region (or potential similar infeasible
        // regions which occur trough symmetries of the robots geometry---like
        // rotations of a cylinder in front of a hole)
        //############################################################################
        // interpolateBasePath(locationOnBasePath + validBaseSpaceSegmentLength_, xBaseTmp_);

        restriction_->interpolateBasePath(location, xBaseTmp_);

        if (!findFeasibleStateOnFiber(xBaseTmp_, xBundleTmp_))
        {
            infeasibleCtr++;
            continue;
        }

        //check that we did not previously visited this area

        // bool found = true;
        // for(uint k = 0; k < size; k++)
        // {
        //     if(bundle->checkMotion(xBundleTemporaries_.at(offset + k), xBundleTmp_))
        //     {
        //         infeasibleCtr++;
        //         // std::cout << "Dismissed: already sampled area before" << std::endl;
        //         found = false;
        //         break;
        //     }
        // }
        // if(!found)
        // {
        //   continue;
        // }

        // bundle->copyState(xBundleTemporaries_.at(offset + size), xBundleTmp_);
        // size++;

        if(bundle->checkMotion(head->getState(), xBundleTmp_))
        {
            ////side step along fiber
            //Configuration *xSideStep = new Configuration(bundle, xBundleTmp_);
            //graph->addConfiguration(xSideStep);
            //graph->addBundleEdge(head->getConfiguration(), xSideStep);

            //head->setCurrent(xSideStep, curLocation);
        }else{
            if(tripleStep(head, xBundleTmp_, location))
            {
                BasePathHeadPtr newHead(head);

                bool feasibleSection = recursivePatternSearch(newHead, false, depth + 1);
                if(feasibleSection)
                {
                    return true;
                }
            }
        }

    }
    COUT(depth) << "Failed depth " << depth 
      << " after sampling " << infeasibleCtr 
      << " infeasible fiber elements on fiber";
    COUT(depth) << std::endl;
    return false;
}


