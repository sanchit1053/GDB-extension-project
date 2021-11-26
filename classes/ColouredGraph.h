#ifndef TV_AJTG_COLOUREDGRAPH_H
#define TV_AJTG_COLOURED_GRAPH_H

#include <memory>
#include <utility>
#include <map>
#include <set>
#include <vector>
#include <iostream>
// #include <z3++.h>

#include "compiler/PassInfo.h"
#include "ajtg/AbstractBlock.h"

/**
 * CCPG Nodes and Edges
 */

namespace tv {

//! TODO: Rename this namespace appropriately
namespace ccpg {

class Node;
class CCPG;
class Graph;
enum class EdgeColor;

//!----------------------------------- Blockified CFG

class CFGBlock;
class BlockifiedCFG;
class CCPGBlock;
class BlockifiedCCPG;
typedef std::shared_ptr<CFGBlock> _blk_ptr_t; // Block pointer
typedef std::shared_ptr<BlockifiedCFG> _bcfg_ptr_t; // Blockified cfg pointer
typedef std::shared_ptr<CCPGBlock> _cblk_ptr_t; // Coloured Block Pointer
typedef std::shared_ptr<BlockifiedCCPG> _bccpg_ptr_t; // Blockified CCPG pointer
typedef std::pair<_blk_ptr_t, _blk_ptr_t> _p_blk_ptrs_t;
typedef std::pair<_cblk_ptr_t, _cblk_ptr_t> _p_cblk_ptrs_t;
typedef std::pair<_cblk_ptr_t, _cblk_ptr_t> _ccpg_edge_t;
typedef std::shared_ptr<compiler::Statement> _stmt_t;

class CFGBlock
{
public:
    typedef std::pair<std::shared_ptr<CFGBlock>, compiler::EdgeLabel> _child_t;

    //methods
    CFGBlock(int _id): id(_id){}
    CFGBlock(std::shared_ptr<compiler::Statement> _start, int _id): id(_id){stmts.push_back(_start);}
    void extend(std::shared_ptr<compiler::Statement> stmt){stmts.push_back(stmt);}
    void outgoing(std::shared_ptr<CFGBlock> childBlk, compiler::EdgeLabel label)
        {children.insert(_child_t({childBlk, label}));};

    //Add a mathcing blocks on the opposite side
    void matching_blk(std::shared_ptr<CFGBlock> blk)
        {matching_blks.insert(blk); is_matching = true;}
    std::shared_ptr<compiler::Statement> getLast()
        {return stmts.size() > 0 ? stmts.back() : nullptr;}

    //data
    int id = 0;
    bool is_matching = false;
    std::set<std::shared_ptr<CFGBlock> > matching_blks; //Matching block on the opposite side
    std::list<std::shared_ptr<compiler::Statement> > stmts; // set of statements this block has
    std::set<_child_t> children;
    std::set<_blk_ptr_t> reachable; ///> set of matching nodes reachable from this node
};

class BlockifiedCFG
{
public:
    typedef std::pair<std::shared_ptr<BlockifiedCFG>, std::shared_ptr<BlockifiedCFG> > pSrcTgt;
    typedef std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > > _stmt2AbsBlks_t;
    typedef std::map<std::shared_ptr<compiler::Statement>, std::shared_ptr<CFGBlock> > _stmt2Blk_t;
    static  pSrcTgt BlockifyCFGs(std::shared_ptr<compiler::CFG>, std::shared_ptr<compiler::CFG>, const _stmt2AbsBlks_t&);
    
    //Methods
    BlockifiedCFG(){};
    BlockifiedCFG(std::shared_ptr<compiler::CFG>, bool blockifiy = true);
    void addBlk(std::shared_ptr<CFGBlock> blk) {
        blocks.insert(blk);
        assert(!(blk -> stmts.empty()));
        stmt2Blk[blk -> stmts.front()] = blk;
    };
    void populate_reachability();

    //Data
    std::shared_ptr<compiler::CFG> cfg = nullptr;
    std::set<std::shared_ptr<CFGBlock> > blocks;
    _stmt2Blk_t stmt2Blk;
    std::set<_p_blk_ptrs_t> backedges;
    std::vector<std::vector<int> > reachability;
    void printdot(std::ofstream&);
};

class CCPGBlock
{
    public:
        //methods
        CCPGBlock(bool _matching): is_matching(_matching){}
        CCPGBlock(_blk_ptr_t _src, _blk_ptr_t _tgt);
        void outgoing(_cblk_ptr_t node, EdgeColor color);
        void incoming(_cblk_ptr_t node, EdgeColor color);
        _p_blk_ptrs_t getCFGBlks(){return _p_blk_ptrs_t({src, tgt});}

