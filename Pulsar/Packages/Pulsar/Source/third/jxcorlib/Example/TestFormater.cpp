
#include <CoreLib/Converter.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <map>

using namespace jxcvt;
using namespace std;


void TestFormater()
{
    vector<int> vec = { 1,2,3,4,5 };
    array<int, 3> arr = { 1,2,3 };
    map<int, string> mp = { {2,"abc"}, {3,"cvb"} };

    cout << to_string(vec) << endl;
    cout << to_string(arr) << endl;
    cout << to_string(string("hello world")) << endl;
    
    //cout << to_string(mp) << endl;
}