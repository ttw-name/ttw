

#ifndef __TTW__JSON__READER_H
#define __TTW__JSON__READER_H

#include "jsonelement.h"

namespace ttw {
	namespace json {

		class Reader {
		public:
			Reader();
			//Reader(std::ostream& os);
			Reader(const std::string& string);
			//bool parser(std::ostream& os, JsonElement& val);
			//bool parser(const std::string& str, JsonElement& val);
			bool parser(JsonElement& val);
			bool good() const;
			JsonElement* getJsonElement() const;

		private:
			enum TokenType {									/// 每个位代表不同的符号
				OBJECT_BEGIN		= 0b0000000000001,			/// {
				OBJECT_END			= 0b0000000000010,			/// }
				ARRAY_BEGIN			= 0b0000000000100,			/// [
				ARRAY_END			= 0b0000000001000,			/// ]
				TOKEN_STRING		= 0b0000000010000,			/// "
				TOKEN_NUMBER		= 0b0000000100000,			/// number
				TOKEN_NULL			= 0b0000001000000,			/// null
				TOKEN_TRUE			= 0b0000010000000,			/// bool true
				TOKEN_FALSE			= 0b0000100000000,			/// bool false
				TOKEN_SEPARATOR		= 0b0001000000000,			/// ,
				TOKEN_COLONE		= 0b0010000000000,			/// :
				TOKEN_EOF			= 0b0100000000000,			/// eof
				TOKEN_ERORR			= 0b1000000000000,			/// error
			};



			size_t skipSpaces();
			//size_t findNextLine(); // \n
			//size_t findNextSeparator(); // ,
			//size_t findElementEnd(); // , or ] or }
			//size_t findNextColone(); // :
			std::string findNextTransDoubleQuoto(bool& next);
            size_t findStringEnd();
			//char getNextVaildChar();
			//size_t skipAnnotation();

			//bool read();
			//JsonElement::JsonObject* readJsonObject();
			//JsonElement::JsonArray* readsonArray();
			void readString();
			void readDouble();
			//float readFloat();

			//long readLong();
			//bool readBool();
			void readTrue();
			void readFalse();
			void readNumber();
			void readNull();

			void addArray();
			void addObject();
			void delArray();
			void delObject();
			
			template<typename T>
			void addJsonElement(const T& val) {

				if (m_stackVal.empty()) {
					m_isErorr = true;
				}
				else {
					if (m_stackVal.top()->m_type == JsonElement::JsonType::OBJECT) {
						if (m_key.empty()) {
							m_isErorr = true;
							return;
						}
						(*m_stackVal.top())[m_key] = val;
						m_key.clear();
					}
					else {
						if (!m_key.empty()) {
							m_isErorr = true;
							return;
						}
						m_stackVal.top()->append(val);
					}
				}
			}
			
			TokenType scan();
			//void findNextTable();

			bool isLegaPunctuation(TokenType type);
			bool isDigitze(size_t pos);

		private:
			std::string m_str;
			std::stack<JsonElement*> m_stackVal;
			std::string m_key;
			size_t m_pos;
			int m_nextLegalPunctuation;				///表示下一个合法字符,如当前符号为:，下一个符号则不能是:
			bool m_isErorr;
			bool m_val;
		};
	}
}


#endif