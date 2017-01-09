/* 
* @Author: anchen
* @Date:   2016-12-14 14:16:48
* @Last Modified by:   anchen
* @Last Modified time: 2017-01-08 18:38:46
*/
#include <iostream>
#include <string>
#include <stdio.h>
#include <boost/regex.hpp>
#include <fstream>  
#include <stdlib.h> 
using namespace std;

bool strSearch(const string &reg, const string &str, string &url)
{
    bool result = false;
    boost::regex e(reg);
    boost::match_results<string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    if(boost::regex_search(str.begin(), str.end(), what, e, flags))
    {
        url = string(what[0].first, what[0].second);
        result = true;
    }
    return result;
}

void getGex(string &content)
{
    ifstream fin("set.ini");    
    while( getline(fin,content));
}

int main(int argc, char const *argv[]) 
{
    string expr;
    getGex(expr);
    string content;
    string out;
    ifstream fin(argv[1]); 
    int index =0;   
    while( getline(fin,content) )
    {
        bool status = strSearch(expr,content,out);
        if (status)
        {
            printf("%d|", index);
            cout <<out << "|";
	    cout << content<<endl;
            index += 1;
        }
        else
        {
            printf("[%d]", index);
            cout <<" " << endl;
            index += 1;
        }
        content ="";
    }
    return 0;
}
