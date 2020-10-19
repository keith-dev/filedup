#include "document.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

Document::Document(const char* filename) : filename_(filename) {
	struct stat info;
	int rc = stat(filename_.c_str(), &info);
	if (rc == -1) throw std::runtime_error(std::string("cannot access file: ") + filename);
	buf_.reset(new char[info.st_size]);

	int fd = open(filename_.c_str(), O_RDONLY);
	if (rc == -1) throw std::runtime_error(std::string("cannot open file: ") + filename);
	int nbytes = read(fd, buf_.get(), info.st_size);
	close(fd);
	if (nbytes == -1) throw std::runtime_error(std::string("cannot read file: ") + filename);

	if (json_.ParseInsitu(buf_.get()).HasParseError()) throw std::runtime_error(std::string("cannot parse json in file: ") + filename);
	if (!parse(json_)) throw std::runtime_error(std::string("cannot process json in file: ") + filename);
}

bool Document::parse(const rapidjson::Document& json, size_t level) {
	if (json.IsObject()) {
/*
		std::cout << std::string(levelMultiplier_*level, ' ')
			<< "Type:" << kTypeNames_[json.GetType()] << '\n';
 */
		object_.reset( parseObject(json.GetObject(), level + 1) );
		return true;
	}

	std::cout << std::string(levelMultiplier_*level, ' ')
		<< "(null)" << '\n';
	return false;
}

mydoc::Object* Document::parseObject(const rapidjson::Value::ConstObject& value, size_t level) {
	std::unique_ptr<mydoc::Object> obj{ mydoc::Object::createObject() };

	for (rapidjson::Value::ConstMemberIterator p = value.MemberBegin(); p != value.MemberEnd(); ++p) {
/*
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout
			<< "name:" << p->name.GetString()
			<< " type:" << kTypeNames_[p->value.GetType()]
			<< '\n';
 */
		switch (p->value.GetType()) {
		case rapidjson::kFalseType:
			obj->object().push_back( mydoc::Object::createBool(false) );
			break;
		case rapidjson::kTrueType:
			obj->object().push_back( mydoc::Object::createBool(true) );
			break;
		case rapidjson::kObjectType:
			obj->object().push_back( parseObject(p->value.GetObject(), level + 1) );
			break;
		case rapidjson::kArrayType:
			obj->object().push_back( parseArray(p->value.GetArray(), level + 1) );
			break;
		case rapidjson::kStringType:
			obj->object().push_back( mydoc::Object::createString(p->value.GetString()) );
			break;
		case rapidjson::kNumberType:
			obj->object().push_back( p->value.IsDouble() ? mydoc::Object::createReal(p->value.GetDouble())
														 : mydoc::Object::createInteger(p->value.GetInt()) );
			break;
		default:
			;
		}
	}

	return obj.release();
}

mydoc::Object* Document::parseArray(const rapidjson::Value::ConstArray& value, size_t level) {
	std::unique_ptr<mydoc::Object> obj{ mydoc::Object::createArray() };

	for (rapidjson::Value::ConstValueIterator p = value.Begin(); p != value.End(); ++p) {
/*
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout
			<< "type:" << kTypeNames_[p->GetType()]
			<< '\n';
 */
		switch (p->GetType()) {
		case rapidjson::kFalseType:
			obj->array().push_back( mydoc::Object::createBool(false) );
			break;
		case rapidjson::kTrueType:
			obj->array().push_back( mydoc::Object::createBool(true) );
			break;
		case rapidjson::kObjectType:
			obj->array().push_back( parseObject(p->GetObject(), level + 1) );
			break;
		case rapidjson::kArrayType:
			obj->array().push_back( parseArray(p->GetArray(), level + 1) );
			break;
		case rapidjson::kStringType:
			obj->array().push_back( mydoc::Object::createString(p->GetString()) );
			break;
		case rapidjson::kNumberType:
			obj->array().push_back( p->IsDouble() ? mydoc::Object::createReal(p->GetDouble())
												  : mydoc::Object::createInteger(p->GetInt()) );
			break;
		default:
			;
		}
	}

	return obj.release();
}

/*
		if (p->IsNull()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << "(null)" << '\n';
		}
		else if (p->IsBool()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << std::boolalpha << p->GetBool() << '\n';
		}
		else if (p->IsInt()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->GetInt() << '\n';
		}
		else if (p->IsDouble()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->GetDouble() << '\n';
		}
		else if (p->IsString()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->GetString() << '\n';
		}
		else if (p->IsArray()) {
			parseArray(doc, p->GetArray(), level + 1);
		}
		else if (p->IsObject()) {
			parseObject(doc, p->GetObject(), level + 1);
		}
		else {
			std::cout << "unhandled" << '\n';
		}
 */