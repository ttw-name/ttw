




#ifndef __TTW__JSON__JSONELEMENT_H
#define __TTW__JSON__JSONELEMENT_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stack>
#include <cctype>


namespace ttw {
	namespace json {
		class Reader;
		class JsonElement {
		friend Reader;
		public:
			using JsonArray = std::vector<JsonElement>;
			using JsonObject = std::map<std::string, JsonElement>;
		public:
			enum class JsonType {
				OBJECT = 0,
				ARRAY,
				BOOL,
				STRING,
				INT,
				UNSIGNEDINT,
				LONG,
				UNSIGNEDLONG,
				FLOAT,
				DOUBLE,
				JSONNULL
			};

			union JsonValueType {

				JsonArray* array_val;
				JsonObject* object_val;
				int int_val;
				unsigned int uint_val;
				long long_val;
				unsigned long ulong_val;
				float float_val;
				double double_val;
				std::string* string_val;
				bool bool_val;
			};

			JsonElement();
			JsonElement(const std::nullptr_t& val);
			JsonElement(JsonElement&& val) noexcept;
			JsonElement(const JsonType type);
			JsonElement(const JsonArray& val, bool deep = false);
			JsonElement(const JsonObject& val, bool deep = false);
			JsonElement(const JsonElement& val, bool deep = false);
			JsonElement(const char* val);
			JsonElement(const std::string& val);
			JsonElement(const bool& val);
			JsonElement(const int& val);
			JsonElement(const unsigned int& val);
			JsonElement(const long& val);
			JsonElement(const unsigned long& val);
			JsonElement(const float& val);
			JsonElement(const double& val);


			~JsonElement();

			std::string toString() const;
			//std::string toSteyString() const;
			//std::ostream dump(std::ostream& os);
			//void steyDump(std::ostream& os);

			std::string asString();
			int asInt();
			unsigned int asUInt();
			long asLong();
			unsigned asULong();
			float asFloat();
			double asDouble();

			bool isArray() const;
			bool isObject() const;
			bool isString() const;
			bool isInt() const;
			bool isUInt() const;
			bool isLong() const;
			bool isULong() const;
			bool isFloat() const;
			bool isDouble() const;
			bool isNull() const;

			friend std::ostream& operator<<(std::ostream& os, const JsonElement& jsonelement);
			//friend std::istream& operator>>(std::ostream& os, const JsonElement& jsonelement);


			JsonElement& operator[](const std::string& key);
			JsonElement& operator[](const int index);
			

			
			JsonElement& operator=(JsonElement& val);
			JsonElement& operator=(JsonElement&& val) noexcept;
			JsonElement& operator=(JsonArray& val);
			JsonElement& operator=(JsonObject& val);
			JsonElement& operator=(const std::string& val);
			JsonElement& operator=(const std::nullptr_t& val);
			JsonElement& operator=(const char* val);
			JsonElement& operator=(const bool& val);
			JsonElement& operator=(const int& val);
			JsonElement& operator=(const unsigned int& val);
			JsonElement& operator=(const long& val);
			JsonElement& operator=(const unsigned long& val);
			JsonElement& operator=(const float& val);
			JsonElement& operator=(const double& val);

			int append(JsonElement& val);
			void append(const std::string &val);
			void append(const int val);
			void append(const bool val);
			void append(const double val);
			void append(const char* val);
			void append(const std::nullptr_t& val);
			
            JsonObject getJsonObject() const;
            JsonArray getJsonArray() const;

            size_t size() const;

		private:

			JsonElement copy(JsonElement& val);
			//JsonValueType copy(const JsonElement* val, bool deep = false);
			JsonArray* copy(JsonArray& val);
			JsonObject* copy(JsonObject& val);
			std::string* copy(std::string& val);
			JsonArray* copy(const JsonArray* val);
			JsonObject* copy(const JsonObject* val);
			std::string* copy(std::string* val);
			//void freemove(JsonElement& val);

			//JsonElement* copyObject();
			//JsonElement* copyArray();
			void free();
			void exceptionT();


		private:
			JsonValueType m_val;
			JsonType m_type;
			bool m_deep; //是否具有释放权
		};

		
}
}

#endif