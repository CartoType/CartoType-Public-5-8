/*
CARTOTYPE_GRAPH.H
Copyright (C) 2014 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_GRAPH_H__
#define CARTOTYPE_GRAPH_H__

#include <cartotype_tree.h>

namespace CartoType
{

/**
A class to implement Dijkstra's algorithm for finding the shortest distance from a source node to all
other nodes, and to store the nodes for which the route has been calculated.

The class TGraph must have the functions:

void Reset() - initialise the graph for a new route calculation;
void Set(TNode* aNode,uint32_t aCost,TArcRef aPrevArc) - store a cost aCost and a previous arc aPrevArc in the node aNode;
void Close(TNode* aNode) - close a node so that its arcs will not be returned by the arc iterator;
uint32_t Cost(TNode* aNode) - return the cost from a node to the start of the route;
TArcRef Previous(TNode* aNode) - return the previous arc along the best route to a node, or 0 if not known;
TGraph::TArcIterator ArcIterator(TNode* aNode,bool aOutgoing) - return an iterator to provide all non-closed arcs from a node.

and if CalculateRoutesBidirectionally is used

uint32_t NodeCostInQuery(TNode* aNode,bool aForwards) - return the cost of a node already opened by the forward or backward query, or UINT32_MAX if not found

The class TGraph must have a nested TArcIterator class with the following functions:

bool Next(TResult& aError) - get the next arc: this function must be called before getting the first arc and all others, and returns false when none are left;
TArcRef Arc() - return the current arc;
uint32_t Cost() - return the cost of the current arc;
TNode* EndNode() - return the end node of the current arc.

The class TNode must fulfil the requirements of CPointerTree.

The class TArcRef is a pointer, or an integer, or any other small type that can be copied and assigned. The value zero must mean null.
*/
template<class TGraph,class TNode,class TArcRef> class TDijkstra
    {
    public:
    TDijkstra(TGraph& aGraph,bool aOwnNodeLists,bool aOutgoing):
        iGraph(aGraph),
        iOpen(aOwnNodeLists),
        iOutgoing(aOutgoing),
        iSteps(0)
        {
        }

    static TResult CalculateRoutesBidirectionally(TGraph& aGraph,TNode* aStartNode,TNode* aEndNode,const TNode*& aMiddleNode)
        {
        assert(aStartNode);
        assert(aEndNode);
        
        TResult error = 0;
        aGraph.Reset();
        aMiddleNode = nullptr;
        
        TDijkstra<TGraph,TNode,TArcRef> forward_dijkstra(aGraph,false,true);
        forward_dijkstra.Open(aStartNode,0,0);
              
        TDijkstra<TGraph,TNode,TArcRef> backward_dijkstra(aGraph,false,false);
        backward_dijkstra.Open(aEndNode,0,0);
        
        uint64_t max_cost = UINT64_MAX;
        while (!error && (forward_dijkstra.iOpen.Count() || backward_dijkstra.iOpen.Count()))
            {
            TNode* f = forward_dijkstra.iOpen.Min();
            uint64_t f_cost = f ? aGraph.Cost(f) : UINT32_MAX;
            TNode* b = backward_dijkstra.iOpen.Min();
            uint64_t b_cost = b ? aGraph.Cost(b) : UINT32_MAX;

            if (f_cost >= max_cost && b_cost >= max_cost)
                {
                if (f)
                    {
                    forward_dijkstra.iOpen.Delete(f,false);
                    aGraph.Close(f);
                    }
                if (b)
                    {
                    backward_dijkstra.iOpen.Delete(b,false);
                    aGraph.Close(b);
                    }
                break;
                }

            if (f_cost < b_cost)
                error = forward_dijkstra.CalculateRouteStep(f);
            else if (b)
                error = backward_dijkstra.CalculateRouteStep(b);

            if (f)
                {
                uint64_t other_cost = aGraph.NodeCostInQuery(f,false);
                if (other_cost < UINT32_MAX && max_cost > f_cost + other_cost)
                    {
                    max_cost = f_cost + other_cost;
                    aMiddleNode = f;
                    }
                }
            if (b)
                {
                uint64_t other_cost = aGraph.NodeCostInQuery(b,true);
                if (other_cost < UINT32_MAX && max_cost > b_cost + other_cost)
                    {
                    max_cost = b_cost + other_cost;
                    aMiddleNode = b;
                    }
                }
            }            
        
        return error;
        }
        
    TResult CalculateRoutes(TNode* aStartNode,int32_t aMaxSteps,uint32_t aMaxCost = UINT32_MAX,TNode* aEndNode = nullptr)
        {
        TResult error = 0;
        iGraph.Reset();
        iOpen.Clear();
        Open(aStartNode,0,0);
        iSteps = 0;
        while (!error && iSteps < aMaxSteps && iOpen.Count())
            {
            TNode* n = iOpen.Min();
            if (n == aEndNode || iGraph.Cost(n) > aMaxCost)
                {
                iOpen.Delete(n);
                iGraph.Close(n);
                break;
                }
            error = CalculateRouteStep(n);
            }
        return error;
        }

    TResult CalculateIsochrone(TNode* aStartNode,uint32_t aMaxCost,std::function<void (const TNode*)> aHandler)
        {
        TResult error = 0;
        iGraph.Reset();
        iOpen.Clear();
        Open(aStartNode,0,0);
        iSteps = 0;
        while (!error && iOpen.Count())
            {
            TNode* n = iOpen.Min();
            if (iGraph.Cost(n) >= aMaxCost)
                {
                iOpen.Delete(n);
                iGraph.Close(n);
                }
            else
                error = CalculateRouteStep(n);
            if (iGraph.Previous(n))
                aHandler(n);
            }
        return error;
        }

    /** Extend an existing query by performing further steps. */
    TResult ExtendRoutes(int32_t aMaxSteps,uint32_t aMaxCost = UINT32_MAX,TNode* aEndNode = nullptr)
        {
        TResult error = 0;
        while (!error && iSteps < aMaxSteps && iOpen.Count())
            {
            TNode* n = iOpen.Min();
            if (n == aEndNode || iGraph.Cost(n) > aMaxCost)
                {
                iOpen.Delete(n);
                iGraph.Close(n);
                break;
                }
            error = CalculateRouteStep(n);
            }
        return error;
        }
    
    private:
    void Open(TNode* aNode,uint32_t aCost,TArcRef aPrevArc)
        {
        iGraph.Set(aNode,aCost,aPrevArc);
        iOpen.Insert(aNode);
        }
    
    void Promote(TNode* aNode,uint32_t aCost,TArcRef aPrevArc)
        {
        iOpen.Delete(aNode,true);
        iGraph.Set(aNode,aCost,aPrevArc);
        iOpen.Insert(aNode);
        }
    
    TResult CalculateRouteStep(TNode* aNode)
        {
        assert(aNode != nullptr);
        iSteps++;
        iOpen.Delete(aNode);
        iGraph.Close(aNode);
        uint32_t node_cost = iGraph.Cost(aNode);
        assert(node_cost >= 0);
        typename TGraph::TArcIterator iter(iGraph.ArcIterator(aNode,iOutgoing));
        TResult error = 0;
        while (iter.Next(error))
            {
            uint64_t iter_cost = iter.Cost();
            uint64_t c = node_cost + iter_cost;
            if (c > UINT32_MAX - 1)
                c = UINT32_MAX - 1;
            uint32_t dest_cost = uint32_t(c);
            assert(dest_cost >= node_cost);
            assert(dest_cost >= iter_cost);
            assert(dest_cost >= 0);
            TNode* end_node = iter.EndNode();
            if (iGraph.Previous(end_node))
                {
                if (dest_cost < iGraph.Cost(end_node))
                    Promote(end_node,dest_cost,iter.Arc());
                }
            else
                Open(end_node,dest_cost,iter.Arc());
            }
        return error;
        }
    
    TGraph& iGraph;
    CPointerTree<TNode,uint32_t> iOpen;
    bool iOutgoing;
    int32_t iSteps;
    };

}

#endif
