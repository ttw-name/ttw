


#include "jsonelement.h"

#include<sstream>

namespace ttw {
	namespace json {
		
		JsonElement::JsonElement() {
			m_deep = false;
			m_type = JsonType::OBJECT;
			m_val.object_val = nullptr;
		}

		JsonElement::JsonElement(const std::nullptr_t& val) {
			m_deep = false;
			m_type = JsonType::JSONNULL;
			m_val.bool_val = 0;
		}

		JsonElement::JsonElement(JsonElement&& val) noexcept {
			//移动构造只将数据释放进行移动。
			this->m_deep = val.m_deep;
			this->m_type = val.m_type;
			this->m_val = val.m_val;
			val.m_deep = false;
		}

		JsonElement::JsonElement(const JsonType type):m_type(type){
			m_deep = false;
			switch (m_type) {

			case JsonType::OBJECT:
				m_val.object_val = nullptr; break;

			case JsonType::ARRAY:
				m_val.array_val = nullptr; break;

			case JsonType::STRING:
				m_val.string_val = nullptr; break;

			case JsonType::BOOL:
				m_val.bool_val = false; break;

			case JsonType::INT:
				m_val.int_val = 0; break;

			case JsonType::UNSIGNEDINT:
				m_val.uint_val = 0u; break;

			case JsonType::LONG:
				m_val.long_val = 0l; break;

			case JsonType::UNSIGNEDLONG:
				m_val.ulong_val = 0ul; break;

			case JsonType::FLOAT:
				m_val.float_val = 0.0f; break;

			case JsonType::DOUBLE:
				m_val.double_val = 0.0; break;

			default:
				exceptionT(); break;
			}
		}

		JsonElement::JsonElement(const JsonArray& val, bool deep) {
			m_type = JsonType::ARRAY;
			m_deep = deep;
			//m_val.array_val = copy(&val, deep);
			if (deep) {
				m_val.array_val =  copy(&val);
			}
			else {
				m_val.array_val = const_cast<JsonArray*>(&val);
			}

		}
	
		JsonElement::JsonElement(const JsonObject& val, bool deep) {
			m_type = JsonType::OBJECT;
			m_deep = deep;
			if (deep) {
				m_val.object_val = copy(&val);
			}
			else {
				m_val.object_val = const_cast<JsonObject*>(&val);
			}
			//m_val.object_val = copy(val, deep);
		}

		JsonElement::JsonElement(const JsonElement& val, bool deep) {
			//m_type = JsonType::OBJECT;
			//m_val.object_val = copy(val, deep);
			m_deep = val.m_deep;
			m_type = val.m_type;
			if (deep) {
				if (val.m_type == JsonType::OBJECT) {
					m_val.object_val = copy(val.m_val.object_val);
				}
				else if (val.m_type == JsonType::ARRAY) {
					m_val.array_val = copy(val.m_val.array_val);
				}
				else if (val.m_type == JsonType::STRING) {
					m_val.string_val = new std::string(*val.m_val.string_val);
				}
			}
			else {
				m_deep = false;
				m_val = val.m_val;
			}
		}

		JsonElement::JsonElement(const char* val) {
			m_type = JsonType::STRING;
			m_deep = true;
			m_val.string_val = new std::string(val);

			//m_val.string_val = copy(val);
		}

		JsonElement::JsonElement(const std::string& val) {
			m_type = JsonType::STRING;
			m_deep = true;
			m_val.string_val = new std::string(val);

			//m_val.string_val = copy(val);
		}

		JsonElement::JsonElement(const bool& val) {
			m_deep = false;
			m_type = JsonType::BOOL;
			m_val.bool_val = val;
		}

		

		JsonElement::JsonElement(const int& val) {
			m_deep = false;
			m_type = JsonType::INT;
			m_val.int_val = val;
		}

		JsonElement::JsonElement(const unsigned int& val) {
			m_deep = false;
			m_type = JsonType::UNSIGNEDINT;
			m_val.uint_val = val;
		}

		JsonElement::JsonElement(const long& val) {
			m_deep = false;
			m_type = JsonType::LONG;
			m_val.long_val = val;
		}

		JsonElement::JsonElement(const unsigned long& val) {
			m_deep = false;
			m_type = JsonType::UNSIGNEDLONG;
			m_val.ulong_val = val;
		}

		JsonElement::JsonElement(const float& val) {
			m_deep = false;
			m_type = JsonType::FLOAT;
			m_val.float_val = val;
		}

		JsonElement::JsonElement(const double& val) {
			m_deep = false;
			m_type = JsonType::DOUBLE;
			m_val.double_val = val;
		}

