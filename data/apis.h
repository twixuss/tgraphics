void set_vsync(bool enable);

void on_window_resize(u32 w, u32 h);
void present();
CameraMatrices calculate_perspective_matrices(v3f position, v3f rotation, f32 aspect_ratio, f32 fov_radians, f32 near_plane, f32 far_plane);

void set_blend(BlendFunction function, Blend source, Blend destination);
void set_topology(Topology topology);
void set_scissor(Viewport viewport);
void disable_scissor();
void set_cull(Cull cull);
void disable_blend();
void disable_depth_clip();
void enable_depth_clip();
void set_viewport(Viewport viewport);

void draw(u32 vertex_count, u32 start_vertex);
void draw_indexed(u32 index_count);

VertexBuffer *create_vertex_buffer(Span<u8> buffer, Span<ElementType> vertex_descriptor);
void set_vertex_buffer(VertexBuffer *buffer);
void update_vertex_buffer(VertexBuffer *buffer, Span<u8> data);

IndexBuffer *create_index_buffer(Span<u8> buffer, u32 index_size);
void set_index_buffer(IndexBuffer *buffer);

Texture2D *create_texture_2d(u32 width, u32 height, void const *data, Format format);
void set_texture_2d(Texture2D *texture, u32 slot);
void resize_texture_2d(Texture2D *texture, u32 w, u32 h);
void read_texture_2d(Texture2D *texture, Span<u8> data);
void update_texture_2d(Texture2D *texture, u32 width, u32 height, void *data);
void generate_mipmaps_2d(Texture2D *texture);

void set_sampler(Filtering filtering, Comparison comparison, u32 slot);

RenderTarget *create_render_target(Texture2D *color, Texture2D *depth);
void set_render_target(RenderTarget *target);
void clear(RenderTarget *render_target, ClearFlags flags, v4f color, f32 depth);

TextureCube *create_texture_cube(u32 size, void **data, Format format);
void set_texture_cube(TextureCube *texture, u32 slot);
void generate_mipmaps_cube(TextureCube *texture, GenerateCubeMipmapParams params);

Shader *create_shader(Span<utf8> source);
void set_shader(Shader *shader);

ShaderConstants *create_shader_constants(umm size);
void update_shader_constants(ShaderConstants *constants, void const *source, u32 offset, u32 size);
void *map_shader_constants(ShaderConstants *constants, Access access);
void unmap_shader_constants(ShaderConstants *constants);
void set_shader_constants(ShaderConstants *constants, u32 slot);

void set_rasterizer(RasterizerState state);
RasterizerState get_rasterizer();

ComputeShader *create_compute_shader(Span<utf8> source);
void set_compute_shader(ComputeShader *shader);
void dispatch_compute_shader(u32 x, u32 y, u32 z);

ComputeBuffer *create_compute_buffer(u32 size);
void read_compute_buffer(ComputeBuffer *buffer, void *data);
void set_compute_buffer(ComputeBuffer *buffer, u32 slot);
void set_compute_texture(Texture2D *texture, u32 slot);
