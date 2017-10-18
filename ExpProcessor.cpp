//
// Created by 王宇超 on 17/10/11.
//

#include <complex>
#include "ExpProcessor.h"

using namespace std;

map<string, char> ExpProcessor::operatorMap = { {"sin", 'a'}, {"cos", 'b'}, {"log", 'c'}, {"ln", 'd'} };
map<char, function<double(double)>> ExpProcessor::unaryFuncMap = { {'+', [](double n){ return 0 + n; }},
        {'-', [](double n){ return 0 - n; }}, {'a', static_cast<double (*)(double)>(sin)},
        {'b', static_cast<double (*)(double)>(cos)}, {'c', static_cast<double (*)(double)>(log10)},
        {'d', static_cast<double (*)(double)>(log)} };
std::map<char, function<double(double, double)>> ExpProcessor::binaryFuncMap =
        { {'+', std::plus<double>()}, {'-', minus<double>()}, {'*', multiplies<double>()}, {'/', divides<double>()},
        {'^', static_cast<double (*)(double, double)>(pow)} };


// 两次遍历expression，时间O(N)
void ExpProcessor::preProcess()
{
    stack<int> brackets;

    for (auto iter = expression.begin(); iter != expression.end(); iter++)
    {
        // 将expression中的特殊操作符字符串转换为单个字符
        if (isalpha(*iter))
        {
            string specialOp;
            auto opBegin = iter;
            while (isalpha(*iter))
                specialOp += *(iter++);
            if (operatorMap.find(specialOp) == operatorMap.end())
                expError("*Invalid operators*", 5);
            *opBegin = operatorMap[specialOp];
            // 检查特殊操作符后面是否有括号
            if (*iter != '(')
                expError("*No bracket after a special operator*", 1);
            expression.erase(opBegin + 1, iter);
            iter = opBegin;
        }
    }

    for (auto iter = expression.begin(); iter != expression.end(); )
    {
        // 检查表达式首部是否有运算符（除单目运算符）
        if (iter == expression.begin() && isOperator(*iter) && !isUnaryOperator(*iter))
            expError("*Invalid operand*", 4);
        // 检查是否存在空括号；检查左右括号顺序是否合法
        else if (*iter == '(')
        {
            if (*(iter + 1) == ')')
                expError("*Empty brackets*", 7);
            brackets.push(1);
            ++iter;
        }
        else if (*iter == ')')
        {
            if (brackets.empty())
                expError("*Invalid order of brackets*", 2);
            else
                brackets.pop();
            ++iter;
        }
        // 检查是否有两个及以上的运算符同时出现（除单目运算符可以是第二个运算符）；以及运算符左右括号是否正确
        else if (isBinaryOperator(*iter))
        {
            if ((*(iter - 1) == '(' && !isUnaryOperator(*iter)) || *(iter + 1) == ')')
                expError("*Invalid operands*", 4);
            ++iter;
            if ((isOperator(*iter) && !isUnaryOperator(*iter)) || (isOperator(*iter) && isOperator(*(iter + 1))))
                expError("*Invalid operands*", 4);
        }
        // 检查是否有不合规范的小数点
        else if (*iter == '.')
        {
            if (!isdigit(*(iter - 1)) || !isdigit(*(iter + 1)))
                expError("*Invalid decimal point*", 6);
            ++iter;
        }
        else
        {
            if (!isdigit(*iter) && !isalpha(*iter))
                expError("*Invalid symbol*", 8);
            ++iter;
        }
    }
    // 检查括号是否对称
    if (!brackets.empty())
        expError("*Unbalanced brackets*", 3);
}

// 一次遍历expression，时间O(N)
void ExpProcessor::buildExpNodes()
{
    // 清空nodes
    nodes.clear();
    for (auto iter = expression.begin(); iter != expression.end(); )
    {
        if (isdigit(*iter))
        {
            auto operandNode = insertNode(OPERAND, ROOT, 0.0, '\0', nullptr, nullptr);
            auto digitBegin = iter;
            int count = 0;
            while (isdigit(*++iter))
                ++count;
            for_each(digitBegin, iter, [&](char c){ operandNode->value += (pow(10, count--) * (c - '0')); });
        }
        else if (*iter == '.')
        {
            auto &operandNode = nodes.back();
            int count = 0;
            while (isdigit(*++iter))
            {
                ++count;
                operandNode->value += (pow(0.1, count) * (*iter - '0'));
            }
        }
        else if (*iter == '(' || *iter == ')')
        {
            insertNode(BRACKET, ROOT, 0.0, *iter, nullptr, nullptr);
            ++iter;
        }
        else if ((iter == expression.begin() || *(iter - 1) == '(') && isUnaryOperator(*iter))
        {
            insertNode(UOPERATOR, ROOT, 0.0, *iter, nullptr, nullptr);
            ++iter;
        }
        else if (isBinaryOperator(*iter))
        {
            insertNode(BOPERATOR, ROOT, 0.0, *iter, nullptr, nullptr);
            ++iter;
            if (isUnaryOperator(*iter))
            {
                insertNode(UOPERATOR, ROOT, 0.0, *iter, nullptr, nullptr);
                ++iter;
            }
        }
        else if (isUnaryOperator(*iter))
        {
            insertNode(UOPERATOR, ROOT, 0.0, *iter, nullptr, nullptr);
            ++iter;
        }
    }
}

