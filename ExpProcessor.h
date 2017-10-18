//
// Created by 王宇超 on 17/10/11.
//

#ifndef EXPRESSION_EXPPROCESSOR_H
#define EXPRESSION_EXPPROCESSOR_H

#include <cmath>
#include <string>
#include <iostream>
#include <map>
#include <stack>
#include <cctype>
#include <vector>
#include <algorithm>
#include <memory>
#include <queue>
#include <functional>

struct ExpTreeNodeRecord;
typedef std::shared_ptr<ExpTreeNodeRecord> ExpTreeNode;
enum NodeType { OPERAND, UOPERATOR, BOPERATOR, BRACKET };
enum NodePosition { ROOT, INNERNODE, SUBTREE };

class ExpProcessor 
{
public:
    static std::map<std::string, char> operatorMap;
    static std::map<char, std::function<double(double)>> unaryFuncMap;
    static std::map<char, std::function<double(double, double)>> binaryFuncMap;

    ExpProcessor() = default;
    explicit ExpProcessor(std::string s) : expression(s), root(nullptr) { }
    // 计算表达式树；返回double类型的计算结果
    double calculate();

    // 单元测试
    void testExpression();
    void testPreProcess();
    void testNodes();
    void testTree();
    void testCalculate();

private:
    // 预处理：将expression中的特殊操作符字符串转换为单个字符；检查特殊操作符后面是否有括号；检查左右括号顺序是否合法；检查括号是否对称
    //        检查是否有两个及以上的运算符同时出现（除+、-等单目运算符可以是第二个运算符）；检查表达式首部是否有运算符（除单目运算符）；
    //        检查是否有不合规范的小数点；
    //        返回该ExpProcessor对象
    void preProcess();
    // 用预处理过的expression构造表达式树的结点
    void buildExpNodes();
    // 构造表达式树；递归函数；返回该ExpProcessor对象
    ExpTreeNode buildExpTree(std::vector<ExpTreeNode>::iterator, std::vector<ExpTreeNode>::iterator);
    // 计算表达式树；返回根结点，其中保存着表达式树的值
    // （其实可以在建立树的过程中同时计算每个结点的值，这里为了算法的清晰，将构造和计算分为两个过程）
    ExpTreeNode calculateExpTree(ExpTreeNode);

    // 工具函数
    // 判断是否为特殊操作符（sin、cos、log、ln等）
    inline bool isSpecialOperator(char);
    // 判断是否为单目操作符
    inline bool isUnaryOperator(char);
    // 判断是否为双目操作符
    inline bool isBinaryOperator(char);
    // 判断是否为操作符
    inline bool isOperator(char);
    // 构造并向nodes中插入一个ExpTreeNode；参数为构造ExpTreeNode所需的参数；返回插入的ExpTreeNode
    ExpTreeNode insertNode(NodeType, NodePosition, double, char, ExpTreeNode, ExpTreeNode);
    // 打印结点信息
    void printNode(ExpTreeNode);
    // 输出表达式的错误信息并结束程序
    void expError(std::string, int);
    // 输出非法运算的错误信息并结束程序
    void operationError(char, double);
    void operationError(char, double, double);

    std::string expression;
    std::vector<ExpTreeNode> nodes;
    ExpTreeNode root;
};

struct ExpTreeNodeRecord
{
    NodeType type;
    NodePosition position;
    double value;
    char op;
    ExpTreeNode left;
    ExpTreeNode right;
};


#endif //EXPRESSION_EXPPROCESSOR_H
