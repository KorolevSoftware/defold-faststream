![](/assets/faststream.png)

## Defold extension
Fast Stream store lua table vector 3 or vector 4 to stream


```lua
local vertex_count = 10
local buf = buffer.create(vertex_count*2, {
    { name = hash("position"), type=buffer.VALUE_TYPE_FLOAT32, count = 3 },
    { name = hash("texcoord0"), type=buffer.VALUE_TYPE_FLOAT32, count = 2 },
    { name = hash("tint"), type=buffer.VALUE_TYPE_FLOAT32, count = 4 },
})
```