ExpTreeNode ExpProcessor::buildExpTree(vector<ExpTreeNode>::iterator p1, vector<ExpTreeNode>::iterator p2)
{
    // 按括号层次进行递归；得到一个不含值为括号的结点容器
    decltype(p1) firstBracket, lastBracket;
    int count = 0;
    for (auto iter = p1; iter != p2; iter++)
    {
        if ((*iter)->type == BRACKET && (*iter)->op== '(')
        {
            if (!count)
            {
                firstBracket = iter;
                (*iter)->position = INNERNODE;
            }
            ++count;
        }
        else if ((*iter)->type == BRACKET && (*iter)->op== ')')
        {
            --count;
            if (!count)
            {
                lastBracket = iter;
                (*iter)->position = INNERNODE;
                auto subtree = buildExpTree(firstBracket + 1, lastBracket);
                subtree->position = SUBTREE;
            }
        }
    }
    // 将不含值为括号的结点容器按优先级顺序构成树
    // 优先级1：将单目操作符（+、-和特殊操作符）和右侧值连接
    // 从右往左遍历单目运算符，以防止两个连续的单目运算符中后一个运算符被隐藏的情况
    for (auto iter = p2 - 1; iter != p1 - 1; iter--)
    {
        if ((*iter)->position == ROOT && (*iter)->type == UOPERATOR)
        {
            auto nextOperand = iter;
            while ((*++nextOperand)->position == INNERNODE) {}
            (*iter)->left = *nextOperand; (*iter)->right = nullptr;
            (*nextOperand)->position = INNERNODE;
        }
    }
    // 优先级2：双目运算符^
    for (auto iter = p1; iter != p2; iter++)
    {
        if ((*iter)->position == ROOT && (*iter)->type == BOPERATOR && (*iter)->op == '^')
        {
            auto left = iter;
            auto right = iter;
            while ((*--left)->position == INNERNODE) {}
            while ((*++right)->position == INNERNODE) {}
            (*iter)->left = *left; (*iter)->right = *right;
            (*left)->position = (*right)->position = INNERNODE;
        }
    }
    // 优先级3：双目运算符*、/
    for (auto iter = p1; iter != p2; iter++)
    {
        if ((*iter)->position == ROOT && (*iter)->type == BOPERATOR && ((*iter)->op == '*' || (*iter)->op == '/'))
        {
            auto left = iter;
            auto right = iter;
            while ((*--left)->position == INNERNODE) {}
            while ((*++right)->position == INNERNODE) {}
            (*iter)->left = *left; (*iter)->right = *right;
            (*left)->position = (*right)->position = INNERNODE;
        }
    }
    // 优先级4：双目运算符+、-
    for (auto iter = p1; iter != p2; iter++)
    {
        if ((*iter)->position == ROOT && (*iter)->type == BOPERATOR && ((*iter)->op == '+' || (*iter)->op == '-'))
        {
            auto left = iter;
            auto right = iter;
            while ((*--left)->position == INNERNODE) {}
            while ((*++right)->position == INNERNODE) {}
            (*iter)->left = *left; (*iter)->right = *right;
            (*left)->position = (*right)->position = INNERNODE;
        }
    }
    for (auto iter = p1; iter != p2; iter++)
        if ((*iter)->position == ROOT)
            return *iter;
}