		JsonElement::~JsonElement() {
			//std::cout << "m_deep" << m_deep << std::endl;
			//std::cout << *this;
			free();
		}

		std::string JsonElement::toString() const {
			std::stringstream ss;
			
			switch (m_type) {
			case JsonType::OBJECT:
				if (m_val.object_val == nullptr) {
                    ss << "{}";
					break;
				}
				ss << "{";
				for (auto it = m_val.object_val->begin(); it != m_val.object_val->end(); ++it) {

					if (it != m_val.object_val->begin()) {
						ss << ",";
					}
					ss << "\"";
					ss << it->first << "\":" ;
					ss << it->second.toString();
				}
				ss << "}";

				break;

			case JsonType::ARRAY:
                if(!m_val.array_val){
                    ss << "[]";
                    break;
                }
				ss << "[";
                for (auto it = m_val.array_val->begin(); it != m_val.array_val->end(); ++it) {
                    if (it != m_val.array_val->begin()) {
                        ss << ",";
                    }
                    ss << it->toString();
                }


				ss << "]";
				break;
			case JsonType::BOOL:
				ss << std::boolalpha << m_val.bool_val;
				break;
			case JsonType::STRING:
				ss << "\"" << *m_val.string_val << "\"";
				break;

			case JsonType::INT:
				ss << m_val.int_val;
				break;

			case JsonType::UNSIGNEDINT:
				ss << m_val.uint_val;
				break;
			case JsonType::LONG:
				ss << m_val.long_val;
				break;
			case JsonType::UNSIGNEDLONG:
				ss << m_val.ulong_val;
				break;
			case JsonType::FLOAT:
				ss << m_val.float_val;
				break;
			case JsonType::DOUBLE:
				ss << m_val.double_val;
				break;
			case JsonType::JSONNULL:
				ss << "null";
				break;
			default:
				throw std::bad_cast();
				break;
			}
			return ss.str();

		}
		/*std::string JsonElement::toSteyString() const {

		}
		std::ostream JsonElement::dump(std::ostream& os) {

		}
		void steyDump(std::ostream& os);*/


		std::string JsonElement::asString(){
			if(m_type == JsonType::STRING){
                return *m_val.string_val;
            }
            return "";
		}

		int JsonElement::asInt(){
			
			if (m_type == JsonType::INT) {
				return m_val.int_val;
			}
			return 0;
		}

		unsigned int JsonElement::asUInt(){
			
			if (m_type == JsonType::UNSIGNEDINT) {
				return m_val.uint_val;
			}
			return 0;
		}

		long JsonElement::asLong(){
			if (m_type == JsonType::UNSIGNEDLONG) {
				return m_val.ulong_val;
			}
			return 0l;
		}

		unsigned JsonElement::asULong(){
			if (m_type == JsonType::UNSIGNEDLONG) {
				return m_val.ulong_val;
			}
			exceptionT();
			return 0ul;
		}

		float JsonElement::asFloat(){

			if (m_type == JsonType::FLOAT) {
				return m_val.float_val;
			}
			exceptionT();
			return 0.0f;
		}

		double JsonElement::asDouble(){

			if (m_type == JsonType::FLOAT || m_type == JsonType::DOUBLE) {
				return m_val.double_val;
			}
			exceptionT();
			return 0.0;
		}

		void JsonElement::exceptionT() {
			std::cout << "exception" << std::endl;
			throw std::bad_cast();
		}

		bool JsonElement::isArray() const {
			
			return m_type == JsonType::ARRAY;
		}

		bool JsonElement::isObject() const {

			return m_type == JsonType::OBJECT;
		}

		bool JsonElement::isString() const {

			return m_type == JsonType::STRING;
		}

		bool JsonElement::isInt() const {

			return m_type == JsonType::INT;
		}

		bool JsonElement::isUInt() const {

			return m_type == JsonType::UNSIGNEDINT;
		}

		bool JsonElement::isLong() const {

			return m_type == JsonType::LONG;
		}

		bool JsonElement::isULong() const {

			return m_type == JsonType::UNSIGNEDLONG;
		}

		bool JsonElement::isFloat() const {
			
			return m_type == JsonType::FLOAT;
		}

		bool JsonElement::isDouble() const {

			return m_type == JsonType::DOUBLE;
		}

		bool JsonElement::isNull() const {
			//该判的不是说它是不是为空，而是类型是不是空值。
			return m_type == JsonType::JSONNULL;
		}

