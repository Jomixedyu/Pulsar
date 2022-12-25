#include "UString.h"
#include <cmath>
#include <stdexcept>



using jxcorlib::string;
using u16string = std::u16string;

static inline uint16_t _ByteSwapInt16(uint16_t number)
{
    return (number >> 8) | (number << 8);
}

static string _Utf16LEToUtf8(const u16string& u16str)
{
    if (u16str.empty()) { return string(); }
    const char16_t* p = u16str.data();
    u16string::size_type len = u16str.length();

    if (p[0] == 0xFEFF) {
        p += 1;	//带有bom标记，后移
        len -= 1;
    }
    string u8str;
    u8str.reserve(len * 3);

    char16_t u16char;
    for (u16string::size_type i = 0; i < len; ++i)
    {
        u16char = p[i];
        // 1字节表示部分
        if (u16char < 0x0080) {
            // u16char <= 0x007f
            // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
            u8str.push_back((char)(u16char & 0x00FF));	// 取低8bit
            continue;
        }
        // 2 字节能表示部分
        if (u16char >= 0x0080 && u16char <= 0x07FF) {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 6) & 0x1F) | 0xC0));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
        // 代理项对部分(4字节表示)
        if (u16char >= 0xD800 && u16char <= 0xDBFF) {
            // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
            uint32_t highSur = u16char;
            uint32_t lowSur = p[++i];
            // 从代理项对到UNICODE代码点转换
            // 1、从高代理项减去0xD800，获取有效10bit
            // 2、从低代理项减去0xDC00，获取有效10bit
            // 3、加上0x10000，获取UNICODE代码点值
            uint32_t codePoint = highSur - 0xD800;
            codePoint <<= 10;
            codePoint |= lowSur - 0xDC00;
            codePoint += 0x10000;
            // 转为4字节UTF8编码表示
            u8str.push_back((char)((codePoint >> 18) | 0xF0));
            u8str.push_back((char)(((codePoint >> 12) & 0x3F) | 0x80));
            u8str.push_back((char)(((codePoint >> 06) & 0x3F) | 0x80));
            u8str.push_back((char)((codePoint & 0x3F) | 0x80));
            continue;
        }
        // 3 字节表示部分
        {
            // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 12) & 0x0F) | 0xE0));
            u8str.push_back((char)(((u16char >> 6) & 0x3F) | 0x80));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
    }

    return u8str;
}

static string _Utf16BEToUtf8(const u16string& u16str)
{
    if (u16str.empty()) { return string(); }
    const char16_t* p = u16str.data();
    u16string::size_type len = u16str.length();
    if (p[0] == 0xFEFF) {
        p += 1;	//带有bom标记，后移
        len -= 1;
    }

    // 开始转换
    string u8str;
    u8str.reserve(len * 2);
    char16_t u16char;	//u16le 低字节存低位，高字节存高位
    for (u16string::size_type i = 0; i < len; ++i) {
        // 这里假设是在小端序下(大端序不适用)
        u16char = p[i];
        // 将大端序转为小端序
        u16char = _ByteSwapInt16(u16char);

        // 1字节表示部分
        if (u16char < 0x0080) {
            // u16char <= 0x007f
            // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
            u8str.push_back((char)(u16char & 0x00FF));
            continue;
        }
        // 2 字节能表示部分
        if (u16char >= 0x0080 && u16char <= 0x07FF) {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 6) & 0x1F) | 0xC0));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
        // 代理项对部分(4字节表示)
        if (u16char >= 0xD800 && u16char <= 0xDBFF) {
            // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
            uint32_t highSur = u16char;
            uint32_t lowSur = _ByteSwapInt16(p[++i]);
            // 从代理项对到UNICODE代码点转换
            // 1、从高代理项减去0xD800，获取有效10bit
            // 2、从低代理项减去0xDC00，获取有效10bit
            // 3、加上0x10000，获取UNICODE代码点值
            uint32_t codePoint = highSur - 0xD800;
            codePoint <<= 10;
            codePoint |= lowSur - 0xDC00;
            codePoint += 0x10000;
            // 转为4字节UTF8编码表示
            u8str.push_back((char)((codePoint >> 18) | 0xF0));
            u8str.push_back((char)(((codePoint >> 12) & 0x3F) | 0x80));
            u8str.push_back((char)(((codePoint >> 06) & 0x3F) | 0x80));
            u8str.push_back((char)((codePoint & 0x3F) | 0x80));
            continue;
        }
        // 3 字节表示部分
        {
            // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 12) & 0x0F) | 0xE0));
            u8str.push_back((char)(((u16char >> 6) & 0x3F) | 0x80));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
    }
    return u8str;
}

