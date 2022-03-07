#include <tl/common.h>
#include <tl/file.h>
#include <tl/console.h>
#include <tl/main.h>

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

umm append(StringBuilder &builder, Token token) {
	return append_format(builder, "'{}:{}:{}'", token.view, token.line, token.column);
}

s32 tl_main(Span<Span<utf8>> args) {
	current_printer = console_printer;
	current_allocator = temporary_allocator;

	auto signature_path = tl_file_string("../data/apis.h"ts);
	auto signature_file = read_entire_file(signature_path);
	if (!signature_file.data) {
		print("Failed to open {}\n", signature_path);
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
			t.view.count = c - t.view.data;
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
			t.view.count = c - t.view.data;
			push_token(t);
		} else {
			Token t = {};
			t.view.data = c;
			t.view.count = 1;
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
					print("Failed to parse input file: character '{}' is not part of the syntax\n", *c);
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

		if (pre_aruments.count < 2) {
			print("Bad syntax before token {}\n", *t);
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
			print("Expected ';' after declaration: {}\n", *t);
			return 3;
		}
		++t;

		funcs.add(f);
	}

	auto write_entire_file = [&](Span<utf8> path, Span<u8> data) {
		if (!tl::write_entire_file(path, data)) {
			print(Print_error, "Failed to write {}\n", path);
			return false;
		}
		return true;
	};

	StringBuilder defn_builder;
	for (auto func : funcs) {
		// POINTER
		append_format(defn_builder, "{} (*_{})(State *_state", func.ret, func.name);
		if (func.args.count) {
			append(defn_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append_format(defn_builder, "{} {}", arg.type, arg.name);
		}
		append(defn_builder, ");\n");

		// FUNCTION
		append_format(defn_builder, "{} {}(", func.ret, func.name);
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append_format(defn_builder, "{} {}", arg.type, arg.name);
		}
		append_format(defn_builder, ") {{ return _{}(this", func.name);
		if (func.args.count) {
			append(defn_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(defn_builder, ", ");
			append(defn_builder, arg.name);
		}
		append(defn_builder, "); }\n");
	}
	write_entire_file(u8"../include/tgraphics/generated/definition.h"s, as_bytes(to_string(defn_builder)));

	StringBuilder check_builder;
	for (auto func : funcs) {
		append_format(check_builder, "if(!state->_{}){{print(\"{} was not initialized.\\n\");result=false;}}\n", func.name, func.name);
	}
	write_entire_file(u8"../include/tgraphics/generated/check.h"s, as_bytes(to_string(check_builder)));

	StringBuilder assign_builder;
	for (auto func : funcs) {
		append_format(assign_builder, "state->_{} = [](State *_state", func.name);
		if (func.args.count) {
			append(assign_builder, ", ");
		}
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(assign_builder, ", ");
			append_format(assign_builder, "{} {}", arg.type, arg.name);
		}
		append_format(assign_builder, ") -> {} {{ return ((StateGL *)_state)->impl_{}(", func.ret, func.name);
		for (auto &arg : func.args) {
			if (&arg != func.args.data)
				append(assign_builder, ", ");
			append(assign_builder, arg.name);
		}
		append(assign_builder, "); };\n");
	}
	write_entire_file(u8"../include/tgraphics/generated/assign.h"s, as_bytes(to_string(assign_builder)));

	return 0;
}