        //data
        std::shared_ptr<CFGBlock> src = nullptr, tgt = nullptr;
        bool is_matching = false;
        std::map<_cblk_ptr_t, EdgeColor> children;
        std::map<_cblk_ptr_t, EdgeColor> parents;
};

/**
 * 
 * ! in the map nodes, start node will not be present
 */
class BlockifiedCCPG
{
    public:

        typedef std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > > _stmt2AbsBlks_t;

        //members
        BlockifiedCCPG(){};
        BlockifiedCCPG(std::shared_ptr<BlockifiedCFG> _src, std::shared_ptr<BlockifiedCFG> _tgt);
        // void addNode(_cblk_ptr_t node){nodes.insert(node);}
        // BlockifiedCCPG(std::shared_ptr<BlockifiedCFG> _src, std::shared_ptr<BlockifiedCFG> _tgt, _stmt2AbsBlks_t &statement_to_blocks);
        // BlockifiedCCPG(std::shared_ptr<compiler::CFG> _src, std::shared_ptr<compiler::CFG> _tgt, _stmt2AbsBlks_t &statement_to_blocks_t){};
        void printSmalldot(std::ofstream &out);
 
        //data
        std::shared_ptr<BlockifiedCFG> src, tgt;
        std::map<_p_blk_ptrs_t, _cblk_ptr_t > nodes;

        _cblk_ptr_t start_node = nullptr;
        _cblk_ptr_t end_node = nullptr; 

        // For CNF 
        void prepare_cnf_core();
        Graph prepare_ajtg();
        Graph prepare_ajtg_with_skip();
        std::vector<std::vector<int> > clauses;

        int num_vars = 0;
        std::map<_ccpg_edge_t, int> e2v;
        std::map<int, _ccpg_edge_t> v2e;
        std::map<_cblk_ptr_t, int> n2v;
        std::map<int, _cblk_ptr_t> v2n;

        std::set<std::pair<_cblk_ptr_t, _cblk_ptr_t> > backedges;

        std::vector<int> matching_nodes_vars;
        std::vector<int> nonmatching_backedges_vars;

        void populate_backedges();
};

//!------------------------------------

enum class EdgeColor {
    BLACK,
    RED,
    BLUE
};

class Node 
{
public:

    Node(std::shared_ptr<compiler::Statement> source, std::shared_ptr<compiler::Statement> target, bool matching): source_(source), target_(target), matching_(matching) {}
    Node(bool matching): matching_(matching) {
        source_ = target_ = nullptr;
    }

    //! Getters and Setters
    bool is_matching() {return matching_;}
    bool is_obligations_sat() {return obligations_sat_;}
    void set_obligations_sat(bool v) {obligations_sat_ = v;}
    
    std::shared_ptr<compiler::Statement> get_source() {return source_;}
    std::shared_ptr<compiler::Statement> get_target() {return target_;}
    void set_source(std::shared_ptr<compiler::Statement> _src) {source_ = _src;}
    void set_target(std::shared_ptr<compiler::Statement> _tgt) {target_ = _tgt;}

    /**
     * Checks if the obligations of a particular node are satisfied
     * using the eq set
     * 
     * Obligations: All corresponding source and target strongly live variables.
     *              This is clearly conservative, as not all strongly live variables may be used in the block
     * TODO: This can be made more precise. Currently it checks for too many redundant obligations.
     * \param ssa_map SSA mapping from source to target
     */
    bool CheckObligations(const std::map<compiler::SSASymbol, std::set<compiler::SSASymbol>> &ssa_map) const;

    eqprop::EqualitySet get_equalities () const { return equalities_; }

    eqprop::EqualitySet equalities_; //Moving equalities_ from Statement to Node as one can have null Statements which can break equality propagation -SS.
    //The equality Set at a node represents the equalities at the top of the node. Making this public as I am getting compilation errors if I make it protected.
    
protected:
    //! TODO: These should be const
    std::shared_ptr<compiler::Statement> source_, target_;
    bool matching_;
    bool obligations_sat_ = false;
};

//* typedefs
typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> Edge;
typedef std::pair<std::shared_ptr<compiler::Statement>, std::shared_ptr<compiler::Statement> > CFGEdge_t;
typedef std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > > statement_to_blocks_t;

class CCPG
{
public:
    std::shared_ptr<Node> start_node, end_node;
    std::set<std::shared_ptr<Node>> nodes;
    std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor>> edges;
    std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor>> redges;
    std::map<std::shared_ptr<Node>, int> number;
    std::set< Edge > backEdges;
    std::set< Edge > straight_segments;
    std::map<std::shared_ptr<Node>, std::set<std::shared_ptr<Node>>> implied_nodes;
    std::map<std::shared_ptr<Node>, std::set<Edge>> implied_edges;

