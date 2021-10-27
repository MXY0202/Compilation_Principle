#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
using namespace std;

string path = "D:\\test.txt";
vector<string> keys = {"auto", "break","case","char", "const","continue","default","do",
                       "else","enum","extern","float","for","goto", "if","int","long",
                       "return","short","signed","sizeof","static","struct","switch","typedef",
                       "union", "unsigned", "void","volatile", "while","register","double"};  //内置关键字

class LexicalAnalyzer
{
private:
    vector<char> token;  //存放当前正在识别的单词字符串
    int forward; //向前指针
    string buffer; //字符缓冲区
    char ch; //存放当前分析字符    

    stack<char> boundStack; //判断括号有没有识别错误    
    ifstream file;

    int errorId = 0;   //错误数量
    int keyNum = 0;   //关键词数量
    int numberNum = 0; //数字数量
    int operatorNum = 0; //操作符数量
    int idNum = 0;  //标识符数量
    int delimiterNum = 0; //分界符数量
    int stringNum = 0; //字符串数量
    int lineNum = 0; //程序行数
    int charNum = 0; //字符个数


public:
    LexicalAnalyzer();
    ~LexicalAnalyzer();

    void analyze(); //总体分析过程
  
    void idAnalyze(); //标识符分析模块
    void digitAnalyze(); //数字分析模块
    void noteAnalyze(); //注释分析模块
    void opAnalyze(); //操作符分析模块
    void delimiterAnalyze(); //分界符分析模块

    void leapBlank(); //跳过空格
    bool isLetter(); //判断ch是否为字母
    bool isDigit(); //判断ch是否为数字
    bool isOperator(); //判断ch是否是操作符
    bool isKey(); //判断token是否是关键词
    void retract(); //将向前指针forward回退一个字符
    bool isNote(); //判断当前ch及下一个字母是否满足注释
    bool isDelimiter(); //判断ch是否是分界符
    void digitAdd(); //如果ch是数字，向token中添加

    void printError(string s); //打印词法错误
    void outPut(string mark, string attribute, bool flag); //以<记号，属性>的二元组形式输出
    void printInfo(); //打印各种记号的数量及程序行数等
};



int main()
{
    LexicalAnalyzer analyzer;

    analyzer.analyze();  //进行分析过程
    analyzer.printInfo(); //打印信息

    system("pause");
    return 0;
}

LexicalAnalyzer::LexicalAnalyzer()
{

}

LexicalAnalyzer::~LexicalAnalyzer()
{

}

void LexicalAnalyzer::analyze()
{
    file.open(path);

    while(getline(file, buffer))
    {
        lineNum++;
        forward = 0;
        while(forward < buffer.size())
        {
            ch = buffer[forward++];
            leapBlank();

            if(ch == '#') //跳过带有头文件的行
            {
                forward = buffer.size();
            }
            else if(isLetter() || ch == '_') //variable
            {
                idAnalyze();
            }
            else if(isDigit()) //number
            {
                digitAnalyze();
            }
            else if(isNote()) //note
            {
                if(buffer[forward] == '*') //处理"/*"类型注释
                    noteAnalyze();
                else            //  "//"类型注释
                    forward = buffer.size();//停止分析本行
            }
            else if(isOperator()) //op
            {
                token.push_back(ch);
                opAnalyze();
            }
            else if(isDelimiter()) //分界符
            {
                token.push_back(ch);
                delimiterAnalyze();
            }
            else //error
            {
                string error = "An unkonwn character:";
                error.push_back(ch);
                printError(error);
            }
            //清空token
            charNum += token.size();
            token.erase(token.begin(), token.end());
            
        }    

        buffer = "";
    }

    file.close();
}

