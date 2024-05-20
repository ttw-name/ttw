


#include "reader.h"

namespace ttw {
	namespace json {

		Reader::Reader() {
			m_val = false;
			m_pos = 0;
			m_isErorr = false;
			m_nextLegalPunctuation = 0;
			m_nextLegalPunctuation |= ARRAY_BEGIN | OBJECT_BEGIN;
		}

		//Reader::Reader(std::ostream& os) {

		//}

		Reader::Reader(const std::string& string): m_str(string){
			//m_str = string;
			//Reader();

			m_val = false;
			m_pos = 0;
			m_isErorr = false;
			m_nextLegalPunctuation = 0;
			m_nextLegalPunctuation |= ARRAY_BEGIN | OBJECT_BEGIN;
		}

		//bool Reader::parser(std::ostream& os, JsonElement& val) {

		//}

		//bool Reader::parser(const std::string& str, JsonElement& val) {
		//	m_str = str;

		//}

		bool Reader::parser(JsonElement& val) {
			m_stackVal.push(&val);
			while (!m_isErorr && m_pos < m_str.size()) {
				TokenType type = scan();
				if (!isLegaPunctuation(type)) {
					m_isErorr = true;
					break;
				}
				m_nextLegalPunctuation &= 0;
				switch (type) {
				case TokenType::TOKEN_EOF:
					if (m_isErorr) {
						return false;
					}
					return true;
					break;
				case TokenType::OBJECT_BEGIN:
					//m_stack.push(m_str[m_pos]);
					addObject();

					m_nextLegalPunctuation |= TOKEN_STRING;
					//parser(*obj);
					break;
				case TokenType::ARRAY_BEGIN:
					addArray();
					m_nextLegalPunctuation &= 0;
					m_nextLegalPunctuation |= ~(TOKEN_COLONE | TOKEN_SEPARATOR | TOKEN_EOF | TOKEN_ERORR);
					break;
				case TokenType::OBJECT_END:
					delObject();
					m_nextLegalPunctuation &= 0;
					m_nextLegalPunctuation |= TOKEN_EOF | TOKEN_SEPARATOR | ARRAY_END | OBJECT_END;
					break;
				case TokenType::ARRAY_END:
					delArray();
					m_nextLegalPunctuation |= TOKEN_EOF | TOKEN_SEPARATOR | ARRAY_END | OBJECT_END;
					break;
				case TokenType::TOKEN_STRING:
					readString();
					m_nextLegalPunctuation |= TOKEN_COLONE | TOKEN_SEPARATOR | OBJECT_END | ARRAY_END;
					break;
				case TokenType::TOKEN_NUMBER:
					readNumber();
					m_nextLegalPunctuation |= TOKEN_SEPARATOR | OBJECT_END | ARRAY_END;
					break;
				case TokenType::TOKEN_NULL:
					readNull();
					m_nextLegalPunctuation |= TOKEN_SEPARATOR | OBJECT_END | ARRAY_END;
					break;
				case TokenType::TOKEN_TRUE:
					readTrue();
					m_nextLegalPunctuation |= TOKEN_SEPARATOR | OBJECT_END | ARRAY_END;
					break;
				case TokenType::TOKEN_FALSE:
					readFalse();
					m_nextLegalPunctuation |= TOKEN_SEPARATOR | OBJECT_END | ARRAY_END;
					break;
				case TokenType::TOKEN_SEPARATOR:
					m_nextLegalPunctuation |= ~(TOKEN_EOF | TOKEN_COLONE | TOKEN_ERORR | ARRAY_END | OBJECT_END);
					++m_pos;
					break;
				case TokenType::TOKEN_ERORR:
					m_isErorr = true;
					break;
				case TokenType::TOKEN_COLONE:
					m_nextLegalPunctuation |= ~(TOKEN_EOF | TOKEN_COLONE | TOKEN_ERORR | ARRAY_END | OBJECT_END | TOKEN_SEPARATOR);
					++m_pos;
				default:
					break;
				}
			}
			if (m_isErorr) {
				return false;
			}
			else {
				return true;
			}
		}

