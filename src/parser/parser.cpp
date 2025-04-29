#include "../lexer/lexer.cpp"
#include "../slr/slr.cpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>

using namespace std;

class SyntaxParser {
private:
    // 分析表结构
    struct AnalysisTables {
        vector<unordered_map<string, TableAction>> action;
        vector<unordered_map<string, int>> goto_;
    };

    AnalysisTables tables_;
    vector<Production> productions_;
    Lexer lexer_;

    // 初始化文法产生式
    void initializeProductions() {
        productions_ = {
            // 增广文法
            {"S'", {"Program"}, 0},
            
            // Program产生式
            {"Program", {"Statements"}, 1},
            {"Statements", {"Statement", "Statements"}, 2},
            {"Statements", {"ε"}, 3},
            
            // Statement产生式
            {"Statement", {"DeclStmt"}, 4},
            {"Statement", {"AssignStmt"}, 5},
            {"Statement", {"IfStmt"}, 6}, 
            {"Statement", {"WhileStmt"}, 7},
            {"Statement", {"Compute"}, 8},
            
            
            // DeclStmt产生式
            {"DeclStmt", {"Type", "IDENTIFIER", "SEMICOLON"}, 9},
            
            // AssignStmt产生式
            {"AssignStmt", {"IDENTIFIER", "ASSIGNMENT", "NUMBER", "SEMICOLON"}, 10},

            {"Compute", {"IDENTIFIER", "ASSIGNMENT", "Expr", "SEMICOLON"}, 11},
            
            // IfStmt产生式
            {"IfStmt", {"IF", "LEFT_PAREN", "Expr", "RIGHT_PAREN", "LEFT_BRACE", "Statements", "RIGHT_BRACE", "ElsePart"}, 12},
            {"ElsePart", {"ELSE", "LEFT_BRACE", "Statements", "RIGHT_BRACE"}, 13},
            {"ElsePart", {"ε"}, 14}, 
            
            // WhileStmt产生式
            {"WhileStmt", {"WHILE", "LEFT_PAREN", "Expr", "RIGHT_PAREN", "LEFT_BRACE", "Statements", "RIGHT_BRACE"}, 15},
            
            // 表达式处理
            {"Expr", {"IDENTIFIER", "OPERATOR", "NUMBER"}, 16},

            
            // OPERATOR → "+" | "-" | "*" | "/" | "==" | "!=" | "<" | ">" | "<=" | ">="
            {"OPERATOR", {"PLUS"}, 17},
            {"OPERATOR", {"MUL"}, 18},
            {"OPERATOR", {"LT"}, 19},
            {"OPERATOR", {"GT"}, 20},

            
            // 类型声明
            {"Type", {"int"}, 21},
            {"Type", {"float"}, 22},
            {"Type", {"bool"}, 23},
            
            // 语句块处理
            {"Statements", {"StmtList"}, 24},  
            {"Statements", {"ε"}, 25},         
            {"StmtList", {"Statement"}, 26},
            {"StmtList", {"Statement", "StmtList"}, 27}
        };
    }
    

    // 构建SLR分析表
    void buildSLRTable() {
        // 确保这些参数正确初始化
        auto nonTerms = getNonTerminals();
        auto terms = getTerminals();
        string startSymbol = "S'"; // 或你的文法起始符号
        SLRParser slrParser(productions_, nonTerms, terms, startSymbol);
        slrParser.buildSLRTable(tables_.action, tables_.goto_);
    }

    // 获取非终结符集合
    unordered_set<string> getNonTerminals() const {
        unordered_set<string> nonTerms;
        for (const auto& prod : productions_) {
            nonTerms.insert(prod.left);
        }
        return nonTerms;
    }

    // 获取终结符集合
    unordered_set<string> getTerminals() const {
        return {
            "IDENTIFIER", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE",
            "RIGHT_BRACE", "SEMICOLON", "ASSIGNMENT", "IF", "ELSE", "WHILE",
            "PLUS", "MUL", "LT", "GT", "int", "float", "bool", "NUMBER", "$"
        };
    }

public:
    explicit SyntaxParser(const Lexer& lexer) : lexer_(lexer) {
        initializeProductions();
        this->buildSLRTable();
    }

