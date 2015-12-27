/* Compatibility for lua 5.3, taken from https://github.com/keplerproject/lua-compat-5.3 */

#ifndef COMPAT53_H_
#define COMPAT53_H_

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <lua.hpp>

#if defined(COMPAT53_PREFIX)
/* - change the symbol names of functions to avoid linker conflicts
 *  * - compat-5.3.c needs to be compiled (and linked) separately
 *   */
#  if !defined(COMPAT53_API)
#    define COMPAT53_API extern
#  endif
#  undef COMPAT53_INCLUDE_SOURCE
#else /* COMPAT53_PREFIX */
/* - make all functions static and include the source.
 *  * - don't mess with the symbol names of functions
 *   * - compat-5.3.c doesn't need to be compiled (and linked) separately
 *    */
#  define COMPAT53_PREFIX lua
#  undef COMPAT53_API
#  if defined(__GNUC__) || defined(__clang__)
#    define COMPAT53_API __attribute__((__unused__)) static
#  else
#    define COMPAT53_API static
#  endif
#  define COMPAT53_INCLUDE_SOURCE
#endif /* COMPAT53_PREFIX */

#define COMPAT53_CONCAT_HELPER(a, b) a##b
#define COMPAT53_CONCAT(a, b) COMPAT53_CONCAT_HELPER(a, b)



/* declarations for Lua 5.1 */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 501

/* XXX not implemented:
 *  * lua_arith (new operators)
 *   * lua_upvalueid
 *    * lua_upvaluejoin
 *     * lua_version
 *      * lua_yieldk
 *       * luaL_loadbufferx
 *        * luaL_loadfilex
 *         */

/* PUC-Rio Lua uses lconfig_h as include guard for luaconf.h,
 *  * LuaJIT uses luaconf_h. If you use PUC-Rio's include files
 *   * but LuaJIT's library, you will need to define the macro
 *    * COMPAT53_IS_LUAJIT yourself! */
#if !defined(COMPAT53_IS_LUAJIT) && defined(luaconf_h)
#  define COMPAT53_IS_LUAJIT
#endif

#define LUA_OK 0
#define LUA_OPADD 0
#define LUA_OPSUB 1
#define LUA_OPMUL 2
#define LUA_OPDIV 3
#define LUA_OPMOD 4
#define LUA_OPPOW 5
#define LUA_OPUNM 6
#define LUA_OPEQ 0
#define LUA_OPLT 1
#define LUA_OPLE 2

typedef size_t lua_Unsigned;

typedef struct luaL_Buffer_53 {
  luaL_Buffer b; /* make incorrect code crash! */
  char *ptr;
  size_t nelems;
  size_t capacity;
  lua_State *L2;
} luaL_Buffer_53;
#define luaL_Buffer luaL_Buffer_53

#define lua_absindex COMPAT53_CONCAT(COMPAT53_PREFIX, _absindex)
COMPAT53_API int lua_absindex (lua_State *L, int i);

#define lua_arith COMPAT53_CONCAT(COMPAT53_PREFIX, _arith)
COMPAT53_API void lua_arith (lua_State *L, int op);

#define lua_compare COMPAT53_CONCAT(COMPAT53_PREFIX, _compare)
COMPAT53_API int lua_compare (lua_State *L, int idx1, int idx2, int op);

#define lua_copy COMPAT53_CONCAT(COMPAT53_PREFIX, _copy)
COMPAT53_API void lua_copy (lua_State *L, int from, int to);

#define lua_getuservalue(L, i) \
      (lua_getfenv(L, i), lua_type(L, -1))
#define lua_setuservalue(L, i) \
      (luaL_checktype(L, -1, LUA_TTABLE), lua_setfenv(L, i))

#define lua_len COMPAT53_CONCAT(COMPAT53_PREFIX, _len)
COMPAT53_API void lua_len (lua_State *L, int i);

#define luaL_newlibtable(L, l) \
      (lua_createtable(L, 0, sizeof(l)/sizeof(*(l))-1))
#define luaL_newlib(L, l) \
      (luaL_newlibtable(L, l), luaL_register(L, NULL, l))

#define lua_pushglobaltable(L) \
      lua_pushvalue(L, LUA_GLOBALSINDEX)

