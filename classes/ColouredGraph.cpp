#include "ajtg/ColouredGraph.h"
#include "utilities.h"
#include <z3++.h>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include "equality_propagation/EqualityPropagation.h"
#include <chrono>

#include <fstream>

namespace tv {

namespace ccpg {

void add_variables(z3::context &context, z3::expr_vector &variables, int start, int num){
    for(int i = 0; i < num; ++i){
        std::string name = std::to_string(start + i);
        variables.push_back(context.bool_const(name.c_str()));
    }
    return;
}

int totalizer(std::vector<std::vector<int> > &clauses, std::vector<int> &vars, int k, int num_vars) {

    if(vars.size() == 0) return num_vars;

    int assigned_var = 0;
    std::queue<std::pair<int,int> > totalizer_nodes;
    std::pair<int, int> root;

    if(vars.size() == 1){
        root = std::pair<int, int>({vars[assigned_var++], 1});
    }else{
        root = std::pair<int, int>({++num_vars, vars.size()});
        num_vars += vars.size() - 1;
        totalizer_nodes.push(root);
    }

    while(!totalizer_nodes.empty()){ // DFS
        auto n = totalizer_nodes.front();

        auto start_var = n.first;
        auto node_label = n.second;

        totalizer_nodes.pop();

        std::pair<int, int> l, r;

        if(node_label/2 == 1)
            l = std::pair<int, int>({vars[assigned_var++], 1});
        else
        {
            l = std::pair<int, int>({++num_vars, node_label/2}); num_vars += l.second - 1;
            totalizer_nodes.push(l);
        }

        if(node_label - l.second == 1)
            r = std::pair<int, int>({vars[assigned_var++], 1});
        else
        {
            r = std::pair<int, int>({++num_vars, node_label - l.second}); num_vars += r.second - 1;
            totalizer_nodes.push(r);
        }

        //Add totalizer internal node clauses here
        for(int sig=0; sig <= node_label; ++sig){
            for(int a=0; a <= l.second; ++a){
                for(int b=0; b <= r.second; ++b){
                    if( a + b != sig ) continue;

                    std::vector<int> C1;
                    if(a > 0) C1.push_back(-(l.first + a - 1));
                    if(b > 0) C1.push_back(-(r.first + b - 1));
                    if(sig > 0) C1.push_back(start_var+sig-1);
                    else C1.clear();
                    if(!C1.empty()) clauses.push_back(C1);

                    std::vector<int> C2;
                    if(a < l.second) C2.push_back(l.first + a);
                    if(b < r.second) C2.push_back(r.first + b);
                    if(sig < node_label) C2.push_back(-(start_var + sig));
                    else C2.clear();
                    if(!C2.empty())
                        clauses.push_back(C2);
                }
            }
        }
    }

    //Add <= k 
    for(int i=k; i < root.second; ++i){
        clauses.push_back(std::vector<int>({-(root.first + i)}));
    }
    return num_vars;
}

std::pair<int, int> totalizer(z3::context &context, z3::solver &solver, z3::expr_vector &variables, std::vector<int> &vars, int &num_vars){

    if(vars.size() == 0) return std::pair<int, int>({num_vars+1, 0});

    int assigned_var = 0;
    std::queue<std::pair<int,int> > totalizer_nodes;
    std::pair<int, int> root;

    if(vars.size() == 1){
        root = std::pair<int, int>({vars[assigned_var++], 1});
    }else{
        root = std::pair<int, int>({++num_vars, vars.size()});
        num_vars += vars.size() - 1;
        add_variables(context, variables, root.first, root.second);
        totalizer_nodes.push(root);
    }

    while(!totalizer_nodes.empty()){ // DFS
        auto n = totalizer_nodes.front();

        auto start_var = n.first;
        auto node_label = n.second;

        totalizer_nodes.pop();

        std::pair<int, int> l, r;

        if(node_label/2 == 1)
            l = std::pair<int, int>({vars[assigned_var++], 1});
        else
        {
            l = std::pair<int, int>({++num_vars, node_label/2}); 
            num_vars += l.second - 1;
            add_variables(context, variables, l.first, l.second);
            totalizer_nodes.push(l);
        }

        if(node_label - l.second == 1)
            r = std::pair<int, int>({vars[assigned_var++], 1});
        else
        {
            r = std::pair<int, int>({++num_vars, node_label - l.second}); 
            num_vars += r.second - 1;
            add_variables(context, variables, r.first, r.second);
            totalizer_nodes.push(r);
        }

        //Add totalizer internal node clauses here
        for(int sig=0; sig <= node_label; ++sig){
            for(int a=0; a <= l.second; ++a){
                for(int b=0; b <= r.second; ++b){
                    if( a + b != sig ) continue;

                    std::vector<int> C1;
                    if(a > 0) C1.push_back(-(l.first + a - 1));
                    if(b > 0) C1.push_back(-(r.first + b - 1));
                    if(sig > 0) C1.push_back(start_var+sig-1);
                    else C1.clear();

                    z3::expr_vector _C1(context);
                    for(int i : C1){
                        i < 0 
                            ? _C1.push_back(!variables[-i -1])
                            : _C1.push_back(variables[i - 1]);
                    }
                    if(!_C1.empty()) solver.add(z3::mk_or(_C1));

                    std::vector<int> C2;
                    if(a < l.second) C2.push_back(l.first + a);
                    if(b < r.second) C2.push_back(r.first + b);
                    if(sig < node_label) C2.push_back(-(start_var + sig));
                    else C2.clear();

                    z3::expr_vector _C2(context);
                    for(int i : C2){
                        i < 0 
                            ? _C2.push_back(!variables[-i -1])
                            : _C2.push_back(variables[i - 1]);
                    }
                    if(!_C2.empty()) solver.add(z3::mk_or(_C2));

               }
            }
        }
    }
    //Add <= k 
    // for(int i=k; i < root.second; ++i){
    //     clauses.push_back(std::vector<int>({-(root.first + i)}));
    // }
    return root;   
}

int totalizerMinMax(z3::context &context, z3::solver &solver, z3::expr_vector &variables, std::vector<int> &vars, int &num_vars){
    //! Add comments here

    if(vars.size() == 0) return 0;

    std::pair<int, int> constraint = totalizer(context, solver, variables, vars, num_vars);


    int n = vars.size();
    int ans = -1;

    int l = 0, r = n;
    while(r - l > 0 && r <= n+1){

        int current = (l + r)/2;

        solver.push();
        for(int i = current; i < constraint.second; ++i){
            solver.add(!variables[(constraint.first + i) - 1]);
        }

        bool solved = false;

        switch (solver.check()) {
            case z3::sat:   solved = true; break;
            case z3::unsat:   break;
            case z3::unknown:  break;
        }
        solver.pop();


        if(solved){
            ans = current;
            r = current;
        }else{
            if(l == current){
                l = r;
                r++;
            }else{
                l = current;
            }
        }
    }

    if(ans == -1) return -1;

    for(int i = ans; i < constraint.second; ++i){
            solver.add(!variables[(constraint.first + i) - 1]);
    }

    //
    return ans;
}

std::string get_stmt_label(std::shared_ptr<compiler::Statement> stmt){
    std::string str = stmt -> get_raw_str();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());   
    return str;
}

// set intersection operator
template<typename T>
bool operator&(const std::set<T>& s1, const std::set<T>& s2) {
    std::vector<T> v(s1.size() + s2.size());
    auto it = std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), v.begin());
    v.resize(it-v.begin()); 
    return !(v.empty());
}

// Checks if a pair of statement is a part of an abstract block
bool CheckInAbstractBlock(std::shared_ptr<compiler::Statement> s, std::shared_ptr<compiler::Statement> t, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >& statement_to_blocks) {
    if (statement_to_blocks.find(s) == statement_to_blocks.end() ||
        statement_to_blocks.find(t) == statement_to_blocks.end()) {
            return false;
    }

    return statement_to_blocks.at(s) & statement_to_blocks.at(t);
}

void FindBackEdgesForCGF(std::shared_ptr<compiler::CFG> cfg, const statement_to_blocks_t &statement_to_blocks, std::set<CFGEdge_t> &backEdges){

    int _dfsN = 0;
    int _finN = 0;
    std::map<std::shared_ptr<compiler::Statement>, int> dfsN;
    std::map<std::shared_ptr<compiler::Statement>, int> finN;

    auto start = cfg -> get_start_statement();

    std::stack<std::shared_ptr<compiler::Statement> > Q1;

    if(start == nullptr){
        assert(false);
    }

    Q1.push(start);
    dfsN[start]= _dfsN++;
    while(!Q1.empty()){

        auto current = Q1.top(); 

        if(current == nullptr) assert(false);

        auto nxt_statements = current -> GetNextStatements();

        bool dfsFin = true;
        std::stack<std::shared_ptr<compiler::Statement> > temp;

        for(auto nxt : nxt_statements)
        {
            if(nxt.second == nullptr) continue;
            if(dfsN.find(nxt.second) != dfsN.end()) {
                if(finN.find(nxt.second) == finN.end()){
                    backEdges.insert(CFGEdge_t({current, nxt.second}));
                }else{
                     //! means this is a cross edge
                }
                continue; //! Might have to do something here
            }
            dfsN[nxt.second] = _dfsN++;
            temp.push(nxt.second);
            dfsFin = false;
        }

        if(dfsFin) {
            assert(temp.empty());
            Q1.pop();
            finN[current] = _finN++;
            // std::cout << dfsN[current] << " " << finN[current] << std::endl;
        }

        while(!temp.empty())
        {
            auto push_this = temp.top();
            temp.pop();

            Q1.push(push_this);
        }
    }

    // std::cout << " " << backEdges.size() << " ";
    std::cout << "Backedges Identified ... " << std::endl; // * Testing
}

//! For Blockification ----------------------- 