    // 执行语法分析
    shared_ptr<SyntaxTreeNode> parse() {
        ParseContext context;
        context.tokens = lexer_.tokenize();
        context.tokens.push_back(Token{$, "$", 0});  // 添加结束标记
    
        // 打印输入的Token流
        cout << "=== Token Stream ===" << endl;
        for (const auto& tok : context.tokens) {
            cout << "[" << tok.type << " \"" << tok.value << "\" line:" << tok.line << "]" << endl;
        }
        cout << "====================" << endl;
    
        while (context.pos < context.tokens.size()) {
            const Token& currentToken = context.tokens[context.pos];
            const string& tokenValue = currentToken.value;
            const TokenType& tokentype = currentToken.type;
            int currentState = context.currentState();
    
            // 打印当前状态和输入符号
            cout << "\nCurrent State: " << currentState 
                 << ", Next Token: [" << currentToken.type
                 << " \"" << tokenValue << "\"]" << endl;
    
            cout << "currentToken.type: " << currentToken.type << endl;
            const TableAction action = getAction(currentState, currentToken);
    
            switch (action.type) {
                case SHIFT: {
                    performShift(action.value, tokenValue, context);
                    // 打印移进后的状态栈和符号栈
                    cout << "  ↪ SHIFT: push state " << action.value << endl;
                    cout << "  State Stack: [ ";
                    for (int s : context.stateStack) cout << s << " ";
                    cout << "]" << endl;
                    cout << "  Symbol Stack: [ ";
                    for (const auto& node : context.symbolStack) cout << node->symbol << " ";
                    cout << "]" << endl;
                    break;
                }
                case REDUCE: {
                    const Production& prod = productions_[action.value];
                    cout << "  ↪ REDUCE: " << prod.left << " -> ";
                    for (const auto& sym : prod.right) cout << sym << " ";
                    cout << endl;
                    performReduction(action.value, context);
                    // 打印归约后的GOTO状态
                    cout << "  ↪ GOTO[" << context.currentState() << ", " << prod.left << "] = "
                         << context.currentState() << endl;
                    cout << "  State Stack: [ ";
                    for (int s : context.stateStack) cout << s << " ";
                    cout << "]" << endl;
                    cout << "  Symbol Stack: [ ";
                    for (const auto& node : context.symbolStack) cout << node->symbol << " ";
                    cout << "]" << endl;
                    break;
                }
                case ACCEPT: {
                    cout << "** ACCEPTED **" << endl;
                    return finalizeParsing(context);
                }
                case ERROR:
                default: {
                    cerr << "  ↪ ERROR: No action defined for token \"" << tokenValue 
                         << "\" in state " << currentState << endl;
                    handleError(context);
                    break;
                }
            }
        }
    
        throw runtime_error("Unexpected end of input");
    }

private:
    // 解析上下文
    struct ParseContext {
        vector<Token> tokens;
        size_t pos = 0;
        vector<int> stateStack = {0};
        vector<shared_ptr<SyntaxTreeNode>> symbolStack;

        int currentState() const { return stateStack.back(); }
    };

