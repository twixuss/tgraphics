#pragma once
#include <tl/common.h>
#include <tl/vector.h>
#include <tl/file.h>
#include <tl/math.h>
#include <tl/console.h>
#include <tl/hash_map.h>

#define TGRAPHICS_API extern

#ifndef TGRAPHICS_TEXTURE_2D_EXTENSION
#define TGRAPHICS_TEXTURE_2D_EXTENSION ::tl::EmptyStruct
#endif

#ifndef TGRAPHICS_TEXTURE_CUBE_EXTENSION
#define TGRAPHICS_TEXTURE_CUBE_EXTENSION ::tl::EmptyStruct
#endif

using namespace tl;

namespace tgraphics {

enum GraphicsApi {
	GraphicsApi_null,
	GraphicsApi_d3d11,
	GraphicsApi_opengl,
};

struct InitInfo {
	NativeWindowHandle window = {};
	bool debug = false;
	bool check_apis = true;
};

struct Texture2D : TGRAPHICS_TEXTURE_2D_EXTENSION {
	v2u size;
};
struct RenderTarget {
	Texture2D *color;
	Texture2D *depth;
};
struct Shader {};
struct VertexBuffer {};
struct IndexBuffer {};


struct ShaderConstants {};

template <class T>
struct TypedShaderConstants {
	ShaderConstants *constants;
};


struct ComputeShader {};
struct ComputeBuffer {};

struct CameraMatrices {
	m4 mvp;
};

enum ElementType : u8 {
	Element_f32x1,
	Element_f32x2,
	Element_f32x3,
	Element_f32x4,
};

enum Format : u8 {
	Format_null,
	Format_depth,
	Format_r_f32,
	Format_rgb_u8n,
	Format_rgb_f16,
	Format_rgb_f32,
	Format_rgba_u8n,
	Format_rgba_f16,
	Format_rgba_f32,
};

enum Filtering : u8 {
	Filtering_none,    // texture will be unsamplable
	Filtering_nearest,
	Filtering_linear,
	Filtering_linear_mipmap,
};

enum Comparison : u8 {
	Comparison_none,
	Comparison_always,
	Comparison_equal,
	Comparison_less,
	Comparison_count,
};

using ClearFlags = u8;
enum : ClearFlags {
	ClearFlags_none  = 0x0,
	ClearFlags_color = 0x1,
	ClearFlags_depth = 0x2,
};

struct RasterizerState {
	u8 depth_test  : 1;
	u8 depth_write : 1;
	u8 depth_func  : log2(ceil_to_power_of_2(Comparison_count));
	RasterizerState &set_depth_test (bool       value) { return depth_test  = value, *this; }
	RasterizerState &set_depth_write(bool       value) { return depth_write = value, *this; }
	RasterizerState &set_depth_func (Comparison value) { return depth_func  = value, *this; }
};

enum BlendFunction {
	BlendFunction_add,
};

enum Blend {
	Blend_one,
	Blend_source_alpha,
	Blend_one_minus_source_alpha,
	Blend_secondary_color,
	Blend_one_minus_secondary_color,
};

enum Topology {
	Topology_triangle_list,
	Topology_line_list,
};

struct TextureCube : TGRAPHICS_TEXTURE_CUBE_EXTENSION {
};
union TextureCubePaths {
	struct {
		Span<utf8> left;
		Span<utf8> right;
		Span<utf8> top;
		Span<utf8> bottom;
		Span<utf8> front;
		Span<utf8> back;
	};
	Span<utf8> paths[6];
};

// min is bottom left
// max is top right
using Viewport = aabb<v2s>;

using Access = u8;
enum : Access {
	Access_read  = 0x1,
	Access_write = 0x2,
};

struct GenerateCubeMipmapParams {
	bool irradiance = false;
};

enum Cull : u8 {
	Cull_none,
	Cull_back,
	Cull_front,
};

struct Pixels {
	void *data;
	v2u size;
	Format format;
	void (*free)(void *data);
};

struct LoadPixelsParams {
	bool flip_y = false;
};

struct LoadTextureParams {
	bool generate_mipmaps = false;
	bool flip_y = false;
};


TGRAPHICS_API Pixels load_pixels(Span<u8> data, LoadPixelsParams params = {});

inline Pixels load_pixels(Span<utf8> path, LoadPixelsParams params = {}) {
	auto file = read_entire_file(path);
	if (!file.data) {
		print(Print_error, "Failed to read file %.\n", path);
		return {};
	}
	return load_pixels(file, params);
}

struct State {
	Allocator allocator;

	GraphicsApi api;

	RenderTarget *back_buffer;
	v2u min_texture_size;

APIS_DEFINITION;

	void draw(u32 vertex_count) { return draw(vertex_count, 0); }
	void set_viewport(u32 w, u32 h) { return set_viewport({.min = {}, .max = {(s32)w, (s32)h}}); }
	void set_viewport(v2u size) { return set_viewport({.min={}, .max=(v2s)size}); }
	void resize_render_targets(v2u size) { return resize_render_targets(size.x, size.y); }

	Texture2D *create_texture_2d(v2u size, void const *data, Format format) {
		return create_texture_2d(size.x, size.y, data, format);
	}

	void set_texture(Texture2D *texture, u32 slot) {
		return set_texture_2d(texture, slot);
	}
	void set_texture(TextureCube *texture, u32 slot) {
		return set_texture_cube(texture, slot);
	}

	void update_texture(Texture2D *texture, v2u size, void *data) {
		return update_texture_2d(texture, size.x, size.y, data);
	}

	void read_texture(Texture2D *texture, Span<u8> data) { return read_texture_2d(texture, data); }

	void generate_mipmaps(Texture2D *texture) {
		return generate_mipmaps_2d(texture);
	}
	void generate_mipmaps(TextureCube *texture, GenerateCubeMipmapParams params) {
		return generate_mipmaps_cube(texture, params);
	}
	void generate_mipmaps(TextureCube *texture) {
		return generate_mipmaps_cube(texture, {});
	}

	Texture2D *load_texture_2d(Span<u8> data, LoadTextureParams params = {}) {
		auto pixels = load_pixels(data, {.flip_y = params.flip_y});
		if (!pixels.data)
			return 0;
		defer { pixels.free(pixels.data); };

		auto result = create_texture_2d(pixels.size.x, pixels.size.y, pixels.data, pixels.format);

		if (params.generate_mipmaps)
			generate_mipmaps_2d(result);

		return result;
	}
	Texture2D *load_texture_2d(Span<utf8> path, LoadTextureParams params = {}) {
		auto file = read_entire_file(path);
		if (!file.data) {
			print(Print_error, "Failed to read file %.\n", path);
			return {};
		}
		return load_texture_2d(file, params);
	}

