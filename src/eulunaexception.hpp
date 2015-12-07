/*
 * Copyright (c) 2016 Euluna <https://github.com/edubart/euluna>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EULUNAEXCEPTION_HPP
#define EULUNAEXCEPTION_HPP

#include "eulunaprereqs.hpp"
#include "eulunatools.hpp"

// Generic exception thrown
class EulunaException : public std::exception {
public:
    explicit EulunaException(const std::string& error, const std::string& message = std::string()) {
        if(!message.empty())
            m_what = euluna_tools::format("%s: %s", error, message).c_str();
        else
            m_what = error;
    }

    virtual const char* what() const throw() { return m_what.c_str(); }

    std::string name() const throw() { return euluna_tools::demangle_name(typeid(*this).name()); }

protected:
    std::string m_what;
};

class EulunaMemoryError : public EulunaException {
public:
    explicit EulunaMemoryError(const std::string& message = std::string())
        : EulunaException("Lua memory error", message) { }
};

class EulunaSyntaxError : public EulunaException {
public:
    explicit EulunaSyntaxError(const std::string& message = std::string())
        : EulunaException("Lua syntax error", message) { }
};

class EulunaRuntimeError : public EulunaException {
public:
    explicit EulunaRuntimeError(const std::string& message = std::string())
        : EulunaException("Lua error", message) { }
};

class EulunaErrorError : public EulunaException {
public:
    explicit EulunaErrorError(const std::string& message = std::string())
        : EulunaException("Lua error handler error", message) { }
};

#endif // EULUNAEXCEPTION_HPP
