// Extension lib defines
#define LIB_NAME "FastStream"
#define MODULE_NAME "faststream"
#define SCRIPT_LIB_NAME "buffer"
#define SCRIPT_TYPE_NAME_BUFFER "buffer"
#define SCRIPT_TYPE_NAME_BUFFERSTREAM "bufferstream"

// include the Defold SDK
#include <dmsdk/sdk.h>
#include <dmsdk/script.h>

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
    // dmScript::Ref
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
    if (data_count == 0) {
        return 0;
    }

    size_t element_count;
    // getVectorData *func;
    if (dmScript::IsVector3(L, -1)) {
        element_count = 3;
        // func = reinterpret_cast<dmVMath::Vector4* (lua_State* L, int index)> (dmScript::ToVector3);
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

// PUSHER

using pusherFunc = void (lua_State* L, int stack_index, BufferStream* stream, int &buffer_index);

void pusherNumberByIndex(BufferStream* stream, int &buffer_index, float value) {
    uint32_t count = buffer_index / stream->m_TypeCount;
    uint32_t component = buffer_index % stream->m_TypeCount;
    stream->m_Set(stream->m_Data, count * stream->m_Stride + component, value);
    buffer_index++;
}

void pusherVector3(lua_State* L, int stack_index, BufferStream* stream, int &buffer_index) { 
    dmVMath::Vector3* v = (dmVMath::Vector3*)lua_touserdata(L, stack_index); //dmScript::ToVector3(L, stack_index);

    pusherNumberByIndex(stream, buffer_index, v->getX());
    pusherNumberByIndex(stream, buffer_index, v->getY());
    pusherNumberByIndex(stream, buffer_index, v->getZ());
}

void pusherVector4(lua_State* L, int stack_index, BufferStream* stream, int &buffer_index){
    dmVMath::Vector4* v = (dmVMath::Vector4*)lua_touserdata(L, stack_index); //dmScript::ToVector4(L, stack_index);

    pusherNumberByIndex(stream, buffer_index, v->getX());
    pusherNumberByIndex(stream, buffer_index, v->getY());
    pusherNumberByIndex(stream, buffer_index, v->getZ());
    pusherNumberByIndex(stream, buffer_index, v->getW());
}

void pusherNumber(lua_State* L, int stack_index, BufferStream* stream, int &buffer_index) {
    pusherNumberByIndex(stream, buffer_index,  lua_tonumber(L, stack_index));
}

// Stack
// 1 stream
// 2 table vector3 || vector4 || number

static int set_table_universe(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    BufferStream* stream = CheckStream(L, 1);
    size_t table_length = lua_objlen(L, 2); // 2 - table

    if (table_length == 0) { 
        return 0;
    }

    #if DM_DEBUG 
        // return 1;
    #endif

    // Check table data type
    lua_pushnil(L); // first key
    lua_next(L,  2);

    pusherFunc *func = nullptr;

    if (dmScript::IsVector3(L, -1)) {
        func = pusherVector3;
    } 

    if (dmScript::IsVector4(L, -1)) {
        func = pusherVector4;
    }

    if (lua_isnumber(L, -1)) {
         func = pusherNumber;
    }

    if (func == nullptr) {
        luaL_typerror(L, -1, "vector3 or vector4 or number");
    }

    // Table iteration
    for(int table_index = 0, buffer_index = 0; table_index < table_length; table_index++, lua_next(L,  2)) {
        func(L, -1, stream, buffer_index);
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
    {"set_table_fast", set_table},
    {"set_table_universe", set_table_universe},
    {0, 0}
};

#pragma region Extension
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
#pragma endregion