#define lua_rawgetp COMPAT53_CONCAT(COMPAT53_PREFIX, _rawgetp)
COMPAT53_API int lua_rawgetp (lua_State *L, int i, const void *p);

#define lua_rawsetp COMPAT53_CONCAT(COMPAT53_PREFIX, _rawsetp)
COMPAT53_API void lua_rawsetp(lua_State *L, int i, const void *p);

#define lua_rawlen(L, i) lua_objlen(L, i)

#define lua_tointegerx COMPAT53_CONCAT(COMPAT53_PREFIX, _tointegerx)
COMPAT53_API lua_Integer lua_tointegerx (lua_State *L, int i, int *isnum);

#define lua_tonumberx COMPAT53_CONCAT(COMPAT53_PREFIX, _tonumberx)
COMPAT53_API lua_Number lua_tonumberx (lua_State *L, int i, int *isnum);

#define luaL_checkversion COMPAT53_CONCAT(COMPAT53_PREFIX, L_checkversion)
COMPAT53_API void luaL_checkversion (lua_State *L);

#define luaL_checkstack COMPAT53_CONCAT(COMPAT53_PREFIX, L_checkstack_53)
COMPAT53_API void luaL_checkstack (lua_State *L, int sp, const char *msg);

#define luaL_getsubtable COMPAT53_CONCAT(COMPAT53_PREFIX, L_getsubtable)
COMPAT53_API int luaL_getsubtable (lua_State* L, int i, const char *name);

#define luaL_len COMPAT53_CONCAT(COMPAT53_PREFIX, L_len)
COMPAT53_API int luaL_len (lua_State *L, int i);

#define luaL_setfuncs COMPAT53_CONCAT(COMPAT53_PREFIX, L_setfuncs)
COMPAT53_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

#define luaL_setmetatable COMPAT53_CONCAT(COMPAT53_PREFIX, L_setmetatable)
COMPAT53_API void luaL_setmetatable (lua_State *L, const char *tname);

#define luaL_testudata COMPAT53_CONCAT(COMPAT53_PREFIX, L_testudata)
COMPAT53_API void *luaL_testudata (lua_State *L, int i, const char *tname);

#define luaL_tolstring COMPAT53_CONCAT(COMPAT53_PREFIX, L_tolstring)
COMPAT53_API const char *luaL_tolstring (lua_State *L, int idx, size_t *len);

#if !defined(COMPAT53_IS_LUAJIT)
#define luaL_traceback COMPAT53_CONCAT(COMPAT53_PREFIX, L_traceback)
COMPAT53_API void luaL_traceback (lua_State *L, lua_State *L1, const char *msg, int level);

#define luaL_fileresult COMPAT53_CONCAT(COMPAT53_PREFIX, L_fileresult)
COMPAT53_API int luaL_fileresult (lua_State *L, int stat, const char *fname);

#define luaL_execresult COMPAT53_CONCAT(COMPAT53_PREFIX, L_execresult)
COMPAT53_API int luaL_execresult (lua_State *L, int stat);
#endif /* COMPAT53_IS_LUAJIT */

#define lua_callk(L, na, nr, ctx, cont) \
      ((void)(ctx), (void)(cont), lua_call(L, na, nr))
#define lua_pcallk(L, na, nr, err, ctx, cont) \
      ((void)(ctx), (void)(cont), lua_pcall(L, na, nr, err))

#define luaL_buffinit COMPAT53_CONCAT(COMPAT53_PREFIX, _buffinit_53)
COMPAT53_API void luaL_buffinit (lua_State *L, luaL_Buffer_53 *B);

#define luaL_prepbuffsize COMPAT53_CONCAT(COMPAT53_PREFIX, _prepbufsize_53)
COMPAT53_API char *luaL_prepbuffsize (luaL_Buffer_53 *B, size_t s);

#define luaL_addlstring COMPAT53_CONCAT(COMPAT53_PREFIX, _addlstring_53)
COMPAT53_API void luaL_addlstring (luaL_Buffer_53 *B, const char *s, size_t l);

#define luaL_addvalue COMPAT53_CONCAT(COMPAT53_PREFIX, _addvalue_53)
COMPAT53_API void luaL_addvalue (luaL_Buffer_53 *B);

#define luaL_pushresult COMPAT53_CONCAT(COMPAT53_PREFIX, _pushresult_53)
COMPAT53_API void luaL_pushresult (luaL_Buffer_53 *B);

