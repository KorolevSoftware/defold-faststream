// myextension.cpp
// Extension lib defines
#define LIB_NAME "FastStream"
#define MODULE_NAME "faststream"
#define SCRIPT_LIB_NAME "buffer"
#define SCRIPT_TYPE_NAME_BUFFER "buffer"
#define SCRIPT_TYPE_NAME_BUFFERSTREAM "bufferstream"

// include the Defold SDK
#include <dmsdk/sdk.h>
#include <dmsdk/script.h>
#include <dmsdk/resource/resource.h>
#include <algorithm>
#define DM_LUA_ERROR(_fmt_, ...)   _DM_LuaStackCheck.Error(_fmt_,  ##__VA_ARGS__); \

namespace dmScript {
    void* CheckUserType(lua_State* L, int user_data_index, uint32_t type_hash, const char* error_message);
    static uint32_t SCRIPT_BUFFERSTREAM_TYPE_HASH = dmHashString32(SCRIPT_TYPE_NAME_BUFFERSTREAM);
}

typedef void (*FStreamSetter)(void* data, int index, lua_Number v);
typedef lua_Number (*FStreamGetter)(void* data, int index);

struct BufferStream
{
    dmBuffer::HBuffer   m_Buffer;
    dmhash_t            m_Name;     // The stream name
    void*               m_Data;     // Pointer to the first struct in the stream
    FStreamSetter       m_Set;
    FStreamGetter       m_Get;
    uint32_t            m_Count;    // Number of structs contained in the stream (or buffer)
    uint32_t            m_Stride;   // The stride of the pointer, measured in the units of the value type
    uint32_t            m_TypeCount;// number of components that make up an "element". E.g. 3 in a Vec3
    dmBuffer::ValueType m_Type;		// The type of elements in the array
    int                 m_BufferRef;// Holds a reference to the Lua object
};

static BufferStream* CheckStream(lua_State* L, int index)
{
    if (lua_type(L, index) == LUA_TUSERDATA)
    {
        BufferStream* stream = (BufferStream*)dmScript::CheckUserType(L, index, dmScript::SCRIPT_BUFFERSTREAM_TYPE_HASH, 0);
        if (stream && dmBuffer::IsBufferValid(stream->m_Buffer))
        {
            return stream;
        }
        luaL_error(L, "The buffer handle is invalid");
    }
    luaL_typerror(L, index, SCRIPT_TYPE_NAME_BUFFERSTREAM);
    return 0x0;
}

static int set_vector2_to_stream(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    BufferStream* stream = CheckStream(L, 1);
    int index = luaL_checkinteger(L, 2) - 1;
    dmVMath::Vector3* v = dmScript::ToVector3(L, 3);

    uint32_t vector_sizeof = sizeof(float)*2;
    memcpy((uint8_t*)(stream->m_Data) + index*stream->m_Stride*sizeof(float), v, vector_sizeof);
    dmBuffer::UpdateContentVersion(stream->m_Buffer);
    return 0;
}

static int set_vector3_to_stream(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    BufferStream* stream = CheckStream(L, 1);
    int index = luaL_checkinteger(L, 2) - 1;
    dmVMath::Vector3* v = dmScript::ToVector3(L, 3);

    uint32_t vector_sizeof = sizeof(float)*3;
    memcpy((uint8_t*)(stream->m_Data) + index*stream->m_Stride*sizeof(float), v, vector_sizeof);
    dmBuffer::UpdateContentVersion(stream->m_Buffer);
    return 0;
}

static int set_vector4_to_stream(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    BufferStream* stream = CheckStream(L, 1);
    int index = luaL_checkinteger(L, 2) - 1;
    dmVMath::Vector4* v = dmScript::ToVector4(L, 3);

    uint32_t vector_sizeof = sizeof(float)*4;
    memcpy((uint8_t*)(stream->m_Data) + index*stream->m_Stride*sizeof(float), v, vector_sizeof);
    dmBuffer::UpdateContentVersion(stream->m_Buffer);
    return 0;
}

using sFStreamSetter = int *(void* data_stream, int *date_value, int data_width);
using getVectorData = uint8_t* (lua_State* L, int index);
// Stack
// 1 stream
// 2 table vector3 || vector4

static int set_table(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    BufferStream* stream = CheckStream(L, 1);
    // lua_pop(L, 1); //remove Stream from stack

    // Table iteration
    lua_pushnil(L); // first key
    size_t data_count = lua_objlen(L, 2); // Stack 1 - table

    if (!lua_next(L,  -2)) { // Check is empty table
        return 0;
    }
    size_t element_count;
    // getVectorData *func;
    if (dmScript::IsVector3(L, -1)){
        element_count = 3;
        // func = (uint8_t* (lua_State* L, int index))dmScript::ToVector3;
    } else {
        element_count = 4;
    }

    uint8_t *data;
    uint32_t vector_sizeof = sizeof(float)*element_count;
    for( int index = 0; index < data_count; index++, lua_next(L,  -2)) {
        if (element_count == 3)
        {
            dmVMath::Vector3* v= dmScript::ToVector3(L, -1);
            data = (uint8_t*)v;
        } else {
            dmVMath::Vector4* v= dmScript::ToVector4(L, -1);
            data = (uint8_t*)v;
        }

        memcpy((uint8_t*)(stream->m_Data) + index*stream->m_Stride*sizeof(float), data, vector_sizeof);
        lua_pop(L, 1);
    }
    dmBuffer::UpdateContentVersion(stream->m_Buffer);
    return 0;
}


// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{   
    {"set_vector2_to_stream", set_vector2_to_stream},
    {"set_vector3_to_stream", set_vector3_to_stream},
    {"set_vector4_to_stream", set_vector4_to_stream},
    {"set_table", set_table},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeMyExtension(dmExtension::Params* params)
{
    // Init Lua
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result OnUpdateMyExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static void OnEventMyExtension(dmExtension::Params* params, const dmExtension::Event* event) {}

DM_DECLARE_EXTENSION(faststream, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, OnUpdateMyExtension, OnEventMyExtension, FinalizeMyExtension)
