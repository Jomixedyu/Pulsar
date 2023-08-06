#include <CoreLib/File.h>
#include <cassert>
#include <string>
#include <array>
#include <CoreLib/UString.h>
#include <CoreLib.Serialization/DataSerializer.h>
#include <CoreLib/Core.h>

using namespace jxcorlib;
using namespace jxcorlib::ser;
using namespace std;

void TestFile()
{
    string filename = "C:\\a\\b/c.ext";

    assert(PathUtil::GetFilename(filename) == "c.ext");

    assert(PathUtil::GetFilenameWithoutExt(filename) == "c");

    assert(PathUtil::GetFilenameExt(filename) == ".ext");

    vector<string> paths = {
        "a/b/c",
        "a/b/d/c",
        "d/b/d",
        "c",
        "a",
        "a/cc",
        ""
    };

    assert(PathUtil::AInB("a/b/c", "a"));
    assert(!PathUtil::AInB("a/b", "a/b"));
    assert(!PathUtil::AInB("a", "a/b/c"));
    auto path1 = PathUtil::Dir("a", paths);
    auto path2 = PathUtil::Dir("a", paths);
    auto path3 = PathUtil::Dir("", paths);
    auto path5 = PathUtil::Dir("", paths);


    {
        FileStream fs{ "D:/a.txt", FileOpenMode::OpenOrCreate };

        bool is_ser = true;
        //write
        int32_t i32 = 257;
        ReadWriteStream(fs, is_ser, i32);

        string str = "jomi";
        ReadWriteStream(fs, is_ser, str);

        std::vector<int> vec = { 2,3,4,8 };
        ReadWriteStream(fs, is_ser, vec);

        std::array<int, 3> stdarr = { 1 };
        ReadWriteStream(fs, is_ser, stdarr);

        int arr[3] = {1,0,5};
        ReadWriteStream(fs, is_ser, arr);

        fs.set_position(0);
        //read
        int32_t oi32;
        ReadWriteStream(fs, !is_ser, oi32);

        string ostr;
        ReadWriteStream(fs, !is_ser, ostr);

        std::vector<int> ovec;
        ReadWriteStream(fs, !is_ser, ovec);

        std::array<int, 3> ostdaar;
        ReadWriteStream(fs, !is_ser, ostdaar);

        int oarr[3];
        ReadWriteStream(fs, !is_ser, oarr);

        //check
        assert(oi32 == i32);
        assert(str == ostr);
        assert(vec == ovec);
        assert(!memcmp(stdarr.data(), ostdaar.data(), stdarr.size()));
        assert(!memcmp(arr, oarr, 3));
    }
    {
        FileStream fs{ "D:/a.txt", FileOpenMode::Read };

        int32_t i32 = 257;
        ReadWriteStream(fs, false, i32);

        assert(257 == i32);


    }

}