static string _Utf16ToUtf8(const u16string& u16str)
{
    if (jxcorlib::StringUtil::IsLittleEndian()) {
        return _Utf16LEToUtf8(u16str);
    }
    else {
        return _Utf16BEToUtf8(u16str);
    }
}

static u16string _Utf8ToUtf16LE(const string& u8str, bool addbom, bool* ok)
{
    u16string u16str;
    u16str.reserve(u8str.size());
    if (addbom) {
        u16str.push_back(0xFEFF);	//bom (字节表示为 FF FE)
    }
    string::size_type len = u8str.length();

    const unsigned char* p = (unsigned char*)(u8str.data());
    // 判断是否具有BOM(判断长度小于3字节的情况)
    if (len > 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        p += 3;
        len -= 3;
    }

    bool is_ok = true;
    // 开始转换
    for (string::size_type i = 0; i < len; ++i) {
        uint32_t ch = p[i];	// 取出UTF8序列首字节
        if ((ch & 0x80) == 0) {
            // 最高位为0，只有1字节表示UNICODE代码点
            u16str.push_back((char16_t)ch);
            continue;
        }
        switch (ch & 0xF0)
        {
        case 0xF0: // 4 字节字符, 0x10000 到 0x10FFFF
        {
            uint32_t c2 = p[++i];
            uint32_t c3 = p[++i];
            uint32_t c4 = p[++i];
            // 计算UNICODE代码点值(第一个字节取低3bit，其余取6bit)
            uint32_t codePoint = ((ch & 0x07U) << 18) | ((c2 & 0x3FU) << 12) | ((c3 & 0x3FU) << 6) | (c4 & 0x3FU);
            if (codePoint >= 0x10000)
            {
                // 在UTF-16中 U+10000 到 U+10FFFF 用两个16bit单元表示, 代理项对.
                // 1、将代码点减去0x10000(得到长度为20bit的值)
                // 2、high 代理项 是将那20bit中的高10bit加上0xD800(110110 00 00000000)
                // 3、low  代理项 是将那20bit中的低10bit加上0xDC00(110111 00 00000000)
                codePoint -= 0x10000;
                u16str.push_back((char16_t)((codePoint >> 10) | 0xD800U));
                u16str.push_back((char16_t)((codePoint & 0x03FFU) | 0xDC00U));
            }
            else
            {
                // 在UTF-16中 U+0000 到 U+D7FF 以及 U+E000 到 U+FFFF 与Unicode代码点值相同.
                // U+D800 到 U+DFFF 是无效字符, 为了简单起见，这里假设它不存在(如果有则不编码)
                u16str.push_back((char16_t)codePoint);
            }
        }
        break;
        case 0xE0: // 3 字节字符, 0x800 到 0xFFFF
        {
            uint32_t c2 = p[++i];
            uint32_t c3 = p[++i];
            // 计算UNICODE代码点值(第一个字节取低4bit，其余取6bit)
            uint32_t codePoint = ((ch & 0x0FU) << 12) | ((c2 & 0x3FU) << 6) | (c3 & 0x3FU);
            u16str.push_back((char16_t)codePoint);
        }
        break;
        case 0xD0: // 2 字节字符, 0x80 到 0x7FF
        case 0xC0:
        {
            uint32_t c2 = p[++i];
            // 计算UNICODE代码点值(第一个字节取低5bit，其余取6bit)
            uint32_t codePoint = ((ch & 0x1FU) << 12) | ((c2 & 0x3FU) << 6);
            u16str.push_back((char16_t)codePoint);
        }
        break;
        default:	// 单字节部分(前面已经处理，所以不应该进来)
            is_ok = false;
            break;
        }
    }
    if (ok != NULL) { *ok = is_ok; }

    return u16str;
}