#undef luaL_buffinitsize
#define luaL_buffinitsize(L, B, s) \
      (luaL_buffinit(L, B), luaL_prepbuffsize(B, s))

#undef luaL_prepbuffer
#define luaL_prepbuffer(B) \
      luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

#undef luaL_addchar
#define luaL_addchar(B, c) \
      ((void)((B)->nelems < (B)->capacity || luaL_prepbuffsize(B, 1)), \
          ((B)->ptr[(B)->nelems++] = (c)))

#undef luaL_addsize
#define luaL_addsize(B, s) \
      ((B)->nelems += (s))

#undef luaL_addstring
#define luaL_addstring(B, s) \
      luaL_addlstring(B, s, strlen(s))

#undef luaL_pushresultsize
#define luaL_pushresultsize(B, s) \
      (luaL_addsize(B, s), luaL_pushresult(B))

#if defined(LUA_COMPAT_APIINTCASTS)
#define lua_pushunsigned(L, n) \
      lua_pushinteger(L, (lua_Integer)(n))
#define lua_tounsignedx(L, i, is) \
      ((lua_Unsigned)lua_tointegerx(L, i, is))
#define lua_tounsigned(L, i) \
      lua_tounsignedx(L, i, NULL)
#define luaL_checkunsigned(L, a) \
      ((lua_Unsigned)luaL_checkinteger(L, a))
#define luaL_optunsigned(L, a, d) \
      ((lua_Unsigned)luaL_optinteger(L, a, (lua_Integer)(d)))
#endif

#endif /* Lua 5.1 only */



/* declarations for Lua 5.1 and 5.2 */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM <= 502

typedef int lua_KContext;

typedef int (*lua_KFunction)(lua_State *L, int status, lua_KContext ctx);

#define lua_dump(L, w, d, s) \
      ((void)(s), lua_dump(L, w, d))

#define lua_getfield(L, i, k) \
      (lua_getfield(L, i, k), lua_type(L, -1))

#define lua_gettable(L, i) \
      (lua_gettable(L, i), lua_type(L, -1))

#define lua_geti COMPAT53_CONCAT(COMPAT53_PREFIX, _geti)
COMPAT53_API int lua_geti (lua_State *L, int index, lua_Integer i);

#define lua_isinteger COMPAT53_CONCAT(COMPAT53_PREFIX, _isinteger)
COMPAT53_API int lua_isinteger (lua_State *L, int index);

#define lua_numbertointeger(n, p) \
      ((*(p) = (lua_Integer)(n)), 1)

#define lua_rawget(L, i) \
      (lua_rawget(L, i), lua_type(L, -1))

#define lua_rawgeti(L, i, n) \
      (lua_rawgeti(L, i, n), lua_type(L, -1))

#define lua_rotate COMPAT53_CONCAT(COMPAT53_PREFIX, _rotate)
COMPAT53_API void lua_rotate (lua_State *L, int idx, int n);

#define lua_seti COMPAT53_CONCAT(COMPAT53_PREFIX, _seti)
COMPAT53_API void lua_seti (lua_State *L, int index, lua_Integer i);

#define lua_stringtonumber COMPAT53_CONCAT(COMPAT53_PREFIX, _stringtonumber)
COMPAT53_API size_t lua_stringtonumber (lua_State *L, const char *s);

#define luaL_getmetafield(L, o, e) \
      (luaL_getmetafield(L, o, e) ? lua_type(L, -1) : LUA_TNIL)

#define luaL_newmetatable(L, tn) \
      (luaL_newmetatable(L, tn) ? (lua_pushstring(L, tn), lua_setfield(L, -2, "__name"), 1) : 0)

#define luaL_requiref COMPAT53_CONCAT(COMPAT53_PREFIX, L_requiref_53)
COMPAT53_API void luaL_requiref (lua_State *L, const char *modname,
                                         lua_CFunction openf, int glb );

#endif /* Lua 5.1 and Lua 5.2 */



/* declarations for Lua 5.2 */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 502

/* XXX not implemented:
 *  * lua_isyieldable
 *   * lua_getextraspace
 *    * lua_arith (new operators)
 *     * lua_pushfstring (new formats)
 *      */

#define lua_getglobal(L, n) \
      (lua_getglobal(L, n), lua_type(L, -1))

