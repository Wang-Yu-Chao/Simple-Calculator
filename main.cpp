#include "ExpProcessor.h"

using namespace std;

int main()
{
    string expression;
    char ch;
    // 利用cin性质将空格、制表符等去掉
    while ((cin >> ch) && (ch != '='))
        expression.push_back(ch);

    ExpProcessor exp(expression);

    // TEST
//    exp.testExpression();
//    cout << endl;
//    exp.testPreProcess();
//    cout << endl;
//    exp.testExpression();
//    cout << endl;
//    exp.testNodes();
//    cout << endl;
//    exp.testTree();
//    cout << endl;
//    exp.testCalculate();

    cout << exp.calculate() << endl;

    return 0;
}

