#include "parser.h"
#include <sstream>

namespace internal
{
	Token::Token() : m_Type(internal::Token::Type::None), m_Value()
	{ }

	Token::Token(Type type) : m_Type(type), m_Value()
	{ }

	Token::Token(Type type, const std::string& value) : m_Type(type), m_Value(value)
	{ }

	Token::Type Token::GetType() const
	{
		return m_Type;
	}

	std::string Token::GetValue() const
	{
		return m_Value;
	}

	Tokenizer::Tokenizer() : 
		m_Stream(nullptr)
	{ }

	Tokenizer::Tokenizer(std::ifstream&& stream) :
		m_Stream(nullptr)
	{ 
		m_Stream = std::make_unique<std::ifstream>(std::move(stream));
	}

	Tokenizer::Tokenizer(std::istringstream&& stream) :
		m_Stream(nullptr)
	{ 
		m_Stream = std::make_unique<std::istringstream>(std::move(stream));
	}

	internal::Token Tokenizer::NextToken()
	{
		std::streampos prevPos;
		char lastChar = ' ';
		do
		{
			if (IsEmpty())
				return {};

			prevPos = m_Stream->tellg();
			lastChar = m_Stream->get();
			
			if (lastChar == -1)
				return {};

		} while (lastChar == ' ' || lastChar == '\r' || lastChar == '\t' || lastChar == '\n');

		switch (lastChar)
		{
		case '=':
			return Token(Token::Type::Assign);
		case '{':
			return Token(Token::Type::ArrayOpen);
		case '}':
			return Token(Token::Type::ArrayClose);
		case '"':
		{
			std::string stringValue;

			if (!ReadString(stringValue))
				return {};

			return Token(Token::Type::String, stringValue);
		}
		default:
			std::string nameValue;
			m_Stream->seekg(prevPos);

			if (!ReadName(nameValue))
				return {};

			return Token(Token::Type::Name, nameValue);
		}
	}

	internal::Token Tokenizer::PeekToken()
	{
		std::streampos prevPos = m_Stream->tellg();
		auto token = NextToken();
		m_Stream->seekg(prevPos);
		return token;
	}

	bool IsNameCharacterCorrect(char c, bool first)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (!first && c >= '0' && c <= '9');
	}

	bool Tokenizer::ReadName(std::string& out)
	{
		if (IsEmpty())
			return false;

		std::stringstream sstream;

		char fstChar = m_Stream->get();
		bool isCorrect = IsNameCharacterCorrect(fstChar, true);

		if (!isCorrect)
			return false;

		sstream << fstChar;

		while (!IsEmpty() && IsNameCharacterCorrect(m_Stream->peek(), false))
		{
			sstream << static_cast<char>(m_Stream->get());
		}

		out = sstream.str();
		return true;
	}

	bool Tokenizer::ReadString(std::string& out)
	{
		if (IsEmpty())
			return false;

		std::stringstream sstream;

		char tempChar = m_Stream->get();
		while (tempChar != '"')
		{
			if (IsEmpty() || tempChar == '\r' || tempChar == '\n')
				return false;

			sstream << tempChar;
			tempChar = m_Stream->get();
		}

		out = sstream.str();
		return true;
	}

	bool Tokenizer::IsEmpty()
	{
		return m_Stream.get() == nullptr || m_Stream->eof();
	}

}

std::optional<Node> Parser::Parse(const std::filesystem::path& filepath)
{
	std::ifstream fs(filepath);
	
	if (!fs.is_open())
		return {};

	m_Tokenizer = internal::Tokenizer(std::move(fs));
	m_LastId = 1;

	return ParseNode();
}

std::optional<Node> Parser::Parse(const std::string& data)
{
	m_Tokenizer = internal::Tokenizer(std::istringstream(data));
	m_LastId = 1;

	return ParseNode();
}

std::optional<Node> Parser::ParseNode()
{
	auto nameToken = m_Tokenizer.NextToken();
	if (nameToken.GetType() != internal::Token::Type::Name)
		return {};

	auto token = m_Tokenizer.NextToken();
	auto tokenValue = token.GetValue();
	if (token.GetType() != internal::Token::Type::Assign)
		return {};

	auto nodeValue = ParseValue();
	if (!nodeValue)
		return {};

	return Node(m_LastId++, nameToken.GetValue(), nodeValue.value());
}

std::optional<NodeValue> Parser::ParseValue()
{
	auto valueStartToken = m_Tokenizer.NextToken();
	if (valueStartToken.GetType() == internal::Token::Type::String)
		return NodeValue(valueStartToken.GetValue());

	if (valueStartToken.GetType() != internal::Token::Type::ArrayOpen)
		return {};

	std::vector<Node> nodeList;
	while (m_Tokenizer.PeekToken().GetType() != internal::Token::Type::ArrayClose)
	{
		auto nodeEntry = ParseNode();
		if (!nodeEntry)
			return {};

		nodeList.push_back(nodeEntry.value());
	}

	m_Tokenizer.NextToken();

	return nodeList;
}