bool LexicalAnalyzer::isDigit()
{
    if(ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}

bool LexicalAnalyzer::isLetter()
{
    if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        return true;
    else
        return false;
}

bool LexicalAnalyzer::isNote()
{
    if(ch == '/' && (buffer[forward] == '*' || buffer[forward] == '/'))
        return true;
    else
        return false;    
}

bool LexicalAnalyzer::isOperator()
{
    if(ch == '>' || ch == '<' || ch == '=' || ch == '+' ||
       ch == '-' || ch == '*' || ch == '/')
        return true;
    else
        return false;
}

bool LexicalAnalyzer::isDelimiter()
{
    if(ch == ',' || ch == ';' || ch == '(' || ch == ')' || 
    ch == '"' || ch == '{' || ch == '}')
        return true;
    else
        return false;
}

bool LexicalAnalyzer::isKey()
{
    string s;
    for(int i = 0; i < token.size(); i++)  //将token中字符复制到s中
        s.push_back(token[i]);
     auto iter = find(keys.begin(), keys.end(), s);
     if(iter == keys.end())
         return  false;
    return true;
}

void LexicalAnalyzer::leapBlank()
{
    while(ch == ' ' || ch == '\t')
        ch = buffer[forward++];
}

void LexicalAnalyzer::retract()
{
    forward--;
}

void LexicalAnalyzer::noteAnalyze()
{
    forward++;
    if(forward == buffer.size())  //如果已经到了行尾，跳过该行
    {
        buffer = "";
        getline(file, buffer);
        lineNum++;
        forward = 0;
    }
    while(!(buffer[forward] == '*' && buffer[forward + 1] == '/')) //直到找到*/
    {
        forward++;
        if(forward == buffer.size()) //到达行尾跳过该行
        {
            buffer = "";
            getline(file, buffer);
            lineNum++;
            forward = 0;
        }   
    }
    forward += 2;  // 找到注释结尾后 跳过 */两个字符
}

void LexicalAnalyzer::opAnalyze()
{
    // > >= < <= = == + - * /
    if(ch == '>' && buffer[forward + 1] == '=') //>=
    {
        ch = buffer[forward++];
        token.push_back(ch);
        outPut("", "GE", false);
    }
    else if(ch == '>')
    {
        outPut("", "GT", false);
    }
    else if(ch == '<' && buffer[forward + 1] == '=') // <=
    {
        ch = buffer[forward++];
        token.push_back(ch);
        outPut("", "LE", false);
    }
    else if(ch == '<')
    {
        outPut("", "LT", false);
    }
    else if(ch == '=' && buffer[forward + 1] == '=') // ==
    {
        ch = buffer[forward++];
        token.push_back(ch);
        outPut("", "EQ", false);
    }
    else if(ch == '=')
    {

        outPut("", "-", false);
    }
    else
    {
        outPut("", "-", false);
    }
    operatorNum++;
}

void LexicalAnalyzer::delimiterAnalyze()
{
    //, ; (  ) " { }
    delimiterNum++;
    outPut("", "-", false);

    if(ch == '"')
    {
        //先输出第一个"然后将字符串加入token，然后再判断另一个"，如果没有报错，有输出
        ch = buffer[forward++];
        token.erase(token.begin(), token.end());
        token.push_back(ch);
        while(forward < buffer.size() && ch != '"')
        {
            ch = buffer[forward++];
            token.push_back(ch);
        }
        if(ch == '"')
        {
            if(token[0] != '"') //存在一个字符串
            {
                token.pop_back();
                stringNum++;
                outPut("string", "", true);
            }
            token.erase(token.begin(), token.end());
            token.push_back(ch);
            delimiterNum++; 
            outPut("", "-", false);
        }
        else//没有第二个"报错
        {
            char c = '"';
            string s = " character"; 
            string error = "missing terminating " + c + s;
            printError(error);
        }
    }
    else if(ch == '(')
    {
        boundStack.push(ch);  //将左括号入栈
    }
    else if (ch == ')')
    {
        if(boundStack.empty())
        {
            printError("expected '(' before ')'");
        }
        else if(boundStack.top() == '(')//如果有一对括号匹配，将上一个左括号出栈
            boundStack.pop();
    }
}

void LexicalAnalyzer::idAnalyze()
{
    while (isLetter() || isDigit() || ch == '_')  //标识符
    {
        token.push_back(ch);
        ch = buffer[forward++];
    }
    retract();
    if(isKey()) //查看是否是关键字
    {
        keyNum++;
        outPut("key", "", true);
    }
    else
    {
        idNum++;
        outPut("id", "", true);
    }
}
void LexicalAnalyzer::digitAnalyze()
{
    //数字类型  123  123.123 1e2   1.1e2
    digitAdd(); //如果是数字，都加入token
    if(ch == '.') //遇到小数点，将之后的数字都加入token
    {
        token.push_back(ch);
        ch = buffer[forward++];
        if(isDigit())
            digitAdd();
    }
    else if(ch == 'E' || ch == 'e') //进入指数分支
    {
        token.push_back(ch);
        ch = buffer[forward++];
    }
    if(isDigit())
        digitAdd();
    else
        retract();
    numberNum++;
    outPut("number", "", true);
}

void LexicalAnalyzer::digitAdd()
{
    while(isDigit())
    {
        token.push_back(ch);
        ch = buffer[forward++];
    } 
}

void LexicalAnalyzer::printError(string error)
{
    errorId++;
    printf("Line:%d errorID:%d errorReason: ", lineNum,errorId);
    cout << error << endl;
}

void LexicalAnalyzer::outPut(string mark, string attribute, bool flag)
{
    if(flag)
    {
        for(int i = 0; i < token.size(); i++)
            attribute.push_back(token[i]);
    }
    else
    {
        for(int i = 0; i < token.size(); i++)
            mark.push_back(token[i]);
    }
    cout << "<" << " "<< mark << ", " << attribute << " " << ">" << endl;
}

void LexicalAnalyzer::printInfo()
{
    printf("line num: %d\n", lineNum);
    printf("char num %d\n", charNum);
    printf("identifier num: %d\n", idNum);
    printf("key num: %d\n", keyNum);
    printf("operator num: %d\n", operatorNum);
    printf("delimiter num: %d\n", delimiterNum);
    printf("string num: %d\n", stringNum);
}