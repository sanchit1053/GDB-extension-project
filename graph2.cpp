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

class asdf{
    public:
        vector<Node> qwer;
        vector<pair<Node,Node>> zxcv; 
};


int main(){
    vector<int> a{1,2,5,4};
    Node *n = new Node(1);
    asdf *p = new asdf();
    p->qwer.push_back(*n);
    for(int i = 2; i< 10; i++){
        Node *x = new Node(i);
        p->qwer.push_back(*x);
        Node *a;
        for( auto q : p->qwer){
            if(q.number == i/2){
                a = &q;
                break;
            }
        }
        p->zxcv.push_back(make_pair(*a,*x));
    }
    return 0; 
}