BlockifiedCFG::pSrcTgt BlockifiedCFG::BlockifyCFGs(std::shared_ptr<compiler::CFG> src, std::shared_ptr<compiler::CFG>tgt, const _stmt2AbsBlks_t& statement_to_blocks){

    std::shared_ptr<BlockifiedCFG> blkSrc(new BlockifiedCFG(src, true));
    std::shared_ptr<BlockifiedCFG> blkTgt(new BlockifiedCFG(tgt, false));

    std::set<std::shared_ptr<compiler::Statement> >  srcStmts, tgtStmts;

    { //* Collect All statements
        std::stack<std::shared_ptr<compiler::Statement> > stk;
        stk.push(src -> get_start_statement());
        srcStmts.insert(stk.top());
        while(!stk.empty()){
            auto cur = stk.top();
            stk.pop();

            if(cur == nullptr) continue;

            for(auto &child: cur -> GetNextStatements() ){
                if(child.second == nullptr) continue;
                if(srcStmts.find(child.second) != srcStmts.end()) continue;
                srcStmts.insert(child.second);
                stk.push(child.second);
            }
        }
        
        stk.push(tgt -> get_start_statement());
        tgtStmts.insert(stk.top());
        while(!stk.empty()){
            auto cur = stk.top();
            stk.pop();

            if(cur == nullptr) continue;

            for(auto &child: cur -> GetNextStatements() ){
                if(child.second == nullptr) continue;
                if(tgtStmts.find(child.second) != tgtStmts.end()) continue;
                tgtStmts.insert(child.second);
                stk.push(child.second);
            }
        }
    }

    std::cout << "Source Nodes: " << srcStmts.size() << std::endl;
    std::cout << "Target Nodes: " << tgtStmts.size() << std::endl;

    typedef std::pair<std::shared_ptr<compiler::Statement>, std::shared_ptr<compiler::Statement> > pSS;
    typedef std::shared_ptr<compiler::Statement> _stmt_t;
    typedef std::set<_stmt_t> _stmts_t;

    //* Naive way for finding matching statements
    std::map<_stmt_t, _stmts_t> matching_stmts;
    for(auto stmt1 : srcStmts){
        for(auto stmt2 : tgtStmts){
            if(CheckInAbstractBlock(stmt1, stmt2, statement_to_blocks))
                matching_stmts[stmt1].insert(stmt2);
        }
    }

    {//* Identify Matching blocks in Source
        for(auto blk : blkSrc -> blocks) {

            blk -> children.clear();

            //* check if first statement is matching
            auto stmtItr = blk -> stmts.begin();
            auto lastItr = stmtItr;
            bool matching = matching_stmts.find(*stmtItr) != matching_stmts.end();
            std::list<std::shared_ptr<CFGBlock> > new_blk_list;

            while(stmtItr != blk -> stmts.end()){
                auto nxtStmtItr = stmtItr; nxtStmtItr++;
                if(nxtStmtItr == blk -> stmts.end()) break;
                bool split = false;
                bool nxt_matching = matching_stmts.find(*nxtStmtItr) != matching_stmts.end();

                if(matching == nxt_matching){
                    if(matching){
                        _stmts_t &parent_set = matching_stmts[*stmtItr];
                        _stmts_t &child_set = matching_stmts[*nxtStmtItr];
                        _stmts_t new_child_set;
                        std::transform(parent_set.begin(), parent_set.end(), std::inserter(new_child_set, new_child_set.begin()),
                            [](_stmt_t s1) {
                                auto nxtStmts = s1 -> GetNextStatements();
                                if(nxtStmts.size() != 1) {
                                    std::cout << "Hag diya" << std::endl;
                                }
                                return nxtStmts.begin() -> second;
                            }
                        );
                        if(child_set != new_child_set) split = true;
                    }
                }
                else{
                    split = true; 
                }

                if(split){
                    std::shared_ptr<CFGBlock> newBlk(new CFGBlock(*lastItr, blkSrc -> blocks.size()));
                    lastItr++;
                    while(lastItr != nxtStmtItr){
                        newBlk -> extend(*lastItr);
                        lastItr++;
                    }
                    newBlk -> is_matching = matching;

                    new_blk_list.push_back(newBlk);
                    // newBlk -> outgoing(blk, compiler::EdgeLabel::NO_LABEL);
                    while(nxtStmtItr != blk -> stmts.begin()) blk -> stmts.pop_front();
                    blkSrc -> addBlk(newBlk);
                }

                matching = nxt_matching;
                stmtItr = nxtStmtItr;
            }

            blk -> is_matching = matching;
            new_blk_list.push_back(blk);
            blkSrc -> addBlk(blk);

            auto it = new_blk_list.begin();
            while(it != new_blk_list.end()){
                auto nxt_it = it; nxt_it++;
                if(nxt_it == new_blk_list.end()) break;
                (*it) -> outgoing(*nxt_it, compiler::EdgeLabel::NO_LABEL);
                it = nxt_it;
            }
        }

        std::map<_blk_ptr_t, int> dfsN;
        std::stack<_blk_ptr_t> Q;

        int dfs_num = 1;
        auto &stmt2Blk = blkSrc -> stmt2Blk;
        auto &backedges = blkSrc -> backedges;
        auto start_blk = stmt2Blk[blkSrc -> cfg -> get_start_statement()];
        Q.push(start_blk);
        dfsN[start_blk] = 0;

        while(!Q.empty()){
            auto blk = Q.top();

            int &cur = dfsN[blk];

            if(cur == 0){
                cur = dfs_num ++;
            }else if(cur > 0){
                cur = -1; //DFS Finished
                Q.pop();
                continue;
            }else{
                assert(false); //sanity check
            }

            // assert(!(blk -> stmts.empty()));
            auto endStmt = blk -> stmts.back();
            for(auto &nxt : endStmt -> GetNextStatements()){
                auto nxt_blk = stmt2Blk[nxt.second];
                blk -> outgoing(nxt_blk, nxt.first);
                if(dfsN.find(nxt_blk) == dfsN.end()){
                    Q.push(nxt_blk);
                    dfsN[nxt_blk] = 0;
                }else{
                    if(dfsN[nxt_blk] > 0) backedges.insert(_p_blk_ptrs_t({blk, nxt_blk}));
                }
            }
        }
    }

    {//* Blockify Target

        _stmt2Blk_t &startStmt2Blk = blkTgt -> stmt2Blk;
        {//* Matching Blocks
            for(auto blk : blkSrc -> blocks){
                if(blk -> stmts.size() == 0) continue;
                if(!(blk -> is_matching)) continue; // only for matching blocks

                int steps = blk -> stmts.size();
                _stmts_t &tgt_set = matching_stmts[blk->stmts.front()];
                std::list<_stmt_t> _list(tgt_set.begin(), tgt_set.end());

                for(auto itr = _list.begin(); itr != _list.end();){
                    if(startStmt2Blk.find(*itr) != startStmt2Blk.end()){
                        auto matching_tgt_blk = startStmt2Blk[*itr];
                        blk -> matching_blk(matching_tgt_blk);
                        matching_tgt_blk -> matching_blk(blk);
                        _list.erase(itr++);
                    }else{
                        itr++;
                    }
                }

                std::list<std::shared_ptr<CFGBlock> > blkList;
                for(auto stmt : _list){
                    blkList.push_back(std::shared_ptr<CFGBlock> (new CFGBlock(stmt, blkTgt -> blocks.size())));
                    blkList.back() -> is_matching = true;
                    blkList.back() -> matching_blk(blk);

                    blkTgt -> addBlk(blkList.back());
                    blk -> matching_blk(blkList.back());

                    // blkTgt -> blocks.insert(blk);
                    // startStmt2Blk[stmt] = blkList.back(); // already being done by addBlk
                }

                while(--steps){
                    assert(_list.size() == blkList.size());
                    
                    std::transform(_list.begin(), _list.end(), _list.begin(),
                    [](_stmt_t s1) {
                        auto nxtStmts = s1 -> GetNextStatements();
                        if(nxtStmts.size() != 1) {
                            std::cout << "Hag diya" << std::endl;
                        }
                        return nxtStmts.begin() -> second;
                    });

                    std::transform(
                        blkList.begin(), blkList.end(),
                        _list.begin(), 
                        blkList.begin(),
                        [] (std::shared_ptr<CFGBlock> _blk, _stmt_t s){
                            _blk -> extend(s);
                            return _blk;
                        }
                    );
                }

                for(auto it = blkList.begin(); it != blkList.end(); it++){
                    assert(*it -> stmts.size() == blk -> stmts.size());
                }
                
            }
        }

        std::map<_stmt_t, int> num_parents;
        {//* Find number of parents
            std::stack<std::shared_ptr<compiler::Statement> > stk;
            std::set<std::shared_ptr<compiler::Statement> > vis;
            stk.push(tgt -> get_start_statement());
            vis.insert(tgt -> get_start_statement());
            while(!stk.empty()){
                auto stmt = stk.top();
                stk.pop();

                for(auto nxt: stmt -> GetNextStatements()){
                    num_parents[nxt.second] += 1;
                    if(vis.find(nxt.second) != vis.end()) continue;
                    vis.insert(nxt.second);
                    stk.push(nxt.second);
                }
            }
        }

        {//* Non matching Blocks
            std::stack<_stmt_t> stk;
            std::set<_stmt_t> vis;

            stk.push(tgt -> get_start_statement());
            vis.insert(tgt -> get_start_statement());
            while(!stk.empty()){
                auto stmt = stk.top(); //* Current Statement
                stk.pop();
                assert(stmt != nullptr); //* Don't remember what this was for

                //* Create ans add new Block to set
                if(startStmt2Blk.find(stmt) != startStmt2Blk.end()){
                    for(auto _stmt : startStmt2Blk[stmt] -> stmts){
                        vis.insert(stmt);
                    }
                    stmt = startStmt2Blk[stmt] -> stmts.back();
                }else{
                    std::shared_ptr<CFGBlock> blk(new CFGBlock(stmt, blkTgt -> blocks.size())); 
                    blkTgt -> addBlk(blk);
                    // blkTgt -> blocks.insert(blk);
                    // startStmt2Blk[stmt] = blk; // already being done by addBlk

                    //* Collect Chain of Stmts
                    auto nxt_stmts = stmt -> GetNextStatements(); 
                    while(nxt_stmts.size() == 1){ // walk till fanout

                        if(nxt_stmts.begin()->second == nullptr) break; //!check if end node
                        if(num_parents[nxt_stmts.begin() -> second] > 1) break; //* break if incoming branch
                        if(startStmt2Blk.find(nxt_stmts.begin() -> second) != startStmt2Blk.end()) break; //* check if end of non matching block
                        if(vis.find(nxt_stmts.begin() -> second) != vis.end()) break;

                        stmt = nxt_stmts.begin() -> second; 
                        nxt_stmts = stmt -> GetNextStatements();

                        blk -> extend(stmt);
                        vis.insert(stmt);
                    }
                }
                
                assert(stmt != nullptr);
                for(auto nxt : stmt -> GetNextStatements()){
                    if(nxt.second == nullptr) continue;//!check if endnode
                    if(vis.find(nxt.second) != vis.end()) continue;
                    vis.insert(nxt.second);
                    stk.push(nxt.second);
                }
            }
        }

        {//* Connect blocks
            std::map<_blk_ptr_t, int> dfsN;
            std::stack<_blk_ptr_t> Q;

            int dfs_num = 1;
            auto &stmt2Blk = blkTgt -> stmt2Blk;
            auto &backedges = blkTgt -> backedges;
            auto start_blk = stmt2Blk[blkTgt -> cfg -> get_start_statement()];
            Q.push(start_blk);
            dfsN[start_blk] = 0;

            while(!Q.empty()){
                auto blk = Q.top();

                int &cur = dfsN[blk];

                if(cur == 0){
                    cur = dfs_num ++;
                }else if(cur > 0){
                    cur = -1; //DFS Finished
                    Q.pop();
                    continue;
                }else{
                    assert(false); //sanity check
                }

                auto endStmt = blk -> stmts.back();
                for(auto &nxt : endStmt -> GetNextStatements()){
                    auto nxt_blk = stmt2Blk[nxt.second];
                    blk -> outgoing(nxt_blk, nxt.first);
                    if(dfsN.find(nxt_blk) == dfsN.end()){
                        Q.push(nxt_blk);
                        dfsN[nxt_blk] = 0;
                    }else{
                        if(dfsN[nxt_blk] > 0) backedges.insert(_p_blk_ptrs_t({blk, nxt_blk}));
                    }
                }
            }
        }
    }

    return pSrcTgt({blkSrc, blkTgt});
}

BlockifiedCFG::BlockifiedCFG(std::shared_ptr<compiler::CFG> _cfg, bool blockify):  cfg(_cfg)
{

    if(!blockify) return;

    auto cfg_start = cfg -> get_start_statement();

    std::map<std::shared_ptr<compiler::Statement>, int> num_parents;
    //* std::map<std::shared_ptr<compiler::Statement>, std::shared_ptr<CFGBlock> > startStmt2Block;
    
    {//* Find number of parents
        std::stack<std::shared_ptr<compiler::Statement> > stk;
        std::set<std::shared_ptr<compiler::Statement> > vis;
        stk.push(cfg_start);
        vis.insert(cfg_start);
        while(!stk.empty()){
            auto stmt = stk.top();
            stk.pop();

            for(auto nxt: stmt -> GetNextStatements()){
                num_parents[nxt.second] += 1;
                if(vis.find(nxt.second) != vis.end()) continue;
                vis.insert(nxt.second);
                stk.push(nxt.second);
            }
        }
    }

    {//* Straight Segments

        //! Check for end nodes? Can a stmt in GetNextStatements() be nullptr?
        std::set<std::shared_ptr<compiler::Statement> > vis; //VIS arrayc
        std::stack<std::shared_ptr<compiler::Statement> > stk; // DFS Stack

        vis.insert(cfg_start);
        stk.push(cfg_start);
        while(!stk.empty()){ //* DFS
            auto stmt = stk.top(); //* Current Statement
            stk.pop();
            assert(stmt != nullptr); //* Don't remember what this was for

            //* Create ans add new Block to set
            std::shared_ptr<CFGBlock> blk(new CFGBlock(stmt, blocks.size())); 
            addBlk(blk);

            //Collect Chain of Stmts
            auto nxt_stmts = stmt -> GetNextStatements(); 
            while(nxt_stmts.size() == 1){ // walk till fanout

                if(nxt_stmts.begin()->second == nullptr) break; //!check if end node
                if(num_parents[nxt_stmts.begin() -> second] > 1) break; //break if incoming branch
                if(vis.find(nxt_stmts.begin() -> second) != vis.end()) break;

                stmt = nxt_stmts.begin() -> second; 
                nxt_stmts = stmt -> GetNextStatements();

                blk -> extend(stmt);
                vis.insert(stmt);
            }

            assert(stmt != nullptr);
            for(auto nxt : stmt -> GetNextStatements()){
                if(nxt.second == nullptr) continue;//!check if endnode
                if(vis.find(nxt.second) != vis.end()) continue;
                vis.insert(nxt.second);
                stk.push(nxt.second);
            }
        }
    }

    {//* Connect blocks
        //* Could merge this with previous block?
        int dfs_num = 1;
        auto start_blk = stmt2Blk[cfg -> get_start_statement()];
        std::stack<_blk_ptr_t> Q;
        Q.push(start_blk);
        std::map<_blk_ptr_t, int> dfsN;
        dfsN[start_blk] = 0;

        while(!Q.empty()){
            auto blk = Q.top();

            int &cur = dfsN[blk];

            if(cur == 0){
                cur = dfs_num ++;
            }else if(cur > 0){
                cur = -1; //DFS Finished
                Q.pop();
                continue;
            }else{
                assert(false); //sanity check
            }

            auto endStmt = blk -> stmts.back();
            for(auto &nxt : endStmt -> GetNextStatements()){
                auto nxt_blk = stmt2Blk[nxt.second];
                blk -> outgoing(nxt_blk, nxt.first);
                if(dfsN.find(nxt_blk) == dfsN.end()){
                    Q.push(nxt_blk);
                    dfsN[nxt_blk] = 0;
                }else{
                    if(dfsN[nxt_blk] > 0) backedges.insert(_p_blk_ptrs_t({blk, nxt_blk}));
                }
            }
        }

        // std::shared_ptr<CFGBlock> endBlk(new CFGBlock(-1));
        // addBlk(endBlk);
        // startStmt2Block[nullptr] = endBlk;
        // for(auto blk : blocks){
        //     assert(!(blk -> stmts.empty()));
        //     auto endStmt = blk -> stmts.back();
        //     assert(endStmt != nullptr);
        //     for(auto &nxt : endStmt -> GetNextStatements()){
        //         assert(nxt.second != nullptr);
        //         blk -> outgoing(stmt2Blk[nxt.second], nxt.first);
        //     }
        // }
    }
}

void BlockifiedCFG::populate_reachability(){
    reachability.resize(blocks.size(), std::vector<int>(blocks.size(), -1));

    for(auto &blk : blocks){ //* bfs for each node
        std::set<int> vis; 
        std::queue<std::pair<_blk_ptr_t, int> > Q;

        vis.insert(blk -> id);
        Q.push(std::make_pair(blk, 0));
        int id = blk->id;

        while(!Q.empty()){
            auto cur = Q.front();
            auto block = cur.first;
            int d = cur.second;
            Q.pop();

            reachability[id][block->id]=d;

            for(auto child : block -> children){
                if(vis.find(child.first -> id) != vis.end()) continue;
                vis.insert(child.first -> id);
                Q.push(std::make_pair(child.first, d + 1));
            }
        }
    }

    return;
}

void BlockifiedCFG::printdot(std::ofstream &out){

    std::map<std::shared_ptr<compiler::Statement>, int> stmt2id;
    int id = 0;
    out << "digraph {\ngraph [compound=true]\n";
    for(auto blk : blocks){
        for(auto stmt : blk -> stmts){
            if(stmt2id.find(stmt) != stmt2id.end()){
                std::cout << "BlockifiedCFG::printdot Statement already a part of another block" << std::endl;
                continue;
            }
            stmt2id[stmt] = id++;
        }
    }

    for(auto blk : blocks){

        out << "subgraph \"cluster_" << blk -> id << "\" {\n";

        for(auto stmt : blk -> stmts){ // Add nodes
            std::string str = stmt -> get_raw_str();
            str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
            out << stmt2id[stmt];
            if(blk -> is_matching) out << " [color=green, label=\"";
            else out << " [color=red, label=\"";
            out << str << "\"]\n";
        }

        for(auto stmt : blk -> stmts){ // Add internal Edges
            if (stmt == blk -> stmts.back()) continue;
            for(auto& child : stmt -> GetNextStatements()){
                if(child.second == nullptr) {
                    continue;
                }
                out << stmt2id[stmt] << " -> " << stmt2id[child.second] 
                    << "[ label=\"" 
                    << ((child.first == compiler::EdgeLabel::FALSE_LABEL) ? "F" : ((child.first == compiler::EdgeLabel::TRUE_LABEL) ?  "T" : "U"));
                if(backedges.find(_p_blk_ptrs_t({blk, stmt2Blk[child.second]})) != backedges.end()){
                    out << " color = magenta ";
                };
                out << "\"]\n";
            }
        }

        out << "}\n";
        for(auto &child : blk -> children){
            out << stmt2id[blk -> stmts.back()] << " -> " << stmt2id[child.first -> stmts.front()]
            << " [ label=\"" 
            << ((child.second == compiler::EdgeLabel::FALSE_LABEL) ? "F" : ((child.second == compiler::EdgeLabel::TRUE_LABEL) ?  "T" : "U"))
            << "\" ltail=\"cluster_" << blk -> id 
            << "\" lhead=\"cluster_" << child.first -> id << "\"]\n";
        }
    }

    out << "}\n";
}

bool CheckIfBlocksMatch(_blk_ptr_t blk1, _blk_ptr_t blk2){

    if(blk1 == nullptr && blk2 == nullptr) return true;

    if(blk1 == nullptr || blk2 == nullptr) return false;

    if(!( blk1 -> is_matching && blk2 -> is_matching)) return false;

    if(blk1 -> matching_blks.find(blk2) == blk1 -> matching_blks.end()) return false;
    if(blk2 -> matching_blks.find(blk1) == blk2 -> matching_blks.end()) return false;

    return true;
}

CCPGBlock::CCPGBlock(_blk_ptr_t _src, _blk_ptr_t _tgt):src(_src), tgt(_tgt) {
    is_matching = CheckIfBlocksMatch(_src, _tgt);
}

void CCPGBlock::outgoing(_cblk_ptr_t node, EdgeColor color){
    if(children.find(node) != children.end()){ //sanity check
        assert(children[node] == color); //sanity check
    }else{
        children[node] = color;
    }
    return;
}

void CCPGBlock::incoming(_cblk_ptr_t node, EdgeColor color){
    if(parents.find(node) != parents.end()){
        assert(parents[node] == color); //sanity check
    }else{
        parents[node] = color;
    }
    return;
}

