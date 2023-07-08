#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <string_view>
#include <array>

namespace jxcorlib::ser
{
    class Stream
    {
    public:
        virtual int64_t get_position() const = 0;
        virtual void set_position(int64_t value) = 0;
    public:
        virtual bool IsEOF() = 0;
        virtual int32_t WriteByte(uint8_t value) = 0;
        virtual int32_t WriteBytes(uint8_t* arr, int32_t offset, int32_t count) = 0;
        virtual int32_t ReadByte(uint8_t* out_byte) = 0;
        virtual int32_t ReadBytes(uint8_t* arr, int32_t offset, int32_t count) = 0;
        int32_t ReadWriteBytes(uint8_t* arr, int32_t offset, int32_t count, bool is_write)
        {
            if (is_write)
                return WriteBytes(arr, offset, count);
            else
                return ReadBytes(arr, offset, count);
        }
        virtual void Flush() = 0;
        virtual ~Stream() { }
    };

    enum class FileOpenMode
    {
        Read, Write, ReadWrite, OpenOrCreate, Append
    };
    class FileStream : public Stream
    {
    protected:
        void* file_;
        FileOpenMode mode_;
    public:
        FileOpenMode get_mode() const { return this->mode_; }
        virtual int64_t get_position() const;
        virtual void set_position(int64_t value);
    public:
        FileStream(std::string_view filename, FileOpenMode mode);
        virtual bool IsEOF();
        virtual int32_t WriteByte(uint8_t value) override;
        virtual int32_t WriteBytes(uint8_t* arr, int32_t offset, int32_t count)  override;
        virtual int32_t ReadByte(uint8_t* out_byte) override;
        virtual int32_t ReadBytes(uint8_t* arr, int32_t offset, int32_t count)  override;
        virtual void Flush() override;
        virtual ~FileStream() override;
        FileStream(const FileStream&) = delete;
        FileStream(FileStream&& r) noexcept;
    };

    Stream& ReadWriteStream(Stream& stream, bool is_write, uint8_t& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, int16_t& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, int32_t& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, int64_t& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, float& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, double& out);
    Stream& ReadWriteStream(Stream& stream, bool is_write, std::string& out);


    template<typename T>
    Stream& ReadWriteStream(Stream& stream, bool is_write, std::vector<T>& arr)
    {
        int32_t len = static_cast<int32_t>(arr.size());
        ReadWriteStream(stream, is_write, len);

        if (is_write)
        {
            for(auto& item : arr)
            {
                ReadWriteStream(stream, is_write, item);
            }
        }
        else
        {
            arr.reserve(len);
            for (int i = 0; i < len; i++)
            {
                T item;
                ReadWriteStream(stream, is_write, item);
                arr.push_back(item);
            }
        }

        return stream;
    }
    template<typename T, int N>
    Stream& ReadWriteStream(Stream& stream, bool is_write, T (&arr)[N])
    {
        int len = N;
        ReadWriteStream(stream, is_write, len);
        for (int i = 0; i < N; i++)
        {
            ReadWriteStream(stream, is_write, arr[i]);
        }

        return stream;
    }

    template<typename T, size_t N>
    Stream& ReadWriteStream(Stream& stream, bool is_write, std::array<T, N>& arr)
    {
        int32_t len = static_cast<int32_t>(arr.size());
        ReadWriteStream(stream, is_write, len);
        for (int i = 0; i < len; i++)
        {
            ReadWriteStream(stream, is_write, arr[i]);
        }

        return stream;
    }

}