static u16string _Utf8ToUtf16BE(const string& u8str, bool addbom, bool* ok)
{
    u16string u16str = _Utf8ToUtf16LE(u8str, addbom, ok);
    // reverse
    for (size_t i = 0; i < u16str.size(); ++i) {
        u16str[i] = _ByteSwapInt16(u16str[i]);
    }
    return u16str;
}

static u16string _Utf8ToUtf16(const string& u8str)
{
    if (jxcorlib::StringUtil::IsLittleEndian()) {
        return _Utf8ToUtf16LE(u8str, false, nullptr);
    }
    else {
        return _Utf8ToUtf16BE(u8str, false, nullptr);
    }
}

static string UTF8ToANSI(const char* src_str);
static string ANSIToUTF8(const char* src_str);

namespace jxcorlib
{

    inline static bool _StringEqualsChar(const u8char& c, const string& str)
    {
        if (str.size() > 6 || str.empty()) {
            return false;
        }
        return u8char::Charcmp(c.value, str.c_str());
    }
    bool operator==(const u8char& left, const string& right)
    {
        return _StringEqualsChar(left, right);
    }

    bool operator==(const string& left, const u8char& right)
    {
        return _StringEqualsChar(right, left);
    }

    bool operator!=(const u8char& left, const string& right)
    {
        return _StringEqualsChar(left, right);
    }

    bool operator!=(const string& left, const u8char& right)
    {
        return _StringEqualsChar(right, left);
    }

    string operator+(const u8char& left, const string& right)
    {
        return left.value + right;
    }

    string operator+(const string& left, const u8char& right)
    {
        return left + right.value;
    }

    StringIndexMapping::StringIndexMapping(string_view str, size_t block_size) : block_size_(block_size)
    {

        size_t len = str.length();
        if (len <= block_size) {
            this->mapping.push_back(0);
            return;
        }

        size_t offset = 0;
        size_t index = 0;

        while (offset < len)
        {
            if ((index % block_size) == 0 || index == 0) {
                this->mapping.push_back(static_cast<int>(offset));
            }

            offset += StringUtil::U8Length(str, offset);
            index++;
        }
    }

    size_t StringIndexMapping::get_block_size() const
    {
        return this->block_size_;
    }

    size_t StringIndexMapping::get_block_count() const
    {
        return this->mapping.size();
    }

    size_t StringIndexMapping::GetOffset(const size_t& pos) const
    {
        return this->mapping[this->GetBlockPos(pos)];
    }
    size_t StringIndexMapping::GetBlockPos(const size_t& pos) const
    {
        if (pos < 0) {
            throw std::invalid_argument("the arg must be greater than zero.");
        }
        return pos / this->block_size_;
    }

    bool StringUtil::IsLittleEndian() noexcept
    {
        static int i = 1;
        static bool b = *(char*)&i;
        return b;
    }

    size_t StringUtil::U8Length(string_view str, size_t byte_pos)
    {
        unsigned char c = static_cast<unsigned char>(str[byte_pos]);
        if ((c & 0b10000000) == 0b00000000)  return 1;
        else if ((c & 0b11100000) == 0b11000000) return 2;
        else if ((c & 0b11110000) == 0b11100000) return 3;
        else if ((c & 0b11111000) == 0b11110000) return 4;
        else if ((c & 0b11111100) == 0b11111000) return 5;
        else if ((c & 0b11111110) == 0b11111100) return 6;
        throw std::invalid_argument("string is invalid");
    }

    string StringUtil::Replace(string_view src, string_view oldstr, string_view newstr)
    {
        size_t src_len = src.length();
        size_t oldstr_len = oldstr.length();
        size_t newstr_len = newstr.length();

        if (src_len == 0) { return string{}; }
        if (oldstr_len == 0 || newstr_len == 0) { return string{ src }; }

        string nstr;
        if (newstr_len <= oldstr_len)
        {
            nstr.reserve(src_len + 1);
        }
        else
        {
            //最多替换次数
            size_t count = src_len / oldstr_len;
            //长度比例，最大2
            double m = std::min(2.0, (double)newstr_len / (double)oldstr_len);
            //可替换字符串放大后的长度
            size_t base = (size_t)((double)(count * oldstr_len) * m) + 1;
            //非可替换字符串长度
            size_t app = src_len - count * oldstr_len + 1;

            size_t len = base + app;

            nstr.reserve(len);
        }
        nstr = src;
        size_t pos = 0;
        while ((pos <= nstr.size()) && ((pos = nstr.find(oldstr, pos)) != nstr.npos))
        {
            nstr.replace(pos, oldstr_len, newstr);
            pos += newstr_len;
        }
        return nstr;
    }

