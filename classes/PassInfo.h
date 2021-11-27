#ifndef PASSINFO_H
#define PASSINFO_H

#include "compiler/Symbol.h"
#include "compiler/Statement.h"

#include <string>
#include <map>
#include <set>
#include <json/json.h>

/**
 * NOTES:
 * Classes can't be templatified - will be used later!
 * Functions _can_ be templatified, but fully specialized
 * All global in cpp files, not headers.
 */



namespace compiler {

/** 
 * GCC Type Basic Block
 * Copied from gcc4.7.2:
 *  A basic block is a sequence of instructions with only entry and
 *  only one exit (BUT CAN HAVE MULTIPLE INCOMING AND OUTGOING EDGES).  If any one of the instructions are executed, they
 *  will all be executed, and in sequence from first to last.
 *
 *  There may be COND_EXEC instructions in the basic block.  The
 *  COND_EXEC *instructions* will be executed -- but if the condition
 *  is false the conditionally executed *expressions* will of course
 *  not be executed.  We don't consider the conditionally executed
 *  expression (which might have side-effects) to be in a separate
 *  basic block because the program counter will always be at the same
 *  location after the COND_EXEC instruction, regardless of whether the
 *  condition is true or not.
 *
 *  Basic blocks need not start with a label nor end with a jump insn.
 *  For example, a previous basic block may just "conditionally fall"
 *  into the succeeding basic block, and the last basic block need not
 *  end with a jump insn.  Block 0 is a descendant of the entry block.
 *
 *  A basic block beginning with two labels cannot have notes between
 *  the labels.
 *
 *  Data for jump tables are stored in jump_insns that occur in no
 *  basic block even though these insns can follow or precede insns in
 *  basic blocks.  
 */

// Forward Declaration
class CFG;

enum class EdgeLabel {
    NO_LABEL,
    TRUE_LABEL,
    FALSE_LABEL
};

class BasicBlock
{
public:

    //! Copy basic blocks
    /**
     * Copies a SINGLE block, and does not form the edges.
     */
    template <typename ...T>
    static std::shared_ptr<BasicBlock> Create(T...);

    virtual Json::Value Serialize() const;

    int get_loop_depth();   //! getter of loop_depth_
    std::shared_ptr<BasicBlock> get_next() {return next_;}
    std::shared_ptr<BasicBlock> get_prev() {return prev_;}
    std::shared_ptr<Statement> get_entry_statement() {return entry_stmt_;}
    std::shared_ptr<Statement> get_exit_statement() {return exit_stmt_;}
    std::vector<std::pair<EdgeLabel, std::shared_ptr<BasicBlock> > > get_succs() {return successors_;}
    std::vector<std::pair<EdgeLabel, std::shared_ptr<BasicBlock> > > get_preds() {return predecessors_;}
    int get_index() {return index_;}
    const std::vector<std::shared_ptr<Statement> >& get_statements() {return statements_;}
    std::shared_ptr<const CFG> get_cfg() const {return cfg_;}
    
    bool IsLastStatement(std::shared_ptr<Statement>);
    static std::pair<bool, EdgeLabel> AreStatementsSequential(std::shared_ptr<Statement>, std::shared_ptr<Statement>);
    
    //! TODO: Complete this.
    bool IsDummyBlock() { }

protected:
    //! next block in index order.
    std::shared_ptr<BasicBlock> next_;

    //! previous block in index order.
    std::shared_ptr<BasicBlock> prev_;

    //! list of successor blocks. This is a many-to-many mapping, hence a vector.
    std::vector<std::pair<EdgeLabel, std::shared_ptr<BasicBlock> > > successors_;

    //! list of predecssor blocks
    std::vector<std::pair<EdgeLabel, std::shared_ptr<BasicBlock> > > predecessors_;        

    //! Depth of loop if there is one, else 0, optional.
    int loop_depth_;

    //! flags for internal use, optional.
    int flags_;

    //! all statements
    std::vector<std::shared_ptr<Statement> > statements_;

    //! first statement of the basic block
    std::shared_ptr<Statement> entry_stmt_;

