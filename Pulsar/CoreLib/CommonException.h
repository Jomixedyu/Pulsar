/*
* @Moudule     : Exception
* @Date        : 2021/05/15
* @Author      : JomiXedYu
* @Description : This file is part of jxcorlib : https://github.com/JomiXedYu/jxcorlib
* @StdRequired : c++23
*/

#pragma once
#include <stdexcept>
#include <cassert>
#include <stacktrace>
#include "Core.h"

namespace jxcorlib
{
    class ExceptionBase : public std::exception
    {
    protected:
        string message_;
        mutable string whatcache_;
        std::stacktrace stacktrace_;
    public:
        ExceptionBase()
        {
            this->stacktrace_ = std::stacktrace::current(1);
        }
        ExceptionBase(const string& message) : message_(message)
        {
            this->stacktrace_ = std::stacktrace::current(1);
        }
    public:
        virtual const char* name() const { return "ExceptionBase"; }
        virtual string get_message() const {
            return this->message_;
        }
        std::stacktrace get_stacktrace() const {
            return this->stacktrace_;
        }

        virtual const char* what() const override
        {
            string stacktrace = std::to_string(this->stacktrace_);
            string ret;
            ret.reserve(64 + this->message_.size() + stacktrace.size());
            ret.append(this->name());
            ret.append(": ");
            ret.append(this->message_);
            ret.append("; stacktrace: ");
            ret.append(std::move(stacktrace));
            this->whatcache_ = std::move(ret);
            return this->whatcache_.c_str();
        }
        virtual string ToString() const {
            return this->what();
        }
    };

    class RangeOutException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "RangeOutException"; }
        using ExceptionBase::ExceptionBase;
    };

    class ArgumentException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "ArgumentException"; }
        using ExceptionBase::ExceptionBase;
    };

    class ArgumentNullException : public ArgumentException
    {
    public:
        virtual const char* name() const override { return "ArgumentNullException"; }
        using ArgumentException::ArgumentException;
    };

    class NotImplementException : public ArgumentException
    {
    public:
        virtual const char* name() const override { return "NotImplementException"; }
        using ArgumentException::ArgumentException;
    };

    class NullPointerException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "NullPointerException"; }
        using ExceptionBase::ExceptionBase;
    };

    class DivisionByZeroException : public ExceptionBase
    {
    public:
        virtual const char* name() const override { return "DivisionByZeroException"; }
        using ExceptionBase::ExceptionBase;
    };
}