    // 在SyntaxParser类中添加以下成员函数
    string tokenTypeToTerminal(const Token& token) const {
        switch (token.type) {
            // 类型关键字映射（int -> "int"）
            case TokenType::TYPE: 
                return token.value;  // 直接返回"int"/"float"等字符串

            // 标识符统一映射
            case TokenType::IDENTIFIER:
                return "IDENTIFIER";

            // 控制关键字映射到对应终结符
            case TokenType::IF:       return "IF";
            case TokenType::ELSE:     return "ELSE";
            case TokenType::WHILE:    return "WHILE";
            case TokenType::FOR:      return "FOR";

            // 运算符映射到文法定义名称
            case TokenType::OPERATOR: {
                static const unordered_map<string, string> opMap = {
                    {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MUL"}, {"/", "DIV"},
                    {"=", "ASSIGN"}, {"==", "EQ"}, {"!=", "NEQ"}, {"<", "LT"},
                    {"<=", "LEQ"}, {">", "GT"}, {">=", "GEQ"}, {"&&", "AND"},
                    {"||", "OR"}, {"!", "NOT"}, {"++", "INC"}, {"--", "DEC"},
                    {"+=", "ADD_ASSIGN"}, {"-=", "SUB_ASSIGN"}, {"*=", "MUL_ASSIGN"}, {"/=", "DIV_ASSIGN"}
                };
                auto it = opMap.find(token.value);
                return (it != opMap.end()) ? it->second : "UNKNOWN_OP";
            }
            case TokenType::ASSIGNMENT: return "ASSIGNMENT";

            // 分隔符映射
            case TokenType::LEFT_PAREN:   return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN:  return "RIGHT_PAREN";
            case TokenType::LEFT_BRACE:   return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE:  return "RIGHT_BRACE";
            case TokenType::SEMICOLON:    return "SEMICOLON";
            case TokenType::COMMA:        return "COMMA";

            // 字面量和特殊符号
            case TokenType::NUMBER:       return "NUMBER";
            case TokenType::STRING:       return "STRING";
            case TokenType::$:            return "$";
                
            default:
                return "UNKNOWN";
        }
    }

    // 获取当前动作
    TableAction getAction(int state, const Token& token) const {
        string terminal = tokenTypeToTerminal(token);
        cout << "Terminal: " << terminal << endl;
        if (tables_.action[state].count(terminal)) {
            return tables_.action[state].at(terminal);
        }
        // 处理未映射的符号（如未识别的运算符）
        return {ERROR, -1};
    }

    // 执行移进动作
    void performShift(int newState, const string& token, ParseContext& context) {
        context.stateStack.push_back(newState);
        context.symbolStack.push_back(
            make_shared<SyntaxTreeNode>(token, context.tokens[context.pos].value));
        context.pos++;
    }

    // 执行归约动作
    void performReduction(int prodId, ParseContext& context) {
        const Production& prod = productions_[prodId];
        auto node = make_shared<SyntaxTreeNode>(prod.left, "");

        // 弹出右部符号
        for (size_t i = 0; i < prod.right.size(); ++i) {
            context.stateStack.pop_back();
            node->children.insert(node->children.begin(), context.symbolStack.back());
            context.symbolStack.pop_back();
        }

        // 处理GOTO
        int newState = tables_.goto_[context.currentState()].at(prod.left);
        context.stateStack.push_back(newState);
        context.symbolStack.push_back(node);

        logReduction(prod);
    }

    // 完成语法分析
    shared_ptr<SyntaxTreeNode> finalizeParsing(ParseContext& context) {
        if (context.symbolStack.size() != 1) {
            throw runtime_error("Invalid parse result");
        }
        cout << "Parsing completed successfully!" << endl;
        return context.symbolStack.back();
    }

    // 错误处理
    void handleError(ParseContext& context) {
        const Token& errorToken = context.tokens[context.pos];
        cerr << "Syntax error at line " << errorToken.line 
             << ": unexpected token '" << errorToken.value << "'" << endl;

        // 恐慌模式恢复
        recoverFromError(context);
    }

    // 错误恢复
    void recoverFromError(ParseContext& context) {
        static const set<string> syncSymbols = {"SEMICOLON", "$"};
    
        // 查找最近的同步符号
        while (context.pos < context.tokens.size() &&
               syncSymbols.count(context.tokens[context.pos].value) == 0) {
            context.pos++;
        }
    
        // 弹出栈直到找到可继续分析的状态
        while (!context.stateStack.empty()) {
            int currentState = context.currentState();
            bool hasValidAction = false;
    
            // 检查当前状态是否有合法的同步符号动作
            if (context.pos < context.tokens.size()) {
                const string& token = context.tokens[context.pos].value;
                hasValidAction = tables_.action[currentState].count(token) &&
                                tables_.action[currentState][token].type != ERROR;
            }
    
            if (hasValidAction) break;
    
            context.stateStack.pop_back();
            if (!context.symbolStack.empty()) {
                context.symbolStack.pop_back();
            }
        }
    
        if (context.stateStack.empty()) {
            throw runtime_error("Fatal parsing error: recovery failed");
        }
    }

    // 检查有效动作
    bool hasValidAction(int state, const string& token) const {
        return tables_.action[state].count(token) && 
               tables_.action[state].at(token).type != ERROR;
    }

    // 记录归约操作
    void logReduction(const Production& prod) const {
        cout << "Reduced by: " << prod.left << " -> ";
        for (const auto& sym : prod.right) cout << sym << " ";
        cout << endl;
    }
};

// 打印语法树
inline void printSyntaxTree(const shared_ptr<SyntaxTreeNode>& node, int depth = 0) {
    if (!node) return;

    // 缩进
    for (int i = 0; i < depth; ++i) cout << "  ";

    // 节点信息
    cout << node->symbol;
    if (!node->value.empty()) {
        cout << " (" << node->value << ")";
    }
    cout << endl;

    // 递归打印子节点
    for (const auto& child : node->children) {
        printSyntaxTree(child, depth + 1);
    }
}