ExpTreeNode ExpProcessor::calculateExpTree(ExpTreeNode tree)
{
    // Post-Order Traversal
    // 递归计算左右子树
    if (tree->left != nullptr)
        tree->left = calculateExpTree(tree->left);
    if (tree->right != nullptr)
        tree->right = calculateExpTree(tree->right);

    if (tree->type == OPERAND)
        return tree;
    else if (tree->type == UOPERATOR)
    {
        auto unaryFunc = unaryFuncMap[tree->op];
        double result = unaryFunc(tree->left->value);
        // 非法运算检查
        if (isinf(result) || isnan(result))
            operationError(tree->op, tree->left->value);
        tree->value = result;
    }
    else if (tree->type == BOPERATOR)
    {
        auto binaryFunc = binaryFuncMap[tree->op];
        double result = binaryFunc(tree->left->value, tree->right->value);
        // 非法运算检查
        if (isinf(result) || isnan(result))
            operationError(tree->op, tree->left->value, tree->right->value);
        tree->value = result;
    }

    return tree;
}

double ExpProcessor::calculate()
{
    preProcess();
    buildExpNodes();
    root = buildExpTree(nodes.begin(), nodes.end());
    root = calculateExpTree(root);
    return root->value;
}


// 工具函数
inline bool ExpProcessor::isSpecialOperator(char c)
{
    return isalpha(c);
}

inline bool ExpProcessor::isUnaryOperator(char c)
{
    return (isSpecialOperator(c) || c == '+' || c == '-');
}

inline bool ExpProcessor::isBinaryOperator(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

inline bool ExpProcessor::isOperator(char c)
{
    return isUnaryOperator(c) || isBinaryOperator(c);
}

ExpTreeNode ExpProcessor::insertNode(NodeType type, NodePosition position, double value,
                                     char op, ExpTreeNode left, ExpTreeNode right)
{
    auto newNode = make_shared<ExpTreeNodeRecord>();
    *newNode = { type, position, value, op, nullptr, nullptr };
    nodes.push_back(newNode);
    return newNode;
}

void ExpProcessor::printNode(ExpTreeNode n)
{
    map<int, string> nodeTypeMap = { {OPERAND, "OPERAND"}, {UOPERATOR, "UOPERATOR"}, {BOPERATOR, "BOPERATOR"},
                                     {BRACKET, "BRACKET"} };
    map<int, string> nodePositionMap = { {ROOT, "ROOT"}, {INNERNODE, "INNERNODE"} };

    cout << string("[") + nodeTypeMap[n->type] +  "," + nodePositionMap[n->position] + ",";
    if (n->type == OPERAND)
        cout << n->value;
    else
        cout << n->op;
    cout << string(",") + ((n->left) ? nodeTypeMap[n->left->type] : "") + ","
         << ((n->right) ? nodeTypeMap[n->right->type] : "") + "]";
}


// 错误检查
void ExpProcessor::expError(std::string msg, int n)
{
    cerr << msg << endl;
    exit(n);
}

void ExpProcessor::operationError(char c, double n)
{
    if ((c == 'c' || c == 'd') && n <= 0)
        // 错误情况：取非正数的对数
        expError("*Illegitimate operation: logarithm to of a negative number*", -1);
}

void ExpProcessor::operationError(char c, double n1, double n2)
{
    if (c == '/' && n2 == 0)
        // 错误情况：除数为0
        expError("*Illegitimate operation: Division by zero*", -1);
    else if (c == '^' && n1 < 0)
        // 错误情况：对负数开方
        expError("*Illegitimate operation: Square root of a negative number*", -1);
}


// 单元测试
void ExpProcessor::testExpression()
{
    for (auto e : expression)
        cout << e << " ";
    cout << "=" << endl;
}

void ExpProcessor::testPreProcess()
{
    preProcess();
    for (auto e : expression)
        cout << e << " ";
}

void ExpProcessor::testNodes()
{
    buildExpNodes();
    for (const auto &n : nodes)
    {
        printNode(n);
        cout << endl;
    }
}

void ExpProcessor::testTree()
{
    // BFS
    queue<ExpTreeNode> q;
    map<ExpTreeNode, int> depthMap;
    for (const auto &n : nodes)
        depthMap[n] = INT_MAX;
    root = buildExpTree(nodes.begin(), nodes.end());
    depthMap[root] = 0;
    q.push(root);
    int lineNum = 0;
    while (!q.empty())
    {
        auto &vNode = q.front();
        q.pop();
        if (lineNum != depthMap[vNode])
        {
            cout << endl;
            lineNum = depthMap[vNode];
        }
        printNode(vNode);
        cout << " ";
        if (vNode->left != nullptr)
        {
            depthMap[vNode->left] = depthMap[vNode] + 1;
            q.push(vNode->left);
        }
        if (vNode->right != nullptr)
        {
            depthMap[vNode->right] = depthMap[vNode] + 1;
            q.push(vNode->right);
        }
    }
    cout << endl;
}

void ExpProcessor::testCalculate()
{
    cout << calculateExpTree(root)->value << endl;
}