	TextureCube *load_texture_cube(TextureCubePaths paths, LoadTextureParams params = {}, GenerateCubeMipmapParams mipmap_params = {}) {
		Pixels pixels[6];
		void *datas[6];
		for (u32 i = 0; i < 6; ++i) {
			pixels[i] = load_pixels(paths.paths[i]);
			if (!pixels[i].data) {
				return 0;
			}

			bool fail = false;
			Span<char> reason;
			if (i == 0) {
				if (pixels[0].size.y != pixels[0].size.x) {
					fail = true;
					reason = "first face is not a square"s;
				}
			} else {
				if (any_true(pixels[i].size != pixels[0].size)) {
					fail = true;
					reason = "sizes of faces do not match"s;
				}
				if (pixels[i].format != pixels[0].format) {
					fail = true;
					reason = "formats of faces do not match"s;
				}
			}
			if (fail) {
				print(Print_error, "Failed to load cube texture (%) with these paths:\n\t%\n\t%\n\t%\n\t%\n\t%\n\t%\n"
					, reason
					, paths.paths[0]
					, paths.paths[1]
					, paths.paths[2]
					, paths.paths[3]
					, paths.paths[4]
					, paths.paths[5]
				);
				return 0;
			}

			datas[i] = pixels[i].data;
		}
		defer {
			for (u32 i = 0; i < 6; ++i) {
				pixels[i].free(pixels[i].data);
			}
		};
		auto result = create_texture_cube(pixels[0].size.x, datas, pixels[0].format);
		if (params.generate_mipmaps) {
			generate_mipmaps_cube(result, mipmap_params);
		}
		return result;
	}

	void resize_texture_2d(Texture2D *texture, v2u size) { return resize_texture_2d(texture, size.x, size.y); }
	void resize_texture(Texture2D *texture, v2u size) { return resize_texture_2d(texture, size.x, size.y); }

	void set_sampler(Filtering filtering, u32 slot) { return set_sampler(filtering, {}, slot); }

	template <class T>
	TypedShaderConstants<T> create_shader_constants() {
		TypedShaderConstants<T> result = {
			.constants = create_shader_constants(sizeof(T)),
		};
		return result;
	}

	template <class T>
	void update_shader_constants(ShaderConstants *constants, T const &source) {
		return update_shader_constants(constants, &source, 0, sizeof(source));
	}

	template <class T>
	void update_shader_constants(TypedShaderConstants<T> &constants, T const &value) {
		return update_shader_constants(constants.constants, &value, 0, sizeof(T));
	}

	template <class T>
	void set_shader_constants(TypedShaderConstants<T> const &constants, u32 slot) {
		return set_shader_constants(constants.constants, slot);
	}

	template <class T>
	T *map(TypedShaderConstants<T> const &constants, Access access) {
		return (T *)map_shader_constants(constants.constants, access);
	}

	template <class T>
	void unmap(TypedShaderConstants<T> const &constants) {
		return unmap_shader_constants(constants.constants);
	}

};

#ifndef TGRAPHICS_IMPL
#undef TGRAPHICS_APIS
#endif

TGRAPHICS_API State *init(GraphicsApi api, InitInfo init_info);
TGRAPHICS_API void free(State *state);

}

#ifdef TGRAPHICS_IMPL

namespace tgraphics {

struct SamplerKey {
	Filtering filtering;
	Comparison comparison;
	bool operator==(SamplerKey const &that) const {
		return filtering == that.filtering && comparison == that.comparison;
	}
};

}

template <>
tl::umm get_hash(tgraphics::SamplerKey key) {
	return key.filtering ^ key.comparison;
}

