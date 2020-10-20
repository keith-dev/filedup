#pragma once

#include "object.hpp"

#include <rapidjson/document.h>

#include <array>
#include <memory>

class Document {
	std::unique_ptr<mydoc::Object> object_;

	static constexpr size_t levelMultiplier_{2};
	static constexpr std::array<const char*, 7> kTypeNames_{
		"Null", "False", "True", "Object", "Array", "String", "Number" };

public:
	Document(const char* filename);

	bool parse(const rapidjson::Document& json, size_t level = 0);

	mydoc::Object* parseObject(const rapidjson::Value::ConstObject& value, size_t level);
	mydoc::Object* parseArray(const rapidjson::Value::ConstArray& value, size_t level);
};