#define lua_getuservalue(L, i) \
      (lua_getuservalue(L, i), lua_type(L, -1))

#define lua_rawgetp(L, i, p) \
      (lua_rawgetp(L, i, p), lua_type(L, -1))

#define LUA_KFUNCTION(_name) \
      static int (_name)(lua_State *L, int status, lua_KContext ctx); \
  static int (_name ## _52)(lua_State *L) { \
      lua_KContext ctx; \
      int status = lua_getctx(L, &ctx); \
      return (_name)(L, status, ctx); \
    } \
  static int (_name)(lua_State *L, int status, lua_KContext ctx)

#define lua_pcallk(L, na, nr, err, ctx, cont) \
      lua_pcallk(L, na, nr, err, ctx, cont ## _52)

#define lua_callk(L, na, nr, ctx, cont) \
      lua_callk(L, na, nr, ctx, cont ## _52)

#define lua_yieldk(L, nr, ctx, cont) \
      lua_yieldk(L, nr, ctx, cont ## _52)

#ifdef lua_call
#  undef lua_call
#  define lua_call(L, na, nr) \
      (lua_callk)(L, na, nr, 0, NULL)
#endif

#ifdef lua_pcall
#  undef lua_pcall
#  define lua_pcall(L, na, nr, err) \
      (lua_pcallk)(L, na, nr, err, 0, NULL)
#endif

#ifdef lua_yield
#  undef lua_yield
#  define lua_yield(L, nr) \
      (lua_yieldk)(L, nr, 0, NULL)
#endif

#endif /* Lua 5.2 only */



/* other Lua versions */
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 501 || LUA_VERSION_NUM > 503

#  error "unsupported Lua version (i.e. not Lua 5.1, 5.2, or 5.3)"

#endif /* other Lua versions except 5.1, 5.2, and 5.3 */



/* helper macro for defining continuation functions (for every version
 *  * *except* Lua 5.2) */
#ifndef LUA_KFUNCTION
#define LUA_KFUNCTION(_name) \
      static int (_name)(lua_State *L, int status, lua_KContext ctx)
#endif


#if defined(COMPAT53_INCLUDE_SOURCE)
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* don't compile it again if it already is included via compat53.h */
#ifndef COMPAT53_C_
#define COMPAT53_C_



/* definitions for Lua 5.1 only */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 501


COMPAT53_API int lua_absindex (lua_State *L, int i) {
  if (i < 0 && i > LUA_REGISTRYINDEX)
    i += lua_gettop(L) + 1;
  return i;
}


static void compat53_call_lua (lua_State *L, char const code[], size_t len,
                                       int nargs, int nret) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, (void*)code);
  if (lua_type(L, -1) != LUA_TFUNCTION) {
      lua_pop(L, 1);
      if (luaL_loadbuffer(L, code, len, "=none"))
        lua_error(L);
      lua_pushvalue(L, -1);
      lua_rawsetp(L, LUA_REGISTRYINDEX, (void*)code);
    }
  lua_insert(L, -nargs-1);
  lua_call(L, nargs, nret);
}


static const char compat53_arith_code[] =
  "local op,a,b=...\n"
    "if op==0 then return a+b\n"
      "elseif op==1 then return a-b\n"
        "elseif op==2 then return a*b\n"
          "elseif op==3 then return a/b\n"
            "elseif op==4 then return a%b\n"
              "elseif op==5 then return a^b\n"
                "elseif op==6 then return -a\n"
                  "end\n";

                  COMPAT53_API void lua_arith (lua_State *L, int op) {
                    if (op < LUA_OPADD || op > LUA_OPUNM)
                      luaL_error(L, "invalid 'op' argument for lua_arith");
                    luaL_checkstack(L, 5, "not enough stack slots");
                    if (op == LUA_OPUNM)
                      lua_pushvalue(L, -1);
                    lua_pushnumber(L, op);
                    lua_insert(L, -3);
                    compat53_call_lua(L, compat53_arith_code,
                                                sizeof(compat53_arith_code)-1, 3, 1);
                  }


static const char compat53_compare_code[] =
  "local a,b=...\n"
    "return a<=b\n";

    COMPAT53_API int lua_compare (lua_State *L, int idx1, int idx2, int op) {
      int result = 0;
      switch (op) {
          case LUA_OPEQ:
            return lua_equal(L, idx1, idx2);
          case LUA_OPLT:
            return lua_lessthan(L, idx1, idx2);
          case LUA_OPLE:
            luaL_checkstack(L, 5, "not enough stack slots");
            idx1 = lua_absindex(L, idx1);
            idx2 = lua_absindex(L, idx2);
            lua_pushvalue(L, idx1);
            lua_pushvalue(L, idx2);
            compat53_call_lua(L, compat53_compare_code,
                                            sizeof(compat53_compare_code)-1, 2, 1);
            result = lua_toboolean(L, -1);
            lua_pop(L, 1);
            return result;
          default:
            luaL_error(L, "invalid 'op' argument for lua_compare");
        }
      return 0;
    }


COMPAT53_API void lua_copy (lua_State *L, int from, int to) {
  int abs_to = lua_absindex(L, to);
  luaL_checkstack(L, 1, "not enough stack slots");
  lua_pushvalue(L, from);
  lua_replace(L, abs_to);
}


COMPAT53_API void lua_len (lua_State *L, int i) {
  switch (lua_type(L, i)) {
      case LUA_TSTRING: /* fall through */
      case LUA_TTABLE:
        if (!luaL_callmeta(L, i, "__len"))
          lua_pushnumber(L, (int)lua_objlen(L, i));
        break;
      case LUA_TUSERDATA:
        if (luaL_callmeta(L, i, "__len"))
          break;
        /* maybe fall through */
      default:
        luaL_error(L, "attempt to get length of a %s value",
                                 lua_typename(L, lua_type(L, i)));
    }
}


COMPAT53_API int lua_rawgetp (lua_State *L, int i, const void *p) {
  int abs_i = lua_absindex(L, i);
  lua_pushlightuserdata(L, (void*)p);
  lua_rawget(L, abs_i);
  return lua_type(L, -1);
}

COMPAT53_API void lua_rawsetp (lua_State *L, int i, const void *p) {
  int abs_i = lua_absindex(L, i);
  luaL_checkstack(L, 1, "not enough stack slots");
  lua_pushlightuserdata(L, (void*)p);
  lua_insert(L, -2);
  lua_rawset(L, abs_i);
}


COMPAT53_API lua_Integer lua_tointegerx (lua_State *L, int i, int *isnum) {
  lua_Integer n = lua_tointeger(L, i);
  if (isnum != NULL) {
      *isnum = (n != 0 || lua_isnumber(L, i));
  }
  return n;
}


COMPAT53_API lua_Number lua_tonumberx (lua_State *L, int i, int *isnum) {
  lua_Number n = lua_tonumber(L, i);
  if (isnum != NULL) {
      *isnum = (n != 0 || lua_isnumber(L, i));
    }
  return n;
}


COMPAT53_API void luaL_checkversion (lua_State *L) {
  (void)L;
}


COMPAT53_API void luaL_checkstack (lua_State *L, int sp, const char *msg) {
  if (!lua_checkstack(L, sp+LUA_MINSTACK)) {
      if (msg != NULL)
        luaL_error(L, "stack overflow (%s)", msg);
      else {
            lua_pushliteral(L, "stack overflow");
            lua_error(L);
          }
    }
}


COMPAT53_API int luaL_getsubtable (lua_State *L, int i, const char *name) {
  int abs_i = lua_absindex(L, i);
  luaL_checkstack(L, 3, "not enough stack slots");
  lua_pushstring(L, name);
  lua_gettable(L, abs_i);
  if (lua_istable(L, -1))
    return 1;
  lua_pop(L, 1);
  lua_newtable(L);
  lua_pushstring(L, name);
  lua_pushvalue(L, -2);
  lua_settable(L, abs_i);
  return 0;
}


COMPAT53_API int luaL_len (lua_State *L, int i) {
  int res = 0, isnum = 0;
  luaL_checkstack(L, 1, "not enough stack slots");
  lua_len(L, i);
  res = (int)lua_tointegerx(L, -1, &isnum);
  lua_pop(L, 1);
  if (!isnum)
    luaL_error(L, "object length is not a number");
  return res;
}


COMPAT53_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup+1, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
      int i;
      lua_pushstring(L, l->name);
      for (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -(nup + 1));
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
      lua_settable(L, -(nup + 3)); /* table must be below the upvalues, the name and the closure */
    }
  lua_pop(L, nup);  /* remove upvalues */
}


