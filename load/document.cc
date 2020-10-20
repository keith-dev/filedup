#include "document.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>
#include <string>

Document::Document(const char* filename) {
	struct stat info;
	int rc = stat(filename, &info);
	if (rc == -1) throw std::runtime_error(std::string("cannot access file: ") + filename);

	std::unique_ptr<char[]> buf;
	buf.reset(new char[info.st_size + 1]);

	int fd = open(filename, O_RDONLY);
	if (rc == -1) throw std::runtime_error(std::string("cannot open file: ") + filename);
	int nbytes = read(fd, buf.get(), info.st_size);
	close(fd); fd = -1;
	if (nbytes == -1) throw std::runtime_error(std::string("cannot read file: ") + filename);

	rapidjson::Document json;
	if (json.ParseInsitu(buf.get()).HasParseError()) throw std::runtime_error(std::string("cannot parse json in file: ") + filename);
	if (!parse(json)) throw std::runtime_error(std::string("cannot process json in file: ") + filename);
}

bool Document::parse(const rapidjson::Document& json, size_t level) {
	if (json.IsObject()) {
		std::cout << std::string(levelMultiplier_*level, ' ')
			<< "Type:" << kTypeNames_[json.GetType()] << '\n';

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
		std::cout << std::string(levelMultiplier_*level, ' ')
			<< "name:" << p->name.GetString()
			<< " type:" << kTypeNames_[p->value.GetType()]
			<< '\n';

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
		case rapidjson::kNullType: // fallthru
		default:
			;
		}
	}

	return obj.release();
}

mydoc::Object* Document::parseArray(const rapidjson::Value::ConstArray& value, size_t level) {
	std::unique_ptr<mydoc::Object> obj{ mydoc::Object::createArray() };

	for (rapidjson::Value::ConstValueIterator p = value.Begin(); p != value.End(); ++p) {
		std::cout << std::string(levelMultiplier_*level, ' ')
			<< "type:" << kTypeNames_[p->GetType()]
			<< '\n';

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
		case rapidjson::kNullType: // fallthru
		default:
			;
		}
	}

	return obj.release();
}