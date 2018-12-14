#include <iostream>
#include <stack>
#include <thread>
#include "ListNode.h"
using namespace std;

//
//ListNode* reverseList (ListNode* l)
//{
//    ListNode* reverselist = new ListNode(0);
//   // reverselist = NULL;
//    return reverselist;
//};
//leetcode 2. Add Two Numbers
//struct ListNode
//{
//    int val;
//    ListNode * next;
//    ListNode(int x): val(x), next(NULL){}
//};
//ListNode* addTwoNumbers(ListNode* l1, ListNode* l2)
//{
//    stack<int> s1, s2;
//    while (l1) {
//        s1.push(l1->val);
//        l1 = l1->next;
//    }
//    while (l2) {
//        s2.push(l2->val);
//        l2 = l2->next;
//    }
//    int sum = 0;
//    ListNode *res = new ListNode(0);
//    while (!s1.empty() || !s2.empty()) {
//        if (!s1.empty()) {sum += s1.top(); s1.pop();}
//        if (!s2.empty()) {sum += s2.top(); s2.pop();}
//        res->val = sum % 10;
//        ListNode *head = new ListNode(sum / 10);
//        head->next = res;
//        res = head;
//        sum /= 10;
//    }
//    return res->val == 0 ? res->next : res;
//
//};
//leetcode 20. Valid Parentheses
bool isValid(string s)
{
    stack<char> tmp;
    for(auto c : s)
    {
        if(c == '(' || c == '[' || c == '{')
        {
            tmp.push(c);
            continue;
        }
        if(tmp.size() == 0) return false;
        if(c == ')'&& tmp.top() == '(' || c == ']' && tmp.top() == '[' || c == '}' && tmp.top() == '{')
        {
            tmp.pop();
        }
        else
            return false;
        
    }
    if(tmp.size() != 0)
        return false;
    return true;
    
};
void printThread()
{
    cout<<" in my thread"<<endl;
}


int main(int argc, const char * argv[])
{
    // insert code here...
//    std::cout << "Hello, World!\n";
//    std::sort(1,2 ,3 );// STD sort 排序 需要注意的是 相等的情况下不能输出 true
//     ListNode* l1  = new ListNode(0);
//     ListNode *l1,*l2,*temp,*temp1 = NULL;
//    for (int i = 0; i<9; i++) {
//        if(temp==NULL)
//        {
//            l1 = temp = new ListNode(1);
//            l2 = temp1 = new ListNode(9);
//        }
//        else
//        {
//            temp->next= new ListNode(9);
//            temp=temp->next;
//
//        }
//    }
//   ListNode* l3 = addTwoNumbers(l2,l1);
//    string s = "()";
//    bool b=  isValid(s);
    thread*  t1 = new thread(printThread);
    
    t1->join();
    delete    t1;
    t1 = nullptr;
    
    cout<<"in major thread"<<endl;//在主线程
    return 0;
    
}
/**
  * Definition for singly-linked list.
  * struct ListNode {
  *     int val;
  *     ListNode *next;
  *     ListNode(int x) : val(x), next(NULL) {}
  * };
  */
