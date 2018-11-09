#include "stdafx.h"

#include "syntax_node.h"

// PARAMS_NODE
params_node::params_node(const stoken &token) :
	syntax_node(token) {
	type = syntax_type::syn_params;
}

// ANNOTATION_NODE
annotation_node::annotation_node(const stoken &token) :
	syntax_node(token) {
	capacity = 1;
	type = syntax_type::syn_annotation;
}

// FIELD_NODE
field_node::field_node(const stoken &token) :
	syntax_node(token) {
	capacity = 1;
	type = syntax_type::syn_field;
}

// INHERIT_NODE
inherit_node::inherit_node(const stoken &token) :
	syntax_node(token) {
	type = syntax_type::syn_inherit;
}

// CLASS_NODE
class_node::class_node(const stoken &token) :
	syntax_node(token) {
	attr = 0;
	type = syntax_type::syn_class;
}

inherit_node *class_node::parents() const {
	if (children.size() <= 1) return nullptr;
	if (children[1]->type == syntax_type::syn_inherit)
		return (inherit_node*)children[1];
	return nullptr;
}

// METHOD_NODE
method_node::method_node(const stoken &token) :
	syntax_node(token) {

	attr = 0;
	capacity = 3;
	type = syntax_type::syn_method;

	local_size = 0;
}
void method_node::push_annotation(annotation_node *node) {
	append(node, false);
}
void method_node::on_complete() {
	auto prev_locals = body()->locals;

	body()->locals.clear();
	for (uint32_t i = 0; i < params()->children.size(); i++) {
		if (params()->children[i]->type == syntax_type::syn_assignment)
			body()->push_local(((ident_node*)params()->children[i]->children[0])->ident);
		if (params()->children[i]->type == syntax_type::syn_ident)
			body()->push_local(((ident_node*)params()->children[i])->ident);
	}

	for (auto &ident : prev_locals)
		body()->push_local(ident);
}