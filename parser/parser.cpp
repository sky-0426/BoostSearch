/*
 作用：用于实现预处理模块
 核心功能：读取并分析boost文档的.html内容
 解析出每个
*/
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "../common/util.hpp"

using namespace std;

//表示从哪个目录中读取boost文档的html
string g_input_path="../data/input/";

//表示对应预处理模块输出结果放到哪里
string g_output_path="../data/output/raw_input";

//创建一个结构体，表示一个文档（一个html）
struct DocInfo
{
    //文档的标题
    string title;
    //文档的URL
    string url;
    //文档的正文
    string content;
};

//函数的输入参数，使用 const & 来表示
//函数的输出参数，使用 指针 来表示
//函数的输入输出参数，使用 引用 来表示

//枚举出html文件的路径
bool EnumFile(const string& input_path, vector<string>* file_list)
{
    //枚举目录需要使用boost来完成
    
    //把 boost::filesystem 这个命名空间定义一个别名
    namespace fs=boost::filesystem;
    fs::path root_path(input_path);
    if(!fs::exists(root_path))
    {
        cout<<"当前的目录不存在"<<endl;
        return false;
    }
    //递归遍历的时候使用到一个核心的类
    //迭代器使用循环的时候可以自动完成递归
    //C++中的常见做法，把迭代器的默认构造函数生成的迭代器作为一个“哨兵”
    fs::recursive_directory_iterator end_iter;
    for(fs::recursive_directory_iterator iter(root_path);  iter != end_iter; iter++)
    {
        //1.判断当前路径对应的是不是一个普通文件，如果是目录，直接跳过
        if(!fs::is_regular_file(*iter))
        {
            continue;
        }
        //2.当前路径对应的文件是不是一个html文件，如果是其他文件直接跳过
        if(iter->path().extension() != ".html")
        {
            continue;
        }
        //3.把得到的路径加入到最终结果的vector中
        file_list->push_back(iter->path().string());
    }
    return true;
}

//解析title——>找到html中的title标签
bool ParserTitle(const string &html,string* title)
{
    //find()函数
    //如果找到，返回字符串第一次出现的位置
    //如果没有找到就返回string::npos
    size_t begin = html.find("<title>");
    if(begin==string::npos)
    {
        cout<<"标题未找到"<<endl;
        return false;
    }
    size_t end = html.find("</title>");
    if(end==string::npos)
    {
        cout<<"标题未找到"<<endl;
        return false;
    }
    begin += string("<title>").size();
    if(begin >= end)
    {
        cout<<"标题位置不合法"<<endl;
        return false;
    }
    //substr()返回本字符串的一个子串，从begin开始，长end-begin个字符
    *title = html.substr(begin,end-begin);
    return true;
}
//解析URL——根据本地路径获取在线文档的路径
//本地路径: ../data/input/html/about.html/
//在线路径：https://www.boost.org/doc/libs/1_53_0/doc/html/about.html
//把本地路径的后半部分截取出来，再拼装上在线路径的前缀即可
bool ParserUrl(const string &file_path, string* url)
{
    //在线路径的前缀
    string Url_head = "https://www.boost.org/doc/1_53_0/doc/";
    //本地路径的后半部分
    string Url_tail = file_path.substr(g_input_path.size());
    //拼接
    *url = Url_head + Url_tail;
    return true;
}
//解析Content——针对html文档进行去标签
//以< >为标记，记性具体内容的判断，按照逐个字符的方式读取内容
//引入一个bool标志位，表示当前是html标志还是正文
//如果遇到 < 说明接下来的东西就是标签，是标签就把内容忽略掉
//如果遇到 > 说明标签结束，接下来的美容就是正文
bool ParserContent(const string &html, string *content)
{
    bool is_content=true;
    for(auto c : html)
    {
        //当前是正文
        if(is_content)
        {
            if(c == '<')
            {
                //遇到了标签
                is_content = false;
            }
            else
            {
                //这里需要单独处理换行符号 \n ,预期的输出结果是一个行文本文件
                //最终结果row_input中的每一个行对应一个原始的html文档
                //此时就需要把html文档中原来的\n都干掉
                if(c == '\n')
                {
                    c = ' ';
                }
                //当前是普通字符，将结果写入content
                content->push_back(c);
            }
        }
        //当前是标签
        else
        {
            if(c == '>')
            {
                //标签结束
                is_content = true;
            }
            //标签中的内容，直接忽略
        }
    }
    return true;
}