COMPAT53_API void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_checkstack(L, 1, "not enough stack slots");
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


COMPAT53_API void *luaL_testudata (lua_State *L, int i, const char *tname) {
  void *p = lua_touserdata(L, i);
  luaL_checkstack(L, 2, "not enough stack slots");
  if (p == NULL || !lua_getmetatable(L, i))
    return NULL;
  else {
      int res = 0;
      luaL_getmetatable(L, tname);
      res = lua_rawequal(L, -1, -2);
      lua_pop(L, 2);
      if (!res)
        p = NULL;
    }
  return p;
}


COMPAT53_API const char *luaL_tolstring (lua_State *L, int idx, size_t *len) {
  if (!luaL_callmeta(L, idx, "__tostring")) {
      int t = lua_type(L, idx);
      switch (t) {
            case LUA_TNIL:
              lua_pushliteral(L, "nil");
              break;
            case LUA_TSTRING:
            case LUA_TNUMBER:
              lua_pushvalue(L, idx);
              break;
            case LUA_TBOOLEAN:
              if (lua_toboolean(L, idx))
                lua_pushliteral(L, "true");
              else
                lua_pushliteral(L, "false");
              break;
            default:
              lua_pushfstring(L, "%s: %p", lua_typename(L, t),
                                                           lua_topointer(L, idx));
              break;
          }
    }
  return lua_tolstring(L, -1, len);
}


