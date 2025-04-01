![](https://github.com/user-attachments/assets/2c11ba97-163e-445f-bac5-21a1bfd97e92)

# FastStream Defold Extension

FastStream is a Defold extension that provides efficient buffer stream operations for handling vector data.

## API Reference

### Functions

#### `faststream.set_table_raw(stream, table)`
Sets multiple vector values from a table to the buffer stream without converting values to stream type. The table should contain either Vector3 or Vector4 values.

**Parameters:**
- `stream` (userdata) - The buffer stream to modify
- `table` (table) - Table containing Vector3 or Vector4 values to set

#### `faststream.set_table_universal(stream, table)`
Sets multiple values from a table to the buffer stream. Supports Vector3, Vector4, or number values.

**Parameters:**
- `stream` (userdata) - The buffer stream to modify
- `table` (table) - Table containing Vector3, Vector4, or number values to set

#### `faststream.set_vector2(stream, index, vector)`
Sets a Vector3 value at the specified index in the buffer stream.

**Parameters:**
- `stream` (userdata) - The buffer stream to modify
- `index` (number) - The 1-based index where to set the vector
- `vector` (vector3) - The Vector3 value to set (x,y components will be used)

#### `faststream.set_vector3(stream, index, vector)`
Sets a Vector3 value at the specified index in the buffer stream.

**Parameters:**
- `stream` (userdata) - The buffer stream to modify
- `index` (number) - The 1-based index where to set the vector
- `vector` (vector3) - The Vector3 value to set

#### `faststream.set_vector4(stream, index, vector)`
Sets a Vector4 value at the specified index in the buffer stream.

**Parameters:**
- `stream` (userdata) - The buffer stream to modify
- `index` (number) - The 1-based index where to set the vector
- `vector` (vector4) - The Vector4 value to set

## Usage Examples

```lua
-- Set a single Vector3 value
local stream = ... -- your buffer stream, like buffer.get_stream(buf, "position")
local vector = vmath.vector3(1, 2, 3)
faststream.set_vector3(stream, 1, vector)

-- Set multiple values using a table
local values = {
    vmath.vector3(1, 2, 3),
    vmath.vector3(4, 5, 6),
    vmath.vector3(7, 8, 9)
}
faststream.set_table_universal(stream, values)

-- Set raw vector values without conversion
local raw_values = {
    vmath.vector4(1, 2, 3, 4),
    vmath.vector4(5, 6, 7, 8)
}
faststream.set_table_raw(stream, raw_values)
```