//根据得到的路径，对文件进行解析
bool ParserFile(const string &file_path, DocInfo *doc_info)
{
    //1.先读取文件内容
    //保存读取出来的内容
    string html;
    //构造一个函数Read(),用来读取文件中的内容
    //file_path是一个输入参数，html是一个输出参数
    //Read函数是一个比较底层的函数，每个模块都可能会用到，所以放在common目录中
    bool ret = common::util::Read(file_path, &html);
    if(!ret)
    {
        cout<<"解析文件——读取文件失败!"<<endl;
        return false;
    }
    //2.根据文件内容解析出标题（html中有一个title标签）
    ret = ParserTitle(html,&doc_info->title);
    if(!ret)
    {
        cout<<"解析文件——解析标题失败!"<<endl;
        return false;
    }
    //3.根据文件的路径，构造出对应的在线文档的URL
    ret=ParserUrl(file_path, &doc_info->url);
    if(!ret)
    {
        cout<<"解析文件——解析URL失败!"<<endl;
        return false;
    }
    //4.根据文件内容，进行去标签，作为doc_info中的content字段的内容
    ret=ParserContent(html, &doc_info->content);
    if(!ret)
    {
        cout<<"解析文件——解析正文失败!"<<endl;
        return false;
    }
    return true;
}

//ofstream是没有拷贝构造函数，
//按照参数传参的时候，只能传引用或者指针
//此处还不能死const引用，否则无法执行里面的写文件操作
//每个doc_info就是一行
void WriteOutput(const DocInfo& doc_info, std::ofstream& ofstream)
{
    //ofstream << doc_info.title << doc_info.url << doc_info.content << endl;
    //如果按照上述方式写入则会产生——粘包问题
    //后续读取文件的时候，就分不清楚哪部分是title，哪部分是URL，那部分是conten
    //
    //此时可以使用分隔符来解决这个问题——分割符的选取必须保证分隔符不能在title、URL、content中出现
    //如果出现就会导致格式出现问题，因此使用不可见字符作为分隔符(ASCII码0-7表示的都是)
    ofstream << doc_info.title <<"\3" << doc_info.url <<"\3"<<doc_info.content << endl;
    
}

//预处理过程的核心流程
//1.把input目录中所有的html路径都枚举出来
//2.根据枚举出来的路径依次读取每个文件内容，并进行解析
//3.把解析结果写入到最终的输出文件中
int main()
{ 
    //1.进行枚举路径
    vector<string> file_list;
    bool ret = EnumFile(g_input_path,&file_list);
    if(!ret)
    {
        cout<<"枚举路径失败!"<<endl;
        return 1;
    }
    //2.遍历枚举出来的路径，针对每个文件，单独进行处理
    //解析文件内容
    std::ofstream output_file(g_output_path.c_str());
    if(!output_file.is_open())
    {
        cout<<"打开output文件失败"<<endl;
        return 1;
    }

    for(const auto& file_path:file_list)
    {
        cout << file_path << endl;
        //先创建一个DocInfo对象
        DocInfo doc_info;
        //通过一个函数来负责这里的解析工作
        ret = ParserFile(file_path, &doc_info);
        if(!ret)
        {
            cout<<"解析文件失败!"<<endl;
            continue;
        }
        //把解析出来的结果写入到最终的文件中
        WriteOutput(doc_info, output_file);
        
    }
    return 0;
}

