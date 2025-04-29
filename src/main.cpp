#include "./parser/parser.cpp"

int main() {
    cout << "Program started" << endl;

    string code = R"(
        int x;
        x = 10;
        float y;
        y = 3.14;
        
        if (x > 5) {
            y = y + 1.0;
        } else {
            while (y < 10.0) {
                y = y * 2.0;
            }
        }
    )";
    
    Lexer lexer(code);
    SyntaxParser parser(lexer);

    vector<Token> tokens = lexer.tokenize();
    printTokens(tokens);
    
    try {
        shared_ptr<SyntaxTreeNode> syntaxTree = parser.parse();
        cout << "\nSyntax Tree:"<< endl;
        printSyntaxTree(syntaxTree);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}