		std::ostream& operator<<(std::ostream& os, const JsonElement& jsonelement) {

			os << jsonelement.toString();

			return os;
		}

		JsonElement& JsonElement::operator[](const std::string& key) {
			if (m_type != JsonType::OBJECT) {
				exceptionT();
			}
			if (m_val.object_val == nullptr) {
				m_deep = true;
				m_val.object_val = new JsonObject;
			}
			auto it = (*m_val.object_val).find(key);
			if (it != (*m_val.object_val).end()) {
				return (*m_val.object_val)[key];
			}
			JsonElement t;
			(*m_val.object_val)[key] = t;
			return (*m_val.object_val)[key];
		}

		JsonElement& JsonElement::operator[](const int index) {
			if (m_type != JsonType::ARRAY) {
				exceptionT();
			}
			return (*m_val.array_val)[index];
		}


		JsonElement& JsonElement::operator=(JsonElement& val) {
			

			if (this == &val) {
				return *this;
			}

			if (m_deep) {
				free();
			}


			this->m_type = val.m_type;

			//if (val.m_type == JsonType::STRING) {
			//	m_deep = true;
			//	this->m_val.string_val = new std::string(*val.m_val.string_val);
			//	return *this;
			//}
			this->m_deep = false;
			this->m_val = val.m_val;

			return *this;
		}

		JsonElement& JsonElement::operator=(JsonElement&& val) noexcept {

			//重载=，也是只将释放权进行移动。

			if (this == &val) {
				return *this;
			}

			if (m_deep) {
				free();
			}


			this->m_type = std::move(val.m_type);

			//if (val.m_type == JsonType::STRING) {
			//	m_deep = true;
			//	this->m_val.string_val = new std::string(*val.m_val.string_val);
			//	return *this;
			//}
			this->m_deep = std::move(val.m_deep);
			this->m_val = std::move(val.m_val);
			val.m_deep = false;
			return *this;
		}

		JsonElement& JsonElement::operator=(JsonObject& val) {
			if (m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::OBJECT;
			this->m_val.object_val = &val;
			return *this;
		}

		JsonElement& JsonElement::operator=(JsonArray& val) {
			if (m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::ARRAY;
			this->m_val.array_val = &val;
			return *this;
		}

		JsonElement& JsonElement::operator=(const std::string& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = true;
			this->m_type = JsonType::STRING;
			this->m_val.string_val = new std::string(val);
			//this->m_val.string_val = &val.;
			return *this;
		}

		JsonElement& JsonElement::operator=(const char* val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = true;
			this->m_type = JsonType::STRING;
			this->m_val.string_val = new std::string(val);
			//this->m_val.string_val = &val.;
			return *this;
		}

		JsonElement& JsonElement::operator=(const std::nullptr_t& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::JSONNULL;
			this->m_val.bool_val = false;
			//this->m_val.string_val = &val.;
			return *this;
		}

		JsonElement& JsonElement::operator=(const bool& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::BOOL;
			this->m_val.bool_val = val;
			return *this;
		}

		JsonElement& JsonElement::operator=(const int& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::INT;
			this->m_val.int_val = val;
			return *this;
		}
		JsonElement& JsonElement::operator=(const unsigned int& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::UNSIGNEDINT;
			this->m_val.uint_val = val;
			return *this;
		}

		JsonElement& JsonElement::operator=(const long& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::LONG;
			this->m_val.long_val = val;
			return *this;
			
		}

		JsonElement& JsonElement::operator=(const unsigned long& val) {
			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::UNSIGNEDLONG;
			this->m_val.ulong_val = val;
			return *this;
		}

		JsonElement& JsonElement::operator=(const float& val) {

			if (this->m_deep) {
				free();
			}
			this->m_deep = false;
			this->m_type = JsonType::FLOAT;
			this->m_val.float_val = val;
			return *this;
		}

		JsonElement& JsonElement::operator=(const double& val) {
			
			if (this->m_deep) {
				free();
			}

			this->m_deep = false;
			this->m_type = JsonType::DOUBLE;
			this->m_val.double_val = val;
			return *this;
		}


		int JsonElement::append(JsonElement& val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}
				//std::cout << m_val.array_val->size() << std::endl;
				(*m_val.array_val).push_back(std::move(val));
				return m_val.array_val->size() - 1;
			}
			return -1;
		}

		void JsonElement::append(const std::string& val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}
				