#if !defined(COMPAT53_IS_LUAJIT)
static int compat53_countlevels (lua_State *L) {
  lua_Debug ar;
  int li = 1, le = 1;
  /* find an upper bound */
  while (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  while (li < le) {
      int m = (li + le)/2;
      if (lua_getstack(L, m, &ar)) li = m + 1;
      else le = m;
    }
  return le - 1;
}

static int compat53_findfield (lua_State *L, int objidx, int level) {
  if (level == 0 || !lua_istable(L, -1))
    return 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  while (lua_next(L, -2)) {  /* for each pair in table */
      if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
            if (lua_rawequal(L, objidx, -1)) {  /* found object? */
                    lua_pop(L, 1);  /* remove value (but keep name) */
                    return 1;
                  }
            else if (compat53_findfield(L, objidx, level - 1)) {  /* try recursively */
                    lua_remove(L, -2);  /* remove table (but keep name) */
                    lua_pushliteral(L, ".");
                    lua_insert(L, -2);  /* place '.' between the two names */
                    lua_concat(L, 3);
                    return 1;
                  }
          }
      lua_pop(L, 1);  /* remove value */
    }
  return 0;  /* not found */
}

static int compat53_pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  int top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  if (compat53_findfield(L, top + 1, 2)) {
      lua_copy(L, -1, top + 1);  /* move name to proper place */
      lua_pop(L, 2);  /* remove pushed values */
      return 1;
    }
  else {
      lua_settop(L, top);  /* remove function and global table */
      return 0;
    }
}