void BlockifiedCCPG::printSmalldot(std::ofstream &out){

    #define _PRINT(a) std::cout << "BlockifiedCCPG::printSmalldot "<< a << std::endl

    std::map<std::shared_ptr<CCPGBlock>, int> blk2id;
    std::map<std::shared_ptr<CCPGBlock>, int> blk2stmtStartid;
    std::map<std::shared_ptr<CCPGBlock>, int> blk2stmtEndid;
    int bid = 0;
    int stmt_id = 0;

    out << "digraph {\ngraph [compound=true]\n"; // start

    _PRINT("Adding start Node");
    { // add start node
        blk2id[start_node] = bid++;
        blk2stmtEndid[start_node] = 0;
        blk2stmtStartid[start_node] = 0;

        out << "subgraph cluster_"<< 0 << " {\n"; // subgraph
        out << "color = green\n";
        out << 0 << "[label=\"START: NULL\"]\n";
        out << "}\n";

        stmt_id ++;
    }

    _PRINT("Adding other nodes");
    for(auto _node : nodes){
        auto node = _node.second; 

        int start = stmt_id;
        blk2stmtStartid[node] = stmt_id;

        out << "subgraph cluster_"<< bid << " {\n"; // subgraph


        if(node -> is_matching){
            out << "color = green\n";

            if(node -> src == nullptr || node -> tgt == nullptr){
                if(node -> src == nullptr && node -> tgt == nullptr){
                    out << stmt_id++ << "[label=\"END: NULL\"]\n";
                }else{
                    assert(false);
                }
            }else{
                std::list<_stmt_t> _stmtList;
                _stmtList.push_back(node -> src -> stmts.front());
                _stmtList.push_back(node -> src -> stmts.back());
                for(auto stmt : _stmtList){
                    out << stmt_id ++ << " [label=\"";
                    std::string str = stmt -> get_raw_str();
                    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
                    out << str << "\"]\n";
                }
            }
       }else{

            out << "color = red\n";

            out << "subgraph cluster_" << bid << "_0" << "{\n";
            out << "style = dotted\n";

            if(node -> src == nullptr){
                out << stmt_id++ << "[label=\"END: SRC NULL\"]\n";
                out << "color = " << "green" << "\n";
            }else{
                out << "color = " << ((node -> src -> is_matching) ? "green" : "red") << "\n";
                std::list<_stmt_t> _srcStmtList;
                _srcStmtList.push_back(node -> src -> stmts.front());
                _srcStmtList.push_back(node -> src -> stmts.back());
                for(auto stmt : _srcStmtList){
                    out << stmt_id ++ << " [label=\"";
                    std::string str = stmt -> get_raw_str();
                    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
                    out << str << "\", color = " << ((node -> src -> is_matching) ? "green" : "red");
                    out << "]\n";
                }
            }
            out << "}\n";

            out << "subgraph cluster_" << bid << "_1" << "{\n";
            out << "style = dotted\n";

            if(node -> tgt == nullptr){
                out << stmt_id++ << "[label=\"END: TGT NULL\"]\n";
                out << "color = " << "green" << "\n";
            }else{
                out << "color = " << ((node -> tgt -> is_matching) ? "green" : "red") << "\n";
                std::list<_stmt_t> _tgtStmtList;
                _tgtStmtList.push_back(node -> tgt -> stmts.front());
                _tgtStmtList.push_back(node -> tgt -> stmts.back());            
                for(auto stmt : _tgtStmtList){
                    out << stmt_id ++ << " [label=\"";
                    std::string str = stmt -> get_raw_str();
                    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
                    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
                    out << str << "\", color = " << ((node -> tgt -> is_matching) ? "green" : "red");
                    out << "]\n";
                }
            }
            out << "}\n";
       }

        blk2stmtEndid[node] = stmt_id - 1;
        int last = blk2stmtEndid[node];
        for(int i = start; i < last; ++i){
            out << i << " -> " << i + 1 << "\n";
        }
        out << "}\n";

        blk2id[node] = bid++;
    }

    //write edges

    _PRINT("ADDIGN EDGES");
    { // edges for start node
        auto node = start_node;
        for(auto child : node -> children){
            out << blk2stmtEndid[node] << " -> " << blk2stmtStartid[child.first] << "[color="
            << ((child.second == EdgeColor::BLACK) ? "black" : (child.second == EdgeColor::RED ? "red" : "blue"))
            << " ltail=\"cluster_" << blk2id[node] << "\" lhead=\"cluster_"<< blk2id[child.first] << "\"]\n";
        }
    }

    for(auto _node : nodes){ 
        auto node = _node.second;
        for(auto child : node -> children){
            out << blk2stmtEndid[node] << " -> " << blk2stmtStartid[child.first];
            if(backedges.find(_ccpg_edge_t({node, child.first})) != backedges.end()){
                out << " [color = magenta ";
            }
            else {
                out  << "[color=" << ((child.second == EdgeColor::BLACK) ? "black" : (child.second == EdgeColor::RED ? "red" : "blue"));
            }
            out << " ltail=\"cluster_" << blk2id[node] << "\" lhead=\"cluster_"<< blk2id[child.first];
            out << "\"]\n";
        }
    }

    out << "}\n";

    #undef _PRINT
}

BlockifiedCCPG::BlockifiedCCPG(std::shared_ptr<BlockifiedCFG> _src, std::shared_ptr<BlockifiedCFG> _tgt): src(_src), tgt(_tgt){

    bool old = false;
    #define _PRINT(a) std::cout << "BlockifiedCCPG::BlockifiedCCPG " << a << std::endl

    EdgeColor srcColor = EdgeColor::RED;
    EdgeColor tgtColor = EdgeColor::BLUE;

    //Populate reachability tables for both CFGs
    _PRINT("Calculating Reachability");
    src -> populate_reachability();
    tgt -> populate_reachability();

    _PRINT("Reachability calculated");

    auto src_start_blk = src -> stmt2Blk[src -> cfg -> get_start_statement()],    
        tgt_start_blk = tgt -> stmt2Blk[tgt -> cfg -> get_start_statement()];

    auto real_start_node = std::make_shared<CCPGBlock>(src_start_blk, tgt_start_blk);
    nodes[real_start_node -> getCFGBlks()] = real_start_node;

    //* Creating Ghost start and end nodes
    start_node = std::make_shared<CCPGBlock>(true);
    start_node -> outgoing(real_start_node, EdgeColor::BLACK);
    real_start_node -> incoming(start_node, EdgeColor::BLACK);

    end_node = std::make_shared<CCPGBlock>(true);
    nodes[_p_blk_ptrs_t({nullptr,nullptr})] = end_node;

    std::stack<_cblk_ptr_t> Q;
    // std::set<_cblk_ptr_t> vis;
    // vis.insert(start_node);
    Q.push(real_start_node);
    while(!Q.empty()){
        auto node = Q.top();
        Q.pop(); 

        auto src_blk = node -> src, tgt_blk = node -> tgt;

        if(node -> is_matching){
            assert(src_blk != nullptr && tgt_blk != nullptr); //sanity check

            for(auto nxt_src_blk : src_blk -> children){
                for(auto nxt_tgt_blk: tgt_blk -> children){
                    if(nxt_src_blk.second != nxt_tgt_blk.second) { 
                        //labels of edges don't match
                        continue; 
                    }

                    //find next block
                    _cblk_ptr_t nxt_node = nullptr;
                    _p_blk_ptrs_t nxt_src_tgt_pair({nxt_src_blk.first, nxt_tgt_blk.first});
                    if(nodes.find(nxt_src_tgt_pair) == nodes.end()){ //need to make a new block
                        nxt_node = std::make_shared<CCPGBlock>(nxt_src_blk.first, nxt_tgt_blk.first); 
                        nodes[nxt_src_tgt_pair] = nxt_node;
                        Q.push(nxt_node); // push to the queue
                        // vis.insert(nxt_node); // mark visited
                    }else{
                        nxt_node = nodes[nxt_src_tgt_pair];
                    }

                    assert(nxt_node != nullptr); //sanity check

                    nxt_node -> incoming(node, EdgeColor::BLACK);
                    node -> outgoing(nxt_node, EdgeColor::BLACK);

                    // insertBackedge(node, nxt_node); //! disabling for now
                }
            }

            if(src_blk -> children.empty() || tgt_blk -> children.empty()){
                if(src_blk -> children.empty() && tgt_blk -> children.empty()){ // if end node
                    _cblk_ptr_t nxt_node = end_node;

                    nxt_node -> incoming(node, EdgeColor::BLACK);
                    node -> outgoing(nxt_node, EdgeColor::BLACK);
                }else{
                    assert(false);
                }
                //can't be a backedge so not inserting
            }
        }else{ // the tough part
            if(!old){

                //Decide before hand whether to move along source or target or both
                bool move_along_src = false, move_along_tgt = false, separate = false;

                if(tgt_blk != nullptr && src_blk != nullptr){
                    //* Can move along both

                    //* Check if a matching source is reachable on the target side and vice versa
                    int src_dist = -1, tgt_dist = -1;

                    for(auto src_blk_ : tgt_blk -> matching_blks){
                        int d = src->reachability[src_blk -> id][src_blk_ -> id];
                        if(d > -1){
                            if(src_dist > -1 && src_dist > d) src_dist = d;
                            else if(src_dist == -1) src_dist = d;
                        }
                    }

                    for(auto tgt_blk_ : src_blk->matching_blks){
                        int d = tgt->reachability[tgt_blk -> id][tgt_blk_ -> id];
                        if(d > -1){
                            if(tgt_dist > -1 && tgt_dist > d) tgt_dist = d;
                            else if(tgt_dist == -1) tgt_dist = d;
                        }
                    }

                    if(src_dist == -1 && tgt_dist == -1){
                        //* Not reachable from both the sides
                        //* This could mean both are non mathching
                        //* Could also mean other things
                        move_along_src = move_along_tgt = true;
                    }
                    else if(src_dist == -1){
                        //* This means, tgt' corresponding to the src is reachable from tgt
                        move_along_tgt = true;
                    }
                    else if(tgt_dist == -1){
                        //* This means src' corresponding to the tgt is reachable from src
                        move_along_src = true; 
                    }else if(src_blk -> children.size() == 1 && tgt_blk -> children.size() == 1){
                        //* If both have only one child, compare distances
                        if(src_dist <= tgt_dist){
                            //* This means src' corresponding to tgt is reachable from src with fewer steps than tgt to tgt'
                                move_along_src = true;
                        }else{
                            //* This means tgt' corresponding to src is reachable from tgt with fewer steps than src to src'
                            move_along_tgt = true;
                        }
                    }else if(src_blk -> children.size() == 1){
                        //* Tgt has a branch, choose source
                        move_along_src = true;
                    }else if(tgt_blk -> children.size() == 1){
                        //* Src has a branch, choose target
                        move_along_tgt = true;
                    }else{
                        //* Both has a branch, move along both.
                        std::cout << "BlockifiedCCPG::BlockifiedCCPG Encountered special case ##########" << std::endl;
                        separate = true;
                        move_along_src = true;
                        move_along_tgt = true;
                    }
                }
                else if(tgt_blk != nullptr){
                    move_along_tgt = true;
                }
                else if(src_blk != nullptr){
                    move_along_src = true;
                }else{
                    _PRINT("Don't know what to do here, this should not happen");
                }

                std::stack<_cblk_ptr_t> nxt_nodes;
                if(move_along_src && !separate){
                    for(auto nxt_src_blk : src_blk -> children){
                        _cblk_ptr_t nxt_node = nullptr;
                        _p_blk_ptrs_t nxt_src_tgt_pair({nxt_src_blk.first, tgt_blk});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){ //need to make a new block
                            nxt_node = std::make_shared<CCPGBlock>(nxt_src_blk.first, tgt_blk); 
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            nxt_nodes.push(nxt_node);
                            // Q.push(nxt_node); // push to the queue
                            // vis.insert(nxt_node); // mark visited
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }
                        assert(nxt_node != nullptr); //sanity check

                        nxt_node -> incoming(node, srcColor);
                        node -> outgoing(nxt_node, srcColor);

                        // insertBackedge(node, nxt_node); //! disabling for now
                    }
                }
                
                if(move_along_tgt && !separate){
                    if(nxt_nodes.empty()) nxt_nodes.push(node);

                    while(!nxt_nodes.empty()) {
                        //* variables redefined
                        auto node = nxt_nodes.top();
                        auto src_blk = nxt_nodes.top() -> src;
                        auto tgt_blk = nxt_nodes.top() -> tgt;
                        nxt_nodes.pop();

                        for(auto nxt_tgt_blk : tgt_blk -> children){
                            _cblk_ptr_t nxt_node = nullptr;
                            _p_blk_ptrs_t nxt_src_tgt_pair({src_blk, nxt_tgt_blk.first});
                            if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                                nxt_node = std::make_shared<CCPGBlock>(src_blk, nxt_tgt_blk.first);
                                nodes[nxt_src_tgt_pair] = nxt_node;
                                Q.push(nxt_node);
                            }else{
                                nxt_node = nodes[nxt_src_tgt_pair];
                            }

                            assert(nxt_node != nullptr); // sanity check

                            nxt_node -> incoming(node, tgtColor);
                            node -> outgoing(nxt_node, tgtColor);

                            // insertBackedge(node, nxt_node); //! disabling for now
                        }
                    }
                }

                if(separate){
                    // Add Red Edges
                    for(auto nxt_src_blk : src_blk -> children){
                        _cblk_ptr_t nxt_node = nullptr;
                        _p_blk_ptrs_t nxt_src_tgt_pair({nxt_src_blk.first, tgt_blk});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(nxt_src_blk.first, tgt_blk);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            nxt_nodes.push(nxt_node);
                            // Q.push(nxt_node);
                            // vis.insert(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }
                        assert(nxt_node != nullptr); // sanity check

                        nxt_node -> incoming(node, srcColor);
                        node -> outgoing(nxt_node, srcColor);

                        // insertBackedge(node, nxt_node); //! disabling for now
                    }

                    // Add Blue Edges
                    for(auto nxt_tgt_blk : tgt_blk -> children){
                        _cblk_ptr_t nxt_node = nullptr;
                        _p_blk_ptrs_t nxt_src_tgt_pair({src_blk, nxt_tgt_blk.first});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(src_blk, nxt_tgt_blk.first);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            nxt_nodes.push(nxt_node);
                            // Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr); // sanity check

                        nxt_node -> incoming(node, tgtColor);
                        node -> outgoing(nxt_node, tgtColor);

                        // insertBackedge(node, nxt_node); //! disabling for now
                    }
                }

                while(!nxt_nodes.empty()){ 
                    //* Insert remaining nodes in the queue 
                    Q.push(nxt_nodes.top());
                    nxt_nodes.pop();
                }
                
                if(!move_along_src && !move_along_tgt){
                    //! This should not happen? 
                    _PRINT("Don't know what to do here, this should not happen");
                }

                //* Add edeges to ghost end node
                if(src_blk != nullptr){
                    if(src_blk -> children.empty()){
                        _cblk_ptr_t nxt_node = nullptr;

                        _p_blk_ptrs_t nxt_src_tgt_pair({nullptr, tgt_blk});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(nullptr, tgt_blk);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr);

                        nxt_node -> incoming(node, srcColor);
                        node -> outgoing(nxt_node, srcColor);

                        //cant be a backedge, so not inserting
                    }
                }

                if(tgt_blk != nullptr){
                    if(tgt_blk -> children.empty()){
                        _cblk_ptr_t nxt_node = nullptr;

                        _p_blk_ptrs_t nxt_src_tgt_pair({src_blk, nullptr});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(src_blk, nullptr);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr);

                        nxt_node -> incoming(node, tgtColor);
                        node -> outgoing(nxt_node, tgtColor);
                        // can't be a backedge so not adding
                    }
                }
            }
            else {
                if(src_blk != nullptr){// Add Red Edges
                    for(auto nxt_src_blk : src_blk -> children){
                        _cblk_ptr_t nxt_node = nullptr;
                        _p_blk_ptrs_t nxt_src_tgt_pair({nxt_src_blk.first, tgt_blk});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(nxt_src_blk.first, tgt_blk);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                            // vis.insert(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }
                        assert(nxt_node != nullptr); // sanity check

                        nxt_node -> incoming(node, srcColor);
                        node -> outgoing(nxt_node, srcColor);

                        // insertBackedge(node, nxt_node); //! disabling for now
                    }

                    if(src_blk -> children.empty()){
                        _cblk_ptr_t nxt_node = nullptr;

                        _p_blk_ptrs_t nxt_src_tgt_pair({nullptr, tgt_blk});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(nullptr, tgt_blk);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr);

                        nxt_node -> incoming(node, srcColor);
                        node -> outgoing(nxt_node, srcColor);

                        //cant be a backedge, so not inserting
                    }
                }

                if(tgt_blk != nullptr){// Add Blue Edges
                    for(auto nxt_tgt_blk : tgt_blk -> children){
                        _cblk_ptr_t nxt_node = nullptr;
                        _p_blk_ptrs_t nxt_src_tgt_pair({src_blk, nxt_tgt_blk.first});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(src_blk, nxt_tgt_blk.first);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr); // sanity check

                        nxt_node -> incoming(node, tgtColor);
                        node -> outgoing(nxt_node, tgtColor);

                        // insertBackedge(node, nxt_node); //! disabling for now
                    }

                    if(tgt_blk -> children.empty()){
                        _cblk_ptr_t nxt_node = nullptr;

                        _p_blk_ptrs_t nxt_src_tgt_pair({src_blk, nullptr});
                        if(nodes.find(nxt_src_tgt_pair) == nodes.end()){
                            nxt_node = std::make_shared<CCPGBlock>(src_blk, nullptr);
                            nodes[nxt_src_tgt_pair] = nxt_node;
                            Q.push(nxt_node);
                        }else{
                            nxt_node = nodes[nxt_src_tgt_pair];
                        }

                        assert(nxt_node != nullptr);

                        nxt_node -> incoming(node, tgtColor);
                        node -> outgoing(nxt_node, tgtColor);
                        // can't be a backedge so not adding
                    }
                }
            
            }
        } 
    }

    populate_backedges();
    #undef _PRINT
}

