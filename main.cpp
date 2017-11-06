#include "ExpProcessor.h"

using namespace std;

int main()
{
    cout << "Enter the expression you want to calculate, or 'q' to quit:\n" << endl;

    while (1)
    {
        string expression;
        char ch;
        // 利用cin性质将空格、制表符等去掉
        while ((cin >> ch) && (ch != '=') && (ch != 'q'))
            expression.push_back(ch);

        if (ch == 'q')
            break;

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

        cout << exp.calculate() << "\n" << endl;
    }
    return 0;
}