    //! last statement of the basic block
    std::shared_ptr<Statement> exit_stmt_;

    //! index no. of the basic block
    int index_;

    //! Back pointer to the holding CFG
    std::shared_ptr<const CFG> cfg_;
    friend class CFG;

};

/**
 *  All Control Flow Info of a program (either source or target) 
 */
class CFG 
{
public:
    //! copy CFG from compiler IR to this
    /**
     * Copies compiler internal CFG to this representation
     */
    template <typename ...T>
    static std::shared_ptr<CFG> Create(T...);

    //! Alias Analysis
    void PointsToAnalysis();
    bool SkipAnalysis(std::string fun_name) const{
        return (points_to_analysis_config_.find("--")!=points_to_analysis_config_.end())
            || (points_to_analysis_config_.find(fun_name)!=points_to_analysis_config_.end());
    }

    //! Strongly Live Expressions Analysis
    void StronglyLiveExpressionsAnalysis();
    
    virtual Json::Value Serialize() const;
    void Visualize(std::string filepath, std::string graph_name, bool with_aliases = false) const;
    
    std::shared_ptr<const SymbolTable> get_symbol_table() const {return symbol_table_;}
    std::shared_ptr<BasicBlock> get_entry_block() const {return entry_block_;}
    std::shared_ptr<BasicBlock> get_exit_block() const {return exit_block_;}
    std::shared_ptr<BasicBlock> get_block_by_index(int i) const {return basic_blocks_[i];}
    std::vector<std::shared_ptr<BasicBlock> > get_basic_blocks() const {return basic_blocks_;}
    std::shared_ptr<Statement> get_start_statement() const {return start_statement_;}
    std::set<std::shared_ptr<Statement>> get_return_statements() const {return return_statements_;}
    int get_num_blocks() const {return basic_blocks_.size();}
    std::shared_ptr<Expression> get_heap_var() const {return heap_var_;}
    std::vector<std::shared_ptr<VariableOperand>> get_global_vars() const {return global_vars_;}
    stronglylive::StronglyLiveSet get_return_expressions() const {return return_expressions_;}
    CFGLabel get_label() const {return label_;}

protected:        
    void PointToAnalysisInitializeGlobals();
    std::shared_ptr<SymbolTable> symbol_table_;      //! Symbol table of CFG 
    std::vector<std::shared_ptr<BasicBlock> > basic_blocks_;  //! all blocks in *index* order.
    std::map<int, std::shared_ptr<Statement> > label_to_statement_;
    std::shared_ptr<BasicBlock> entry_block_;
    std::shared_ptr<BasicBlock> exit_block_;
    std::shared_ptr<Statement> start_statement_;
    std::shared_ptr<VariableOperand> heap_var_;
    std::vector<std::shared_ptr<VariableOperand> > global_vars_; 
    std::set<std::string> points_to_analysis_config_;
    std::set<std::shared_ptr<Statement>> return_statements_;

    stronglylive::StronglyLiveSet return_expressions_;

    CFGLabel label_;
    friend class BasicBlock;
};


/**
 *  All Control and Data flow information of both source & target of a single pass 
 */
class PassInfo
{
public:

    template <typename ...T>
    bool CreateSSAMap(T...);
    virtual Json::Value Serialize() const;
    
    PassInfo(std::string name, int num, std::string function_name);
    std::shared_ptr<CFG> src_cfg_;                                                          
    std::shared_ptr<CFG> tgt_cfg_;                                                          
    std::string get_name() const {return name_;}                                 //! getter for name_
    int get_num() const {return num_;}                                           //! getter for num_
    std::string get_function_name() const {return function_name_;}               //! getter for function_name_
    const std::map<SSASymbol, std::set<SSASymbol> >& get_ssa_map() const {return ssa_map_;}

    //! Equality Propagation: 
    //void EqualityPropagation();

protected:
    std::string name_;                                                     //! name of the pass
    int num_;                                                              //! number of the pass
    std::string function_name_;                                            //! name of the function
    std::map<SSASymbol, std::set<SSASymbol> > ssa_map_;                           //! maps target ssa to source ssa
};

} // namespace compiler

#endif // PASSINFO_H