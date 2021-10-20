#include <iostream>
#include <vector>
using namespace std;
class Node{
    public:
        int number;
        int p=rand();
    
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
    Node *n[2] = {new Node(1), new Node(2)};
    asdf *p = new asdf();
    p->qwer.push_back(*n[0]);
    p->qwer.push_back(*n[1]);
    p->zxcv.push_back(make_pair(*n[0],*n[1]));
    for(int i = 3; i< 10; i++){
        Node *x = new Node(i);
        p->qwer.push_back(*x);
        p->zxcv.push_back(make_pair(*n[rand()%2],*x));
    }
   
    return 0; 
}