    //For CCPG -> AJTG
    int num_vars = 0;
    int num_edges = 0;
    std::vector<std::vector<int>> clauses; //holding clauses
    std::map<std::shared_ptr<Node>, int> n2v;
    std::map<int, std::shared_ptr<Node>> v2n;
    std::map<Edge, int> e2v;
    std::map<int, Edge> v2e;
    std::vector<int> unmatched_node_vars;
    void calculate_straight_segments();
    void add_node(std::shared_ptr<Node>, int);
    void add_edge(Edge, int);
    int prepare_cnf_core();
    Graph prepare_ajtg();
};

void FindBackEdgesForCGF(std::shared_ptr<compiler::CFG> cfg, const statement_to_blocks_t &statement_to_blocks, std::set<CFGEdge_t> &backEdges);

class Graph
{
public:
    std::shared_ptr<Node> start_node, end_node;
    std::set<std::shared_ptr<Node>> nodes;
    std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor>> edges;

    //! Original pass information
    std::shared_ptr<compiler::PassInfo> pass_info_;

    eqprop::EqualitySet InitEqualities();
    void EqualityPropagation();

    // void TestFoo();

    //! Returns true if source and target's return variables are equal at the end
    bool ReturnCheck() const;
    
    //! Returns true if source and target's heap are equal at the end of every return statement block
    bool HeapCheck() const;

    //! Returns true if source and target globals are equal at the end of every return statement block. EqualitySet is the equality set at that return block
    bool GlobalCheck() const;

    //! Returns true if source and target globals are equal at the end of every return statement block. EqualitySet is the equality set at that return block
    bool GlobalCheck_sub(const eqprop::EqualitySet &) const;
    //! Returns true if all blocks are matching
    bool AllMatched() const;
//Returns a set of strongly live vars at return point of the source or target. Used in equality propagation
    stronglylive::StronglyLiveSet GetStronglyLiveAtReturn(bool source);
};

// Nodes: a, b, c
// EdgeColor: map[a][b]
/**
 * creates CCPG using standard DFS.
 * For single source to multiple target statement blocks,
 * Only the first abstract block as seen in the DFS is considered
 * as a sync point. Other overlapping abstract blocks are ignored.
 *  
 * \brief From given abstract blocks, creates a CCPG
 * \param pass_info The main PassInfo object with both source and target CFGs
 * \param statement_to_block Maps individual statements to abstract blocks
 * \returns <Nodes, Edges> where each Edge is map (Node -> (Node -> Color))
 */ 
CCPG
CreateCCPG(const compiler::PassInfo&, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >&);

/** Create Z3 constraints and create Graph from CCPG
 * \brief Given CCPG construct AJTG using Z3, refer to encoding
 * \param ccpgraph Pair of Vertices, Edges (the output of CreateCCPG)
 * \returns <Nodes, Edges> of final AJTG.
 */
Graph
CreateZ3AJTG_Old(const compiler::PassInfo&, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >&);

Graph 
CreateZ3AJTG(const compiler::PassInfo&, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >&);

/**
 * Magic sauce to reduce the size of the CCPG.
 */
void OptimizeCCPG(CCPG&);

void
Visualize(Graph&, FILE *);

void
VisualizeJson(Graph&, FILE *);

/*Creates a .dot file with the difference in equality sets for better visualization */
void
VisualizeDiff(Graph&, FILE *);

/* A dfs walk over graph G to collect the equality sets and print the difference of the set at a given node with that of a parent */
void CollectEqSetDiff (Graph& g, std::shared_ptr<Node> node, std::set<std::shared_ptr<Node>>& visited, eqprop::EqualitySet& prev_equalities, std::map<std::string, std::string>& labels, std::map<std::shared_ptr<Node>, std::string>& ids,
int& label);

bool 
CheckInAbstractBlock(std::shared_ptr<compiler::Statement> s, std::shared_ptr<compiler::Statement> t, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >& statement_to_blocks);

/** 
 * DFS on CCPG, starting from the RED (or BLUE, depending upon color) successors to the start node
 * Finds all reachable matching nodes (aka single statement abstract blocks.)
 * If DFS is started from the RED (BLUE) nodes, then the BLUE (RED) edges out of start node are ignored.
 * No change in the graph otherwise.
 * \param start_node Node from which the coloured edges are taken
 * \param edges The edge set
 * \param abstract_blocks For each edge (of a particular color), the reachable abstract blocks are copied to a set.
 * \param color Color of the edge(s) to take from start_node, the destination of the edge(s) is from where the DFS starts.
 */
void
GetReachableAbstractBlocks(std::shared_ptr<Node> start_node, const std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor> >& edges, std::vector<std::set<std::shared_ptr<Node>>>& abstract_blocks, EdgeColor color);
} // namespace ccpg


} // namespace tv

#endif
