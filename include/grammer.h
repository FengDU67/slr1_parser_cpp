#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

using namespace std;

// 文法产生式
struct Production {
    string left;  // 左部非终结符
    vector<string> right;  // 右部符号串
    int id;  // 产生式编号

    bool operator==(const Production& other) const {
        return left == other.left && 
               right == other.right && 
               id == other.id;
    }
};

// 项目(Item)
struct Item {
    int prodId;
    int dotPos;

    Item(int pid, int dpos)
        : prodId(pid), dotPos(dpos) {}

    bool operator==(const Item& other) const {
        return prodId == other.prodId &&
               dotPos == other.dotPos;
    }
};

// 语法分析表动作
enum ActionType {
    SHIFT,
    REDUCE,
    ACCEPT,
    ERROR
};

struct TableAction {
    ActionType type;
    int value;  // 移进状态或归约产生式编号
};

// 语法树节点
struct SyntaxTreeNode {
    string symbol;
    string value;
    vector<shared_ptr<SyntaxTreeNode>> children;
    
    // 添加构造函数
    SyntaxTreeNode(const string& sym, const string& val) 
        : symbol(sym), value(val) {}
    
    // 默认构造函数
    SyntaxTreeNode() = default;
    
    // 如果需要，添加拷贝和移动构造函数
    SyntaxTreeNode(const SyntaxTreeNode&) = default;
    SyntaxTreeNode(SyntaxTreeNode&&) = default;
};