#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

using namespace std;

// 定义词法单元类型
enum TokenType {
    KEYWORD,    // 关键字
    IDENTIFIER, // 标识符
    NUMBER,     // 数字
    OPERATOR,   // 运算符
    DELIMITER,  // 分隔符
    STRING,     // 字符串
    COMMENT,    // 注释
    $,         // 结束符
    UNKNOWN     // 未知
};

// 词法单元结构
struct Token {
    TokenType type;
    string value;
    size_t line;

    // 构造函数
    Token() : type(UNKNOWN), value(""), line(0) {}
    Token(TokenType t, const string& v, size_t l) : type(t), value(v), line(l) {}
    
    // 如果需要，可以添加比较运算符
    bool operator==(const Token& other) const {
        return type == other.type && 
               value == other.value && 
               line == other.line;
    }
};

// 词法分析器类
class Lexer {
private:
    string source;
    size_t pos;
    size_t line;

    const unordered_map<string, TokenType> Keywords = {
        {"int", KEYWORD}, {"float", KEYWORD}, {"double", KEYWORD},
        {"char", KEYWORD}, {"void", KEYWORD}, {"bool", KEYWORD},
        {"if", KEYWORD}, {"else", KEYWORD}, {"while", KEYWORD},
        {"for", KEYWORD}, {"return", KEYWORD}, {"class", KEYWORD},
        {"struct", KEYWORD}, {"true", KEYWORD}, {"false", KEYWORD}
    };

    // 运算符集合
    const unordered_map<string, TokenType> operators = {
        {"+", OPERATOR}, {"-", OPERATOR}, {"*", OPERATOR}, {"/", OPERATOR},
        {"=", OPERATOR}, {"==", OPERATOR}, {"!=", OPERATOR}, {"<", OPERATOR},
        {"<=", OPERATOR}, {">", OPERATOR}, {">=", OPERATOR}, {"&&", OPERATOR},
        {"||", OPERATOR}, {"!", OPERATOR}, {"++", OPERATOR}, {"--", OPERATOR},
        {"+=", OPERATOR}, {"-=", OPERATOR}, {"*=", OPERATOR}, {"/=", OPERATOR}
    };

    // 分隔符集合
    const unordered_map<string, TokenType> delimiters = {
        {"(", DELIMITER}, {")", DELIMITER}, {"{", DELIMITER}, {"}", DELIMITER},
        {"[", DELIMITER}, {"]", DELIMITER}, {";", DELIMITER}, {",", DELIMITER},
        {".", DELIMITER}, {":", DELIMITER}, {"::", DELIMITER}
    };

    char peek(size_t offset = 0) const {
        if (pos + offset >= source.length()) return '\0';
        return source[pos + offset];
    }

    void consume() {
        pos++;
    }

    bool isOperator(char c) const {
        string op(1, c);
        return operators.find(op) != operators.end();
    }

    bool isDelimiter(char c) const {
        string delim(1, c);
        return delimiters.find(delim) != delimiters.end();
    }

    Token readNumber() {
        string value;
        bool hasDecimal = false;
        
        while (pos < source.length()) {
            char current = peek();
            
            if (isdigit(current)) {
                value += current;
                consume();
            }
            else if (current == '.' && !hasDecimal) {
                hasDecimal = true;
                value += current;
                consume();
            }
            else {
                break;
            }
        }
        
        return {NUMBER, value, line};
    }

    Token readIdentifier() {
        string value;
        
        while (pos < source.length()) {
            char current = peek();
            
            if (isalnum(current) || current == '_') {
                value += current;
                consume();
            }
            else {
                break;
            }
        }

        // 检查关键字
        if (Keywords.find(value) != Keywords.end()) {
            auto it = Keywords.find(value);
            if (it != Keywords.end()) {
                return {it->second, value, line};
            }
        }
        // 其他情况视为标识符
        else {
            return {IDENTIFIER, value, line};
        }
    }

    Token readString() {
        string value;
        consume(); // 跳过开始的引号
        
        while (pos < source.length()) {
            char current = peek();
            
            if (current == '\\') { // 处理转义字符
                consume();
                char next = peek();
                switch (next) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case '"': value += '"'; break;
                    case '\\': value += '\\'; break;
                    default: value += next; break;
                }
                consume();
            }
            else if (current == '"') { // 结束引号
                consume();
                break;
            }
            else {
                value += current;
                consume();
            }
        }
        
        return {STRING, value, line};
    }

    void readLineComment() {
        consume(); // 跳过第一个'/'
        consume(); // 跳过第二个'/'
        
        while (pos < source.length()) {
            if (peek() == '\n') {
                line++;
                consume();
                break;
            }
            consume();
        }
    }

    void readBlockComment() {
        consume(); // 跳过第一个'/'
        consume(); // 跳过第二个'*'
        
        while (pos < source.length()) {
            if (peek() == '*' && peek(1) == '/') {
                consume(); // 跳过'*'
                consume(); // 跳过'/'
                break;
            }
            if (peek() == '\n') {
                line++;
            }
            consume();
        }
    }

    Token readOperator() {
        string value;
        value += peek();
        consume();
        
        // 检查是否是双字符运算符
        if (pos < source.length()) {
            string twoCharOp = value + peek();
            if (operators.find(twoCharOp) != operators.end()) {
                value += peek();
                consume();
            }
        }
        
        auto it = operators.find(value);
        if (it != operators.end()) {
            return {it->second, value, line};
        }
    }

    Token readDelimiter() {
        string value;
        value += peek();
        consume();
        
        // 处理单字符分隔符
        auto it = delimiters.find(value);
        if (it != delimiters.end()) {
            return {it->second, value, line};
        }
        
        // 处理多字符分隔符（如 "::"）
        if (value.size() > 1) {
            it = delimiters.find(value);
            if (it != delimiters.end()) {
                return {it->second, value, line};
            }
        }
        
        return {UNKNOWN, value, line}; // 无效分隔符
    }

public:
    Lexer(const string& source) : source(source), pos(0), line(1) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        
        while (pos < source.length()) {
            char current = peek();
            
            if (isspace(current)) {
                consume();
                if (current == '\n') line++;
            }
            else if (isdigit(current)) {
                tokens.push_back(readNumber());
            }
            else if (isalpha(current) || current == '_') {
                tokens.push_back(readIdentifier());
            }
            else if (current == '"') {
                tokens.push_back(readString());
            }
            else if (current == '/' && peek(1) == '/') {
                readLineComment();
            }
            else if (current == '/' && peek(1) == '*') {
                readBlockComment();
            }
            else if (isOperator(current)) {
                tokens.push_back(readOperator());
            }
            else if (isDelimiter(current)) {
                tokens.push_back(readDelimiter());
            }
            else {
                tokens.push_back({UNKNOWN, string(1, current), line});
                consume();
            }
        }
        
        return tokens;
    }
};

// 打印词法分析结果
inline void printTokens(const vector<Token>& tokens) {
    for (const auto& token : tokens) {
        string typeStr;
        switch (token.type) {
            case KEYWORD: typeStr = "KEYWORD"; break;
            case IDENTIFIER: typeStr = "IDENTIFIER"; break;
            case NUMBER: typeStr = "NUMBER"; break;
            case OPERATOR: typeStr = "OPERATOR"; break;
            case DELIMITER: typeStr = "DELIMITER"; break;
            case STRING: typeStr = "STRING"; break;
            case COMMENT: typeStr = "COMMENT"; break;
            case UNKNOWN: typeStr = "UNKNOWN"; break;
        }
        cout << "[" << typeStr << ": " << token.value << "] (Line " << token.line << ")" << endl;
    }
}