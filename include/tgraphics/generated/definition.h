void (*_set_vsync)(State *_state, bool enable);
void set_vsync(bool enable) { return _set_vsync(this, enable); }
void (*_on_window_resize)(State *_state, u32 w, u32 h);
void on_window_resize(u32 w, u32 h) { return _on_window_resize(this, w, h); }
void (*_present)(State *_state);
void present() { return _present(this); }
CameraMatrices (*_calculate_perspective_matrices)(State *_state, v3f position, v3f rotation, f32 aspect_ratio, f32 fov_radians, f32 near_plane, f32 far_plane);
CameraMatrices calculate_perspective_matrices(v3f position, v3f rotation, f32 aspect_ratio, f32 fov_radians, f32 near_plane, f32 far_plane) { return _calculate_perspective_matrices(this, position, rotation, aspect_ratio, fov_radians, near_plane, far_plane); }
void (*_set_blend)(State *_state, BlendFunction function, Blend source, Blend destination);
void set_blend(BlendFunction function, Blend source, Blend destination) { return _set_blend(this, function, source, destination); }
void (*_set_topology)(State *_state, Topology topology);
void set_topology(Topology topology) { return _set_topology(this, topology); }
void (*_set_scissor)(State *_state, Viewport viewport);
void set_scissor(Viewport viewport) { return _set_scissor(this, viewport); }
void (*_disable_scissor)(State *_state);
void disable_scissor() { return _disable_scissor(this); }
void (*_set_cull)(State *_state, Cull cull);
void set_cull(Cull cull) { return _set_cull(this, cull); }
void (*_disable_blend)(State *_state);
void disable_blend() { return _disable_blend(this); }
void (*_disable_depth_clip)(State *_state);
void disable_depth_clip() { return _disable_depth_clip(this); }
void (*_enable_depth_clip)(State *_state);
void enable_depth_clip() { return _enable_depth_clip(this); }
void (*_set_viewport)(State *_state, Viewport viewport);
void set_viewport(Viewport viewport) { return _set_viewport(this, viewport); }
void (*_draw)(State *_state, u32 vertex_count, u32 start_vertex);
void draw(u32 vertex_count, u32 start_vertex) { return _draw(this, vertex_count, start_vertex); }
void (*_draw_indexed)(State *_state, u32 index_count);
void draw_indexed(u32 index_count) { return _draw_indexed(this, index_count); }
VertexBuffer * (*_create_vertex_buffer)(State *_state, Span<u8> buffer, Span<ElementType> vertex_descriptor);
VertexBuffer * create_vertex_buffer(Span<u8> buffer, Span<ElementType> vertex_descriptor) { return _create_vertex_buffer(this, buffer, vertex_descriptor); }
void (*_set_vertex_buffer)(State *_state, VertexBuffer * buffer);
void set_vertex_buffer(VertexBuffer * buffer) { return _set_vertex_buffer(this, buffer); }
void (*_update_vertex_buffer)(State *_state, VertexBuffer * buffer, Span<u8> data);
void update_vertex_buffer(VertexBuffer * buffer, Span<u8> data) { return _update_vertex_buffer(this, buffer, data); }
IndexBuffer * (*_create_index_buffer)(State *_state, Span<u8> buffer, u32 index_size);
IndexBuffer * create_index_buffer(Span<u8> buffer, u32 index_size) { return _create_index_buffer(this, buffer, index_size); }
void (*_set_index_buffer)(State *_state, IndexBuffer * buffer);
void set_index_buffer(IndexBuffer * buffer) { return _set_index_buffer(this, buffer); }
Texture2D * (*_create_texture_2d)(State *_state, u32 width, u32 height, void const * data, Format format);
Texture2D * create_texture_2d(u32 width, u32 height, void const * data, Format format) { return _create_texture_2d(this, width, height, data, format); }
void (*_set_texture_2d)(State *_state, Texture2D * texture, u32 slot);
void set_texture_2d(Texture2D * texture, u32 slot) { return _set_texture_2d(this, texture, slot); }
void (*_resize_texture_2d)(State *_state, Texture2D * texture, u32 w, u32 h);
void resize_texture_2d(Texture2D * texture, u32 w, u32 h) { return _resize_texture_2d(this, texture, w, h); }
void (*_read_texture_2d)(State *_state, Texture2D * texture, Span<u8> data);
void read_texture_2d(Texture2D * texture, Span<u8> data) { return _read_texture_2d(this, texture, data); }
void (*_update_texture_2d)(State *_state, Texture2D * texture, u32 width, u32 height, void * data);
void update_texture_2d(Texture2D * texture, u32 width, u32 height, void * data) { return _update_texture_2d(this, texture, width, height, data); }
void (*_generate_mipmaps_2d)(State *_state, Texture2D * texture);
void generate_mipmaps_2d(Texture2D * texture) { return _generate_mipmaps_2d(this, texture); }
void (*_set_sampler)(State *_state, Filtering filtering, Comparison comparison, u32 slot);
void set_sampler(Filtering filtering, Comparison comparison, u32 slot) { return _set_sampler(this, filtering, comparison, slot); }
RenderTarget * (*_create_render_target)(State *_state, Texture2D * color, Texture2D * depth);
RenderTarget * create_render_target(Texture2D * color, Texture2D * depth) { return _create_render_target(this, color, depth); }
void (*_set_render_target)(State *_state, RenderTarget * target);
void set_render_target(RenderTarget * target) { return _set_render_target(this, target); }
void (*_clear)(State *_state, RenderTarget * render_target, ClearFlags flags, v4f color, f32 depth);
void clear(RenderTarget * render_target, ClearFlags flags, v4f color, f32 depth) { return _clear(this, render_target, flags, color, depth); }
TextureCube * (*_create_texture_cube)(State *_state, u32 size, void ** data, Format format);
TextureCube * create_texture_cube(u32 size, void ** data, Format format) { return _create_texture_cube(this, size, data, format); }
void (*_set_texture_cube)(State *_state, TextureCube * texture, u32 slot);
void set_texture_cube(TextureCube * texture, u32 slot) { return _set_texture_cube(this, texture, slot); }
void (*_generate_mipmaps_cube)(State *_state, TextureCube * texture, GenerateCubeMipmapParams params);
void generate_mipmaps_cube(TextureCube * texture, GenerateCubeMipmapParams params) { return _generate_mipmaps_cube(this, texture, params); }
Shader * (*_create_shader)(State *_state, Span<utf8> source);
Shader * create_shader(Span<utf8> source) { return _create_shader(this, source); }
void (*_set_shader)(State *_state, Shader * shader);
void set_shader(Shader * shader) { return _set_shader(this, shader); }
ShaderConstants * (*_create_shader_constants)(State *_state, umm size);
ShaderConstants * create_shader_constants(umm size) { return _create_shader_constants(this, size); }
void (*_update_shader_constants)(State *_state, ShaderConstants * constants, void const * source, u32 offset, u32 size);
void update_shader_constants(ShaderConstants * constants, void const * source, u32 offset, u32 size) { return _update_shader_constants(this, constants, source, offset, size); }
void * (*_map_shader_constants)(State *_state, ShaderConstants * constants, Access access);
void * map_shader_constants(ShaderConstants * constants, Access access) { return _map_shader_constants(this, constants, access); }
void (*_unmap_shader_constants)(State *_state, ShaderConstants * constants);
void unmap_shader_constants(ShaderConstants * constants) { return _unmap_shader_constants(this, constants); }
void (*_set_shader_constants)(State *_state, ShaderConstants * constants, u32 slot);
void set_shader_constants(ShaderConstants * constants, u32 slot) { return _set_shader_constants(this, constants, slot); }
void (*_set_rasterizer)(State *_state, RasterizerState state);
void set_rasterizer(RasterizerState state) { return _set_rasterizer(this, state); }
RasterizerState (*_get_rasterizer)(State *_state);
RasterizerState get_rasterizer() { return _get_rasterizer(this); }
ComputeShader * (*_create_compute_shader)(State *_state, Span<utf8> source);
ComputeShader * create_compute_shader(Span<utf8> source) { return _create_compute_shader(this, source); }
void (*_set_compute_shader)(State *_state, ComputeShader * shader);
void set_compute_shader(ComputeShader * shader) { return _set_compute_shader(this, shader); }
void (*_dispatch_compute_shader)(State *_state, u32 x, u32 y, u32 z);
void dispatch_compute_shader(u32 x, u32 y, u32 z) { return _dispatch_compute_shader(this, x, y, z); }
ComputeBuffer * (*_create_compute_buffer)(State *_state, u32 size);
ComputeBuffer * create_compute_buffer(u32 size) { return _create_compute_buffer(this, size); }
void (*_read_compute_buffer)(State *_state, ComputeBuffer * buffer, void * data);
void read_compute_buffer(ComputeBuffer * buffer, void * data) { return _read_compute_buffer(this, buffer, data); }
void (*_set_compute_buffer)(State *_state, ComputeBuffer * buffer, u32 slot);
void set_compute_buffer(ComputeBuffer * buffer, u32 slot) { return _set_compute_buffer(this, buffer, slot); }
void (*_set_compute_texture)(State *_state, Texture2D * texture, u32 slot);
void set_compute_texture(Texture2D * texture, u32 slot) { return _set_compute_texture(this, texture, slot); }