		Reader::TokenType Reader::scan() {
			//auto pos = m_pos;
			while (!m_isErorr && m_pos < m_str.size()) {
				char ch = m_str[m_pos];
				switch (ch) {
				case ' ':
				case '\n':
				case '\t':
				case '\b':
				case '\f':
					skipSpaces();
					break;
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '6':
				case '7':
				case '8':
				case '9':
					return TOKEN_NUMBER;
				case 'f':
					return TOKEN_FALSE;
				case 't':
					return TOKEN_TRUE;
				case 'n':
					return TOKEN_NULL;
				case '"':
					return TOKEN_STRING;
				case ',':
					return TOKEN_SEPARATOR;
				case ':':
					return TOKEN_COLONE;
				case '[':
					return ARRAY_BEGIN;
				case ']':
					return ARRAY_END;
				case '{':
					return OBJECT_BEGIN;
				case '}':
					return OBJECT_END;
				default:
					return TOKEN_ERORR;
				}
			}

			if (m_isErorr) {
				return TOKEN_ERORR;
			}
			else {
				return TOKEN_EOF;
			}

		}

		bool Reader::good() const {
			if (m_val) {
				return !m_isErorr;
			}
			else {
				return false;
			}
		}

		JsonElement* Reader::getJsonElement() const {
			return nullptr;
		}

		size_t Reader::skipSpaces() {
			size_t pos = 0;
			while (m_pos < m_str.size()) {
				char c = m_str[m_pos];
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\b') {
					++pos;
					++m_pos;
					continue;
				}
				break;
			}
			return pos;
		}

		/*size_t Reader::findNextLine() {

		}
		size_t Reader::findNextSeparator() {

		}
		size_t Reader::findElementEnd() {

		}
		size_t Reader::findNextColone() {

		}*/

