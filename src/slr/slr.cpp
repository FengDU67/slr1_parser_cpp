#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <memory>
#include <fstream>
#include "grammer.h"

using namespace std;

inline void printTables(const vector<unordered_map<string, TableAction>>& actionTable,
    const vector<unordered_map<string, int>>& gotoTable,
    const string& filename);

class SLRParser {
private:
    vector<Production> productions;
    unordered_set<string> nonTerminals;
    unordered_set<string> terminals;
    string startSymbol;
    unordered_map<string, unordered_set<string>> firstSets;
    unordered_map<string, unordered_set<string>> followSets;

    void initializeFirstSets() {
        // 终结符的FIRST集是它自己
        for (const auto& term : terminals) {
            firstSets[term].insert(term);
        }
        
        // 非终结符的FIRST集初始为空
        for (const auto& nt : nonTerminals) {
            firstSets[nt] = {};
        }

        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                const string& A = prod.left;
                size_t originalSize = firstSets[A].size();

                bool canDeriveEpsilon = true;
                for (const auto& symbol : prod.right) {
                    // 处理当前符号的FIRST集
                    for (const auto& s : firstSets[symbol]) {
                        if (s != "ε" && firstSets[A].insert(s).second) {
                            changed = true;
                        }
                    }

                    // 若当前符号不能推导ε，则后续符号无需处理
                    if (!firstSets[symbol].count("ε")) {
                        canDeriveEpsilon = false;
                        break;
                    }
                }

                // 若所有符号均可推导ε，则添加ε到FIRST(A)
                if (canDeriveEpsilon && firstSets[A].insert("ε").second) {
                    changed = true;
                }

                if (canDeriveEpsilon && !firstSets[A].count("ε")) {
                    firstSets[A].insert("ε");
                    changed = true;
                }

                if (firstSets[A].size() > originalSize) {
                    changed = true;
                }
            }
        } while (changed);

        // 打印FIRST集用于调试
        cout << "\nFIRST Sets:\n";
        for (const auto& nt : nonTerminals) {
            cout << "  FIRST(" << nt << ") = { ";
            for (const auto& s : firstSets[nt]) cout<< s << " ";
            cout << "}\n";
        }
    }

    unordered_set<string> computeStringFirst(const vector<string>& symbols) {
        unordered_set<string> result;
        bool canDeriveEpsilon = true;
    
        for (const auto& symbol : symbols) {
            // 添加当前符号的FIRST集（排除ε）
            for (const auto& s : firstSets[symbol]) {
                if (s != "ε") {
                    result.insert(s);
                }
            }
    
            // 若当前符号不能推导ε，则后续符号无需处理
            if (!firstSets[symbol].count("ε")) {
                canDeriveEpsilon = false;
            }
        }
    
        // 若所有符号均可推导ε，添加ε
        if (canDeriveEpsilon) {
            result.insert("ε");
        }
    
        return result;
    }

    void initializeFollowSets() {
        for (const auto& nt : nonTerminals) {
            followSets[nt] = {};
        }
        followSets[startSymbol].insert("$");

        bool changed;
        do {
            changed = false;
            for (const auto& prod : productions) {
                const string& A = prod.left;
                const vector<string>& beta = prod.right;
        
                for (size_t i = 0; i < beta.size(); ++i) {
                    const string& B = beta[i];
                    if (!nonTerminals.count(B)) continue;
        
                    // 计算B的FOLLOW集
                    unordered_set<string> firstOfRest;
                    bool canDeriveEpsilon = true;
                    for (size_t j = i + 1; j < beta.size(); ++j) {
                        const string& symbol = beta[j];
                        for (const auto& s : firstSets[symbol]) {
                            if (s != "ε") {
                                firstOfRest.insert(s);
                            }
                        }
                        if (!firstSets[symbol].count("ε")) {
                            canDeriveEpsilon = false;
                            break;
                        }
                    }
        
                    // 添加firstOfRest到B的FOLLOW集
                    for (const auto& s : firstOfRest) {
                        if (followSets[B].insert(s).second) {
                            changed = true;
                        }
                    }
        
                    // 若后续符号可推导ε，或B是最后一个符号，添加A的FOLLOW集
                    if (canDeriveEpsilon || i == beta.size() - 1) {
                        for (const auto& s : followSets[A]) {
                            if (followSets[B].insert(s).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        } while (changed);

        cout << "\nFOLLOW Sets:\n";
        for (const auto& nt : nonTerminals) {
            cout << "  FOLLOW(" << nt << ") = { ";
            for (const auto& s : followSets[nt]) cout<< s << " ";
            cout << "}\n";
        }
    }


    vector<Item> closure(const vector<Item>& items) {
        vector<Item> closureItems = items;
        bool changed;
        do {
            changed = false;
            vector<Item> newItems;
    
            for (const auto& item : closureItems) {
                if (item.dotPos >= productions[item.prodId].right.size()) continue;
    
                const string& symbol = productions[item.prodId].right[item.dotPos];
                if (!nonTerminals.count(symbol)) continue;
    
                // 处理非终结符后的所有产生式
                for (const auto& prod : productions) {
                    if (prod.left != symbol) continue;
                
                    Item newItem{prod.id, 0};
    
                    // 检查是否已存在相同项
                    bool exists = false;
                    for (const auto& ci : closureItems) {
                        if (ci == newItem) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        exists = find(newItems.begin(), newItems.end(), newItem) != newItems.end();
                    }
    
                    if (!exists) {
                        newItems.push_back(newItem);
                        changed = true;
                    }
                }
            }
    
            closureItems.insert(closureItems.end(), newItems.begin(), newItems.end());
        } while (changed);
    
        return closureItems;
    }

    vector<Item> goTo(const vector<Item>& items, const string& symbol) {
        vector<Item> nextItems;
        for (const auto& item : items) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos < prod.right.size() && prod.right[item.dotPos] == symbol) {
                // 创建新项并计算闭包
                Item newItem{item.prodId, item.dotPos + 1};
                nextItems.push_back(newItem);
            }
        }
        return closure(nextItems);  // 返回闭包后的项集
    }

    vector<vector<Item>> constructCanonicalCollection() {
        vector<vector<Item>> canonicalCollection;
        // 初始化第一个项集
        vector<Item> initialItems = {
            Item(0, 0),
        };
        auto initialClosure = closure(initialItems);
        // // 调试输出
        // for (const auto& item : initialClosure) {
        //     const Production& prod = productions[item.prodId];
        //     cout << "Item: " << prod.left << " -> ";
        //     for (size_t i = 0; i < prod.right.size(); ++i) {
        //         if (i == item.dotPos) cout << ".";
        //         cout << prod.right[i] << " ";
        //     }
        //     if (item.dotPos == prod.right.size()) cout << ".";
        //     cout << ", Lookahead: { ";
        //     for (const auto& s : item.lookahead) cout << s << " ";
        //     cout << "}\n";
        // }
        canonicalCollection.push_back(initialClosure);
    
        bool changed;
        int iteration = 0;
        size_t last_size = 0;
        size_t oldSize = 0;
        do {
            iteration++;
            changed = false;
            last_size = oldSize;
            oldSize = canonicalCollection.size();
            vector<vector<Item>> temp_canonicalCollection;

            
            for (size_t i = last_size; i < oldSize; ++i) {
                const auto& items = canonicalCollection[i];
                
                unordered_set<string> symbols;
                for (const auto& item : items) {
                    const Production& prod = productions[item.prodId];
                    if (item.dotPos < prod.right.size()) {
                        string symbol = prod.right[item.dotPos];
                        symbols.insert(symbol);
                        
                    }
                }

                
                for (const auto& symbol : symbols) {
                    auto nextItems = goTo(items, symbol);
                    if (!nextItems.empty()) {
                        bool exists = false;
                        for (const auto& existing : canonicalCollection) {
                            if (existing == nextItems) {
                                exists = true;
                                break;
                            }
                        }

                        for (const auto& existing : temp_canonicalCollection) {
                            if (existing == nextItems) {
                                exists = true;
                                break;
                            }
                        }
                        
                        if (!exists) {
                            temp_canonicalCollection.push_back(nextItems);
                            changed = true;

                            // 调试输出
                            cerr << "Added new state with " << nextItems.size() << " items" << endl;
                        }
                    }
                }
            }

            // 合并 temp_canonicalCollection 到 canonicalCollection
            vector<vector<Item>> merged = canonicalCollection; // 复制原有项集
            for (const auto& newItems : temp_canonicalCollection) {
                bool exists = false;
                // 检查 newItems 是否已存在于 merged 中
                for (const auto& existingItems : merged) {
                    if (existingItems == newItems) { // 直接比较项集
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    merged.push_back(newItems);
                    changed = true;
                }
            }
            canonicalCollection = merged; // 更新规范族
            
            
            // 输出调试信息
            cerr << "Iteration " << iteration 
                 << ", Canonical size: " << canonicalCollection.size() 
                 << endl;

            
        } while (changed);
    
        return canonicalCollection;
    }


public:
    SLRParser(const vector<Production>& prods, 
             const unordered_set<string>& nts,
             const unordered_set<string>& terms,
             const string& start)
        : productions(prods), nonTerminals(nts), terminals(terms), startSymbol(start) 
    {
        initializeFirstSets();
        initializeFollowSets();
    }

    void buildSLRTable(vector<unordered_map<string, TableAction>>& actionTable,
                              vector<unordered_map<string, int>>& gotoTable) {
    auto canonicalCollection = constructCanonicalCollection();
    actionTable.resize(canonicalCollection.size());
    gotoTable.resize(canonicalCollection.size());

    for (size_t i = 0; i < canonicalCollection.size(); ++i) {
        const auto& items = canonicalCollection[i];
        unordered_map<string, TableAction>& actionRow = actionTable[i];
        unordered_map<string, int>& gotoRow = gotoTable[i];

        // 处理归约和接受动作
        for (const auto& item : items) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos == prod.right.size()) { // 归约项
                if (prod.left == startSymbol) {
                    // 接受动作（仅在$符号列）
                    actionRow["$"] = {ActionType::ACCEPT, -1};
                } else {
                    // 归约动作：仅添加到FOLLOW集的符号列
                    for (const auto& followSym : followSets[prod.left]) {
                        actionRow[followSym] = {ActionType::REDUCE, prod.id};
                    }
                }
            }
        }

        // 处理移进和GOTO
        unordered_set<string> processedSymbols;
        
        for (const auto& item : items) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos < prod.right.size()) {
                const string& symbol = prod.right[item.dotPos];
                if (processedSymbols.count(symbol)) continue; // 避免重复处理
                processedSymbols.insert(symbol);

                vector<Item> nextItems = goTo(items, symbol);
                int nextState = getStateIndex(nextItems, canonicalCollection);
                if (nextState == -1) continue;

                if (terminals.count(symbol)) { // 移进动作
                    actionRow[symbol] = {ActionType::SHIFT, nextState};
                } else if (nonTerminals.count(symbol)) { // GOTO转移
                    gotoRow[symbol] = nextState;
                }
            }
        }
    }

    printTables(actionTable, gotoTable, "../slr_table.txt");
}

    // 辅助函数：获取项集对应的状态编号
    int getStateIndex(const vector<Item>& items,vector<vector<Item>> canonicalCollection) {
        for (size_t i = 0; i < canonicalCollection.size(); ++i) {
            if (canonicalCollection[i] == items) {
                return i;
            }
        }
        return -1;
    }
};