void BlockifiedCCPG::populate_backedges(){ 
    //* Simple dfs with a finish number to identify backedges in the ccpg
    int finN = 0;
    std::map<_cblk_ptr_t, int> fin;
    std::stack<_cblk_ptr_t> Q;

    Q.push(start_node);

    while(!Q.empty()){
        auto v = Q.top();
        if(fin.find(v) != fin.end()){
            Q.pop();
            fin[v] = finN++;
            continue;
        }
        fin[v] = -1;

        for(auto child: v -> children){
            if(fin.find(child.first) != fin.end()){
                if(fin[child.first] == -1)
                    backedges.insert(_ccpg_edge_t({v, child.first}));
            }else{
                Q.push(child.first);
            }
        }
    }
};

void BlockifiedCCPG::prepare_cnf_core(){
    #define _PRINT(a) std::cout << "BlockifiedCCPG::prepare_cnf_core " << a << std::endl

    clauses.clear();
    num_vars = 0;

    _PRINT("Add Node And Variables");
    //! Adding start node here, because it is not there in `nodes`
    n2v[start_node] = ++num_vars;
    v2n[num_vars] = start_node;
    for(auto child: start_node -> children){
        _ccpg_edge_t edge({start_node, child.first});
        e2v[edge] = ++num_vars;
        v2e[num_vars] = edge;
    }

    for(auto &_node: nodes){
        auto node = _node.second;

        //* Add nodes
        n2v[node] = ++num_vars;
        v2n[num_vars] = node;

        if(node -> is_matching) matching_nodes_vars.push_back(-num_vars);

        //* Add edges
        for(auto child :node -> children){
            _ccpg_edge_t edge({node, child.first});
            e2v[edge] = ++num_vars;
            v2e[num_vars] = edge;

            if(! (node -> is_matching))
                if(backedges.find(edge) != backedges.end())
                    nonmatching_backedges_vars.push_back(num_vars);
        }

    } //! All variables added?

    {
        _PRINT("For start and end nodes");
        //* Adding clauses for start node
        clauses.push_back(std::vector<int>({n2v[start_node]}));
        for(auto child: start_node ->children){//* for edges going out of start ndoe
            int edge_n = e2v[_ccpg_edge_t({start_node, child.first})];
            int c = n2v[child.first];

            clauses.push_back(std::vector<int>({-n2v[start_node], edge_n}));

            //* edge -> parent and child
            clauses.push_back(std::vector<int>({-edge_n, n2v[start_node]}));
            clauses.push_back(std::vector<int>({-edge_n, c}));
        }

        //* Adding clauses for end node
        assert(n2v.find(end_node) != n2v.end()); //sanity check
        clauses.push_back(std::vector<int>({n2v[end_node]}));
    }

    { //* Adding constraints for outgoing and incoming edges
        _PRINT("Add Constraints for incoming and Outgoing edges");
        for(auto& _node : nodes){
            auto node = _node.second;
            int n = n2v[node];

            //* Outgoing Edges
            if(node -> is_matching ){//* Matching Node
                for(auto child: node -> children){
                    int edge_n = e2v[_ccpg_edge_t({node, child.first})];
                    int c = n2v[child.first];

                    clauses.push_back(std::vector<int>({-n, edge_n}));

                    //* edge -> parent and child
                    clauses.push_back(std::vector<int>({-edge_n, n}));
                    clauses.push_back(std::vector<int>({-edge_n, c}));
                }
            }else{//* Non matching Node
                int red = 0, blue = 0;
                std::vector<int> reds, blues;

                for(auto &child: node -> children){
                    int edge_n = e2v[_ccpg_edge_t({node, child.first})];
                    int c = n2v[child.first];

                    //* edge -> parent and child
                    clauses.push_back(std::vector<int>({-edge_n, n}));
                    clauses.push_back(std::vector<int>({-edge_n, c}));

                    if(child.second == EdgeColor::BLUE){
                        blue = edge_n;
                        blues.push_back(blue);
                    }else if(child.second == EdgeColor::RED){
                        red = edge_n;
                        reds.push_back(red);
                    }else{
                        assert(false);
                    }
                }

                if(red && blue){
                    clauses.push_back(std::vector<int>({-n, red, blue}));
                }else if(red){
                    clauses.push_back(std::vector<int>({-n, red}));
                }else if(blue){
                    clauses.push_back(std::vector<int>({-n, blue}));
                }

                for(auto &r1: reds){
                    for(auto &b1: blues){
                        //* red -> !blue
                        clauses.push_back(std::vector<int>({-r1, -b1}));
                        for(auto &b2: blues){
                            //* b1 -> b2
                            if(b1 != b2) clauses.push_back(std::vector<int>({-b1, b2}));
                        }
                    }
                    for(auto &r2: reds){
                        //* r1 -> r2
                        if(r1 != r2) clauses.push_back(std::vector<int>({-r1, r2}));
                    }
                }
            }

            //* Incoming Edges
            std::vector<int> tmp_vector;
            for(auto &parent: node -> parents){
                if(backedges.find(_ccpg_edge_t({parent.first, node})) == backedges.end()){
                    tmp_vector.push_back(e2v[_ccpg_edge_t({parent.first, node})]);
                }
            }
            if(!tmp_vector.empty()){
                tmp_vector.push_back(-n);
                clauses.push_back(tmp_vector);
            }
        }
    } //* Added clauses for incoming and outgoing edges

    return;

    #undef _PRINT
}

