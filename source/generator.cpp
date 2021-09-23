#define TL_IMPL
#define TL_MAIN
#include <tl/common.h>
#include <tl/file.h>
#include <tl/console.h>

using namespace tl;

using TokenKind = u16;
enum : TokenKind {
	Token_identifier = 0x100,
	Token_number     = 0x101,
};
struct Token {
	TokenKind kind;
	Span<char> view;
	u32 line;
	u32 column;
};

void append(StringBuilder &builder, Token token) {
	append_format(builder, "'%:%:%'", token.view, token.line, token.column);
}

s32 tl_main(Span<Span<utf8>> args) {
	current_printer = console_printer;
	current_allocator = temporary_allocator;

	auto signature_path = tl_file_string("../data/apis.h"ts);
	auto signature_file = read_entire_file(signature_path);
	if (!signature_file.data) {
		print("Failed to open %\n", signature_path);
		return 1;
	}

	char *c = (char *)signature_file.data;
	char *end = (char *)signature_file.end();

	List<Token> tokens;
	u32 line = 1;
	char *start_of_line = c;

	auto push_token = [&](Token t) {
		t.line = line;
		t.column = t.view.data - start_of_line;
		tokens.add(t);
	};

	while (c != end) {
		while (*c == ' ' || *c == '\t' || *c == '\n' || *c == '\r') {
			if (*c == '\n') {
				start_of_line = c;
				++line;
			}
			++c;
			if (c == end) {
				goto begin_parse;
			}
		}
		if (is_alpha(*c)) {
			Token t = {};
			t.kind = Token_identifier;
			t.view.data = c;
			++c;
			if (c != end) {
				while (is_alpha(*c) || is_digit(*c) || *c == '_') {
					++c;
				}
			}
			t.view.size = c - t.view.data;
			push_token(t);
		} else if (is_digit(*c)) {
			Token t = {};
			t.kind = Token_number;
			t.view.data = c;
			++c;
			if (c != end) {
				while (is_digit(*c)) {
					++c;
				}
			}
			t.view.size = c - t.view.data;
			push_token(t);
		} else {
			Token t = {};
			t.view.data = c;
			t.view.size = 1;
			t.kind = *c;
			switch (*c) {
				case '(':
				case ')':
				case ',':
				case ';':
				case '<':
				case '>':
				case '[':
				case ']':
				case '*': break;
				default:
					print("Failed to parse input file: character '%' is not part of the syntax\n", *c);
					return 2;
			}
			++c;
			push_token(t);
		}
	}

begin_parse:
	struct Arg {
		Span<char> name;
		Span<char> type;
	};
	struct Func {
		Span<char> ret;
		Span<char> name;
		List<Arg> args;
	};

	List<Func> funcs;

	Token *t = tokens.data;
	while (t != tokens.end()) {
		Func f = {};

		List<Token *> pre_aruments;
		while (t->kind != '(') {
			pre_aruments.add(t);
			++t;
		}
		++t;

		if (pre_aruments.size < 2) {
			print("Bad syntax before token %\n", *t);
			return 3;
		}

		f.name = pre_aruments.back()->view;
		f.ret = {pre_aruments[0]->view.data, pre_aruments.end()[-2]->view.end()};

		while (t->kind != ')') {
			List<Token *> arg_tokens;
			while (t->kind != ',' && t->kind != ')') {
				arg_tokens.add(t);
				++t;
			}
			if (t->kind == ',')
				++t;

			Arg arg;
			arg.name = arg_tokens.back()->view;
			arg.type = { arg_tokens[0]->view.data, arg_tokens.end()[-2]->view.end() };
			f.args.add(arg);
		}
		++t;
		if (t->kind != ';') {
			print("Expected ';' after declaration: %\n", *t);
			return 3;
		}
		++t;

		funcs.add(f);
	}




	auto source_file = open_file(tl_file_string("../data/tgraphics.h"), {.read = true});
	defer { close(source_file); };

	auto mapped_source = map_file(source_file);
	defer { unmap_file(mapped_source); };


	auto destination_file = open_file(tl_file_string("../include/tgraphics/tgraphics.h"), {.write = true});
	defer {close(destination_file);};


	StringBuilder defn_builder;
	for (auto func : funcs) {
		// POINTER
		append_format(defn_builder, "\t% (*_%)(State *_state", func.ret, func.name);
		if (func.args.size) {
			append(defn_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append_format(defn_builder, "% %", arg.type, arg.name);
		}
		append(defn_builder, ");\n");

		// FUNCTION
		append_format(defn_builder, "\t% %(", func.ret, func.name);
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append_format(defn_builder, "% %", arg.type, arg.name);
		}
		append_format(defn_builder, ") { return _%(this", func.name);
		if (func.args.size) {
			append(defn_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append(defn_builder, arg.name);
		}
		append(defn_builder, "); }\n");
	}
#define A(ret, name, args, values)

	StringBuilder check_builder;
	for (auto func : funcs) {
		append_format(check_builder, "\tif(!state->_%){print(\"% was not initialized.\\n\");result=false;}\n", func.name, func.name);
	}

	StringBuilder remap_builder;
	for (auto func : funcs) {
		append_format(remap_builder, "\tstate->_% = [](State *_state", func.name);
		if (func.args.size) {
			append(remap_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(remap_builder, ", ");
			append_format(remap_builder, "% %", arg.type, arg.name);
		}
		append_format(remap_builder, ") -> % { return ((StateGL *)_state)->impl_%(", func.ret, func.name);
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(remap_builder, ", ");
			append(remap_builder, arg.name);
		}
		append(remap_builder, "); };\n");
	}
	auto definition_token = find(mapped_source.data, "APIS_DEFINITION;"b);
	auto check_token = find(mapped_source.data, "APIS_CHECK;"b);
	auto remap_token = find(mapped_source.data, "APIS_REMAP;"b);

	write(destination_file, Span(mapped_source.data.begin(), definition_token.begin()));
	write(destination_file, as_bytes(to_string(defn_builder)));
	write(destination_file, Span(definition_token.end(), check_token.begin()));
	write(destination_file, as_bytes(to_string(check_builder)));
	write(destination_file, Span(check_token.end(), remap_token.begin()));
	write(destination_file, as_bytes(to_string(remap_builder)));
	write(destination_file, Span(remap_token.end(), mapped_source.data.end()));

	print("Done\n");

	return 0;
}
