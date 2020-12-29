//保证头文件只包含一次
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace common
{
    class util
    {
    public:
        //负责从指定的路径中，读取出文件的整体内容，读到output这个string中
        static bool Read(const string &input_path, string *output)
        {
            std::ifstream file(input_path.c_str());
            if(!file.is_open())
            {
                return false;
            }
            //读取整个文件内容，只要按行读取就行，把读到的每行结果追加到output中
            //getline读取文件中的一行
            //如果读取成功，就把内容放到line中，并返回true
            //如果读取失败(或者读到文件末尾)，就返回false
            string line;
            while(std::getline(file,line))
            {
                *output+=(line+"\n"); 
            }
            file.close();
            return true;
        }
        //基于boost中的字符串切分，封装一下
        //delimiter 表示分隔符，按照什么进行分割
        static void Split(const string& input, const string& delimiter, vector<string>* output)
        {
            //理解一下token_compress_off
            //eg: aaa\3bbb\3\3ccc
            //有两种切分结果
            //1.aaa bbb ccc token_compress_on 压缩切分结果
            //2.aaa bbb "" ccc token_compress_off 不会压缩切分结果
            boost::split(*output,input,boost::is_any_of(delimiter),boost::token_compress_off);
        }
    private:
    };
}

