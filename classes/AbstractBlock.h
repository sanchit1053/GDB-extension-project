#ifndef TV_AJTG_ABSTRACTBLOCK_H
#define TV_AJTG_ABSTRACTBLOCK_H

#include <compiler/PassInfo.h>
// #include "TvCommon.h"
#include <utility>
#include <vector>
#include <memory>

namespace tv {

namespace ajtg {

typedef std::pair<std::shared_ptr<compiler::Statement>, std::shared_ptr<compiler::Statement>> StatementPair;

// Forward Declaration
class AbstractBlock;

/**
 * Exit Pair
 * Holds a Statement Pair which serves as an exit to an abstract block.
 * Info on: 
 *  - no. of edges from the statement pairs
 *  - labels
 */
class ExitPair {

public:
	StatementPair exit_pair_;
	int num_exits_;

    //! At max, each exit can diverge into two edges.
	compiler::EdgeLabel labels_[2];


	ExitPair(StatementPair pair, int num_exits, compiler::EdgeLabel label0, compiler::EdgeLabel label1): exit_pair_(pair), num_exits_(num_exits) {
        labels_[0] = label0;
        labels_[1] = label1;
    }
    ExitPair(const ExitPair& ep): exit_pair_(ep.exit_pair_), num_exits_(ep.num_exits_) {
        labels_[0] = ep.labels_[0];
        labels_[1] = ep.labels_[1];
    }
	ExitPair();

	bool operator ==(const ExitPair& ob) const {
        if (num_exits_ != ob.num_exits_)
		    return false;
        for (int i = 0; i < ob.num_exits_; i++) {
            if (labels_[i] != ob.labels_[i])
                return false;
        }

	    return (exit_pair_.first == ob.exit_pair_.first) && 
               (exit_pair_.second == ob.exit_pair_.second) && 
               (num_exits_ == ob.num_exits_);
    }

	bool operator <(const ExitPair& ob) const {
        if (exit_pair_ < ob.exit_pair_) return true;

        if (exit_pair_ > ob.exit_pair_) return false;

        if (num_exits_ < ob.num_exits_) return true;

        if (num_exits_ == ob.num_exits_) {
            for (int i = 0; i < num_exits_; i++)
            {
                if (labels_[i] < ob.labels_[i])
                    return true;
            }
        }

        return false;
    }

	void dump();

    static std::vector<StatementPair> GetStatementPairs(std::vector<ExitPair> &);

    std::vector<std::shared_ptr<AbstractBlock>> GetNextAbstractBlocks();
};


class AbstractBlock : public std::enable_shared_from_this<AbstractBlock>
{
public:
    /* Static Variables */
    static long last_id_;
    static std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<AbstractBlock> > > statement_to_abstract_blocks_;
    std::vector<ExitPair> exits_;

    /* Member Variables (TODO: Change to private) */
    StatementPair header_;
    std::vector<StatementPair> interior_statements_;
    std::vector<StatementPair> loop_backs_statements_;
    long uid_;

    /* Constructors */
    AbstractBlock(StatementPair _h);
	AbstractBlock(const std::shared_ptr<AbstractBlock>& blk);


    /* Combining Basic Blocks */
    // combine()
    std::shared_ptr<AbstractBlock> Combine(std::shared_ptr<AbstractBlock>);
    // Combine properties check | canCombine()
    bool CanCombine(std::shared_ptr<AbstractBlock>);
    // BackEdge Property check bool backEdgePropAbsBlk(AbstractBlockPtr abs1, AbstractBlockPtr abs2);
    bool BackEdgePropertyCheck(std::shared_ptr<AbstractBlock>);
    // isSequentialInGraph
    bool AreStatementPairsSequential(StatementPair, StatementPair);
    bool AreStrictNotSequentialPairs(StatementPair, StatementPair);

    template<typename ...T>
    static std::shared_ptr<AbstractBlock> Create(T ...);

    /* Public Utilities */

    // Source Projection | getProjection(..., SOURCE)
    std::vector<std::shared_ptr<compiler::Statement>> get_source (const std::vector<StatementPair>&) const;
    // Target Projection | getProjection(..., TARGET)
    std::vector<std::shared_ptr<compiler::Statement>> get_target (const std::vector<StatementPair>&) const;;

    bool is_source_header(std::shared_ptr<compiler::Statement> s) {return s == header_.first;}
    bool is_target_header(std::shared_ptr<compiler::Statement> s) {return s == header_.second;}

    bool is_source_exit(std::shared_ptr<compiler::Statement> stmt) {
        for (const auto& exit : exits_) {
            if (stmt == exit.exit_pair_.first)
                return true;
        }
        return false;        
    }
    bool is_target_exit(std::shared_ptr<compiler::Statement> stmt) {
        for (const auto& exit : exits_) {
            if (stmt == exit.exit_pair_.second)
                return true;
        }
        return false; 
    }
    
    bool has_statement(std::shared_ptr<compiler::Statement>);

    // getLabelOfEdge
    compiler::EdgeLabel get_edge_label(StatementPair p1, StatementPair p2);

    // eraseAllStmtMapFromAbs
    void erase_block();

    /* Debug | Print */
	Json::Value dump();

private:
    /* Internal Utilities  (TODO: INCOMPLETE)*/
    void set_pair_to_abs(StatementPair);
    void erase_pair_to_abs(StatementPair);
    void replace_statement_to_abs(std::shared_ptr<compiler::Statement>, std::shared_ptr<AbstractBlock>); //replaceStmtToAbs needs to be called from insert_abs
    void replace_pair_to_abs(StatementPair& , std::shared_ptr<AbstractBlock>);
    void replace_all_statement_to_abs(std::shared_ptr<AbstractBlock>); //replaceAllStmtMapToAbsFromAbs
    std::vector<std::shared_ptr<compiler::Statement>> get_exit_source() const;
    std::vector<std::shared_ptr<compiler::Statement>> get_exit_target() const;

    //Replaces isFirstStmtOfBB
    inline bool IsSrcHeaderFirstInBB(){
        std::shared_ptr<compiler::BasicBlock> bb= (header_.first)->get_basic_block();
        return bb->get_entry_statement() == header_.first;
    }

    //Replaces isFirstStmtOfBB
    inline bool IsTgtHeaderFirstInBB(){
        std::shared_ptr<compiler::BasicBlock> bb= (header_.second)->get_basic_block();
        return bb->get_entry_statement() == header_.first;
    }

    //Replaces backEdgePropGimple
    bool BackEdgePropBB(std::shared_ptr<compiler::Statement>, std::vector< std::shared_ptr<compiler::Statement> >&, std::vector< std::shared_ptr<compiler::Statement> >&, std::vector< std::shared_ptr<compiler::Statement> >& );
};


} // namespace ajtg

} // namespace tv

#endif // TV_AJTG_ABSTRACTBLOCK_H