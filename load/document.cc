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
	if (!parse(*this, json_)) throw std::runtime_error(std::string("cannot process json in file: ") + filename);
}

bool Document::parse(Document& doc, const rapidjson::Document& json, size_t level) {
	std::cout << std::string(levelMultiplier_*level, ' ') << "Type:" << kTypeNames_[json.GetType()] << '\n';

	if (json.IsObject()) {
		for (rapidjson::Value::ConstMemberIterator p = json.MemberBegin(); p != json.MemberEnd(); ++p) {
			std::cout << std::string(levelMultiplier_*level, ' ')
				<< "name:" << p->name.GetString()
				<< " type:" << kTypeNames_[p->value.GetType()]
				<< '\n';
			switch (json.GetType()) {
			case rapidjson::kNullType:
				std::cout << std::string(levelMultiplier_*level, ' ')
					<< "(null)" << '\n';
				break;
			case rapidjson::kFalseType: // fallthru
			case rapidjson::kTrueType:
				std::cout << std::string(levelMultiplier_*level, ' ')
					<< std::boolalpha << p->value.GetBool() << '\n';
				break;
			case rapidjson::kObjectType:
				parseObject(doc, p->value.GetObject(), level + 1);
				break;
			case rapidjson::kArrayType: 
				parseArray(doc, p->value.GetArray(), level + 1);
				break;
			case rapidjson::kStringType:
				std::cout << std::string(levelMultiplier_*level, ' ')
					<< p->value.GetString() << '\n';
				break;
			case rapidjson::kNumberType:
				if (p->value.IsInt()) {
					std::cout << std::string(levelMultiplier_*level, ' ')
						<< p->value.GetInt() << '\n';
				}
				else if (p->value.IsDouble()) {
					std::cout << std::string(levelMultiplier_*level, ' ')
						<< p->value.GetDouble() << '\n';
				}
				break;
			default:
				std::cout << std::string(levelMultiplier_*level, ' ')
					<< "unhandled type: " << json.GetType() << '\n';
			}
		}
		return true;
	}

	std::cout << std::string(levelMultiplier_*level, ' ');
	std::cout << "(null)" << '\n';
	return false;
}

/*
void Document::parse(Document& doc, const rapidjson::Value& value, size_t level) {
	std::cout << std::string(levelMultiplier_*level, ' ') << "Type:" << kTypeNames_[value.GetType()] << '\n';

	if (value.IsNull()) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout << "(null)" << '\n';
	}
	else if (value.IsBool()) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout << std::boolalpha << value.GetBool() << '\n';
	}
	else if (value.IsInt()) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout << value.GetInt() << '\n';
	}
	else if (value.IsDouble()) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout << value.GetDouble() << '\n';
	}
	else if (value.IsString()) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout << value.GetString() << '\n';
	}
	else if (value.IsArray()) {
		parseArray(doc, value.GetArray(), level + 1);
	}
	else if (value.IsObject()) {
		parseObject(doc, value.GetObject(), level + 1);
	}
}
 */

void Document::parseNull(Document& doc, const rapidjson::Value& value, size_t level) {
}

void Document::parseObject(Document& doc, const rapidjson::Value::ConstObject& value, size_t level) {
	for (rapidjson::Value::ConstMemberIterator p = value.MemberBegin(); p != value.MemberEnd(); ++p) {
		std::cout << std::string(levelMultiplier_*level, ' ');
		std::cout
			<< "name:" << p->name.GetString()
			<< " type:" << kTypeNames_[p->value.GetType()]
			<< '\n';

		if (p->value.IsNull()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << "(null)" << '\n';
		}
		else if (p->value.IsBool()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << std::boolalpha << p->value.GetBool() << '\n';
		}
		else if (p->value.IsInt()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->value.GetInt() << '\n';
		}
		else if (p->value.IsDouble()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->value.GetDouble() << '\n';
		}
		else if (p->value.IsString()) {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << p->value.GetString() << '\n';
		}
		else if (p->value.IsArray()) {
			parseArray(doc, p->value.GetArray(), level + 1);
		}
		else if (p->value.IsObject()) {
			parseObject(doc, p->value.GetObject(), level + 1);
		}
		else {
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << "unhandled" << '\n';
		}
	}
}

/*
void Document::parseArray(Document& doc, const rapidjson::Document& json, size_t level) {
	for (rapidjson::Value::ConstValueIterator p = json.Begin(); p != json.End(); ++p) {
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
			std::cout << std::string(levelMultiplier_*level, ' ');
			std::cout << "unhandled" << '\n';
		}
	}
}
 */

void Document::parseArray(Document& doc, const rapidjson::Value::ConstArray& value, size_t level) {
	for (rapidjson::Value::ConstValueIterator p = value.Begin(); p != value.End(); ++p) {
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
	}
}