    inline static u8char _StringUtil_At(
        const string_view& src, const size_t& pos,
        const size_t& start_offset = 0, const size_t& start_char_count = 0)
    {
        size_t size = src.length();
        size_t offset = start_offset;
        for (size_t i = start_char_count; i < pos; i++)
        {
            offset += StringUtil::U8Length(src, offset);
        }
        u8char ch;
        size_t len = StringUtil::U8Length(src, offset);
        for (size_t i = 0; i < len; i++)
        {
            ch.value[i] = src[offset + i];
        }
        return ch;
    }
    size_t StringUtil::PosAt(const string_view& src, const size_t& bytepos, u8char* out_char)
    {
        size_t len = StringUtil::U8Length(src, bytepos);
        for (size_t i = 0; i < len; i++)
        {
            out_char->value[i] = src[bytepos + i];
        }
        return len;
    }
    u8char StringUtil::CharAt(const string_view& src, const size_t& charpos)
    {
        return _StringUtil_At(src, charpos);
    }

    u8char StringUtil::CharAt(const string_view& src, const size_t& charpos, const StringIndexMapping& mapping)
    {
        size_t block_size = mapping.get_block_size();
        if (block_size == 0) {
            return _StringUtil_At(src, charpos);
        }
        size_t block_pos = mapping.GetOffset(charpos);
        //位置前面的已有块*块大小=开始字符数
        size_t start_char_count = mapping.GetBlockPos(charpos) * mapping.get_block_size();
        return _StringUtil_At(src, charpos, mapping.GetOffset(charpos), start_char_count);
    }

    inline static size_t _StringUtil_U8Length_Internal(const string_view& src, const size_t& start = 0)
    {
        size_t size = src.length();
        size_t index = start;
        size_t len = 0;

        while (index < size) {
            index += StringUtil::U8Length(src, index);
            len++;
        }
        return len;
    }

    size_t StringUtil::Length(const string_view& src)
    {
        return _StringUtil_U8Length_Internal(src);
    }

    size_t StringUtil::Length(const string_view& src, const StringIndexMapping& mapping)
    {
        return (mapping.get_block_count() - 1) * mapping.get_block_size()
            + _StringUtil_U8Length_Internal(src, mapping.mapping[mapping.get_block_count() - 1]);
    }

    void StringUtil::ForEach(string_view str, const std::function<bool(u8char ch, size_t char_pos, size_t byte_pos)>& it)
    {
        size_t size = str.length();
        size_t num = 0;
        for (size_t offset = 0; offset < size; )
        {
            u8char ch;
            auto char_len = StringUtil::PosAt(str, offset, &ch);
            if (!it.operator()(ch, num, offset)) { break; }
            offset += char_len;
            ++num;
        }
    }

    std::vector<uint8_t> StringUtil::GetBytes(const string_view& str)
    {
        std::vector<uint8_t> c;
        c.reserve(str.size());
        c.assign(str.begin(), str.end());
        return c;
    }

    u16string StringUtil::Utf8ToUtf16(const string& str)
    {
        return _Utf8ToUtf16(str);
    }

    string StringUtil::Utf16ToUtf8(const std::u16string& str)
    {
        return _Utf16ToUtf8(str);
    }

    string StringUtil::Utf8ToAnsi(string_view str)
    {
        return UTF8ToANSI(str.data());
    }

