#include <iostream>
#include <vector>
using namespace std;
class Node{
    public:
        int number;
    
    Node(int val){
        number = val;
    }
};

class Graph{
    public:
        vector<Node> nodes;
        vector<pair<Node,Node>> edges; 
};


int main(){
    vector<int> a{1,2,5,4};
    Node *n = new Node(1);
    Graph *p = new Graph();
    p->nodes.push_back(*n);
    for(int i = 2; i< 10; i++){
        Node *x = new Node(i);
        p->nodes.push_back(*x);
        p->edges.push_back(make_pair(*n,*x));
    }
    return 0; 
}