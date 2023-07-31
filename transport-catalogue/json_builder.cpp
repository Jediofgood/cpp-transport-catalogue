#include "json_builder.h"

namespace json {

Builder::Builder() {
	nodes_stack_.push_back(&root_);
}

Builder::Context::Context(Builder* builder)
	:builder_(builder)
{}
Builder::DictKey::DictKey(Builder* builder)
	:Context(builder)
{}

Builder::DictValue::DictValue(Builder* builder)
	:Context(builder)
{}

Builder::ArrayValue::ArrayValue(Builder* builder)
	:Context(builder)
{}

Node* Builder::AddNewObj() {
	if (nodes_stack_.size() == 0) {
		throw std::logic_error("");
	}

	return nodes_stack_.back();
}

Node* Builder::Context::AddNewObj() {
	if (builder_->nodes_stack_.size() == 0) {
		throw std::logic_error("");
	}

	return builder_->nodes_stack_.back();
}

Builder::Context Builder::Value(Node value) {
	Node* last_node = AddNewObj();

	if (last_node->IsDict()) {
		if (key_.has_value()) {
			Dict& dict = std::get<Dict>(last_node->GetValueRed());
			dict[key_.value()] = value;
			key_ = std::nullopt;
		}
		else {
			throw std::logic_error("Need a key, before value");
		}
	}
	else if (last_node->IsArray()) {
		Array& arr = std::get<Array>(last_node->GetValueRed());
		Node node = value;
		arr.emplace_back(value);
	}
	else if (root_.IsNull()) {
		root_.GetValueRed() = std::move(value.GetValueRed());
		nodes_stack_.pop_back();
	}
	else {
		throw std::logic_error("");
	}

	return Context{ this };
}

Node Builder::Build() {
	if (nodes_stack_.size() != 0 || root_ == nullptr) {
		throw std::logic_error("Not Finished");
	}
	else {
		return root_;
	}
}

Builder::DictValue Builder::Context::Key(std::string key) {
	Node* last_node = AddNewObj();

	if (!last_node->IsDict()) {
		throw std::logic_error("Not a Dict");
	}
	else if (builder_->key_.has_value()) {
		throw std::logic_error("Already have a key");
	}
	else {
		builder_->key_ = std::move(key);
	}
	return DictValue{ builder_ };
}

Builder::DictKey Builder::Context::StartDict() {

	Node* last_node = AddNewObj();

	if (last_node->IsDict()) {
		if (!builder_->key_.has_value()) {
			throw std::logic_error("");
		}
		else {
			Dict& dict = std::get<Dict>(last_node->GetValueRed());
			Dict newdict{};
			dict[builder_->key_.value()] = newdict;
			builder_->nodes_stack_.emplace_back(&dict[builder_->key_.value()]);
			builder_->key_ = std::nullopt;
		}
	}
	else if (last_node->IsArray()) {
		Array& arr = std::get<Array>(last_node->GetValueRed());
		Dict newdict{};
		arr.emplace_back(newdict);
		builder_->nodes_stack_.push_back(&arr.back());
	}
	else if (builder_->root_.IsNull()) {
		builder_->root_.GetValueRed() = Dict{};
	}
	else {
		throw std::logic_error("");
	}
	return DictKey{ builder_ };
}

Builder::ArrayValue Builder::Context::StartArray() {
	Node* last_node = AddNewObj();

	if (last_node->IsDict()) {
		if (!builder_->key_.has_value()) {
			throw std::logic_error("");
		}
		else {
			Dict& dict = std::get<Dict>(last_node->GetValueRed());
			Array arr{};
			dict[builder_->key_.value()] = arr;
			builder_->nodes_stack_.emplace_back(&dict[builder_->key_.value()]);
			builder_->key_ = std::nullopt;
		}
	}
	else if (last_node->IsArray()) {
		Array& arr = std::get<Array>(last_node->GetValueRed());
		Array newdarr{};
		arr.emplace_back(newdarr);
		builder_->nodes_stack_.push_back(&arr.back());
	}
	else if (builder_->root_.IsNull()) {
		builder_->root_.GetValueRed() = Array{};
	}
	else {
		throw std::logic_error("");
	}
	return ArrayValue{ builder_ };
}

Builder::Context Builder::Context::EndDict() {
	Node* last_node = AddNewObj();

	if (!last_node->IsDict()) {
		throw std::logic_error("");
	}
	builder_->nodes_stack_.pop_back();
	return Context{ builder_ };
}

Builder::Context Builder::Context::EndArray() {
	Node* last_node = AddNewObj();

	if (!last_node->IsArray()) {
		throw std::logic_error("");
	}
	builder_->nodes_stack_.pop_back();
	return Context{ builder_ };
}

Node Builder::Context::Build() {
	return builder_->Build();
}

Builder::DictKey Builder::StartDict() {
	return Context{this}.StartDict();
}

Builder::ArrayValue Builder::StartArray() {
	return Context{ this }.StartArray();
}


void Builder::Context::ValueWorker(Node value) {
	Node* last_node = AddNewObj();


	if (last_node->IsDict()) {
		if (builder_->key_.has_value()) {
			Dict& dict = std::get<Dict>(last_node->GetValueRed());
			dict[builder_->key_.value()] = value;
			builder_->key_ = std::nullopt;
		}
		else {
			throw std::logic_error("Need a key, before value");
		}
	}
	else if (last_node->IsArray()) {
		Array& arr = std::get<Array>(last_node->GetValueRed());
		Node node = value;
		arr.emplace_back(value);
	}
	else if (builder_->root_.IsNull()) {
		builder_->root_.GetValueRed() = std::move(value.GetValueRed());
		builder_->nodes_stack_.pop_back();
	}
	else {
		throw std::logic_error("");
	}
}

Builder::Context Builder::Context::Value(Node value) {
	ValueWorker(std::move(value));
	return Context{ builder_ };
}

Builder::DictKey Builder::DictValue::Value(Node node) {
	ValueWorker(std::move(node));
	return DictKey{ builder_ };
}

Builder::ArrayValue Builder::ArrayValue::Value(Node node) {
	ValueWorker(std::move(node));
	return ArrayValue{ builder_ };
}


}