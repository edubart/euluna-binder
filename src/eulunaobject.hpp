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

#ifndef EULUNAOBJECT_HPP
#define EULUNAOBJECT_HPP
#include "eulunainterface.hpp"

class EulunaObject {
public:
    void lua_getMetatable(EulunaInterface* lua, const std::string& className) {
        lua->getRegistryField(className + "_mt");
        return 1;
    }

    void lua_getFieldsTable(EulunaInterface *lua) {
        // create fields table on the fly
        if(m_fieldsTableRef == -1) {
            lua->newTable(); // create fields table
            m_fieldsTableRef = lua->ref(); // save a reference for it
        }
        lua->getRef(m_fieldsTableRef);
    }

    void lua_releaseFieldsTable(EulunaInterface *lua) {
        if(m_fieldsTableRef != -1) {
            lua->unref(m_fieldsTableRef);
            m_fieldsTableRef = -1;
        }
    }

    void lua_rawGet(EulunaInterface *lua) {
        if(m_fieldsTableRef != -1) {
            lua->getRef(m_fieldsTableRef); // push the obj's fields table
            lua->insert(-2); // move the key to the top
            lua->rawGet(); // push the field value
            lua->remove(-2); // remove the table
        } else {
            lua->pop();
            lua->pushNil();
        }
    }

    void lua_rawSet(EulunaInterface *lua) {
        lua_getFieldsTable(lua);
        lua->insert(-3); // move the fields table to the top
        lua->rawSet();
        lua->pop();
    }

    void lua_unref(EulunaInterface *lua) {
        m_refs--;
    }

    void lua_ref(EulunaInterface *lua) {
        m_refs++;
    }

    void lua_release(EulunaInterce *lua) {
        lua_releaseFieldsTable(lua);
        m_refs = 0;
    }

private:
    int m_fieldsTableRef = -1;
    int m_refs = 0;
};

#endif

