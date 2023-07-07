#pragma once

#include <vector>
#include <optional>
#include <string>

#include "json.h"

namespace json {
class Builder {
public:
	class Context;
	class DictKey;
	class DictValue;
	class ArrayValue;

public:
	Builder();
	Node Build();

	Context Value(Node node);

	//DictValue Key(std::string);
	DictKey StartDict();
	ArrayValue StartArray();
	//Context EndDict();
	//Context EndArray();
private:
	Node root_{};
	std::vector<Node*> nodes_stack_;
	std::optional<std::string> key_{};

public:
	class Context {
	public:
		Context(Builder* builder);

		Context Value(Node node); //???

		Node Build();
		DictValue Key(std::string);
		DictKey StartDict();
		ArrayValue StartArray();
		Context EndDict();
		Context EndArray();
	protected:
		void ValueWorker(Node node);
	//private:
		Builder* builder_;
	};

	class DictKey : public Context {
	public:
		DictKey(Builder* builder);

		Builder& Value(Node node) = delete;//Просто удалить. 

		Node Build() = delete;
		DictKey StartDict() = delete;
		ArrayValue StartArray() = delete;
		Context EndArray() = delete;
	protected:
		void ValueWorker(Node) = delete;
	};
	class DictValue : public Context {
	public:
		DictValue(Builder* builder);

		DictKey Value(Node node); //???

		Node Build() = delete;
		DictValue Key(std::string) = delete;
		Context EndDict() = delete;
		Context EndArray() = delete;
	};
	class ArrayValue: public Context {
	public:
		ArrayValue(Builder* builder);

		ArrayValue Value(Node node); //???

		Node Build() = delete;
		DictValue Key(std::string) = delete;

		Context EndDict() = delete;
	};
};





}//json