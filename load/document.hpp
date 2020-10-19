#pragma once

#include "object.hpp"

#include <rapidjson/document.h>

#include <array>
#include <memory>
#include <string>

class Document {
	std::string filename_;
	rapidjson::Document json_;
	std::unique_ptr<char[]> buf_;
	mydoc::Object object_;

	static constexpr size_t levelMultiplier_{2};
	static constexpr std::array<const char*, 7> kTypeNames_{
		"Null", "False", "True", "Object", "Array", "String", "Number" };

public:
	Document(const char* filename);

	bool parse(const rapidjson::Document& json, size_t level = 0);

	void parseObject(mydoc::Object& obj, const rapidjson::Value::ConstObject& value, size_t level);

	static void parseNull(Document& doc, const rapidjson::Value& value, size_t level);
	static void parseObject(Document& doc, const rapidjson::Value::ConstObject& value, size_t level);
	static void parseArray(Document& doc, const rapidjson::Value::ConstArray& value, size_t level);
};