inline void printTables(const vector<unordered_map<string, TableAction>>& actionTable,
                 const vector<unordered_map<string, int>>& gotoTable,
                 const string& filename = "../slr_table.txt") {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Failed to open file " << filename << endl;
        return;
    }

    // 分离动作符号（终结符）和转移符号（非终结符）
    unordered_set<string> actionSymbols;
    unordered_set<string> gotoSymbols;

    // 收集所有终结符（包括$）
    for (const auto& row : actionTable) {
        for (const auto& entry : row) {
            actionSymbols.insert(entry.first);
        }
    }

    // 收集所有非终结符
    for (const auto& row : gotoTable) {
        for (const auto& entry : row) {
            gotoSymbols.insert(entry.first);
        }
    }

    // 排序符号
    vector<string> sortedActionSymbols(actionSymbols.begin(), actionSymbols.end());
    sort(sortedActionSymbols.begin(), sortedActionSymbols.end());
    vector<string> sortedGotoSymbols(gotoSymbols.begin(), gotoSymbols.end());
    sort(sortedGotoSymbols.begin(), sortedGotoSymbols.end());

    // 输出表头：终结符 | 非终结符
    file << "State\t";
    for (const auto& sym : sortedActionSymbols) file << sym << "\t";
    file << "|\t";
    for (const auto& sym : sortedGotoSymbols) file << sym << "\t";
    file << "\n";

    // 输出每个状态的行
    for (size_t i = 0; i < actionTable.size(); ++i) {
        file << "State " << i << "\t";
        
        // 动作表部分（终结符）
        for (const auto& sym : sortedActionSymbols) {
            const auto& actionRow = actionTable[i];
            auto it = actionRow.find(sym);
            if (it != actionRow.end()) {
                const auto& action = it->second;
                switch (action.type) {
                    case ActionType::SHIFT:    file << "s" << action.value; break;
                    case ActionType::REDUCE:   file << "r" << action.value; break;
                    case ActionType::ACCEPT:   file << "acc"; break;
                    default:                   file << "?";
                }
            } else {
                file << ""; // 空单元格
            }
            file << "\t";
        }
        
        file << "|\t"; // 分隔符
        
        // 转移表部分（非终结符）
        for (const auto& sym : sortedGotoSymbols) {
            const auto& gotoRow = gotoTable[i];
            auto it = gotoRow.find(sym);
            if (it != gotoRow.end()) {
                file << it->second;
            } else {
                file << ""; // 空单元格
            }
            file << "\t";
        }
        file << "\n";
    }

    file.close();
    cout << "SLR table has been saved to " << filename << endl;
}