Graph BlockifiedCCPG::prepare_ajtg(){

    #define _ERR(a) std::cerr << "BlockifiedCCPG::prepare_ajtg " << a << std::endl
    #define _PRINT(a) std::cout << "BlockifiedCCPG::prepare_ajtg " << a << std::endl

    prepare_cnf_core();
    
    z3::context context;
    z3::solver solver(context);
    z3::expr_vector variables(context);
    for(int i=0; i < num_vars; ++i){
        std::string bool_const_name = std::to_string(i + 1);
        variables.push_back(context.bool_const(bool_const_name.c_str()));
    }
    std::cout << "ccpg::create_ajtg added variables to context" << std::endl;  

    for(auto &clause: clauses){
        z3::expr_vector clause_expr(context);
        for(int literal: clause){
            if(literal < 0){
                if( -literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(!variables[-literal - 1]);
            }else{
                if(literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(variables[literal - 1]);
            }
        }
        solver.add(z3::mk_or(clause_expr));
    }
    std::cout << "ccpg::create_ajtg added all base clauses" << std::endl;

    int num_matching_nodes = -1;
    _PRINT("Maximizing Matching Nodes: " << matching_nodes_vars.size());
    { //* Maxmimzing Matching Nodes
        num_matching_nodes = totalizerMinMax(context, solver, variables, matching_nodes_vars, num_vars);

        if(num_matching_nodes < 0){
            _PRINT("Could not maximize matching Nodes");
        }else{
            _PRINT("MAximized Matchign Nodes: " << matching_nodes_vars.size() - num_matching_nodes);
        }
    }

    _PRINT("Minimizing Non mathcing backedges: " << nonmatching_backedges_vars.size());
    int num_nonmatching_backedges = -1;
    { //* Minimizing Non Matching Backedges
        num_nonmatching_backedges = totalizerMinMax(context, solver, variables, nonmatching_backedges_vars, num_vars);

        if(num_nonmatching_backedges < 0){
            _PRINT("Could not minimize backedges");
        }else{
            _PRINT("Minimizing non matching backedges: " << num_nonmatching_backedges);
        }
    }

    std::cout << "ccpg::create_ajtg Finally Solving CNF" << std::endl;
    switch (solver.check()) {
        case z3::sat:   std::cout << "\ncnf is sat\n"; break;
        case z3::unsat:     std::cout << "\ncnf is unsat\n"; break;
        case z3::unknown: std::cout << "\nunknown\n"; break;
    }

    z3::model m = solver.get_model();
    std::cout<< "ccpg::create_ajtg Model Obtained"<<std::endl;

    //* Start and end node should be present
    assert(z3::eq(m.eval(variables[n2v[start_node] - 1]), context.bool_val(true)));
    assert(z3::eq(m.eval(variables[n2v[end_node] - 1]), context.bool_val(true)));

    Graph ajtg;
    ajtg.start_node = std::make_shared<Node>(true);
    ajtg.end_node = std::make_shared<Node>(true);
    ajtg.nodes.insert(ajtg.start_node);
    ajtg.nodes.insert(ajtg.end_node);

    typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node> > _p_node_ptrs_t;

    std::map<_cblk_ptr_t, std::shared_ptr<Node> > blk2startNode, blk2endNode;

    //* Pair because a non matching node can have two exits
    // std::map<_cblk_ptr_t, _p_node_ptrs_t> blk2endNode; 
    int count_true_nodes = 0;
    { //* Unfold nodes
        for(auto _node: nodes){
            auto node = _node.second; // ccpg block

            auto src = node -> src;
            auto tgt = node -> tgt;

            int v = n2v[node];
            if(!(z3::eq(m.eval(variables[v-1]), context.bool_val(true)))) continue;

            count_true_nodes ++;

            if(node -> is_matching){

                if(src == nullptr || tgt == nullptr){
                    if(src == nullptr && tgt == nullptr){
                        assert(_node.first == _p_blk_ptrs_t({nullptr, nullptr})); //sanity check

                        blk2startNode[node] = ajtg.end_node;
                        blk2endNode[node] = ajtg.end_node;
                        continue;
                    }else{
                        assert(false); //sanity check
                    }
                }

                auto it1 = src -> stmts.begin();
                auto it2 = tgt -> stmts.begin();
                assert(src -> stmts.size() == tgt -> stmts.size());

                auto prev_node = std::make_shared<Node>(*it1, *it2, node -> is_matching);
                blk2startNode[node] = prev_node;

                ajtg.nodes.insert(prev_node);
                it1 ++; it2 ++;

                for(; it1 != src -> stmts.end() && it2 != tgt -> stmts.end(); it1++, it2++){
                    auto new_node = std::make_shared<Node>(*it1, *it2, node -> is_matching);
                    ajtg.nodes.insert(new_node);
                    ajtg.edges[prev_node][new_node] = EdgeColor::BLACK;
                    prev_node = new_node;
                }

                blk2endNode[node] = prev_node;
            }else{
                //first check the node color
                int red = 0, blue = 0;

                for(auto child: node -> children){
                    _p_cblk_ptrs_t edge({node, child.first});

                    if(! z3::eq(m.eval(variables[e2v[edge] - 1]), context.bool_val(true))) continue;

                    if(child.second == EdgeColor::RED) red = 1;
                    if(child.second == EdgeColor::BLUE) blue = 1;
                }

                //* Node should have only one color
                if(! ((!red || !blue) && (red || blue)) ){
                    _ERR("Non matching node given more than one color or no color");
                    exit(1);
                }

                if(red){
                    assert(src != nullptr);//! Src cannot be null here

                    auto it1 = src -> stmts.begin();
                    auto tgt_stmt = tgt == nullptr ? nullptr : tgt -> stmts.front();

                    auto prev_node = std::make_shared<Node>(*it1, tgt_stmt, node -> is_matching);
                    blk2startNode[node] = prev_node;

                    ajtg.nodes.insert(prev_node);
                    it1 ++;

                    for(; it1 != src -> stmts.end(); it1++){
                        auto new_node = std::make_shared<Node>(*it1, tgt_stmt, node -> is_matching);
                        ajtg.nodes.insert(new_node);
                        ajtg.edges[prev_node][new_node] = EdgeColor::RED;
                        prev_node = new_node;
                    }

                    blk2endNode[node] = prev_node;

                }else{
                    assert(tgt != nullptr); //! tgt cannot be null here

                    auto src_stmt = src == nullptr ? nullptr : src -> stmts.front();
                    auto it2 = tgt -> stmts.begin();

                    auto prev_node = std::make_shared<Node>(src_stmt, *it2, node -> is_matching);
                    blk2startNode[node] = prev_node;

                    ajtg.nodes.insert(prev_node);
                    it2 ++;

                    for(; it2 != tgt -> stmts.end(); it2++){
                        auto new_node = std::make_shared<Node>(src_stmt, *it2, node -> is_matching);
                        ajtg.nodes.insert(new_node);
                        ajtg.edges[prev_node][new_node] = EdgeColor::BLUE;
                        prev_node = new_node;
                    }

                    blk2endNode[node] = prev_node;
                
                }
            }
        }

        //* Don't forget start Node
        blk2startNode[start_node] = ajtg.start_node;
        blk2endNode[start_node] = ajtg.end_node;
    }

    std::cout << "count true nodes: " << count_true_nodes << std::endl;

    int num_backedges = 0;

    {//* Add edges between blocks
        //* For start Node
        for(auto child : start_node -> children){
            if(!(z3::eq(m.eval(variables[e2v[_ccpg_edge_t({start_node, child.first})]]), context.bool_val(true)))){
                _ERR("Start Node's child not present in ajtg");
            }
            ajtg.edges[ajtg.start_node][blk2startNode[child.first]] = child.second;
        }

        for(auto _node : nodes){

            auto node = _node.second;

            int v = n2v[node];
            if(!(z3::eq(m.eval(variables[v-1]), context.bool_val(true)))) continue;

            auto __node = blk2endNode[node];

            for(auto child : node -> children){
                _ccpg_edge_t edge({node, child.first});
                if(!(z3::eq(m.eval(variables[e2v[edge] -1]), context.bool_val(true)))) continue;
                if(backedges.find(edge) != backedges.end())
                    num_backedges++;
                ajtg.edges[__node][blk2startNode[child.first]] = child.second;
            }
        }
    }

    _PRINT("Backedges in AJTG: " << num_backedges);

    #undef _ERR
    #undef _PRINT

    return ajtg;
}

Graph BlockifiedCCPG::prepare_ajtg_with_skip(){

    bool insert_skipped_stmt_at_end = false; //toggle this to change the behaviour

    #define _ERR(a) std::cerr << "BlockifiedCCPG::prepare_ajtg " << a << std::endl
    #define _PRINT(a) std::cout << "BlockifiedCCPG::prepare_ajtg " << a << std::endl

    prepare_cnf_core();
    
    z3::context context;
    z3::solver solver(context);
    z3::expr_vector variables(context);
    for(int i=0; i < num_vars; ++i){
        std::string bool_const_name = std::to_string(i + 1);
        variables.push_back(context.bool_const(bool_const_name.c_str()));
    }
    std::cout << "ccpg::create_ajtg added variables to context" << std::endl;  

    for(auto &clause: clauses){
        z3::expr_vector clause_expr(context);
        for(int literal: clause){
            if(literal < 0){
                if( -literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(!variables[-literal - 1]);
            }else{
                if(literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(variables[literal - 1]);
            }
        }
        solver.add(z3::mk_or(clause_expr));
    }
    std::cout << "ccpg::create_ajtg added all base clauses" << std::endl;

    int num_matching_nodes = -1;
    _PRINT("Maximizing Matching Nodes: " << matching_nodes_vars.size());
    { //* Maxmimzing Matching Nodes
        num_matching_nodes = totalizerMinMax(context, solver, variables, matching_nodes_vars, num_vars);

        if(num_matching_nodes < 0){
            _PRINT("Could not maximize matching Nodes");
        }else{
            _PRINT("MAximized Matchign Nodes: " << matching_nodes_vars.size() - num_matching_nodes);
        }
    }

    _PRINT("Minimizing Non mathcing backedges: " << nonmatching_backedges_vars.size());
    int num_nonmatching_backedges = -1;
    { //* Minimizing Non Matching Backedges
        num_nonmatching_backedges = totalizerMinMax(context, solver, variables, nonmatching_backedges_vars, num_vars);

        if(num_nonmatching_backedges < 0){
            _PRINT("Could not minimize backedges");
        }else{
            _PRINT("Minimizing non matching backedges: " << num_nonmatching_backedges);
        }
    }

    std::cout << "ccpg::create_ajtg Finally Solving CNF" << std::endl;
    switch (solver.check()) {
        case z3::sat:   std::cout << "\ncnf is sat\n"; break;
        case z3::unsat:     std::cout << "\ncnf is unsat\n"; break;
        case z3::unknown: std::cout << "\nunknown\n"; break;
    }

    z3::model m = solver.get_model();
    std::cout<< "ccpg::create_ajtg Model Obtained"<<std::endl;

    //* Start and end node should be present
    assert(z3::eq(m.eval(variables[n2v[start_node] - 1]), context.bool_val(true)));
    assert(z3::eq(m.eval(variables[n2v[end_node] - 1]), context.bool_val(true)));

    Graph ajtg;
    ajtg.start_node = std::make_shared<Node>(true);
    ajtg.end_node = std::make_shared<Node>(true);
    ajtg.nodes.insert(ajtg.start_node);
    ajtg.nodes.insert(ajtg.end_node);

    typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<Node> > _p_node_ptrs_t;

    std::map<_cblk_ptr_t, std::shared_ptr<Node> > blk2startNode, blk2endNode;

    //* Pair because a non matching node can have two exits
    // std::map<_cblk_ptr_t, _p_node_ptrs_t> blk2endNode; 
    int count_true_nodes = 0;
    { //* Unfold blocks
        //nodes is a map from a pair of CFG Blocks to CCPG Blocks
        for(auto _node: nodes){
            auto node = _node.second; // ccpg block

            auto src = node -> src; //source CFG block
            auto tgt = node -> tgt; //target CFG Block

            int v = n2v[node];
            if(!(z3::eq(m.eval(variables[v-1]), context.bool_val(true)))) continue;

            count_true_nodes ++;

            if(node -> is_matching){

                if(src == nullptr || tgt == nullptr){
                    if(src == nullptr && tgt == nullptr){
                        assert(_node.first == _p_blk_ptrs_t({nullptr, nullptr})); //sanity check

                        blk2startNode[node] = ajtg.end_node;
                        blk2endNode[node] = ajtg.end_node;
                        continue;
                    }else{
                        assert(false); //sanity check
                    }
                }

                //Unfold both source and target CFG Block together
                auto it1 = src -> stmts.begin();
                auto it2 = tgt -> stmts.begin();
                assert(src -> stmts.size() == tgt -> stmts.size());

                auto prev_node = std::make_shared<Node>(*it1, *it2, node -> is_matching);
                blk2startNode[node] = prev_node;

                ajtg.nodes.insert(prev_node);
                it1 ++; it2 ++;

                for(; it1 != src -> stmts.end() && it2 != tgt -> stmts.end(); it1++, it2++){
                    auto new_node = std::make_shared<Node>(*it1, *it2, node -> is_matching);
                    ajtg.nodes.insert(new_node);
                    ajtg.edges[prev_node][new_node] = EdgeColor::BLACK;
                    prev_node = new_node;
                }

                blk2endNode[node] = prev_node;
            }else{
                //first check the node color
                int red = 0, blue = 0;

                for(auto child: node -> children){
                    _p_cblk_ptrs_t edge({node, child.first});

                    if(! z3::eq(m.eval(variables[e2v[edge] - 1]), context.bool_val(true))) continue;

                    if(child.second == EdgeColor::RED) red = 1;
                    if(child.second == EdgeColor::BLUE) blue = 1;
                }

                //* Node should have only one color
                if(! ((!red || !blue) && (red || blue)) ){
                    _ERR("Non matching node given more than one color or no color");
                    exit(1);
                }

                if(red){
                    assert(src != nullptr);//! Src cannot be null here

                    auto it1 = src -> stmts.begin();
                    auto original_tgt_stmt = tgt == nullptr ? nullptr : tgt -> stmts.front(); 
                    std::shared_ptr<compiler::Statement> tgt_stmt = std::make_shared<compiler::SkipStatement>();   
                    // auto tgt_stmt = original_tgt_stmt;

                    auto prev_node = std::make_shared<Node>(*it1, tgt_stmt, node -> is_matching);
                    blk2startNode[node] = prev_node;

                    ajtg.nodes.insert(prev_node);
                    it1 ++;

                    for(; it1 != src -> stmts.end(); it1++){
                        tgt_stmt = std::make_shared<compiler::SkipStatement>();   
                        auto new_node = std::make_shared<Node>(*it1, tgt_stmt, node -> is_matching);
                        ajtg.nodes.insert(new_node);
                        ajtg.edges[prev_node][new_node] = EdgeColor::RED;
                        prev_node = new_node;
                    }

                    blk2endNode[node] = prev_node;

                    // if(insert_skipped_stmt_at_end){
                    //     blk2endNode[node] -> set_target(original_tgt_stmt);
                    // }else{
                    //     blk2startNode[node] -> set_target(original_tgt_stmt);
                    // }

                }else{
                    assert(tgt != nullptr); //! tgt cannot be null here

                    auto original_src_stmt = src == nullptr ? nullptr : src -> stmts.front();
                    std::shared_ptr<compiler::Statement> src_stmt = std::make_shared<compiler::SkipStatement>();
                    // auto src_stmt = original_src_stmt;
                    auto it2 = tgt -> stmts.begin();

                    auto prev_node = std::make_shared<Node>(src_stmt, *it2, node -> is_matching);
                    blk2startNode[node] = prev_node;

                    ajtg.nodes.insert(prev_node);
                    it2 ++;

                    for(; it2 != tgt -> stmts.end(); it2++){
                        src_stmt = std::make_shared<compiler::SkipStatement>();
                        auto new_node = std::make_shared<Node>(src_stmt, *it2, node -> is_matching);
                        ajtg.nodes.insert(new_node);
                        ajtg.edges[prev_node][new_node] = EdgeColor::BLUE;
                        prev_node = new_node;
                    }

                    blk2endNode[node] = prev_node;

                    // if(insert_skipped_stmt_at_end){
                    //     blk2endNode[node] -> set_source(original_src_stmt);
                    // }else{
                    //     blk2startNode[node] -> set_source(original_src_stmt);
                    // }
                
                }
            }
        }

        //* Don't forget start Node
        blk2startNode[start_node] = ajtg.start_node;
        blk2endNode[start_node] = ajtg.start_node;
    }

    std::cout << "count true nodes: " << count_true_nodes << std::endl;

    int num_backedges = 0;

    {//* Add edges between blocks
        //* For start Node
        for(auto child : start_node -> children){
            if(!(z3::eq(m.eval(variables[e2v[_ccpg_edge_t({start_node, child.first})]]), context.bool_val(true)))){
                _ERR("Start Node's child not present in ajtg");
            }
            ajtg.edges[ajtg.start_node][blk2startNode[child.first]] = child.second;
        }

        for(auto _node : nodes){

            auto node = _node.second;

            int v = n2v[node];
            if(!(z3::eq(m.eval(variables[v-1]), context.bool_val(true)))) continue;

            auto __node = blk2endNode[node];

            for(auto child : node -> children){
                _ccpg_edge_t edge({node, child.first});
                if(!(z3::eq(m.eval(variables[e2v[edge] -1]), context.bool_val(true)))) continue;
                if(backedges.find(edge) != backedges.end())
                    num_backedges++;
                ajtg.edges[__node][blk2startNode[child.first]] = child.second;
            }
        }
    }

    _PRINT("Backedges in AJTG: " << num_backedges);

    #undef _ERR
    #undef _PRINT

    return ajtg;
}

//!----------------------- For Blockification

//! CCPG AND Z3 -------------------------------------
void CCPG::add_node(std::shared_ptr<Node> n, int v)
{
    n2v[n] = v;
    v2n[v] = n;
}

void CCPG::add_edge(Edge e, int v)
{
    e2v[e] = v;
    v2e[v] = e;
}

int CCPG::prepare_cnf_core(){
    /** 
     * *the follosing clauses will be added:
     * * - start node and end node should be present - done
     * * - node -> outgoing black edges xor outgoing red edges xor outgoing blue edges - done
     * * - (u,v) -> v and u - done
     * * - v -> atleast one edge (u,v) which is not a backedge
     */

    clauses.clear();
    num_vars = 0; //number of variables needed

    std::cout << "ccpg::prepare_base Preparing Base..." << std::endl;
    std::cout << "ccpg::prepare_base adding nodes as variables" << std::endl;
    //add node and edge variables

    int count_deg_1_nodes = 0;
    for (std::shared_ptr<Node> n : nodes)
    {
        add_node(n, ++num_vars);
        if (!(n->is_matching()))
            unmatched_node_vars.push_back(num_vars);
        for(auto e : edges[n]){
            num_edges ++;
            add_edge(Edge({n, e.first}), ++num_vars);
            redges[e.first][n] = e.second;
        }
        if(edges[n].size() == 1) count_deg_1_nodes ++;
    }
    
    //time to add clauses

    std::cout<<"ccpg::prepare_base adding start and end nodes"<<std::endl;

    // start and end should be present
    clauses.push_back(std::vector<int>({n2v[start_node]}));
    clauses.push_back(std::vector<int>({n2v[end_node]}));

    std::cout<<"ccpg::prepare_base Adding Clauses"<<std::endl;
    //Add clauses for taking outgoing edges if a node is present

    std::stack<std::shared_ptr<Node>> Q;
    std::set<std::shared_ptr<Node>> vis;
    Q.push(start_node);
    vis.insert(start_node);

    int num_straight = 0;
    while(!Q.empty()){
        num_straight ++;
        auto node = Q.top();
        Q.pop();
        int node_n = n2v[node];
        auto start = node; //start of the segment

        //* For v -> atleast one (u, v) where (u, v) is not a backedge
        std::vector<int> temp_vector;
        if(node != start_node){
            for(auto &e: redges[node]) 
                if(backEdges.find(Edge({e.first, node})) == backEdges.end()) //! Concerning
                    temp_vector.push_back(e2v[Edge({e.first, node})]);
        }

        if(!temp_vector.empty()){
            temp_vector.push_back(-node_n);
            clauses.push_back(temp_vector);
        }

        while(
            edges[node].size() == 1 // if only one child
            && redges[(*edges[node].begin()).first].size() == 1  // if this is the only parent of that child
            )
        {
            Edge e = Edge({node, edges[node].begin() -> first});
            node = (*edges[node].begin()).first;
            vis.insert(node);

            //* hard code
            implied_nodes[start].insert(node);
            implied_edges[start].insert(e);

            // * Also insert clause for edge -> both ends present
            // clauses.push_back(std::vector<int>({-node_n, child_n}));
            // clauses.push_back(std::vector<int>({node_n, -child_n}));
            // clauses.push_back(std::vector<int>({-node_n, edge_n}));
            // clauses.push_back(std::vector<int>({node_n, -edge_n}));
        } 

        if(start != node)
            clauses.push_back(std::vector<int>({-n2v[start], n2v[node]})),
            clauses.push_back(std::vector<int>({n2v[start], -n2v[node]}));

        //* Add Clauses for outgoing edges
        if(node->is_matching()){ //* Matching Node
            for(auto &e : edges[node]){
                int child_n = n2v[e.first];
                int edge_n = e2v[Edge({node, e.first})];
                clauses.push_back(std::vector<int>({-node_n, edge_n}));

                //* Also insert clause for edge -> both ends present
                clauses.push_back(std::vector<int>({-edge_n, child_n}));
                clauses.push_back(std::vector<int>({-edge_n, node_n}));
            }
        }else{ //* Non matching Node
            int red = 0, blue = 0;
            std::vector<int> reds, blues;
            for(auto &e : edges[node]){               
                int child_n = n2v[e.first];
                int edge_n = e2v[Edge({node, e.first})];

                //* Also insert clause for edge -> both ends present
                clauses.push_back(std::vector<int>({-edge_n, child_n}));
                clauses.push_back(std::vector<int>({-edge_n, node_n}));

                if(e.second == EdgeColor::RED) red = edge_n, reds.push_back(red);
                else if(e.second == EdgeColor::BLUE) blue = edge_n, blues.push_back(blue);
                else {
                    tv::utils::logger.LOG(tv::utils::ERROR) << "Non matching dead end node!";
                    exit(1);
                }
            }

            if(red && blue)
                clauses.push_back(std::vector<int>({-node_n, red, blue}));
            else if(red)
                clauses.push_back(std::vector<int>({-node_n, red}));
            else if(blue)
                clauses.push_back(std::vector<int>({-node_n, blue}));

            for(int r1 : reds){
                for(int b1 : blues){
                    clauses.push_back(std::vector<int>({-r1, -b1}));
                    for(int b2 : blues) if(b1 != b2) clauses.push_back(std::vector<int>({-b1, b2}));
                }
                for(int r2 : reds) if(r1 != r2) clauses.push_back(std::vector<int>({-r1, r2}));
            }
        }

        for(auto &e : edges[node]){
            if(vis.find(e.first) == vis.end()) {
                vis.insert(e.first);
                Q.push(e.first);
            }
        }
    }

    std::cout << "\nccpg::prepare_base Base Prepared\n"
    << "\nccpg::prepare_base BackEdges: " << backEdges.size() 
    << "\nccpg::prepare_base Straight segments: " << num_straight 
    << "\nccpg::prepare_base Deg 1 nodes: " << count_deg_1_nodes 
    << "\nccpg::prepare_base Base Variables: " << num_vars 
    << "\nccpg::prepare_base Nodes: " << nodes.size() 
    << "\nccpg::prepare_base Nonmatching nodes: " << unmatched_node_vars.size()
    << "\nccpg::prepare_base Edges: " << num_edges 
    << "\nccpg::prepare_base Base Clauses: " << clauses.size() << std::endl;

    return num_vars;
}

Graph CCPG::prepare_ajtg(){

    prepare_cnf_core();
    
    z3::context context;
    z3::solver solver(context);
    z3::expr_vector variables(context);
    for(int i=0; i < num_vars; ++i){
        std::string bool_const_name = std::to_string(i + 1);
        variables.push_back(context.bool_const(bool_const_name.c_str()));
    }
    std::cout << "ccpg::create_ajtg added variables to context" << std::endl;  

    for(auto &clause: clauses){
        z3::expr_vector clause_expr(context);
        for(int literal: clause){
            if(literal < 0){
                if( -literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(!variables[-literal - 1]);
            }else{
                if(literal - 1 >= variables.size()) {
                    std::cout << literal << " " << variables.size() << std::endl;
                    continue;
                }
                clause_expr.push_back(variables[literal - 1]);
            }
        }
        solver.add(z3::mk_or(clause_expr));
    }
    std::cout << "ccpg::create_ajtg added all base clauses" << std::endl;

    // For maximizing matching nodes
    std::vector<int> maxNumMatchNodes;
    for(auto n : nodes){
        if(n -> is_matching()) maxNumMatchNodes.push_back(-n2v[n]);
    }
    std::pair<int, int> matchingNodesConstraint = totalizer(context, solver, variables, maxNumMatchNodes, num_vars);
    int bs1l = 0, bs1r = maxNumMatchNodes.size();
    int ans1 = -1;
    std::cout << "ccpg::create_ajtg Maximizing Matching Nodes: " << maxNumMatchNodes.size() << std::endl;
    while(bs1l < bs1r){
        int current_bound = (bs1l+bs1r)/2;

        solver.push();
        for(int i=current_bound; i<matchingNodesConstraint.second; ++i) {
            solver.add(!variables[(matchingNodesConstraint.first + i) - 1]);
        }


        bool solved = false;
        switch (solver.check()) {
            case z3::sat:   solved = true; break;
            case z3::unsat:   break;
            case z3::unknown:  break;
        }
        solver.pop();

        if(solved){
            ans1 = current_bound;
            bs1r = current_bound;
            if(current_bound == bs1l)
                break;
        }else{
            if(current_bound == bs1l){
                ans1=bs1r;
                break;
            }
            bs1l = current_bound;
        }

    }
    if(ans1 < 0) std::cout << "ccpg::create_ajtg Couldn't find answer for maxNumMatchedNodes" << std::endl;
    else{
        for(int i = ans1; i < matchingNodesConstraint.second; ++i){
            solver.add(!variables[(i + matchingNodesConstraint.first) - 1]);
        }
    }
    std::cout << "ccpg::create_ajtg Matching Nodes Maximized: " << maxNumMatchNodes.size() - ans1 << " "
        << "Num Variables: "  << variables.size() << " "
        << "Num Vars: " << num_vars << " "
        << std::endl;

    //Minimizing Backedges
    std::vector<int> minNumBackEdges;
    for(auto &ed : backEdges){
        minNumBackEdges.push_back(e2v[ed]);
    }
    std::pair<int, int> backEdgesConstraint = totalizer(context, solver, variables, minNumBackEdges, num_vars);
    int bs2l = 0, bs2r = minNumBackEdges.size();
    int ans2 = -1;
    std::cout << "ccpg::create_ajtg Minimizing BackEdges" << std::endl;
    while(bs2l < bs2r){
        int current_bound = (bs2l+bs2r)/2;

        solver.push();
        for(int i = current_bound; i < backEdgesConstraint.second; ++i){
            solver.add(!variables[(i + backEdgesConstraint.first) - 1]);
        }

        bool solved = false;
        switch (solver.check()) {
            case z3::sat:   solved = true; break;
            case z3::unsat:   break;
            case z3::unknown:  break;
        }
        solver.pop();

        if(solved){
            ans2 = current_bound;
            bs2r = current_bound;
            if(current_bound == bs2l)
                break;
        }else{
            if(current_bound == bs2l){
                ans2=bs2r;
                break;
            }
            bs2l = current_bound;
        }
    }
    if(ans2 < 0) std::cout << "ccpg::create_ajtg Couldn't find answer for minNumBackEdgesNodes" << std::endl;
    else {
        for(int i=ans2; i<backEdgesConstraint.second; ++i){
            solver.add(!variables[(i + backEdgesConstraint.first) - 1]);
        }
    }
    std::cout << "ccpg::create_ajtg Back Edges Minimized: "  << ans2 << " "
        << "Num Variables: "  << variables.size() << " "
        << "Num Vars: " << num_vars << " "
        << std::endl;


    std::cout << "ccpg::create_ajtg Finally Solving CNF" << std::endl;
    switch (solver.check()) {
        case z3::sat:   std::cout << "\ncnf is sat\n"; break;
        case z3::unsat:     std::cout << "\ncnf is unsat\n"; break;
        case z3::unknown: std::cout << "\nunknown\n"; break;
    }

    z3::model m = solver.get_model();
    // std::cout << cnf_model << std::endl;
    std::cout<< "ccpg::create_ajtg Model Obtained"<<std::endl;

    Graph ajtg;
    ajtg.start_node = start_node;
    ajtg.end_node = end_node;

    std::cout<<"ccpg::create_ajtg Adding Nodes to AJTG"<<std::endl;
    for(auto n: nodes){
        if(z3::eq(m.eval(variables[n2v[n] - 1]), context.bool_val(true))){
            ajtg.nodes.insert(n);
            for(auto &implied_node: implied_nodes[n]){
                ajtg.nodes.insert(implied_node);
            }
        }
    }

    int ajtgBackEdges = 0;
    int faulty_edges = 0;
    std::cout<<"ccpg::create_ajtg Adding Edges to AJTG"<<std::endl;

    //Correctness Checking
    for (auto n : nodes)
    {
        if (n->is_matching())
        {
            for (auto e: edges[n])
            {
                if (e.second != EdgeColor::BLACK)
                    tv::utils::logger.LOG(tv::utils::ERROR) << " CreateCCPG(): Wrong CCPG Construction. Matching node has a non-black edge \n";
            }
        }
        else
        {
            for (auto e: edges[n])
            {
                if (e.second == EdgeColor:: BLACK)
                 tv::utils::logger.LOG(tv::utils::ERROR) << " CreateCCPG: Wrong CCPG Construction. Non-Matching node has a black edge \n";
            }
        }

        tv::utils::logger.LOG(tv::utils::INFO) << "CreateCCPG(): Number of Nodes: " << nodes.size() << '\n';

        if(z3::eq(m.eval(variables[n2v[n] - 1]), context.bool_val(true))){
            for(auto &e: implied_edges[n]){
                if(backEdges.find(Edge({n, e.first})) != backEdges.end()) ajtgBackEdges ++;
                ajtg.edges[e.first][e.second] = edges[e.first][e.second];

                if(n -> is_matching() && edges[e.first][e.second] != EdgeColor::BLACK) faulty_edges++;
                if(!n -> is_matching() && edges[e.first][e.second] == EdgeColor::BLACK)  faulty_edges++;
            }
        }

        for(auto &e: edges[n]){
            if(z3::eq(m.eval(variables[e2v[Edge({n, e.first})] - 1]), context.bool_val(true))) {
                if(backEdges.find(Edge({n, e.first})) != backEdges.end()) ajtgBackEdges ++;
                ajtg.edges[n][e.first] = edges[n][e.first];
                if(n -> is_matching() && edges[n][e.first] != EdgeColor::BLACK) faulty_edges++;
                if(!n -> is_matching() && edges[n][e.first] == EdgeColor::BLACK)  faulty_edges++;
            }
        }
    }

    std::cout<<"\nccpg::prepare_ajtg AJTG Completed\n"
    << "\nccpg::prepare_ajtg Faulty Edges in AJTG: " << faulty_edges
    << "\nccpg::prepare_ajtg BackEdges in AJTG: " << ajtgBackEdges
    << "\nccpg::prepare_ajtg Number of AJTG Nodes: " << ajtg.nodes.size()
    << std::endl;
    return ajtg;
}

CCPG
CreateCCPG(const compiler::PassInfo& pass_info, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >& statement_to_blocks) {

    CCPG g; 
    std::set<std::shared_ptr<Node>>& nodes = g.nodes;
    std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor> >& edges = g.edges;

    std::queue<std::shared_ptr<Node>> worklist;
    std::set<std::shared_ptr<Node>> visited;

    // Insert start and end node
    auto start_node = std::make_shared<Node>(true);
    auto end_node = std::make_shared<Node>(true);
    nodes.insert(start_node);
    nodes.insert(end_node);
    g.start_node = start_node;
    g.end_node = end_node;

    // Insert (source, target) starting statements and attach
    auto source_start = pass_info.src_cfg_->get_start_statement();
    auto target_start = pass_info.tgt_cfg_->get_start_statement();
    if (source_start == nullptr || target_start == nullptr) {
        tv::utils::logger.LOG(tv::utils::ERROR) << "CreateCCPG(): Empty CFG\n";
        tv::utils::logger.LOG(tv::utils::ERROR) << "\tSource Start Statement: " << source_start << '\n';
        tv::utils::logger.LOG(tv::utils::ERROR) << "\tTarget Start Statement: " << target_start << '\n';
        assert(false);
        return g;
    }

    auto first_real_node = std::make_shared<Node>(source_start, 
                                                  target_start,
                                                  CheckInAbstractBlock(source_start, target_start, statement_to_blocks));

    nodes.insert(first_real_node);
    edges[start_node][first_real_node] = EdgeColor::BLACK;

    worklist.push(first_real_node);

    std::map<std::pair<std::shared_ptr<compiler::Statement>, std::shared_ptr<compiler::Statement>>, std::shared_ptr<Node>> statement_to_node;
    while(!worklist.empty()) {

        //* Visit a Node
        auto node = worklist.front();
        worklist.pop();

        if (visited.find(node) != visited.end()) {
            // tv::utils::logger.LOG(tv::utils::DEBUG) << "CreateCCPG(): already visited!\n";
            continue;
        }
        visited.insert(node);

        // tv::utils::logger.LOG(tv::utils::DEBUG) << "CreateCCPG(): Node: " << (node->is_matching() ? "true\n" : "false\n");
        //! Set of pair(Label, Statement)
        auto next_source_statements = std::set<std::pair<compiler::EdgeLabel, std::shared_ptr<compiler::Statement>>>();
        auto next_target_statements = std::set<std::pair<compiler::EdgeLabel, std::shared_ptr<compiler::Statement>>>();

        if (node->get_source() != nullptr) {
            // tv::utils::logger.LOG(tv::utils::DEBUG) << "CreateCCPG(): Source: " << node->get_source()->Serialize() << '\n';
            next_source_statements = node->get_source()->GetNextStatements();

            //! If this is the last real source statement, add the exit dummy statement
            //! for the case where source statements run out, but there are still target statements
            if (next_source_statements.empty()) {
                next_source_statements.insert({compiler::EdgeLabel::NO_LABEL, nullptr});
            }
        }
        if (node->get_target() != nullptr) {
            // tv::utils::logger.LOG(tv::utils::DEBUG) << "CreateCCPG(): Target: " << node->get_target()->Serialize() << '\n';
            next_target_statements = node->get_target()->GetNextStatements();

            //! If this is the last real target statement, add the exit dummy statement
            //! for the case where target statements run out, but there are still source statements
            if (next_target_statements.empty()) {
                next_target_statements.insert({compiler::EdgeLabel::NO_LABEL, nullptr});
            }
        }

        // Both statements should never be empty together. (Exit node should never enter this while loop's body.)
        assert(node->get_source() != nullptr || node->get_target() != nullptr);

        // Insert all combinations of next statements
        if (node->is_matching()) {
            assert(node->get_source() != nullptr && node->get_target() != nullptr); //! Why this?
            for (auto& source_statement_label : next_source_statements) {
                auto source_statement = source_statement_label.second;
                auto source_label = source_statement_label.first;
                for (auto& target_statement_label : next_target_statements) {
                    auto target_statement = target_statement_label.second;
                    auto target_label = target_statement_label.first;

                    if (source_statement == nullptr && target_statement == nullptr) { // If Exit Node
                        edges[node][end_node] = EdgeColor::BLACK;
                    }
                    else if(source_label == target_label) { // Else if same labels
                        std::shared_ptr<Node> next_node;
                        auto it = statement_to_node.find({source_statement, target_statement});
                        if (it != statement_to_node.end()) {
                            next_node = it->second;
                        }
                        else {
                            bool matching_pair = false;
                            if (statement_to_blocks.find(source_statement) != statement_to_blocks.end() &&
                                statement_to_blocks.find(target_statement) != statement_to_blocks.end()) {
                                matching_pair = CheckInAbstractBlock(source_statement, target_statement, statement_to_blocks);
                                if(matching_pair) assert((source_statement!=nullptr)&&(target_statement!=nullptr));
                            }
                            next_node = std::make_shared<Node>(source_statement, target_statement, matching_pair);
                            nodes.insert(next_node);
                            worklist.push(next_node);
                            statement_to_node.insert({{source_statement, target_statement}, next_node});
                        }
                        edges[node][next_node] = EdgeColor::BLACK;
                    }
                }
            }
        }
        else {
            for (auto source_statement_label : next_source_statements) {
                auto source_statement = source_statement_label.second;
                auto source_label = source_statement_label.first;
                assert(node->get_source()!=nullptr);
                if (source_statement == nullptr && node->get_target() == nullptr) { // If Exit Node
                    edges[node][end_node] = EdgeColor::RED;
                }
                else {
                    std::shared_ptr<Node> next_node;
                    auto it = statement_to_node.find({source_statement, node->get_target()});
                    if (it != statement_to_node.end()) {
                        next_node = it->second;
                    }
                    else {
                        bool matching_pair = false;
                        if (statement_to_blocks.find(source_statement) != statement_to_blocks.end() &&
                            statement_to_blocks.find(node->get_target()) != statement_to_blocks.end()) {
                            matching_pair = CheckInAbstractBlock(source_statement, node->get_target(), statement_to_blocks);
                            if(matching_pair) assert((source_statement!=nullptr)&&(node->get_target()!=nullptr));
                        }
                        next_node = std::make_shared<Node>(source_statement, node->get_target(), matching_pair);
                        nodes.insert(next_node);
                        worklist.push(next_node);
                        statement_to_node.insert({{source_statement, node->get_target()}, next_node});
                    }
                    edges[node][next_node] = EdgeColor::RED;
                }
            }

            for (auto target_statement_label : next_target_statements) {
                auto target_statement = target_statement_label.second;
                auto target_label = target_statement_label.first;
                assert(node->get_target()!=nullptr);
                if (node->get_source() == nullptr && target_statement == nullptr) { // If Exit Node
                    edges[node][end_node] = EdgeColor::BLUE;
                }
                else {
                    std::shared_ptr<Node> next_node;
                    auto it = statement_to_node.find({node->get_source(), target_statement});
                    if (it != statement_to_node.end()) {
                        next_node = it->second;
                    }
                    else {
                        bool matching_pair = false;
                        if (statement_to_blocks.find(node->get_source()) != statement_to_blocks.end() &&
                            statement_to_blocks.find(target_statement) != statement_to_blocks.end()) {
                            matching_pair = CheckInAbstractBlock(node->get_source(), target_statement, statement_to_blocks);
                            if(matching_pair) assert((node->get_source()!=nullptr)&&(target_statement!=nullptr));
                        }
                        next_node = std::make_shared<Node>(node->get_source(), target_statement, matching_pair);
                        worklist.push(next_node);
                        nodes.insert(next_node);
                        statement_to_node.insert({{node->get_source(), target_statement}, next_node});
                    }
                    edges[node][next_node] = EdgeColor::BLUE;
                }
            }
        }
    }

    //! Identifying BackEdges for CCPG Here;
    std::set<CFGEdge_t> src_backEdges;
    std::set<CFGEdge_t> tgt_backEdges;
    FindBackEdgesForCGF(pass_info.src_cfg_, statement_to_blocks, src_backEdges);
    FindBackEdgesForCGF(pass_info.tgt_cfg_, statement_to_blocks, tgt_backEdges);

    int faulty_edges = 0;
    for(auto n1 : g.nodes){
        for(auto &ed : g.edges[n1]){

            if(n1 -> is_matching() && ed.second != EdgeColor::BLACK)  faulty_edges ++;

            if(!(n1 -> is_matching()) && ed.second == EdgeColor::BLACK) faulty_edges ++;

            auto n2 = ed.first;
            bool src_c = src_backEdges.find(CFGEdge_t({n1 -> get_source(), n2 -> get_source()})) != src_backEdges.end();
            bool tgt_c = src_backEdges.find(CFGEdge_t({n1 -> get_target(), n2 -> get_target()})) != tgt_backEdges.end();
            bool is_backEdge = false;

            if(src_c && tgt_c){
                is_backEdge = true;
            }
            else if(src_c)
            {
                auto t1 = n1 -> get_target(), t2 = n2 -> get_target();
                // if(t1 == t2 || CheckInAbstractBlock(t1, t2, statement_to_blocks)) is_backEdge = true;
            }
            else if(tgt_c)
            {
                auto s1 = n1 -> get_source(), s2 = n2 -> get_source();
                // if(s1 == s2 || CheckInAbstractBlock(s1, s2, statement_to_blocks)) is_backEdge = true;
            }

            if(is_backEdge){
                g.backEdges.insert({n1, n2});
            }
        }
    }

    std::cout << "\nCreateCCPG: Original CCPG Nodes: " << g.nodes.size()
    << "\nCreateCCPG: Backedges in Source: " << src_backEdges.size()
    << "\nCreateCCPG: Backedges in Target: " << tgt_backEdges.size()
    << "\nCreateCCPG: CCPG BackEdges Calculated: " << g.backEdges.size()
    << "\nCreateCCPG: Faulty Edges in CCPG: " << faulty_edges << std::endl;
    //! Done Identifying BackEdges

    tv::utils::logger.LOG(tv::utils::INFO) << "CreateCCPG(): Number of Nodes: " << nodes.size() << '\n';

    int num_edges = 0;
    for (auto& u : edges) {
        num_edges += u.second.size();
    }
    tv::utils::logger.LOG(tv::utils::INFO) << "CreateCCPG(): Number of Edges: " << num_edges << '\n';

    return g;
}

//! ------------------------------------- CCPG AND Z3

//! CREATE AJTG ---------------------------

Graph CreateZ3AJTG_Old(const compiler::PassInfo& pass_info, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >& statement_to_blocks){
    auto ccpgraph = CreateCCPG(pass_info, statement_to_blocks);
    Graph ajtg;
    try{
        ajtg = ccpgraph.prepare_ajtg();
    }
    catch(z3::exception e)
    {
        std::cout << e.msg() << std::endl;
    }
    return ajtg;
}

Graph CreateZ3AJTG(const compiler::PassInfo& pass_info, const std::map<std::shared_ptr<compiler::Statement>, std::set<std::shared_ptr<tv::ajtg::AbstractBlock> > >& statement_to_blocks){
    //! Blockified CFG
    std::cout << "BLOCKIFIED CFG **************************" << std::endl;
    std::cout << "Creating Blockified CFG" << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto result = BlockifiedCFG::BlockifyCFGs(pass_info.src_cfg_, pass_info.tgt_cfg_, statement_to_blocks);
    std::cout << "Blockified Src Nodes: " << result.first -> blocks.size() << "\n";
    std::cout << "Blockified Tgt Nodes: " << result.second -> blocks.size() << "\n";

    std::cout << "Blockified Src BackEdges: " << result.first -> backedges.size() << "\n";
    std::cout << "Blockified Tgt BackEdges: " << result.second -> backedges.size() << "\n";

    std::cout << "Printing Blockified Source" << std::endl;
    std::ofstream blkSrcCFG("/home/tv/dots/" + pass_info.get_function_name() + "_" + pass_info.get_name() + "_" + std::to_string(pass_info.get_num()) + "_blkSrcCFG.dot");
    result.first -> printdot(blkSrcCFG);
    blkSrcCFG.close();

    std::cout << "Printing Blockified Ttg" << std::endl;
    std::ofstream blkTgtCFG("/home/tv/dots/" + pass_info.get_function_name() + "_" + pass_info.get_name() + "_" + std::to_string(pass_info.get_num()) + "_blkTgtCFG.dot");
    result.second -> printdot(blkTgtCFG);
    blkTgtCFG.close();

    std::cout << "Creating Blockified CCPG" << std::endl;
    BlockifiedCCPG bccpg(result.first, result.second);
    std::ofstream blkCCPG("/home/tv/dots/" + pass_info.get_function_name() + "_" + pass_info.get_name() + "_" + std::to_string(pass_info.get_num()) + "_blkCCPG.dot");
    bccpg.printSmalldot(blkCCPG);
    blkCCPG.close();
    std::cout << "Blockified CCPG Nodes: " << bccpg.nodes.size() << std::endl;
    std::cout << "Blockified CCPG Backedges: " << bccpg.backedges.size() << std::endl;

    auto ajtg = bccpg.prepare_ajtg_with_skip();//testing Graph
    std::cout << "AJTG Nodes: " << ajtg.nodes.size() << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

    std::string fname = "/home/tv/dots/" + pass_info.get_function_name() + "_" + pass_info.get_name() + "_" + std::to_string(pass_info.get_num()) + "_blkAJTG1.json";
    FILE* ajtg_out = fopen(fname.c_str(), "w");
    VisualizeJson(ajtg, ajtg_out);
    fclose(ajtg_out);
    std::cout << "------------------------------------" << std::endl;

    return ajtg;
    //! Blockified CFG END
}

//! --------------------------- CREATE AJTG

// VISUALIZE _______________________________

void repr(std::string &pp) {
    size_t pos;
    //! TODO: Fix this please
    while((pos = pp.find("&")) != std::string::npos) {
        pp.replace(pos, 1, "~");
    }
    while((pos = pp.find("~")) != std::string::npos) {
        pp.replace(pos, 1, "&amp;");
    }
    while((pos = pp.find("<")) != std::string::npos) {
        pp.replace(pos, 1, "&lt;");
    }
    while((pos = pp.find(">")) != std::string::npos) {
        pp.replace(pos, 1, "&gt;");
    }
}

void CollectEqSetDiff (Graph& g, std::shared_ptr<Node> node, std::set<std::shared_ptr<Node>>& visited, eqprop::EqualitySet& prev_equalities, std::map<std::string, std::string>& labels, std::map<std::shared_ptr<Node>, std::string>& ids, int& label)
{
    if (visited.find(node) != visited.end()) {
        // outgoing edges already mapped
        return;
    }
    visited.insert(node);
    std::string str_label = std::to_string(label);
    ids[node] = str_label;
    auto src = node->get_source();
    auto tgt = node->get_target();
    stronglylive::StronglyLiveSet src_sl_set, tgt_sl_set, new_sl_set;
    if (src)
    {
        src_sl_set = src->TransferStronglyLiveExpressions();
//node->is_matching()? src->get_strongly_live_expressions() : src->TransferStronglyLiveExpressions();
        new_sl_set = src_sl_set;
    }
    if (tgt) 
    {
        tgt_sl_set =  tgt->TransferStronglyLiveExpressions();
//node->is_matching() ? tgt->get_strongly_live_expressions() : tgt->TransferStronglyLiveExpressions();
        new_sl_set = tgt_sl_set;

    }
    if (src && tgt)
        new_sl_set = src_sl_set.Union(tgt_sl_set);

    //Print the strongly live set.

    //std::cout << "Strongly Live Set" << new_sl_set.Print() << std::endl;
    std::string src_stmt = src ? (src->get_raw_str() + " // EQ: " + " + " + node->get_equalities().Difference(prev_equalities, new_sl_set, true).Print() + " - " + node->get_equalities().Difference(prev_equalities, new_sl_set, false).Print() + " // SL: " + src->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(src->get_points_to_map()))  : "NULL";
    repr(src_stmt);
    
    //std::cout <<"\t src  "<<src_stmt <<'\n';
    std::string tgt_stmt = tgt ? (tgt->get_raw_str() + " //  SL: " + tgt->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(tgt->get_points_to_map()))  : "NULL";
    repr(tgt_stmt);
    labels[str_label] = "<td>src:</td><td>" + src_stmt + "</td></tr><tr><td>tgt:</td><td>" + tgt_stmt + "</td>";
    //std::cout <<"\t tgt  "<<tgt_stmt <<'\n';
    label++;
    prev_equalities = node->get_equalities();

    auto& edges = g.edges;
    if (edges.find(node) != edges.end()) {
        // print poutgoing edges
        for (auto &outgoing_node: edges.at(node)) {
            CollectEqSetDiff (g, outgoing_node.first, visited, prev_equalities, labels, ids, label);
        }
    }
}

void VisualizeDiff(Graph& g, FILE * dotf) {
    auto& nodes = g.nodes;
    auto& edges = g.edges;

    if (!dotf) {
        tv::utils::logger.LOG(tv::utils::ERROR) << "tv::ccpg::Visualize(): early exit because dot file is null. Perhaps a directory needs to be made?\n";
        return;
    }
    std::set<std::shared_ptr<Node>> visited;
    std::map<std::shared_ptr<Node>, std::string> ids;
    std::map<std::string, std::string> labels;
    auto prev_equalities = eqprop::EqualitySet();
    // initialize labels
    int label = 0;
    
    std::set<std::shared_ptr<Node>> nodes_to_visit;
    nodes_to_visit.insert(g.start_node);
   //A dfs walk to collect the equality sets and print the differences 
    CollectEqSetDiff (g, g.start_node, visited, prev_equalities, labels, ids, label);
    visited.clear();
    fprintf(dotf, "digraph AJTG {\norientation = Portrait;\n");

    while(!nodes_to_visit.empty()) {
        std::set<std::shared_ptr<Node>> next_to_visit;
    //    auto prev_equalities = eqprop::EqualitySet();

        for (auto &node: nodes_to_visit)
        {
            if (visited.find(node) != visited.end()) {
                // outgoing edges already mapped
                continue;
            }
            visited.insert(node);

            // get map of outgoing edges, and identity of this node
            if (edges.find(node) != edges.end()) {
                // print poutgoing edges
                for (auto &outgoing_node: edges.at(node)) {
                    fprintf(dotf, "\t%s -> %s\n", ids[node].c_str(), ids[outgoing_node.first].c_str());
                }
                for (auto next_node_color : edges.at(node))
                {
                    next_to_visit.insert(next_node_color.first);
                }
            }
        }
        nodes_to_visit = next_to_visit;
    }
    for (auto &node: nodes)
    {
        fprintf(dotf, "%s [shape = none, label = <<table bgcolor=\"%s\"><tr>%s</tr></table>>]\n", ids[node].c_str(), (node->is_matching()?"#00ff00":"#ff0000"), labels[ids[node]].c_str());
    }
    fprintf(dotf, "}\n");
}

void VisualizeJson(Graph& g, FILE * jsonf) {
    auto& nodes = g.nodes;
    auto& edges = g.edges;

    if (!jsonf) {
        tv::utils::logger.LOG(tv::utils::ERROR) << "tv::ccpg::Visualize(): early exit because json file is null. Perhaps a directory needs to be made?\n";
        return;
    }
    //Print Preamble:
    fprintf(jsonf, " {\n \t\"name\": \"AJTG\",\n \t\"directed\": true, \n \t\"strict\": false, \n \t\"orientation\": \"Portrait\",\n ");
    fprintf(jsonf, "\t\"_subgraph_cnt\": 0, \n \t\"objects\": [\n ");
    
    std::set<std::shared_ptr<Node>> visited;
    std::map<std::shared_ptr<Node>, std::string> ids;
    std::map<std::string, std::string> labels;

    // initialize labels
    int label = 0;
    //Print in json
    int size = nodes.size();
    int edges_count = 0;
    for(auto& node: nodes) {
        std::string str_label = std::to_string(label);
        ids[node] = str_label;
        label++;
        fprintf(jsonf, "\t {\n");
        fprintf(jsonf, "\t\t\"_gvid\" : \"%s\",\n", str_label.c_str());
        fprintf(jsonf, "\t\t\"matching\" : %s,\n", node->is_matching()? "true": "false");
        auto src = node->get_source();

        //std::string src_stmt = src ? (src->get_raw_str()) : "NULL";
        // + " // EQ: " + node->get_equalities().Print() + " // SL: " + src->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(src->get_points_to_map())) :  "NULL";
         
        std::string str = node->get_equalities().Print();
        str.erase( std::remove( str.begin(), str.end(), '\"' ), str.end());

        fprintf(jsonf, "\t\t\"equality_set\" : \"%s\",\n", str.c_str());
        if (src != nullptr)
        {
            std::string stmt = src->get_raw_str( ).c_str();
            int pos = stmt.find("\n");
            str=stmt.substr(0, pos);
            str.erase( std::remove( str.begin(), str.end(), '\"' ), str.end());
            fprintf(jsonf, "\t\t\"src\": \"%s \",\n", str.c_str());
            //fprintf(jsonf, "\t \"source_id\" : %s,\n", ids[src]);

            std::string sl_set = src->get_strongly_live_expressions().Print();
            sl_set.erase( std::remove( sl_set.begin(), sl_set.end(), '\"' ), sl_set.end()); //Remove double quotes - it is messing up in json rendering

            fprintf(jsonf, "\t\t\"src_stronglylive_set\": \"%s\",\n", sl_set.c_str());
            std::string pts_set =AA::PointsToMapPrinter(src->get_points_to_map());
            pts_set.erase( std::remove( pts_set.begin(), pts_set.end(), '\"' ), pts_set.end()); //Remove double quotes - it is messing up in json rendering

            fprintf(jsonf, "\t\t\"src_pointsTo_set\": \"%s\",\n", pts_set.c_str());
        }
        else
        {
            fprintf(jsonf, "\t\t \"src\": \"NULL\",\n");
            //fprintf(jsonf, "\t \"source_id\" : \"%s\",\n", "0");
            fprintf(jsonf, "\t\t \"src_stronglylive_set\": \"{}\",\n"); 
            fprintf(jsonf, "\t\t \"src_pointsTo_set\": \"{}\",\n" );
        } 
        auto tgt = node->get_target();
        if (tgt != nullptr)
        {
            std::string stmt = tgt->get_raw_str( ).c_str();
            int pos = stmt.find("\n");
            str=stmt.substr(0, pos);
            str.erase( std::remove( str.begin(), str.end(), '\"' ), str.end());

            fprintf(jsonf, "\t\t \"tgt\" : \"%s \",\n", str.c_str());
            //fprintf(jsonf, "\t \"target_id\" : \"%s\",\n", ids[tgt]);
            std::string sl_set = tgt->get_strongly_live_expressions().Print();
            sl_set.erase( std::remove( sl_set.begin(), sl_set.end(), '\"' ), sl_set.end()); //Remove double quotes - it is messing up in json rendering
            fprintf(jsonf, "\t\t \"tgt_stronglylive_set\": \"%s\",\n", sl_set.c_str());
            std::string pts_set =AA::PointsToMapPrinter(tgt->get_points_to_map());
            pts_set.erase( std::remove( pts_set.begin(), pts_set.end(), '\"' ), pts_set.end()); //Remove double quotes - it is messing up in json rendering
            fprintf(jsonf, "\t\t \"tgt_pointsTo_set\": \"%s\"\n", pts_set.c_str());
        }
        else
        {
            fprintf(jsonf, "\t\t \"tgt\" : \"NULL\",\n");
            //fprintf(jsonf, "\t \"target_id\" : \"%s\",\n", "0");
            fprintf(jsonf, "\t\t \"tgt_stronglylive_set\": \"{}\",\n"); 
            fprintf(jsonf, "\t\t \"tgt_pointsTo_set\": \"{}\"\n" );
        } 
            if (label < size)
                fprintf(jsonf, "\t },\n" );
            else
                fprintf(jsonf, "\t }\n" );

            if (edges.find(node) != edges.end()) 
                edges_count += edges.at(node).size() ;

    }
    fprintf(jsonf, "\t],\n");
        //std::string tgt_stmt = tgt ? (tgt->get_raw_str() + " // EQ: " + node->get_equalities().Print()+ " // SL: " + tgt->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(tgt->get_points_to_map())) : "NULL";
        //repr(tgt_stmt);
        //labels[str_label] = "<td>src:</td><td>" + src_stmt + "</td></tr><tr><td>tgt:</td><td>" + tgt_stmt + "</td>";
    // std::set<std::shared_ptr<Node>> not_starts;
    // for (auto it = edges.begin(); it != edges.end(); it++)
    // {
    //     not_starts.insert(it->second.begin(), it->second.end());
    // }
    // std::set<std::shared_ptr<Node>> start_node_set;
    // std::set_difference(nodes.begin(), nodes.end(), not_starts.begin(), not_starts.end(),
    //                     std::inserter(start_node_set, start_node_set.end()));
    // if (start_node_set.size() != 1) {
    //     tv::utils::logger.LOG(tv::utils::ERROR) <<"\tStart node set size (should be 1): "<<start_node_set.size()<<'\n';
    //     exit(1);
    // }

    int faulty_edges = 0;
    fprintf(jsonf, "\"edges\": [\n");
    std::set<std::shared_ptr<Node>> nodes_to_visit;
    nodes_to_visit.insert(g.start_node);
    int count = 0;
    while(!nodes_to_visit.empty()) {
        std::set<std::shared_ptr<Node>> next_to_visit;
        for (auto &node: nodes_to_visit)
        {
            if (visited.find(node) != visited.end()) {
                // outgoing edges already mapped
                continue;
            }
            visited.insert(node);

            // these will be in the labels

            // get map of outgoing edges, and identity of this node
            if (edges.find(node) != edges.end()) {
                // print poutgoing edges
                for (auto &outgoing_node: edges.at(node)) {
                    
                    fprintf(jsonf, "{\t\"_gvid\": \"%d\", \n \t\"tail\": \"%s\",\n \t\"head\": \"%s\",\n", count, ids[node].c_str(), ids[outgoing_node.first].c_str());
                    if (outgoing_node.second == EdgeColor::BLACK){
                        if(!node->is_matching()) faulty_edges++;
                        fprintf(jsonf, "\t\"color\": \"Black\"\n");
                    }
                    else
                    {
                        if(node->is_matching()) faulty_edges++;
                        if (outgoing_node.second == EdgeColor::BLUE)
                            fprintf(jsonf, "\t\"color\": \"Blue\"\n");
                        else
                             fprintf(jsonf, "\t\"color\": \"Red\"\n");
                    }
                    if (count < edges_count -1)
                        fprintf(jsonf, "},\n");
                    else
                        fprintf(jsonf, "}\n");
                    count++;
                }
                for (auto next_node_color : edges.at(node))
                {
                    next_to_visit.insert(next_node_color.first);
                }
            }
        }
        nodes_to_visit = next_to_visit;
    }
   // for (auto &node: nodes)
    //{
//        fprintf(jsonf, "%s [shape = none, label = <<table bgcolor=\"%s\"><tr>%s</tr></table>>]\n", ids[node].c_str(), (node->is_matching()?"#00ff00":"#ff0000"), labels[ids[node]].c_str());
    //}
    fprintf(jsonf, "\t]\n");
    fprintf(jsonf, "}\n");

    std::cout << "VisualizeJson:: faulty edges: " << faulty_edges << std::endl;
}

void Visualize(Graph& g, FILE * dotf) {
    auto& nodes = g.nodes;
    auto& edges = g.edges;

    if (!dotf) {
        tv::utils::logger.LOG(tv::utils::ERROR) << "tv::ccpg::Visualize(): early exit because dot file is null. Perhaps a directory needs to be made?\n";
        return;
    }
    std::set<std::shared_ptr<Node>> visited;
    std::map<std::shared_ptr<Node>, std::string> ids;
    std::map<std::string, std::string> labels;

    // initialize labels
    int label = 0;
    for(auto& node: nodes) {
        std::string str_label = std::to_string(label);
        ids[node] = str_label;
        auto src = node->get_source();
        std::string src_stmt = src ? (src->get_raw_str() + " // EQ: " + node->get_equalities().Print() + " // SL: " + src->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(src->get_points_to_map())) :  "NULL";
        
        ;
        repr(src_stmt);
        
        auto tgt = node->get_target();
        std::string tgt_stmt = tgt ? (tgt->get_raw_str() + " // EQ: " + node->get_equalities().Print()+ " // SL: " + tgt->TransferStronglyLiveExpressions().Print() + " // PtsTo : " + AA::PointsToMapPrinter(tgt->get_points_to_map())) : "NULL";
        repr(tgt_stmt);
        labels[str_label] = "<td>src:</td><td>" + src_stmt + "</td></tr><tr><td>tgt:</td><td>" + tgt_stmt + "</td>";
        label++;
    }
    // std::set<std::shared_ptr<Node>> not_starts;
    // for (auto it = edges.begin(); it != edges.end(); it++)
    // {
    //     not_starts.insert(it->second.begin(), it->second.end());
    // }
    // std::set<std::shared_ptr<Node>> start_node_set;
    // std::set_difference(nodes.begin(), nodes.end(), not_starts.begin(), not_starts.end(),
    //                     std::inserter(start_node_set, start_node_set.end()));
    // if (start_node_set.size() != 1) {
    //     tv::utils::logger.LOG(tv::utils::ERROR) <<"\tStart node set size (should be 1): "<<start_node_set.size()<<'\n';
    //     exit(1);
    // }
    std::set<std::shared_ptr<Node>> nodes_to_visit;
    nodes_to_visit.insert(g.start_node);
    fprintf(dotf, "digraph AJTG {\norientation = Portrait;\n");

    while(!nodes_to_visit.empty()) {
        std::set<std::shared_ptr<Node>> next_to_visit;
        for (auto &node: nodes_to_visit)
        {
            if (visited.find(node) != visited.end()) {
                // outgoing edges already mapped
                continue;
            }
            visited.insert(node);

            // these will be in the labels

            // get map of outgoing edges, and identity of this node
            if (edges.find(node) != edges.end()) {
                // print poutgoing edges
                for (auto &outgoing_node: edges.at(node)) {
                    fprintf(dotf, "\t%s -> %s\n", ids[node].c_str(), ids[outgoing_node.first].c_str());
                }
                for (auto next_node_color : edges.at(node))
                {
                    next_to_visit.insert(next_node_color.first);
                }
            }
        }
        nodes_to_visit = next_to_visit;
    }
    for (auto &node: nodes)
    {
        fprintf(dotf, "%s [shape = none, label = <<table bgcolor=\"%s\"><tr>%s</tr></table>>]\n", ids[node].c_str(), (node->is_matching()?"#00ff00":"#ff0000"), labels[ids[node]].c_str());
    }
    fprintf(dotf, "}\n");
}

void 
GetReachableAbstractBlocks(std::shared_ptr<Node> start_node, const std::map<std::shared_ptr<Node>, std::map<std::shared_ptr<Node>, EdgeColor> >& edges, std::vector<std::set<std::shared_ptr<Node>>>& abstract_blocks, EdgeColor color) {
    auto it = edges.find(start_node);
    if (it == edges.end()) return;

    auto out_edges = it->second;

    for (auto edge : out_edges) {
        if (edge.second != color) continue;
        std::stack<std::shared_ptr<Node>> stack;
        stack.push(edge.first);
        abstract_blocks.push_back(std::set<std::shared_ptr<Node>>());

        std::unordered_set<std::shared_ptr<Node>> visited;

        for (auto other_edge : out_edges) {
            if (edge != other_edge) {
                visited.insert(other_edge.first);
            }
        }

        while(!(stack.empty())) {
            auto node = stack.top();
            stack.pop();
            if (visited.find(node) != visited.end()) continue;
            visited.insert(node);

            if (node->is_matching()) {
                abstract_blocks[abstract_blocks.size()-1].insert(node);
            }

            auto it = edges.find(node);
            if (it != edges.end()) {
                for (auto succ_nodes_color : it->second) {
                    stack.push(succ_nodes_color.first);
                }
            } 
        }
    }
    
}

void OptimizeCCPG(CCPG& ccpg) {
    auto& nodes = ccpg.nodes;
    auto& edges = ccpg.edges;
    int num_edges_erased = 0;

    for (auto node : nodes) {
        // If a matching node, continue.
        if (node->is_matching()) continue;

        // if no edges, continue.
        auto it = edges.find(node);
        if (it == edges.end()) continue;

        auto& out_edges = it->second;
        // if only blue or only red edges, continue.
        bool red_found = false, blue_found = false;
        for (auto edge : out_edges) {
            red_found = edge.second == EdgeColor::RED || red_found;
            blue_found = edge.second == EdgeColor::BLUE || blue_found;
            if (red_found && blue_found) break;
        }
        if (!(red_found && blue_found)) continue;

        // if there are more than 2 edges, then continue. :(
        // if (out_edges.size() > 2) continue;
        
        std::vector<std::set<std::shared_ptr<Node>>> abstract_blocks_from_red, abstract_blocks_from_blue;

        // Get reachable abstract blocks from the red and blue edge(s)
        GetReachableAbstractBlocks(node, edges, abstract_blocks_from_red, EdgeColor::RED);
        GetReachableAbstractBlocks(node, edges, abstract_blocks_from_blue, EdgeColor::BLUE);
        assert(abstract_blocks_from_blue.size() > 0);
        assert(abstract_blocks_from_red.size() > 0);

        // tv::utils::logger.LOG(tv::utils::INFO) << "OptimizeCCPG(): Red Reachable Abstract Blocks: " << abstract_blocks_from_red.size() << '\n';
        // tv::utils::logger.LOG(tv::utils::INFO) << "OptimizeCCPG(): Blue Reachable Abstract Blocks: " << abstract_blocks_from_blue.size() << '\n';

        // if all RED-reachable abstract blocks are a subset of BLUE-reachable abstract blocks, remove the RED edges.
        bool red_is_worse_than_blue = true;
        bool blue_is_worse_than_red = true;
        for (auto& red_set : abstract_blocks_from_red) {
            for (auto& blue_set : abstract_blocks_from_blue) {
                if (!std::includes(blue_set.begin(), blue_set.end(), red_set.begin(), red_set.end())) {
                    red_is_worse_than_blue = false;
                }
                else {
                    assert(blue_set.size() >= red_set.size());
                }
                if (!std::includes(red_set.begin(), red_set.end(), blue_set.begin(), blue_set.end())) {
                    blue_is_worse_than_red = false;
                }
                else {
                    assert(red_set.size() >= blue_set.size());
                }
            }
        }
        if (red_is_worse_than_blue) {
            auto it = out_edges.begin();
            while (it != out_edges.end()) {
                if ( it->second == EdgeColor::RED ) {
                    it = out_edges.erase(it);
                    num_edges_erased++;
                }
                else {
                    it++;
                }
            }
        }

        // else if all BLUE-reachable abstract blocks are a subset of RED-reachable abstract blocks, remove the BLUE edges.
        else if (blue_is_worse_than_red) {
            auto it = out_edges.begin();
            while (it != out_edges.end()) {
                if (it->second == EdgeColor::BLUE) {
                    it = out_edges.erase(it);
                    num_edges_erased++;
                }
                else {
                    it++;
                }
            }
        }
    }

    // nodes cleanup
    nodes.clear();
    std::stack<std::shared_ptr<Node>> stack;
    stack.push(ccpg.start_node);
    while (!(stack.empty())) {
        auto node = stack.top();
        stack.pop();
        if (nodes.find(node) != nodes.end()) {
            continue;
        }
        nodes.insert(node);

        if (edges.find(node) != edges.end()) {
            for (auto dest : edges[node]) {
                stack.push(dest.first);
            }
        }
    }

    // edges cleanup
    auto it = edges.begin();
    while (it != edges.end()) {
        if (nodes.find(it->first) == nodes.end()) {
            it = edges.erase(it);
        }
        else {
            auto it2 = it->second.begin();
            while (it2 != it->second.end()) {
                if (nodes.find(it2->first) == nodes.end()) {
                    it2 = it->second.erase(it2);
                }
                else {
                    it2++;
                }
            }
            it++;
        }
    }
    tv::utils::logger.LOG(tv::utils::INFO) << "OptimizeCCPG(): number of edges erased: " << num_edges_erased << '\n';

    std::cout << "OptimizeCCPG:: Number of nodes after Optimization: " << ccpg.nodes.size() << std::endl;
}

bool Graph::AllMatched() const
{
    for (const auto& edge : edges)
    {
        for (const auto& node_color : edge.second)
        {
            if (node_color.second != EdgeColor::BLACK)
            {
                return false;
            }
        }
    }

    return true;
}

} // namespace ccpg

} // namespace tv