    string StringUtil::AnsiToUtf8(string_view str)
    {
        return ANSIToUTF8(str.data());
    }

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    string StringUtil::EncodeBase64(uint8_t* buf, int32_t len)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];  // store 3 byte of bytes_to_encode
        unsigned char char_array_4[4];  // store encoded character to 4 bytes

        while (len--) {
            char_array_3[i++] = *(buf++);  // get three bytes (24 bits)
            if (i == 3) {
                // eg. we have 3 bytes as ( 0100 1101, 0110 0001, 0110 1110) --> (010011, 010110, 000101, 101110)
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2; // get first 6 bits of first byte,
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4); // get last 2 bits of first byte and first 4 bit of second byte
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6); // get last 4 bits of second byte and first 2 bits of third byte
                char_array_4[3] = char_array_3[2] & 0x3f; // get last 6 bits of third byte

                for (i = 0; (i < 4); i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';

        }

        return ret;
    }

    std::vector<uint8_t> StringUtil::DecodeBase64(const string& data)
    {
        size_t in_len = data.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<uint8_t> ret;
        ret.reserve(data.size() + 3);

        while (in_len-- && (data[in_] != '=') && is_base64(data[in_])) {
            char_array_4[i++] = data[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]) & 0xff;

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i) {
            for (j = 0; j < i; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]) & 0xff;

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
        }

        return ret;
    }

    string StringUtil::StringCast(const std::u8string& str)
    {
        const size_t size = str.size();
        string ostr(size, 0);
        if constexpr (sizeof(char) == sizeof(char8_t))
        {
            memcpy(const_cast<char*>(ostr.c_str()), str.c_str(), size);
        }
        else
        {
            for (size_t i = 0; i < size; i++)
            {
                ostr.push_back((char)str.at(i));
            }
        }
        return ostr;
    }

    std::vector<string> StringUtil::Split(string_view str, u8char c)
    {
        std::vector<string> vec;

        auto u8len = Length(str);

        string add;
        StringIndexMapping mapping{ str, 9 };

        for (size_t i = 0; i < u8len; i++)
        {
            if (CharAt(str, i, mapping) != c)
            {
                add.append(CharAt(str, i).value);
            }
            else
            {
                vec.push_back(add);
                add.clear();
            }
        }
        vec.push_back(add);
        add.clear();
        return vec;
    }

    string StringUtil::Substring(string_view str, size_t offset_char_pos, size_t char_count)
    {

        int32_t start_bpos = 0, end_bpos = 0;

        ForEach(str, [offset_char_pos, char_count, &start_bpos](u8char ch, size_t cpos, size_t bpos) -> bool
            {
                if (cpos == offset_char_pos)
                {
                    start_bpos = bpos;
                    return false;
                }
                return true;
            });

        int32_t ch_count = 0;
        end_bpos = str.length();

        for (size_t offset = start_bpos; offset < str.length(); )
        {
            size_t u8len = U8Length(str, offset);
            offset += u8len;
            ++ch_count;

            if (ch_count == char_count)
            {
                end_bpos = offset;
                break;
            }
        }

        return string(str.substr(start_bpos, end_bpos - start_bpos));

    }
    static constexpr inline bool is_trimable(char c)
    {
        return c == ' ' || c == '\r' || c == '\n';
    }

    string& StringUtil::TrimSelf(string& str)
    {
        if (str.empty()) return str;
        for (int32_t i = 0; i < str.length(); i++)
        {
            if (is_trimable(str[i]))
                str[i] = ' ';
            else
                break;
        }

        for (int32_t i = str.length() - 1; i > -1; i--)
        {
            if (is_trimable(str[i]))
                str[i] = ' ';
            else
                break;
        }

        str.erase(0, str.find_first_not_of(" "));
        str.erase(str.find_last_not_of(" ") + 1);
        return str;
    }

    string StringUtil::Trim(string_view str)
    {
        string ret(str);
        TrimSelf(ret);
        return ret;
    }


}

#include <Windows.h>
static string UTF8ToANSI(const char* src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszANSI = new wchar_t[len + 1];
    memset(wszANSI, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszANSI, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, NULL, 0, NULL, NULL);
    char* szANSI = new char[len + 1];
    memset(szANSI, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszANSI, -1, szANSI, len, NULL, NULL);
    string strTemp(szANSI);
    if (wszANSI) delete[] wszANSI;
    if (szANSI) delete[] szANSI;
    return strTemp;
}

static string ANSIToUTF8(const char* src_str)
{
    string outUtf8 = "";
    int n = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    WCHAR* str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char* str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    outUtf8 = str2;
    delete[]str1;
    str1 = NULL;
    delete[]str2;
    str2 = NULL;
    return outUtf8;
}