		void Reader::addArray() {

			if (!m_val) {
				m_val = true;
				m_stackVal.top()->free();
				m_stackVal.top()->m_type = JsonElement::JsonType::ARRAY;
				m_stackVal.top()->m_val.array_val = nullptr;
				++m_pos;
				return;
			}

			JsonElement* obj;
			JsonElement t(JsonElement::JsonType::ARRAY);
			if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::OBJECT) {
				if (m_key.empty()) {
					m_isErorr = true;
				}
				else {
					(*m_stackVal.top())[m_key] = t;
					obj = &(*m_stackVal.top())[m_key];
					m_key.clear();
				}
			}
			else if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::ARRAY) {
				int ind = (*m_stackVal.top()).append(t);
				obj = &(*m_stackVal.top())[ind];
			}

			m_stackVal.push(obj);
			++m_pos;
		}

		void Reader::addObject() {

			if (!m_val) {
				m_val = true;
				m_stackVal.top()->free();
				m_stackVal.top()->m_type = JsonElement::JsonType::OBJECT;
				m_stackVal.top()->m_val.object_val = nullptr;
				++m_pos;
				return;
			}

			JsonElement* obj;
			JsonElement t(JsonElement::JsonType::OBJECT);
			if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::OBJECT) {
				if (m_key.empty()) {
					m_isErorr = true;
				}
				else {
					(*m_stackVal.top())[m_key] = t;
					obj = &(*m_stackVal.top())[m_key];
					m_key.clear();
				}
			}
			else if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::ARRAY) {
				int ind = (*m_stackVal.top()).append(t);
				obj = &(*m_stackVal.top())[ind];
			}


			m_stackVal.push(obj);
			++m_pos;
		}

		void Reader::delArray() {
			if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::ARRAY) {
				m_stackVal.pop();
			}
			else {
				m_isErorr = true;
			}
			++m_pos;
		}

		void Reader::delObject() {

			if (m_stackVal.size() && m_stackVal.top()->m_type == JsonElement::JsonType::OBJECT) {
				m_stackVal.pop();
			}
			else {
				m_isErorr = true;
			}
			++m_pos;
		}

		void Reader::readString() {
			++m_pos;
            size_t pos = findStringEnd();
            if(m_isErorr){
                return;
            }
            std::string str = m_str.substr(m_pos, pos - m_pos);
            m_pos = pos + 1;
			skipSpaces();
            if(m_pos >= m_str.size()){
                m_isErorr = true;
                return;
            }
			if (m_str[m_pos] == ':') {
				if (m_key.empty()) {
					m_key = str;
				}
				else {
					m_isErorr = true;
				}
			}
			else {
				addJsonElement(str);
				
				/*if (m_stackVal.empty()) {
					m_isErorr = true;
				}
				else {
					if (m_stackVal.top()->m_type == JsonElement::JsonType::OBJECT) {
						if (m_key.empty()) {
							m_isErorr = true;
							return;
						}
						(*m_stackVal.top())[m_key] = str;
						m_key.clear();
					}
					else {
						m_stackVal.top()->append(str);
					}
				}*/

			}
			//++m_pos;
		}

		void Reader::readDouble() {

		}

		void Reader::readTrue() {
			if (m_str.substr(m_pos, 4).compare("true")) {
				m_isErorr = true;
			}
			m_pos += 4;
			addJsonElement(true);
		}

		void Reader::readFalse() {
			if (m_str.substr(m_pos, 4).compare("false")) {
				m_isErorr = true;
			}
			m_pos += 4;
			addJsonElement(false);
		}
		void Reader::readNumber() {
			/*bool negative = false;
			
			if (m_str[m_pos] == '-') {
				negative = true;
			}*/

			size_t pos = m_pos + 1;
			bool flag = false;
			while (pos < m_str.size()) {
				if (isDigitze(pos)) {
					++pos;	
				}
				else {
					if (m_str[pos] == '.') {
						if (flag) {
							m_isErorr = true;
							return;
						}
						flag = true;
						if (!isDigitze(pos + 1)) {
							m_isErorr = true;
							return;
						}
					}
					else {
						break;
					}
				}
			}
			std::string number = m_str.substr(m_pos, pos - m_pos);
			if (flag) {
				double n = std::stod(number);
				addJsonElement(n);
			}
			else {
				int n = std::stoi(number);
				addJsonElement(n);
			}
			m_pos = pos;
		}

		void Reader::readNull() {
			if (m_str.substr(m_pos, 4).compare("null")) {
				m_isErorr = true;
				return;
			}
			m_pos += 4;
			addJsonElement(nullptr);
		}


		bool Reader::isLegaPunctuation(TokenType type) {
			return m_nextLegalPunctuation & type;
		}

		bool Reader::isDigitze(size_t pos) {
			if (pos < m_str.size()) {
				return std::isdigit(m_str[pos]);
			}
			else {
				return false;
			}

		}

		std::string Reader::findNextTransDoubleQuoto(bool& next) {
			next = false;
			size_t pos = m_pos;
			while (pos < m_str.size()) {
				if (m_str[pos] == '\\') {
					if (++pos < m_str.size()) {
						if (m_str[pos] == '\"') {
							next = true;
							auto str = m_str.substr(m_pos, pos - m_pos - 1);
							m_pos = pos + 1;
							return str + "\"";
						}
					}
					else {
						m_isErorr = true;
						return "";
					}
				}
				else if (m_str[pos] == '\"') {
					auto str = m_str.substr(m_pos, pos - m_pos);
					m_pos = pos + 1;
					return str;
				}
				++pos;
			}
			return "";
		}

        size_t Reader::findStringEnd(){
            size_t pos = m_pos;
			while (pos < m_str.size()) {
				if (m_str[pos] == '\\') {
					++pos;
				}
				else if (m_str[pos] == '\"') {
					return pos;
				}
				++pos;
			}
            m_isErorr = true;
            return m_pos;
        }
		
	}
}