template <>
tl::umm get_hash(tl::Span<tgraphics::ElementType> types) {
	tl::umm hash = 0x13579BDF2468ACE;
	for (auto &type : types) {
		hash = tl::rotate_left(hash, 1) | type;
	}
	return hash;
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT assert
#include <stb_image.h>
#include <tl/console.h>
#include <tl/masked_block_list.h>
#include <tl/hash_map.h>
#include <tl/window.h>

namespace tgraphics {

namespace d3d11 { State *init(InitInfo init_info); void deinit(State *); }
namespace gl    { State *init(InitInfo init_info); void deinit(State *); }

static bool check_api(State *state) {
	bool result = true;
APIS_CHECK;
	return result;
}

State *init(GraphicsApi api, InitInfo init_info) {
	if (!init_info.window) {
		print(Print_error, "init_info.window is null\n");
		return 0;
	}

	State *result = 0;

	switch (api) {
		//case GraphicsApi_d3d11:  result = d3d11::init(init_info); break;
		case GraphicsApi_opengl: result =    gl::init(init_info); break;
	}

	result->api = api;

	if (init_info.check_apis)
		if (!check_api(result))
			return 0;

	return result;
}

void deinit(State *state) {
	switch (state->api) {
		//case GraphicsApi_d3d11:  return d3d11::deinit(state);
		case GraphicsApi_opengl: return    gl::deinit(state);
	}
	state->api = {};
}

Pixels load_pixels(Span<u8> data, LoadPixelsParams params) {
	Pixels result;

	stbi_set_flip_vertically_on_load(params.flip_y);

	int width, height;
	if (stbi_is_hdr_from_memory(data.data, data.size)) {
		result.data = stbi_loadf_from_memory(data.data, data.size, &width, &height, 0, 4);
		result.format = Format_rgba_f32;
	} else {
		result.data = stbi_load_from_memory(data.data, data.size, &width, &height, 0, 4);
		result.format = Format_rgba_u8n;
	}
	if (!result.data) {
		print(Print_error, "Failed to parse texture. Reason: %.\n", stbi_failure_reason());
		return {};
	}
	result.size = {(u32)width, (u32)height};
	result.free = stbi_image_free;
	return result;
}

}

#include <tl/opengl.h>

namespace tgraphics::gl {

using namespace tl::gl;

struct ShaderImpl : Shader {
	GLuint program;
};

struct ShaderConstantsImpl : ShaderConstants {
	GLuint uniform_buffer;
	u32 values_size;
};

struct VertexBufferImpl : VertexBuffer {
	GLuint buffer;
	GLuint array;
};

struct IndexBufferImpl : IndexBuffer {
	GLuint buffer;
	GLuint type;
	u32 count;
};

struct Texture {
	GLuint texture;
	GLuint format;
	GLuint internal_format;
	GLuint type;
	GLuint target;
	u32 bytes_per_texel;
};

struct Texture2DImpl : Texture2D, Texture {};
struct TextureCubeImpl : TextureCube, Texture {};

struct RenderTargetImpl : RenderTarget {
	GLuint frame_buffer;
};

struct ComputeShaderImpl : ComputeShader {
	GLuint program;
};
struct ComputeBufferImpl : ComputeBuffer {
	GLuint buffer;
	u32 size;
};

u32 get_element_scalar_count(ElementType element) {
	switch (element) {
		case Element_f32x1: return 1;
		case Element_f32x2:	return 2;
		case Element_f32x3:	return 3;
		case Element_f32x4:	return 4;
	}
	invalid_code_path();
	return 0;
}

u32 get_element_size(ElementType element) {
	switch (element) {
		case Element_f32x1: return 4;
		case Element_f32x2:	return 8;
		case Element_f32x3:	return 12;
		case Element_f32x4:	return 16;
	}
	invalid_code_path();
	return 0;
}

u32 get_element_type(ElementType element) {
	switch (element) {
		case Element_f32x1: return GL_FLOAT;
		case Element_f32x2:	return GL_FLOAT;
		case Element_f32x3:	return GL_FLOAT;
		case Element_f32x4:	return GL_FLOAT;
	}
	invalid_code_path();
	return 0;
}

u32 get_index_type_from_size(u32 size) {
	switch (size) {
		case 2: return GL_UNSIGNED_SHORT;
		case 4: return GL_UNSIGNED_INT;
	}
	invalid_code_path();
	return 0;
}

GLuint get_min_filter(Filtering filter) {
	switch (filter) {
		case Filtering_nearest:        return GL_NEAREST;
		case Filtering_linear:         return GL_LINEAR;
		case Filtering_linear_mipmap:  return GL_LINEAR_MIPMAP_LINEAR;
	}
	invalid_code_path();
	return 0;
}
GLuint get_mag_filter(Filtering filter) {
	switch (filter) {
		case Filtering_nearest:        return GL_NEAREST;
		case Filtering_linear:         return GL_LINEAR;
		case Filtering_linear_mipmap:  return GL_LINEAR;
	}
	invalid_code_path();
	return 0;
}
GLuint get_func(Comparison comparison) {
	switch (comparison) {
		case Comparison_none:   return GL_NONE;
		case Comparison_always: return GL_ALWAYS;
		case Comparison_equal:  return GL_EQUAL;
		case Comparison_less:   return GL_LESS;
	}
	invalid_code_path();
	return 0;
}

GLuint get_format(Format format) {
	switch (format) {
		case Format_depth:    return GL_DEPTH_COMPONENT;
		case Format_r_f32:    return GL_RED;
		case Format_rgb_u8n:  return GL_RGB;
		case Format_rgb_f16:  return GL_RGB;
		case Format_rgb_f32:  return GL_RGB;
		case Format_rgba_u8n: return GL_RGBA;
		case Format_rgba_f16: return GL_RGBA;
		case Format_rgba_f32: return GL_RGBA;
	}
	invalid_code_path();
	return 0;
}

GLuint get_internal_format(Format format) {
	switch (format) {
		case Format_depth:    return GL_DEPTH_COMPONENT;
		case Format_r_f32:    return GL_R32F;
		case Format_rgb_u8n:  return GL_RGB8;
		case Format_rgb_f16:  return GL_RGB16F;
		case Format_rgb_f32:  return GL_RGB32F;
		case Format_rgba_u8n: return GL_RGBA8;
		case Format_rgba_f16: return GL_RGBA16F;
		case Format_rgba_f32: return GL_RGBA32F;
	}
	invalid_code_path();
	return 0;
}

GLuint get_type(Format format) {
	switch (format) {
		case Format_depth:    return GL_FLOAT;
		case Format_r_f32:    return GL_FLOAT;
		case Format_rgb_u8n:  return GL_UNSIGNED_BYTE;
		case Format_rgb_f16:  return GL_FLOAT;
		case Format_rgb_f32:  return GL_FLOAT;
		case Format_rgba_u8n: return GL_UNSIGNED_BYTE;
		case Format_rgba_f16: return GL_FLOAT;
		case Format_rgba_f32: return GL_FLOAT;
	}
	invalid_code_path();
	return 0;
}
u32 get_bytes_per_texel(Format format) {
	switch (format) {
		case Format_depth:    return 4;
		case Format_r_f32:    return 4;
		case Format_rgb_u8n:  return 3;
		case Format_rgb_f16:  return 6;
		case Format_rgb_f32:  return 12;
		case Format_rgba_u8n: return 4;
		case Format_rgba_f16: return 8;
		case Format_rgba_f32: return 16;
	}
	invalid_code_path();
	return 0;
}

GLuint get_blend(Blend blend) {
	switch (blend) {
		case Blend_one:                       return GL_ONE;
		case Blend_source_alpha:              return GL_SRC_ALPHA;
		case Blend_one_minus_source_alpha:    return GL_ONE_MINUS_SRC_ALPHA;
		case Blend_secondary_color:           return GL_SRC1_COLOR;
		case Blend_one_minus_secondary_color: return GL_ONE_MINUS_SRC1_COLOR;
	}
	invalid_code_path();
	return 0;
}

GLuint get_equation(BlendFunction function) {
	switch (function) {
		case BlendFunction_add: return GL_FUNC_ADD;
	}
	invalid_code_path();
	return 0;
}

GLuint get_topology(Topology topology) {
	switch (topology) {
		case Topology_triangle_list: return GL_TRIANGLES;
		case Topology_line_list:     return GL_LINES;
	}
	invalid_code_path();
	return 0;
}

GLuint get_access(Access access) {
	switch (access) {
		case Access_read: return GL_READ_ONLY;
		case Access_write: return GL_WRITE_ONLY;
		case Access_write | Access_read: return GL_READ_WRITE;
	}
	invalid_code_path();
	return 0;
}

GLenum get_cull(Cull cull) {
	switch (cull) {
		case Cull_back:	 return GL_BACK;
		case Cull_front: return GL_FRONT;
	}
	invalid_code_path();
	return 0;
}

void resize_texture_gl(Texture2D *_texture, u32 width, u32 height) {
	auto &texture = *(Texture2DImpl *)_texture;
	texture.size = {width, height};
	glBindTexture(texture.target, texture.texture);
	glTexImage2D(texture.target, 0, texture.internal_format, width, height, 0, texture.format, texture.type, NULL);
	glBindTexture(texture.target, 0);
}

struct StateGL : State {
	MaskedBlockList<ShaderImpl, 256> shaders;
	MaskedBlockList<VertexBufferImpl, 256> vertex_buffers;
	MaskedBlockList<IndexBufferImpl, 256> index_buffers;
	MaskedBlockList<RenderTargetImpl, 256> render_targets;
	MaskedBlockList<Texture2DImpl, 256> textures_2d;
	MaskedBlockList<TextureCubeImpl, 256> textures_cube;
	MaskedBlockList<ShaderConstantsImpl, 256> shader_constants;
	MaskedBlockList<ComputeShaderImpl, 256> compute_shaders;
	MaskedBlockList<ComputeBufferImpl, 256> compute_buffers;
	StaticHashMap<SamplerKey, GLuint, 256> samplers;
	IndexBufferImpl *current_index_buffer;
	RenderTargetImpl back_buffer;
	Texture2DImpl back_buffer_color;
	Texture2DImpl back_buffer_depth;
	RenderTargetImpl *currently_bound_render_target;
	RasterizerState current_rasterizer;
	BlendFunction current_blend_function;
	Blend current_blend_source;
	Blend current_blend_destination;
	GLuint current_topology = GL_TRIANGLES;
	Cull current_cull = Cull_back;
	bool scissor_enabled = false;
	bool blend_enabled = false;
	bool depth_clip_enabled = true;

	auto impl_clear(RenderTarget *_render_target, ClearFlags flags, v4f color, f32 depth) {
		assert(_render_target);
		auto &render_target = *(RenderTargetImpl *)_render_target;

		auto previously_bound_render_target = currently_bound_render_target;
		bind_render_target(render_target);

		GLbitfield mask = 0;
		if (flags & ClearFlags_color) { mask |= GL_COLOR_BUFFER_BIT; glClearColor(color.x, color.y, color.z, color.w); }
		if (flags & ClearFlags_depth) { mask |= GL_DEPTH_BUFFER_BIT; glClearDepth(depth); }
		glClear(mask);

		if (previously_bound_render_target) {
			bind_render_target(*previously_bound_render_target);
		}
	}
	auto impl_present() {
		gl::present();
	}
	auto impl_draw(u32 vertex_count, u32 start_vertex) {
		assert(vertex_count, "tgraphics::draw called with 0 vertices");
		glDrawArrays(current_topology, start_vertex, vertex_count);
	}
	auto impl_draw_indexed(u32 index_count) {
		assert(current_index_buffer, "Index buffer was not bound");
		glDrawElements(current_topology, index_count, current_index_buffer->type, 0);
	}
	auto impl_set_viewport(Viewport viewport) {
		assert(viewport.max.x - viewport.min.x > 0);
		assert(viewport.max.y - viewport.min.y > 0);
		glViewport(viewport.min.x, viewport.min.y, viewport.size().x, viewport.size().y);
	}
	auto impl_resize_render_targets(u32 width, u32 height) {
		back_buffer_color.size = back_buffer_depth.size = {width, height};
	}
	auto impl_set_shader(Shader *_shader) {
		assert(_shader);
		auto &shader = *(ShaderImpl *)_shader;
		glUseProgram(shader.program);
	}
	auto impl_set_shader_constants(ShaderConstants *_constants, u32 slot) {
		assert(_constants);
		auto &constants = *(ShaderConstantsImpl *)_constants;
		glBindBuffer(GL_UNIFORM_BUFFER, constants.uniform_buffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, constants.uniform_buffer);
	}
	auto impl_update_shader_constants(ShaderConstants *_constants, void const *source, u32 offset, u32 size) {
		assert(_constants);
		auto &constants = *(ShaderConstantsImpl *)_constants;
		glBindBuffer(GL_UNIFORM_BUFFER, constants.uniform_buffer);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, source);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	auto impl_create_shader(Span<utf8> source) -> Shader * {
		auto &shader = shaders.add();
		auto vertex   = tl::gl::create_shader(GL_VERTEX_SHADER, 430, true, (Span<char>)source);
		auto fragment = tl::gl::create_shader(GL_FRAGMENT_SHADER, 430, true, (Span<char>)source);
		assert(vertex);
		assert(fragment);
		shader.program = create_program({
			.vertex = vertex,
			.fragment = fragment,
		});
		assert(shader.program);
		return &shader;
	}
	auto impl_create_shader_constants(umm size) -> ShaderConstants * {
		auto &constants = shader_constants.add();
		glGenBuffers(1, &constants.uniform_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, constants.uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		constants.values_size = size;
		return &constants;
	}
	auto impl_calculate_perspective_matrices(v3f position, v3f rotation, f32 aspect_ratio, f32 fov, f32 near_plane, f32 far_plane) {
		CameraMatrices result;
		result.mvp = m4::perspective_right_handed(aspect_ratio, fov, near_plane, far_plane)
				   * m4::rotation_r_yxz(-rotation)
			       * m4::translation(-position);
		return result;
	}
	auto impl_create_vertex_buffer(Span<u8> buffer, Span<ElementType> vertex_descriptor) -> VertexBuffer * {
		VertexBufferImpl &result = vertex_buffers.add();
		glGenBuffers(1, &result.buffer);
		glGenVertexArrays(1, &result.array);

		glBindVertexArray(result.array);

		glBindBuffer(GL_ARRAY_BUFFER, result.buffer);
		glBufferData(GL_ARRAY_BUFFER, buffer.size, buffer.data, GL_STATIC_DRAW);

		u32 stride = 0;
		for (auto &element : vertex_descriptor) {
			stride += get_element_size(element);
		}

		u32 offset = 0;
		for (u32 element_index = 0; element_index < vertex_descriptor.size; ++element_index) {
			auto &element = vertex_descriptor[element_index];
			glVertexAttribPointer(element_index, get_element_scalar_count(element), get_element_type(element), false, stride, (void const *)offset);
			glEnableVertexAttribArray(element_index);
			offset += get_element_size(element);
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return &result;
	}
	auto impl_set_vertex_buffer(VertexBuffer *_buffer) {
		auto buffer = (VertexBufferImpl *)_buffer;
		glBindVertexArray(buffer ? buffer->array : 0);
	}
	auto impl_create_index_buffer(Span<u8> buffer, u32 index_size) -> IndexBuffer * {
		IndexBufferImpl &result = index_buffers.add();
		result.type = get_index_type_from_size(index_size);
		result.count = buffer.size / index_size;

		glGenBuffers(1, &result.buffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size, buffer.data, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return &result;
	}
	auto impl_set_index_buffer(IndexBuffer *_buffer) {
		auto buffer = (IndexBufferImpl *)_buffer;
		current_index_buffer = buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer ? buffer->buffer : 0);
	}
	auto impl_set_vsync(bool enable) {
		wglSwapIntervalEXT(enable);
	}
	auto impl_set_render_target(RenderTarget *_render_target) {
		assert(_render_target);
		auto &render_target = *(RenderTargetImpl *)_render_target;
		bind_render_target(render_target);
	}
	auto impl_create_render_target(Texture2D *_color, Texture2D *_depth) -> RenderTarget * {
		assert(_color || _depth);
		auto color = (Texture2DImpl *)_color;
		auto depth = (Texture2DImpl *)_depth;

		auto &result = render_targets.add();

		result.color = color;
		result.depth = depth;

		glGenFramebuffers(1, &result.frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, result.frame_buffer);
		if (depth) {
			if (!color) {
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->texture, 0);
		}
		if (color) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->texture, 0);
		}

		switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
#define C(x) case x: print(#x "\n"); invalid_code_path(); break;
			C(GL_FRAMEBUFFER_UNDEFINED)
			C(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
			C(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
			C(GL_FRAMEBUFFER_UNSUPPORTED)
			C(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
#undef C
			case GL_FRAMEBUFFER_COMPLETE: break;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return &result;
	}
	auto impl_set_sampler(Filtering filtering, Comparison comparison, u32 slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindSampler(slot, get_sampler(filtering, comparison));
	}
	auto impl_set_texture_2d(Texture2D *_texture, u32 slot) {
		auto &texture = *(Texture2DImpl *)_texture;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(texture.target, _texture ? texture.texture : 0);
	}
	auto impl_set_texture_cube(TextureCube *_texture, u32 slot) {
		auto &texture = *(TextureCubeImpl *)_texture;
		glActiveTexture(GL_TEXTURE0 + slot);
		if (_texture) {
			glBindTexture(texture.target, texture.texture);
		} else {
			glBindTexture(texture.target, 0);
		}
	}
	auto impl_create_texture_2d(u32 width, u32 height, void const *data, Format format) -> Texture2D * {
		auto &result = textures_2d.add();

		result.size = {width, height};

		result.internal_format = get_internal_format(format);
		result.format          = get_format(format);
		result.type            = get_type(format);
		result.bytes_per_texel = get_bytes_per_texel(format);
		result.target = GL_TEXTURE_2D;

		glGenTextures(1, &result.texture);
		glBindTexture(GL_TEXTURE_2D, result.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, result.internal_format, width, height, 0, result.format, result.type, data);
		glTexParameteri(GL_TEXTURE_2D,  GL_TEXTURE_MAX_LEVEL, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return &result;
	}
	auto impl_set_rasterizer(RasterizerState rasterizer) {
		if (current_rasterizer.depth_test != rasterizer.depth_test) {
			if (rasterizer.depth_test) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
		}

		if (rasterizer.depth_test) {
			if (current_rasterizer.depth_func != rasterizer.depth_func) {
				glDepthFunc(get_func((Comparison)rasterizer.depth_func));
			}
		}

		//if (current_rasterizer.depth_write != rasterizer.depth_write) {
		//	glDepthMask(rasterizer.depth_write);
		//}

		current_rasterizer = rasterizer;
	}
	auto impl_get_rasterizer() -> RasterizerState {
		return current_rasterizer;
	}
	auto impl_create_compute_shader(Span<utf8> source) -> ComputeShader * {
		auto &result = compute_shaders.add();
		result.program = create_program({
			.compute = tl::gl::create_shader(GL_COMPUTE_SHADER, 430, true, (Span<char>)source),
		});
		return &result;
	}
	auto impl_set_compute_shader(ComputeShader *_shader) {
		assert(_shader);
		auto &shader = *(ComputeShaderImpl *)_shader;
		glUseProgram(shader.program);
	}
	auto impl_dispatch_compute_shader(u32 x, u32 y, u32 z) {
		glDispatchCompute(x, y, z);
	}
	auto impl_resize_texture_2d(Texture2D *texture, u32 width, u32 height) { resize_texture_gl(texture, width, height); }
	auto impl_create_compute_buffer(u32 size) -> ComputeBuffer * {
		auto &result = compute_buffers.add();
		result.size = size;
		glGenBuffers(1, &result.buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, result.buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, 0, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, result.buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return &result;
	}
	auto impl_set_compute_buffer(ComputeBuffer *_buffer, u32 slot) {
		assert(_buffer);
		auto &buffer = *(ComputeBufferImpl *)_buffer;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer.buffer);
	}
	auto impl_read_compute_buffer(ComputeBuffer *_buffer, void *data) {
		assert(_buffer);
		auto &buffer = *(ComputeBufferImpl *)_buffer;

		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.buffer);
		void* resultData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer.size, GL_MAP_READ_BIT);
		memcpy(data, resultData, buffer.size);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	auto impl_set_compute_texture(Texture2D *_texture, u32 slot) {
		assert(_texture);
		auto &texture = *(Texture2DImpl *)_texture;
		glBindImageTexture(slot, texture.texture, 0, GL_FALSE, 0, GL_READ_ONLY, texture.internal_format);
	}
	auto impl_read_texture_2d(Texture2D *_texture, Span<u8> data) {
		assert(_texture);
		auto &texture = *(Texture2DImpl *)_texture;
		glGetTextureImage(texture.texture, 0, texture.format, texture.type, data.size, data.data);
	}
	auto impl_set_blend(BlendFunction function, Blend source, Blend destination) {
		if (!blend_enabled) {
			blend_enabled = true;
			glEnable(GL_BLEND);
		}

		if (function != current_blend_function) {
			current_blend_function = function;
			glBlendEquation(get_equation(function));
		}
		if (source != current_blend_source || destination != current_blend_destination) {
			current_blend_source = source;
			current_blend_destination = destination;
			glBlendFunc(get_blend(source), get_blend(destination));
		}
	}
	auto impl_disable_blend() {
		if (blend_enabled) {
			blend_enabled = false;
			glDisable(GL_BLEND);
		}
	}
	auto impl_disable_depth_clip() {
		if ( depth_clip_enabled) { depth_clip_enabled = false; glEnable (GL_DEPTH_CLAMP); }
	}
	auto impl_enable_depth_clip () {
		if (!depth_clip_enabled) { depth_clip_enabled = true ; glDisable(GL_DEPTH_CLAMP); }
	}
	auto impl_create_texture_cube(u32 size, void *data[6], Format format) -> TextureCube * {
		auto &result = textures_cube.add();

		// result.size = {width, height}

		result.internal_format = get_internal_format(format);
		result.format          = get_format(format);
		result.type            = get_type(format);
		result.bytes_per_texel = get_bytes_per_texel(format);
		result.target          = GL_TEXTURE_CUBE_MAP;

		glGenTextures(1, &result.texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, result.texture);
		for (u32 i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, result.internal_format, size, size, 0, result.format, result.type, data[i]);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return &result;
	}
	auto impl_set_topology(Topology topology) {
		current_topology = get_topology(topology);
	}
	auto impl_update_vertex_buffer(VertexBuffer *_buffer, Span<u8> data) {
		auto &buffer = *(VertexBufferImpl *)_buffer;
		glBindBuffer(GL_ARRAY_BUFFER, buffer.buffer);
		glBufferData(GL_ARRAY_BUFFER, data.size, data.data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	auto impl_update_texture_2d(Texture2D *_texture, u32 width, u32 height, void *data) {
		auto &texture = *(Texture2DImpl *)_texture;
		glBindTexture(texture.target, texture.texture);
		glTexImage2D(texture.target, 0, texture.internal_format, width, height, 0, texture.format, texture.type, data);
		glBindTexture(texture.target, 0);
	}
	auto impl_generate_mipmaps_2d(Texture2D *_texture) {
		assert(_texture);
		auto &texture = *(Texture2DImpl *)_texture;
		glGenerateTextureMipmap(texture.texture);
	}
	auto impl_generate_mipmaps_cube(TextureCube *_texture, GenerateCubeMipmapParams params) {
		assert(_texture);
		auto &texture = *(TextureCubeImpl *)_texture;
		glGenerateTextureMipmap(texture.texture);
	}
	auto impl_set_scissor(Viewport viewport) {
		if (!scissor_enabled) {
			scissor_enabled = true;
			glEnable(GL_SCISSOR_TEST);
		}
		assert(viewport.max.x - viewport.min.x > 0);
		assert(viewport.max.y - viewport.min.y > 0);
		if (!scissor_enabled) {
			print(Print_warning, "tgraphics::set_scissor was called when scessor test is not enabled\n");
		}
		glScissor(viewport.min.x, viewport.min.y, viewport.size().x, viewport.size().y);
	}
	auto impl_disable_scissor() {
		if (scissor_enabled) {
			scissor_enabled = false;
			glDisable(GL_SCISSOR_TEST);
		}
	}
	auto impl_map_shader_constants(ShaderConstants *_constants, Access access) {
		assert(_constants);
		auto &constants = *(ShaderConstantsImpl *)_constants;
		return glMapNamedBuffer(constants.uniform_buffer, get_access(access));
	}
	auto impl_unmap_shader_constants(ShaderConstants *_constants) {
		assert(_constants);
		auto &constants = *(ShaderConstantsImpl *)_constants;
		glUnmapNamedBuffer(constants.uniform_buffer);
	}
	auto impl_set_cull(Cull cull) {
		if (current_cull != cull) {
			current_cull = cull;

			if (cull == Cull_none) {
				glDisable(GL_CULL_FACE);
			} else {
				if (cull == Cull_none) {
					glEnable(GL_CULL_FACE);
				}
				glCullFace(get_cull(cull));
			}
		}
	}


	void bind_render_target(RenderTargetImpl &render_target) {
		if (&render_target == currently_bound_render_target)
			return;

		currently_bound_render_target = &render_target;
		glBindFramebuffer(GL_FRAMEBUFFER, render_target.frame_buffer);
	}

	GLuint get_sampler(Filtering filtering, Comparison comparison) {
		if (filtering == Filtering_none)
			return 0;

		auto &result = samplers.get_or_insert({filtering, comparison});
		if (!result) {
			glGenSamplers(1, &result);
			if (comparison != Comparison_none) {
				glSamplerParameteri(result, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				auto func = get_func(comparison);
				glSamplerParameteri(result, GL_TEXTURE_COMPARE_FUNC, func);
			}

			glSamplerParameteri(result, GL_TEXTURE_MIN_FILTER, get_min_filter(filtering));
			glSamplerParameteri(result, GL_TEXTURE_MAG_FILTER, get_mag_filter(filtering));
			glSamplerParameteri(result, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glSamplerParameteri(result, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		return result;
	}

};

State *init(InitInfo init_info) {
	if (!init_opengl(init_info.window, init_info.debug)) {
		return 0;
	}

	auto allocator = current_allocator;

	auto state = allocator.allocate<StateGL>();
	state->allocator = allocator;

	((State *)state)->back_buffer        = &state->back_buffer;
	((State *)state)->back_buffer->color = &state->back_buffer_color;
	((State *)state)->back_buffer->depth = &state->back_buffer_depth;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

APIS_REMAP;

	return state;
}

void deinit(State *state) {
	/*
	MaskedBlockList<ShaderImpl, 256> shaders;
	MaskedBlockList<VertexBufferImpl, 256> vertex_buffers;
	MaskedBlockList<IndexBufferImpl, 256> index_buffers;
	MaskedBlockList<RenderTargetImpl, 256> render_targets;
	MaskedBlockList<Texture2DImpl, 256> textures;
	MaskedBlockList<ShaderConstantsImpl, 256> shader_constants;
	MaskedBlockList<ComputeShaderImpl, 256> compute_shaders;
	MaskedBlockList<ComputeBufferImpl, 256> compute_buffers;
	IndexBufferImpl *current_index_buffer;
	RenderTargetImpl back_buffer;
	Texture2DImpl back_buffer_color;
	Texture2DImpl back_buffer_depth;
	RenderTargetImpl *currently_bound_render_target;
	StaticHashMap<SamplerKey, GLuint, 256> samplers;
	RasterizerState rasterizer;
	BlendFunction blend_function;
	Blend blend_source;
	Blend blend_destination;
	*/
	//free(state->shaders);
	//free(state->vertex_buffers);
	//free(state->index_buffers);
	//free(state->render_targets);
	//free(state->textures_2d);
	//free(state->shader_constants);
	//free(state->compute_shaders);
	//free(state->compute_buffers);
	//free(state->samplers);
}

}

#if 0
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")

namespace tgraphics::d3d11 {

template <class T, class U>
T *QueryInterface(U *u) {
	T *t = 0;
	u->QueryInterface(&t);
	return t;
}

struct ShaderImpl : Shader {
	ID3D11VertexShader *vertex_shader = 0;
	ID3D11PixelShader *pixel_shader = 0;
	ID3D11Buffer *constant_buffer = 0;
	void *constant_buffer_data = 0;
};

struct InputLayout {
	ID3D11InputLayout *layout = 0;
	u32 stride = 0;
};

struct VertexBufferImpl : VertexBuffer {
	ID3D11Buffer *buffer = 0;
	InputLayout *layout = 0;
};

struct IndexBufferImpl : IndexBuffer {
	ID3D11Buffer *buffer = 0;
	DXGI_FORMAT format = {};
	u32 count = 0;
};

struct RenderTargetImpl : RenderTarget {
	ID3D11RenderTargetView *color_target = 0;
	ID3D11DepthStencilView *depth_target = 0;
};

struct ShaderConstantsImpl : ShaderConstants {
	ID3D11Buffer *buffer = 0;
};

struct Texture2DImpl : Texture2D {
	ID3D11Texture2D *texture = 0;
};

struct State {
	ID3D11Device *device = 0;
	ID3D11DeviceContext *immediate_context = 0;
	RenderTargetImpl back_buffer;
	Texture2DImpl back_buffer_color;
	Texture2DImpl back_buffer_depth;
	IDXGISwapChain *swap_chain = 0;
	UINT sync_interval = 1;
	MaskedBlockList<ShaderImpl, 256> shaders;
	MaskedBlockList<VertexBufferImpl, 256> vertex_buffers;
	MaskedBlockList<IndexBufferImpl, 256> index_buffers;
	MaskedBlockList<RenderTargetImpl, 256> render_targets;
	MaskedBlockList<ShaderConstantsImpl, 256> shader_constants;
	MaskedBlockList<Texture2DImpl, 256> textures_2d;
	LinearSet<RenderTargetImpl *> render_targets_resized_with_window;
	StaticHashMap<Span<ElementType>, InputLayout, 256> input_layouts;
};

static State state;

bool create_back_buffer(u32 width, u32 height) {
	ID3D11Texture2D *back_buffer_texture = 0;
	if (FAILED(state.swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer_texture)))) {
		return false;
	}

	if (FAILED(state.device->CreateRenderTargetView(back_buffer_texture, 0, &state.back_buffer.color_target))) {
		return false;
	}
	back_buffer_texture->Release();

	{
		auto format = DXGI_FORMAT_D32_FLOAT;
		ID3D11Texture2D *depth_texture = 0;
		{
			D3D11_TEXTURE2D_DESC desc = {
				.Width = width,
				.Height = height,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = format,
				.SampleDesc = {1, 0},
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			};
			assert(SUCCEEDED(state.device->CreateTexture2D(&desc, 0, &depth_texture)));
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC desc = {
			.Format = format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		};
		assert(SUCCEEDED(state.device->CreateDepthStencilView(depth_texture, &desc, &state.back_buffer.depth_target)));
	}

	state.immediate_context->OMSetRenderTargets(1, &state.back_buffer.color_target, state.back_buffer.depth_target);
	return true;
}

DXGI_FORMAT get_element_format(ElementType type) {
	switch (type) {
		case Element_f32x1: return DXGI_FORMAT_R32_FLOAT;
		case Element_f32x2: return DXGI_FORMAT_R32G32_FLOAT;
		case Element_f32x3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case Element_f32x4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	invalid_code_path();
	return DXGI_FORMAT_UNKNOWN;
}

u32 get_element_size(ElementType type) {
	switch (type) {
		case Element_f32x1: return 4;
		case Element_f32x2: return 8;
		case Element_f32x3: return 12;
		case Element_f32x4: return 16;
	}
	invalid_code_path();
	return 0;
}

DXGI_FORMAT get_index_format_from_size(u32 size) {
	switch (size) {
		case 2: return DXGI_FORMAT_R16_UINT;
		case 4: return DXGI_FORMAT_R32_UINT;
	}
	invalid_code_path();
	return DXGI_FORMAT_UNKNOWN;
}


char const *get_hlsl_type(ElementType type) {
	switch (type) {
		case Element_f32x1: return "float";
		case Element_f32x2: return "float2";
		case Element_f32x3: return "float3";
		case Element_f32x4: return "float4";
	}
	invalid_code_path();
	return 0;
}

ID3DBlob *compile_shader(Span<utf8> source, char const *name, char const *entry, char const *profile) {
	ID3DBlob *bytecode = 0;
	ID3DBlob *messages = 0;
	D3DCompile(source.data, source.size, name, 0, 0, entry, profile, 0, 0, &bytecode, &messages);
	if (messages) {
		print(Span((char *)messages->GetBufferPointer(), messages->GetBufferSize()));
		messages->Release();
		messages = 0;
	}
	return bytecode;
}

InputLayout *get_input_layout(Span<ElementType> vertex_descriptor) {
	auto &result = state.input_layouts.get_or_insert(vertex_descriptor);

	if (!result.layout) {
		List<D3D11_INPUT_ELEMENT_DESC> element_descs;
		element_descs.allocator = temporary_allocator;

		StringBuilder shader_builder;
		shader_builder.allocator = temporary_allocator;
		append(shader_builder, "struct Input{");

		for (u32 element_index = 0; element_index < vertex_descriptor.size; ++element_index) {
			auto &type = vertex_descriptor[element_index];
			auto semantic = (char)('A' + element_index);
			D3D11_INPUT_ELEMENT_DESC element = {
				.SemanticName = tformat("%%", semantic, '\0').data,
				.Format = get_element_format(type),
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			};
			element_descs.add(element);
			result.stride += get_element_size(type);
			append_format(shader_builder, "% field%:%;", get_hlsl_type(type), element_index, semantic);
		}
		append(shader_builder, "};float4 main(in Input i):SV_Position{return 0;}");

		ID3DBlob *vertex_shader_bytecode = compile_shader((List<utf8>)to_string(shader_builder), "input_layout", "main", "vs_5_0");

		assert(SUCCEEDED(state.device->CreateInputLayout(element_descs.data, vertex_descriptor.size, vertex_shader_bytecode->GetBufferPointer(), vertex_shader_bytecode->GetBufferSize(), &result.layout)));
	}

	return &result;
}

DXGI_FORMAT get_format(Format format) {
	switch (format) {
		case tgraphics::Format_r_f32: return DXGI_FORMAT_R32_FLOAT;
	}
	invalid_code_path();
	return DXGI_FORMAT_UNKNOWN;
}

bool init(InitInfo init_info) {
	IDXGIFactory *factory = 0;
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		return false;
	}
	defer {
		factory->Release();
	};

	IDXGIAdapter *adapter = 0;
	for (UINT adapter_index = 0; SUCCEEDED(factory->EnumAdapters(adapter_index, &adapter)); ++adapter_index) {
		if (auto adapter1 = QueryInterface<IDXGIAdapter1>(adapter)) {
			DXGI_ADAPTER_DESC1 desc;
			if (FAILED(adapter1->GetDesc1(&desc))) {
				continue;
			}

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
				continue;
			} else {
				break;
			}
		}
	}

	if (!adapter) {
		return false;
	}

	if (FAILED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, init_info.debug ? D3D11_CREATE_DEVICE_DEBUG : 0, 0, 0, D3D11_SDK_VERSION, &state.device, 0, &state.immediate_context))) {
		return false;
	}

	auto client_size = get_client_size(init_info.window);

	DXGI_SWAP_CHAIN_DESC desc = {
		.BufferDesc = {
			.Width = client_size.x,
			.Height = client_size.y,
			.RefreshRate = {
				.Numerator = 1,
				.Denominator = 60,
			},
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		},
		.SampleDesc = {.Count = 1, .Quality = 0},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 1,
		.OutputWindow = (HWND)init_info.window,
		.Windowed = true,
	};

	if (FAILED(factory->CreateSwapChain(state.device, &desc, &state.swap_chain))) {
		return false;
	}

	if (!create_back_buffer(client_size.x, client_size.y)) {
		return false;
	}

	back_buffer = &state.back_buffer;
	state.back_buffer.color = &state.back_buffer_color;
	state.back_buffer.depth = &state.back_buffer_depth;

	state.immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11RasterizerState *rasterizer_state = 0;
	{
		D3D11_RASTERIZER_DESC desc = {
			.FillMode = D3D11_FILL_SOLID,
			.CullMode = D3D11_CULL_FRONT,
			.AntialiasedLineEnable = false,
		};
		if (FAILED(state.device->CreateRasterizerState(&desc, &rasterizer_state))) {
			return false;
		}
	}
	state.immediate_context->RSSetState(rasterizer_state);

	ID3D11DepthStencilState *depth_state = 0;
	{
		D3D11_DEPTH_STENCIL_DESC desc = {
			.DepthEnable = false,
			.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
			.DepthFunc = D3D11_COMPARISON_LESS,
		};
		if (FAILED(state.device->CreateDepthStencilState(&desc, &depth_state))) {
			return false;
		}
	}
	state.immediate_context->OMSetDepthStencilState(depth_state, 0);

	_clear = [](RenderTarget *_render_target, ClearFlags flags, v4f color, f32 depth) {
		auto render_target = (RenderTargetImpl *)_render_target;
		if (!render_target) {
			render_target = &state.back_buffer;
		}
		if (flags & ClearFlags_color) state.immediate_context->ClearRenderTargetView(render_target->color_target, color.s);
		if (flags & ClearFlags_depth) state.immediate_context->ClearDepthStencilView(render_target->depth_target, D3D11_CLEAR_DEPTH, depth, 0);
	};
	_present = []() {
		assert(SUCCEEDED(state.swap_chain->Present(state.sync_interval, 0)));
	};
	_draw = [](u32 vertex_count, u32 start_vertex) {
		state.immediate_context->Draw(vertex_count, start_vertex);
	};
	_draw_indexed = [](u32 index_count) {
		state.immediate_context->DrawIndexed(index_count, 0, 0);
	};
	_set_viewport = [](Viewport viewport) {
		D3D11_VIEWPORT v = {
			.TopLeftX = (FLOAT)viewport.min.x,
			.TopLeftY = (FLOAT)viewport.min.y,
			.Width = (FLOAT)viewport.size().x,
			.Height = (FLOAT)viewport.size().y,
			.MinDepth = 0,
			.MaxDepth = 1,
		};
		state.immediate_context->RSSetViewports(1, &v);
	};
	_resize_render_targets = [](u32 width, u32 height) {
		state.back_buffer.color_target->Release();
		state.back_buffer.depth_target->Release();
		assert(SUCCEEDED(state.swap_chain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0)));
		create_back_buffer(width, height);
	};
	_set_shader = [](Shader *_shader) {
		auto &shader = *(ShaderImpl *)_shader;
		state.immediate_context->VSSetShader(shader.vertex_shader, 0, 0);
		state.immediate_context->VSSetConstantBuffers(0, 1, &shader.constant_buffer);

		state.immediate_context->PSSetShader(shader.pixel_shader, 0, 0);
		state.immediate_context->PSSetConstantBuffers(0, 1, &shader.constant_buffer);
	};
	//_set_value = [](Shader *_shader, ShaderValueLocation dest, void const *source) {
	//	auto &shader = *(ShaderImpl *)_shader;
	//	memcpy((u8 *)shader.constant_buffer_data + dest.start, source, dest.size);
	//	state.immediate_context->UpdateSubresource(shader.constant_buffer, 0, 0, shader.constant_buffer_data, 0, 0);
	//};
	_create_shader = [](Span<utf8> source) -> Shader * {
		auto &shader = state.shaders.add();
		if (&shader) {
			ID3DBlob *vertex_shader_bytecode = compile_shader(source, "vertex_shader", "vertex_main", "vs_5_0");
			if (FAILED(state.device->CreateVertexShader(vertex_shader_bytecode->GetBufferPointer(), vertex_shader_bytecode->GetBufferSize(), 0, &shader.vertex_shader))) {
				return false;
			}

			ID3DBlob *pixel_shader_bytecode = compile_shader(source, "pixel_shader", "pixel_main", "ps_5_0");
			if (FAILED(state.device->CreatePixelShader(pixel_shader_bytecode->GetBufferPointer(), pixel_shader_bytecode->GetBufferSize(), 0, &shader.pixel_shader))) {
				return false;
			}

			//{
			//	D3D11_BUFFER_DESC desc = {
			//		.ByteWidth = (UINT)values_size,
			//		.Usage = D3D11_USAGE_DEFAULT,
			//		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			//	};
			//	if (FAILED(state.device->CreateBuffer(&desc, 0, &shader.constant_buffer))) {
			//		return false;
			//	}
			//
			//	shader.constant_buffer_data = default_allocator.allocate(values_size, 16);
			//}
		}
		return &shader;
	};
	_calculate_perspective_matrices = [](v3f position, v3f rotation, f32 aspect_ratio, f32 fov, f32 near_plane, f32 far_plane) {
		CameraMatrices result;
		result.mvp = m4::perspective_left_handed(aspect_ratio, fov, near_plane, far_plane)
				    * m4::rotation_r_yxz(rotation)
					* m4::translation(-position);
		return result;
	};
	_create_shader_constants = [](umm size) -> ShaderConstants * {
		auto &result = state.shader_constants.add();

		D3D11_BUFFER_DESC desc = {
			.ByteWidth = (UINT)size,
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		};

		assert(SUCCEEDED(state.device->CreateBuffer(&desc, 0, &result.buffer)));

		return &result;
	};
	_set_shader_constants = [](ShaderConstants *_constants, u32 slot) {
		assert(_constants);
		auto &constants = *(ShaderConstantsImpl *)_constants;
		state.immediate_context->VSSetConstantBuffers(slot, 1, &constants.buffer);
		state.immediate_context->PSSetConstantBuffers(slot, 1, &constants.buffer);
	};
	/*
	_create_texture_2d = [](u32 width, u32 height, void *data, Format format, Filtering filtering, Comparison comparison) -> Texture2D * {
		auto &result = state.textures_2d.add();

		result.size = {width, height};

		state.device->CreateTexture2D(&desc, &initial_data, &result.texture);

		result.internal_format = get_internal_format(format);
		result.format          = get_format(format);
		result.type            = get_type(format);
		result.bytes_per_texel = get_bytes_per_texel(format);
		result.target = GL_TEXTURE_2D;

		glGenTextures(1, &result.texture);
		glBindTexture(GL_TEXTURE_2D, result.texture);
		glTexImage2D(GL_TEXTURE_2D, 0, result.internal_format, width, height, 0, result.format, result.type, data);
		glTexParameteri(GL_TEXTURE_2D,  GL_TEXTURE_MAX_LEVEL, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		result.sampler = get_sampler(filtering, comparison);

		return &result;
	};
	*/
	_create_vertex_buffer = [](Span<u8> buffer, Span<ElementType> vertex_descriptor) -> VertexBuffer * {
		VertexBufferImpl &result = state.vertex_buffers.add();

		result.layout = get_input_layout(vertex_descriptor);

		D3D11_BUFFER_DESC desc = {
			.ByteWidth = (UINT)buffer.size,
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.StructureByteStride = result.layout->stride,
		};

		D3D11_SUBRESOURCE_DATA initial_data = {
			.pSysMem = buffer.data,
		};

		assert(SUCCEEDED(state.device->CreateBuffer(&desc, &initial_data, &result.buffer)));

		return &result;
	};
	_set_vertex_buffer = [](VertexBuffer *_buffer) {
		auto buffer = (VertexBufferImpl *)_buffer;
		UINT offset = 0;
		state.immediate_context->IASetVertexBuffers(0, 1, &buffer->buffer, &buffer->layout->stride, &offset);
		state.immediate_context->IASetInputLayout(buffer->layout->layout);
	};
	_create_index_buffer = [](Span<u8> buffer, u32 index_size) -> IndexBuffer * {
		IndexBufferImpl &result = state.index_buffers.add();

		result.format = get_index_format_from_size(index_size);

		D3D11_BUFFER_DESC desc = {
			.ByteWidth = (UINT)buffer.size,
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
		};

		D3D11_SUBRESOURCE_DATA initial_data = {
			.pSysMem = buffer.data,
		};

		assert(SUCCEEDED(state.device->CreateBuffer(&desc, &initial_data, &result.buffer)));

		return &result;
	};
	_set_index_buffer = [](IndexBuffer *_buffer) {
		auto buffer = (IndexBufferImpl *)_buffer;
		state.immediate_context->IASetIndexBuffer(buffer->buffer, buffer->format, 0);
	};
	_set_vsync = [](bool enable) {
		state.sync_interval = enable ? 1 : 0;
	};
	_set_render_target = [](RenderTarget *_target) {
		auto target = (RenderTargetImpl *)_target;
		if (!target)
			target = &state.back_buffer;
		state.immediate_context->OMSetRenderTargets(1, &target->color_target, target->depth_target);
	};
	/*
	_create_render_target = [](CreateRenderTargetFlags flags, Format _format, Filtering filtering, TextureComparison comparison, u32 width, u32 height) -> RenderTarget * {
		auto &result = state.render_targets.add();

		auto format = get_format(_format);

		if (flags & CreateRenderTarget_resize_with_window) {
			state.render_targets_resized_with_window.insert(&result);
		}

		{
			D3D11_TEXTURE2D_DESC desc = {
				.Width = width,
				.Height = height,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = format,
				.SampleDesc = {1, 0},
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
			};
			assert(SUCCEEDED(state.device->CreateTexture2D(&desc, 0, &result.color_texture)));
		}
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc = {
				.Format = format,
				.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
			};
			assert(SUCCEEDED(state.device->CreateRenderTargetView(result.color_texture, &desc, &result.color_target)));
		}

		return &result;
	};
	*/

	return true;
}

void deinit() {

}
}
#endif

#endif

