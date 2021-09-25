#define TGRAPHICS_IMPL
#include <tgraphics/tgraphics.h>
#include <tl/window.h>

using namespace tl;
namespace tg = tgraphics;

tg::Shader *test_shader;

/*
List<utf8> to_glsl(Span<utf8> source) {

}
List<utf8> to_hlsl(Span<utf8> source) {

}
*/

tg::State *t;

void run_test() {

	auto window = create_window({
		.on_create = [](Window &window) {
			t = ::tg::init(tg::GraphicsApi_opengl, {.window = window.handle, .debug = true,});
			/*
			auto source = u8R"(
vertex_shader {
	float2 positions[] = {
		float2( 0.0,  0.5),
		float2(-0.5, -0.5),
		float2( 0.5, -0.5)
	};
	sv_position = float4(positions[sv_vertex_id], 0, 1);
}

pixel_shader {
	sv_color = float4(.3, .6, .9, 1.);
}
)"s;

			print("GLSL:\n%", to_glsl(source));
			print("HLSL:\n%", to_hlsl(source));
			*/

			switch (t->api) {
				case tg::GraphicsApi_opengl: {
					test_shader = t->create_shader(u8R"(
#ifdef VERTEX_SHADER
void main() {
	vec2 positions[] = vec2[](
		vec2( 0.0,  0.5),
		vec2(-0.5, -0.5),
		vec2( 0.5, -0.5)
	);
	gl_Position = vec4(positions[gl_VertexID], 0, 1);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 fragment_color;
void main() {
	fragment_color = vec4(.3, .6, .9, 1.);
}
#endif

)"s);
					break;
				}
				case tg::GraphicsApi_d3d11: {
					test_shader = t->create_shader(u8R"(
float4 vertex_main(uint vertex_id : SV_VertexID) : SV_Position {
	float2 positions[] = {
		float2( 0.0,  0.5),
		float2(-0.5, -0.5),
		float2( 0.5, -0.5)
	};
	return float4(positions[vertex_id], 0, 1);
}

float4 pixel_main() : SV_Target {
	return float4(.3, .6, .9, 1.);
}

)"s);
					break;
				}
			}

			t->set_shader(test_shader);
		},
		.on_size = [](Window &window) {
			t->resize_render_targets(window.client_size);
			t->set_viewport(window.client_size);
		},
		.on_draw = [](Window &window) {
			t->draw(3);
			t->present();
		},
	});

	while (update(window)) {
	}
}