static void compat53_pushfuncname (lua_State *L, lua_Debug *ar) {
  if (*ar->namewhat != '\0')  /* is there a name? */
    lua_pushfstring(L, "function " LUA_QS, ar->name);
  else if (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  else if (*ar->what == 'C') {
      if (compat53_pushglobalfuncname(L, ar)) {
            lua_pushfstring(L, "function " LUA_QS, lua_tostring(L, -1));
            lua_remove(L, -2);  /* remove name */
          }
      else
        lua_pushliteral(L, "?");
    }
  else
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
}

#define COMPAT53_LEVELS1 12  /* size of the first part of the stack */
#define COMPAT53_LEVELS2 10  /* size of the second part of the stack */

COMPAT53_API void luaL_traceback (lua_State *L, lua_State *L1,
                                          const char *msg, int level) {
  lua_Debug ar;
  int top = lua_gettop(L);
  int numlevels = compat53_countlevels(L1);
  int mark = (numlevels > COMPAT53_LEVELS1 + COMPAT53_LEVELS2) ? COMPAT53_LEVELS1 : 0;
  if (msg) lua_pushfstring(L, "%s\n", msg);
  lua_pushliteral(L, "stack traceback:");
  while (lua_getstack(L1, level++, &ar)) {
      if (level == mark) {  /* too many levels? */
            lua_pushliteral(L, "\n\t...");  /* add a '...' */
            level = numlevels - COMPAT53_LEVELS2;  /* and skip to last ones */
          }
      else {
            lua_getinfo(L1, "Slnt", &ar);
            lua_pushfstring(L, "\n\t%s:", ar.short_src);
            if (ar.currentline > 0)
              lua_pushfstring(L, "%d:", ar.currentline);
            lua_pushliteral(L, " in ");
            compat53_pushfuncname(L, &ar);
            lua_concat(L, lua_gettop(L) - top);
          }
    }
  lua_concat(L, lua_gettop(L) - top);
}


COMPAT53_API int luaL_fileresult (lua_State *L, int stat, const char *fname) {
  int en = errno;  /* calls to Lua API may change this value */
  if (stat) {
      lua_pushboolean(L, 1);
      return 1;
    }
  else {
      lua_pushnil(L);
      if (fname)
        lua_pushfstring(L, "%s: %s", fname, strerror(en));
      else
        lua_pushstring(L, strerror(en));
      lua_pushnumber(L, (lua_Number)en);
      return 3;
    }
}


#if !defined(l_inspectstat) && \
        (defined(unix) || defined(__unix) || defined(__unix__) || \
              defined(__TOS_AIX__) || defined(_SYSTYPE_BSD))
/* some form of unix; check feature macros in unistd.h for details */
#  include <unistd.h>
/* check posix version; the relevant include files and macros probably
 *  * were available before 2001, but I'm not sure */
#  if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
#    include <sys/wait.h>
#    define l_inspectstat(stat,what) \
      if (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
  else if (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }
#  endif
#endif

/* provide default (no-op) version */
#if !defined(l_inspectstat)
#  define l_inspectstat(stat,what) ((void)0)
#endif


COMPAT53_API int luaL_execresult (lua_State *L, int stat) {
  const char *what = "exit";
  if (stat == -1)
    return luaL_fileresult(L, 0, NULL);
  else {
      l_inspectstat(stat, what);
      if (*what == 'e' && stat == 0)
        lua_pushboolean(L, 1);
      else
        lua_pushnil(L);
      lua_pushstring(L, what);
      lua_pushinteger(L, stat);
      return 3;
    }
}
#endif /* not COMPAT53_IS_LUAJIT */


COMPAT53_API void luaL_buffinit (lua_State *L, luaL_Buffer_53 *B) {
  /* make it crash if used via pointer to a 5.1-style luaL_Buffer */
  B->b.p = NULL;
  B->b.L = NULL;
  B->b.lvl = 0;
  /* reuse the buffer from the 5.1-style luaL_Buffer though! */
  B->ptr = B->b.buffer;
  B->capacity = LUAL_BUFFERSIZE;
  B->nelems = 0;
  B->L2 = L;
}


COMPAT53_API char *luaL_prepbuffsize (luaL_Buffer_53 *B, size_t s) {
  if (B->capacity - B->nelems < s) { /* needs to grow */
      char* newptr = NULL;
      size_t newcap = B->capacity * 2;
      if (newcap - B->nelems < s)
        newcap = B->nelems + s;
      if (newcap < B->capacity) /* overflow */
        luaL_error(B->L2, "buffer too large");
      newptr = (char*)lua_newuserdata(B->L2, newcap);
      memcpy(newptr, B->ptr, B->nelems);
      if (B->ptr != B->b.buffer)
        lua_replace(B->L2, -2); /* remove old buffer */
      B->ptr = newptr;
      B->capacity = newcap;
    }
  return B->ptr+B->nelems;
}


COMPAT53_API void luaL_addlstring (luaL_Buffer_53 *B, const char *s, size_t l) {
  memcpy(luaL_prepbuffsize(B, l), s, l);
  luaL_addsize(B, l);
}


COMPAT53_API void luaL_addvalue (luaL_Buffer_53 *B) {
  size_t len = 0;
  const char *s = lua_tolstring(B->L2, -1, &len);
  if (!s)
    luaL_error(B->L2, "cannot convert value to string");
  if (B->ptr != B->b.buffer)
    lua_insert(B->L2, -2); /* userdata buffer must be at stack top */
  luaL_addlstring(B, s, len);
  lua_remove(B->L2, B->ptr != B->b.buffer ? -2 : -1);
}


void luaL_pushresult (luaL_Buffer_53 *B) {
  lua_pushlstring(B->L2, B->ptr, B->nelems);
  if (B->ptr != B->b.buffer)
    lua_replace(B->L2, -2); /* remove userdata buffer */
}


#endif /* Lua 5.1 */



/* definitions for Lua 5.1 and Lua 5.2 */
#if defined( LUA_VERSION_NUM ) && LUA_VERSION_NUM <= 502


COMPAT53_API int lua_geti (lua_State *L, int index, lua_Integer i) {
  index = lua_absindex(L, index);
  lua_pushinteger(L, i);
  lua_gettable(L, index);
  return lua_type(L, -1);
}


COMPAT53_API int lua_isinteger (lua_State *L, int index) {
  if (lua_type(L, index) == LUA_TNUMBER) {
      lua_Number n = lua_tonumber(L, index);
      lua_Integer i = lua_tointeger(L, index);
      if (i == n)
        return 1;
    }
  return 0;
}


static void compat53_reverse (lua_State *L, int a, int b) {
  for (; a < b; ++a, --b) {
      lua_pushvalue(L, a);
      lua_pushvalue(L, b);
      lua_replace(L, a);
      lua_replace(L, b);
    }
}


COMPAT53_API void lua_rotate (lua_State *L, int idx, int n) {
  int n_elems = 0;
  idx = lua_absindex(L, idx);
  n_elems = lua_gettop(L)-idx+1;
  if (n < 0)
    n += n_elems;
  if ( n > 0 && n < n_elems) {
      luaL_checkstack(L, 2, "not enough stack slots available");
      n = n_elems - n;
      compat53_reverse(L, idx, idx+n-1);
      compat53_reverse(L, idx+n, idx+n_elems-1);
      compat53_reverse(L, idx, idx+n_elems-1);
    }
}


COMPAT53_API void lua_seti (lua_State *L, int index, lua_Integer i) {
  luaL_checkstack(L, 1, "not enough stack slots available");
  index = lua_absindex(L, index);
  lua_pushinteger(L, i);
  lua_insert(L, -2);
  lua_settable(L, index);
}


#if !defined(lua_str2number)
#  define lua_str2number(s, p)  strtod(s, p)
#endif

COMPAT53_API size_t lua_stringtonumber (lua_State *L, const char *s) {
  char* endptr;
  lua_Number n = lua_str2number(s, &endptr);
  if (endptr != s) {
      while (*endptr != '\0' && isspace((unsigned char)*endptr))
        ++endptr;
      if (*endptr == '\0') {
            lua_pushnumber(L, n);
            return endptr - s + 1;
          }
    }
  return 0;
}


COMPAT53_API void luaL_requiref (lua_State *L, const char *modname,
                                         lua_CFunction openf, int glb) {
  luaL_checkstack(L, 3, "not enough stack slots available");
  luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
  if (lua_getfield(L, -1, modname) == LUA_TNIL) {
      lua_pop(L, 1);
      lua_pushcfunction(L, openf);
      lua_pushstring(L, modname);
      lua_call(L, 1, 1);
      lua_pushvalue(L, -1);
      lua_setfield(L, -3, modname);
    }
  if (glb) {
      lua_pushvalue(L, -1);
      lua_setglobal(L, modname);
    }
  lua_replace(L, -2);
}


#endif /* Lua 5.1 and 5.2 */


#endif /* COMPAT53_C_ */


/*********************************************************************
 * * This file contains parts of Lua 5.2's and Lua 5.3's source code:
 * *
 * * Copyright (C) 1994-2014 Lua.org, PUC-Rio.
 * *
 * * Permission is hereby granted, free of charge, to any person obtaining
 * * a copy of this software and associated documentation files (the
 * * "Software"), to deal in the Software without restriction, including
 * * without limitation the rights to use, copy, modify, merge, publish,
 * * distribute, sublicense, and/or sell copies of the Software, and to
 * * permit persons to whom the Software is furnished to do so, subject to
 * * the following conditions:
 * *
 * * The above copyright notice and this permission notice shall be
 * * included in all copies or substantial portions of the Software.
 * *
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * *********************************************************************/

#endif

#endif /* COMPAT53_H_ */