				(*m_val.array_val).emplace_back(val);
			}
		}

		void JsonElement::append(const char* val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}

				(*m_val.array_val).emplace_back(val);
			}
		}

		void JsonElement::append(const std::nullptr_t& val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}

				(*m_val.array_val).emplace_back(val);
			}
		}

		void JsonElement::append(const int val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}

				(*m_val.array_val).emplace_back(val);
			}
		}

		void JsonElement::append(const double val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}

				(*m_val.array_val).emplace_back(val);
			}
		}

		void JsonElement::append(const bool val) {
			if (m_type == JsonType::ARRAY) {
				if (m_val.array_val == nullptr) {
					m_deep = true;
					m_val.array_val = new JsonArray;
				}

				(*m_val.array_val).emplace_back(val);
			}
		}
		
        JsonElement::JsonObject JsonElement::getJsonObject() const{
            if(m_type == JsonType::OBJECT)
                return *m_val.object_val;
            //JsonObject t;
            return {};
        }

        JsonElement::JsonArray JsonElement::getJsonArray() const{
            if(m_type == JsonType::ARRAY){
                return *m_val.array_val;
            }

            return {};
        }

        size_t JsonElement::size() const {
            if(m_type == JsonType::ARRAY){
                if(m_val.array_val == nullptr)
                    return 0;
                return m_val.array_val->size();
            }else if(m_type == JsonType::OBJECT){
                if(m_val.object_val == nullptr)
                    return 0;
                return m_val.object_val->size();
            }else{
                return 1;
            }
        }



		JsonElement JsonElement::copy(JsonElement& val) {

			JsonElement t;
			t.m_deep = true;
			if (val.m_type == JsonType::ARRAY) {
				t.m_type = JsonType::ARRAY;
				t.m_val.array_val = copy(val.m_val.array_val);
			}

			if (val.m_type == JsonType::OBJECT) {
				t.m_type = JsonType::OBJECT;
				t.m_val.object_val = copy(val.m_val.object_val);
			}

			if (val.m_type == JsonType::STRING) {
				t.m_type = JsonType::STRING;
				t.m_val.string_val = new std::string(*val.m_val.string_val);
			}
			return std::move(t);
			//return t;
		}


		JsonElement::JsonArray* JsonElement::copy(JsonArray& val) {
			return copy(&val);
		}

		JsonElement::JsonObject* JsonElement::copy(const JsonObject* val) {
			

			JsonObject* t = new JsonObject;
			auto& v = *val;
			for (auto it = v.begin(); it != v.end(); ++it) {
				std::string key;
				JsonElement temp;
				key = it->first;
				temp.m_deep = true;
				temp.m_type = it->second.m_type;
				if (it->second.m_type == JsonType::OBJECT) {
					temp.m_val.object_val = copy(it->second.m_val.object_val);
				} else
				if (it->second.m_type == JsonType::ARRAY) {
					temp.m_val.array_val = copy(it->second.m_val.array_val);

				} else
				if (it->second.m_type == JsonType::STRING) {
						temp.m_val.string_val = new std::string(*it->second.m_val.string_val);
				} else {
					temp.m_deep = false;
					temp.m_val = it->second.m_val;
				}
				t->insert({ key, std::move(temp) });
			}
			return t;
			
		}

		JsonElement::JsonObject* JsonElement::copy(JsonObject& val) {
			return copy(&val);
		}
		std::string* JsonElement::copy(std::string& val) {
			return new std::string(val);
		}

		JsonElement::JsonArray* JsonElement::copy(const JsonArray* vl) {
			auto& val = *vl;
			JsonArray* t = new JsonArray;

			for (auto it = val.begin(); it != val.end(); ++it) {
				JsonElement temp;
				temp.m_deep = true;
				temp.m_type = it->m_type;
				if (it->m_type == JsonType::ARRAY) {
					temp.m_val.array_val = copy(it->m_val.array_val);
				}
				else
					if (it->m_type == JsonType::OBJECT) {
						temp.m_val.object_val = copy(it->m_val.object_val);
					}
					else
						if (it->m_type == JsonType::STRING) {
							temp.m_val.string_val = new std::string(*it->m_val.string_val);
						}
						else {
							temp.m_deep = false;
							temp.m_val = m_val;
						}

				t->push_back(std::move(temp));
			}

			return t;


		}


		std::string* JsonElement::copy(std::string* val) {
			return new std::string(*val);
		}

		void JsonElement::free() {
			if (m_deep) {
				switch (m_type) {
				case JsonType::OBJECT:
					delete m_val.object_val;
					break;
				case JsonType::ARRAY:
					delete m_val.array_val;
					break;
				case JsonType::STRING:
					delete m_val.string_val;
					break;
				default:
					break;
				}
			}
		}

		///Reader



	}
}       